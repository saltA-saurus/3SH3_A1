#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>

#define MAX_LINE 80 /* The maximum length command */

int main(void) {
    char *args[MAX_LINE / 2 + 1];
    char history[5][MAX_LINE] = {"", "", "", "", ""}; // history array
    int hnext = 0; // for history; circular array pointer
    int should_run = 1; /* flag to determine when to exit program */
    pid_t pid; // for forking a child
    int commandCount = 0; // for printing history
    char input[MAX_LINE]; // user input
    bool lastCalled = false; // for !! input

    while (should_run) {

        if(!lastCalled) { // don't take new input if !! entered
            printf("osh>");
            fflush(stdout);

            // read input from user
            fgets(input, MAX_LINE, stdin);
        }
        lastCalled = false;

        // copy input to another variable before tokenization
        char inputCopy[MAX_LINE];
        strcpy(inputCopy, input);

        // tokenizing input line
        char *token = strtok(input, " \n");
        int i = 0;
        bool concurrent = false;
        while (token != NULL) {
            args[i] = token;
            token = strtok(NULL, " \n");
            i++;
        }
        args[i] = NULL; // last element NULL for execvp

        if (strcmp(args[i-1], "&") == 0) { // checking for ampersand as last argument
            concurrent = true;
            args[i-1] = NULL; // avoids adding & to the arguments array (for execvp)
        }
        
        // exit shell on inputting "exit"
        if (strcmp(args[0], "exit") == 0) {
            should_run = 0;
            continue;
        }

        // printing history
        // note that history adds itself to the history after execution
        // i.e. the call itself will not show up in the table
        int histCount = commandCount;
        bool historyCalled = false;
        if (strcmp(args[0], "history") == 0) {
            int k;
            for (k = 0; k < 5; k++) {
                hnext--;
                if (hnext < 0) {
                    hnext = 4;
                }
                if (history[hnext][0] != '\0') {
                    printf("%d %s", histCount, history[hnext]);
                    histCount--;
                }
            }
            historyCalled = true;
        }

        // parsing "!!"
        else if (strcmp(args[0], "!!") == 0) {
            int hcurr;
            if (hnext == 0) {
                hcurr = 4;
            }
            else {
                hcurr = hnext - 1;
            }
            if (history[hcurr][0] == '\0') {
                printf("No commands in history.\n");
                continue;
            }
            strcpy(input, history[hcurr]);
            lastCalled = true;
            continue;
        }

        // adding to history
        strcpy(history[hnext], inputCopy);
        commandCount++; // counting commands
        if (hnext >= 4) {
            hnext = 0;
        }
        else {
            hnext++;
        }
        
        // if history was the last command entered, break iteration
        if(historyCalled) {
            continue;
        }

        pid = fork(); //fork child process

        // child process executes command
        if (pid == 0) {
            if (execvp(args[0], args) == -1) { // check for invalid command
                printf("Invalid command\n");
                should_run = 0;
            }
        }
        // parent waits for child unless apersand entered
        else  {
            if (!concurrent) {
                wait(NULL);
            }
        }
    }
    return 0;
}
