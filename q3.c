#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>

#define N 10 // number of students
#define MAX_CHAIRS 3 // number of chairs

void *student(void *num);
void *ta(void *);
 
sem_t taOffice; // barberChair
sem_t taSleep; // barberPillow
sem_t explaining; // seatBelt
pthread_mutex_t mutex;

int chairs = MAX_CHAIRS;
int allDone = 0;
bool sleeping = true;


int main() {

    if(pthread_mutex_init(&mutex, NULL) != 0){
        printf("Error initializing mutex\n");
    }
    pthread_t taid;
    pthread_t sid[N];
    int studentNames[N];
    int i;

    srand(time(NULL));

    for (i = 0; i < N; i++) {
        studentNames[i] = i;
    }

    sem_init(&taOffice, 0, 1);
    sem_init(&taSleep, 0, 0);
    sem_init(&explaining, 0, 0);

    // Create the TA.
    pthread_create(&taid, NULL, ta, NULL);

    // Create the students.
    for (i = 0; i<N; i++) {
    pthread_create(&sid[i], NULL, student, (void *)&studentNames[i]);
    }

    // Join student threads
    for (i=0; i<N; i++) {
    pthread_join(sid[i],NULL);
    }

    // When students are done, TA will finish
    allDone = 1;
    sem_post(&taSleep);
    pthread_join(taid,NULL);
}

void *student(void *studentName) {

    int name = *(int *)studentName;
    bool waiting = true;

    // waiting in hall loop
    while(waiting) {

        // student programs for random amount of time
        printf("%d is programming.\n", name);
        sleep(rand() % 8 + 1);

        // Check if there are chairs available:
        printf("%d checking if there are chairs available.\n", name);
        pthread_mutex_lock(&mutex); // mutex as chairs-- is not atomic
        // checks if chairs are available, if not goes back to programming
        if(chairs >= 0) {
            chairs--;
            waiting = false;
        }
        pthread_mutex_unlock(&mutex);
    }

    // waiting on chair
    printf("%d waiting for TA on a chair.\n", name);
    sem_wait(&taOffice);

    pthread_mutex_lock(&mutex);
    chairs++; // mutex as chairs++ is not atomic
    pthread_mutex_unlock(&mutex);

    // waking up TA
    // ensures student wakes up TA only when TA is actually sleeping
    bool waiting2 = true;
    while(waiting2) {
        if(sleeping) {
            printf("%d waking up the TA.\n", name);
            sem_post(&taSleep);
            waiting2 = false;
        }
    }

    // waits for TA to finish explaining
    sem_wait(&explaining);
    
    // leaves office, allowing next student through
    sem_post(&taOffice);
    printf("%d is leaving the TA's office.\n", name);

    pthread_exit(0);
}

void *ta(void *nothing) {

    // initially TA sleeping
    printf("The TA is sleeping.\n");

    while (!allDone) {

        // make TA sleep
        sleeping = true;
        sem_wait(&taSleep);
        sleeping = false;

        if (!allDone) {
            // TA explains for a random amount of time
            printf("TA is explaining.\n");
            sleep(rand() % 2 + 1);
            printf("TA is finished explaining.\n");
            // TA finishes explaining, letting student leave
            sem_post(&explaining);
            // Print message that TA is sleeping
            printf("The TA is sleeping.\n");
        } else {
            printf("TA is finished explaining to all the students.\n");
            pthread_exit(0);
        }
    }
}