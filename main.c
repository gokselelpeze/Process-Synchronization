#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

// For Warnings
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
// Constants
#define PATIENT_NUM 30
#define NURSE_COUNT 8
#define UNIT_COUNT 8
#define UNIT_PLACE_COUNT 3
#define PATIENT_CREATION_TIME 3
#define TEST_TIME 9
// States of nurses
#define STARTED -1
#define ENTRY_FREE 0
#define IDLE 1
#define FULL_AND_BUSY 2
#define FINISHED 3
// Colors
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define BACKGROUND "\x1B[22;1m"

int nurseStates[NURSE_COUNT] = {-1, -1, -1, -1, -1, -1, -1, -1};
int currentPlaces[UNIT_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0};
int unitWorkedCount[NURSE_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0};
int testSeconds[NURSE_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0};

int patientsCreated = 0;
int patientsTested = 0;
int patientsInUnits = 0;

pthread_t nurseThreads[NURSE_COUNT];
pthread_t patientThreads[PATIENT_NUM];

// For the patients in unit
sem_t waitingInUnit[UNIT_COUNT];
// If there is no empty unit, patient gonna wait
sem_t waitingAtHospital;
// For the units
sem_t unit[UNIT_COUNT];
// For the critical sections
sem_t mutex;
bool flag = true;
bool lastUnit = false;

int countPatientsInUnit() {
    patientsInUnits = 0;
    for (int i = 0; i < UNIT_COUNT; i++) {
        patientsInUnits += currentPlaces[i];
    }
    return patientsInUnits;
}

void simulate() {

    printf(BACKGROUND GRN "╔════════╦═══════╦══════════════════════╦════════════════════════════════════════╗\n");
    printf("╠ UNITS  ║ SEATS ║ UNIT TEST COUNT \t║ CURRENT STATE                          ║\n");
    printf("╠════════╬═══════╬══════════════════════╬════════════════════════════════════════╣\n");

    for (int i = 0; i < UNIT_COUNT; i++) {
        printf("╠ Unit %d ║ ", i + 1);

        for (int j = 0; j < currentPlaces[i]; j++) {
            printf(RED "▓ " GRN);
        }
        for (int j = 0; j < UNIT_PLACE_COUNT - currentPlaces[i]; j++) {
            printf("░ ");
        }
        printf("║ Unit Tested: %d\t║ ", unitWorkedCount[i]);
        switch (nurseStates[i]) {
            case STARTED:
                printf("STARTING                               ║\n");
                break;
            case IDLE:
                printf("VENTILATING                            ║\n");
                break;
            case ENTRY_FREE:
                printf("ANNOUNCING: Last %d people. Lets Start! ║\n", UNIT_PLACE_COUNT - currentPlaces[i]);
                break;
            case FULL_AND_BUSY:
                printf("TESTING NOW - %d seconds remaining...\t ║\n", testSeconds[i]);
                break;
            case FINISHED:
                printf("FINISHED                               ║\n");
                break;
        }
        if (i + 1 < UNIT_COUNT) {
            printf("╠════════╬═══════╬══════════════════════╬════════════════════════════════════════╣\n");
        } else {
            printf("╠════════╩═══════╩══════════════════════╬════════════════════════════════════════╣\n");
        }
    }
    int remainingPatients = patientsCreated - patientsTested;
    printf("╠ TOTAL PATIENTS TESTED: %d\t\t║ REMAINING PATIENTS: %d  \t\t ║ \n", patientsTested, remainingPatients);
    printf("╚═══════════════════════════════════════╩════════════════════════════════════════╝\n\n");
}

void *patient(void *patientID) {
    /**
     * sem_wait -> decrements the semaphore (lock)
     * STARTED CRITICAL SECTION
     * // Sometimes we use common variables in threads and we use semaphores to ensure that threads
     * // do not interfere with each other and work regularly in accordance with the structure we have built.
     * END CRITICAL SECTION
     * sem_post -> increments the semaphore (unlock)
     *
     * I locked the argument that I gave in sem_wait and I unlocked the argument in sem_post.
     */
    int unitID = -1;
    sem_wait(&mutex);
    // Patient created
    patientsCreated++;
    sem_post(&mutex);
    // Tell patient to wait
    sem_wait(&waitingAtHospital);
    sem_wait(&mutex);

    while (true) {
        int maxSeat = 0;
        int minWorkedUnit = unitWorkedCount[0];

        for (int i = 1; i < UNIT_COUNT; i++) {
            // Getting minimum worked unit
            if (unitWorkedCount[i] < unitWorkedCount[i - 1])
                minWorkedUnit = unitWorkedCount[i];
        }

        for (int i = 0; i < UNIT_COUNT; i++) {
            // If the unit is currently testing patients or if there is no seat for new patient, continue
            if (nurseStates[i] == STARTED || nurseStates[i] == FULL_AND_BUSY || currentPlaces[i] == UNIT_PLACE_COUNT) {
                continue;
            }
                // If empty seat number is more than maxSeat set maxSeat to current
            else if (currentPlaces[i] > maxSeat && currentPlaces[i] < UNIT_PLACE_COUNT) {
                maxSeat = currentPlaces[i];
                unitID = i;
            }
                // If empty seats are equal, look for how many times worked each unit to prevent starvation?
            else if (currentPlaces[i] == maxSeat && currentPlaces[i] < UNIT_PLACE_COUNT &&
                     unitWorkedCount[i] <= minWorkedUnit) {
                unitID = i;
            }
        }
        // We found the unit to test the patient, patient enters the room
        if (nurseStates[unitID] != FULL_AND_BUSY && nurseStates[unitID] != STARTED &&
            currentPlaces[unitID] < UNIT_PLACE_COUNT) {
            currentPlaces[unitID]++;
            if (currentPlaces[unitID] == UNIT_PLACE_COUNT) {
                // If unit is filled start to test
                unitWorkedCount[unitID]++;
            }
            if (currentPlaces[unitID] < UNIT_PLACE_COUNT) {
                // Call the patient
                sem_post(&waitingAtHospital);
            }
            break;
        }
    }
    // If the unit is ventilating, patient have to call the nurse for test
    if (nurseStates[unitID] == IDLE) {
        sem_post(unit + unitID);
    }
    sem_post(&mutex);

    sem_wait(&mutex);
    // Patients have to wait in the unit until the unit is full
    if (currentPlaces[unitID] < UNIT_PLACE_COUNT) {
        sem_post(&mutex);
        sem_wait(waitingInUnit + unitID);
        flag = true;
        sleep(1);
    }
        // We can start the test when the last patient has arrived
    else if (currentPlaces[unitID] == UNIT_PLACE_COUNT) {
        sem_post(&mutex);
        sem_post(waitingInUnit + unitID);
        sem_post(waitingInUnit + unitID);
        sem_post(waitingInUnit + unitID);
    }
    return NULL;
}

void stopThreads() {
    for (int i = 0; i < NURSE_COUNT; i++) {
        pthread_cancel(nurseThreads[i]);
    }

    for (int i = 0; i < PATIENT_NUM; i++) {
        pthread_cancel(patientThreads[i]);
    }
}

void *nurse(void *nurseID) {

    int id = (int) nurseID;

    while (true) {
        // Call the patient
        sem_post(&waitingAtHospital);
        sem_wait(&mutex);
        nurseStates[id] = ENTRY_FREE;
        if(lastUnit)
            nurseStates[id] = FULL_AND_BUSY;
        sem_post(&mutex);
        // Ventilating control
        sem_wait(&mutex);
        // If there is no patient in the waiting hall, the nurse is going to ventilate until the patients come
        if (patientsCreated == 0 ||
            (currentPlaces[id] == 0 && patientsCreated - countPatientsInUnit() - patientsTested == 0)) {
            nurseStates[id] = IDLE;
            simulate();
            flag = false;
            sem_post(&mutex);
            sem_wait(unit + id);

        } else {
            sem_post(&mutex);
        }
        sem_wait(&mutex);
        nurseStates[id] = ENTRY_FREE;
        if(lastUnit)
            nurseStates[id] = FULL_AND_BUSY;
        sem_post(&mutex);
        // The nurse is announcing the remaining places in the unit, until the unit is full
        while (true) {
            sem_wait(&mutex);
            // If there is people less than unit capacity start to test last time
            if(PATIENT_NUM - patientsTested < UNIT_PLACE_COUNT){
                lastUnit = true;
                // Find the last working unit
                for(int i = 0; i< UNIT_COUNT; i++){
                    if(currentPlaces[i] > 0){
                        id = i;
                    }
                }
            }
            // Start to test
            if (lastUnit || currentPlaces[id] == UNIT_PLACE_COUNT) {
                nurseStates[id] = FULL_AND_BUSY;
                sem_post(&mutex);
                break;
            }
            simulate();
            sem_post(&mutex);
            sleep(1);
        }
        // Calculating remaining test time
        for (int i = 0; i < TEST_TIME; i++) {
            testSeconds[id] = TEST_TIME - i;
            sleep(1);
        }
        sem_wait(&mutex);
        // Finish testing
        simulate();
        patientsTested += currentPlaces[id];
        currentPlaces[id] = 0;
        sem_post(&mutex);

        // If all patients tested close the hospital
        if (patientsTested == PATIENT_NUM) {
            for (int i = 0; i < NURSE_COUNT; i++) {
                nurseStates[i] = FINISHED;
            }
            simulate();
            stopThreads();
            //pthread_cancel(thr)
            exit(0);
        }
    }
}

void initSem() {
    /**
     * sem_init(
     *          <The value we will handle the semaphore>,
     *          <It is 0 because semaphore will be shared between threads of the process>,
     *          <Start value of the semaphore> )
     */

    sem_init(&waitingAtHospital, 0, 0);
    sem_init(&mutex, 0, 1);

    for (int i = 0; i < UNIT_COUNT; i++) {
        sem_init(unit + i, 0, 0);
    }

    for (int i = 0; i < UNIT_COUNT; i++) {
        sem_init(waitingInUnit + i, 0, 0);
    }
}


// Starting Threads
void startThreads() {

    srand(time(NULL));

    for (int i = 0; i < NURSE_COUNT; i++) {
        pthread_create(nurseThreads + i, NULL, &nurse, (void *) i);
    }

    for (int i = 0; i < PATIENT_NUM; i++) {
        pthread_create(patientThreads + i, NULL, &patient, (void *) i);
        sleep(rand() % PATIENT_CREATION_TIME);
    }
    simulate();
    sleep(1);
    for (int i = 0; i < NURSE_COUNT; i++) {
        pthread_join(nurseThreads[i], NULL);
    }
    for (int i = 0; i < PATIENT_NUM; i++) {
        pthread_join(patientThreads[i], NULL);
    }
}

int main() {
    initSem();
    startThreads();
    return 0;
}
