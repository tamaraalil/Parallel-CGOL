#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "timer.h"

/*
 * Program: gol_task.c
 * Author: Tamara Alilovic
 */

// GLOBAL VARIABLES
int gridSize;
int nIterations;
int printFlag;
char **rboard;
char **wboard;

pthread_mutex_t mutexLive;
pthread_mutex_t mutexDead;

typedef struct queue {
    int size;
    int front;
    int rear;
    int **q;
} Queue;
Queue *live;
Queue *dead;

// Returns if inputted queue is empty
int isEmpty(Queue *q) {
    return (q->size == 0);
}

// Adds coordinate to the end of the queue
void append(Queue *queue, int x, int y) {
    queue->rear++;
    queue->q[queue->rear - 1][0] = x;
    queue->q[queue->rear - 1][1] = y;
    queue->size++;
}

// Returns first coordinate from the queue and removes it
int *pop(Queue *queue) {
    int *retval = malloc(sizeof(int) * 2);
    int temp[2];
    // Save element at front of queue
    retval[0] = queue->q[queue->front][0];
    retval[1] = queue->q[queue->front][1];
    // Move every element up one
    for (int i = 1; i < queue->rear; i++) {
        //printf("i %d < rear %d\n",i, queue->rear);
        temp[0] = queue->q[i][0];
        temp[1] = queue->q[i][1];
        queue->q[i-1][0] = temp[0];
        queue->q[i-1][1] = temp[1];
        queue->q[i][0] = -1;
        queue->q[i][1] = -1;
    }
    // Update counters
    queue->rear--;
    queue->size--;
    return retval;
}

// Prints out the 2D array passed into it with borders around it.
void printArray(char **a) {
    for(int k = 0; k < gridSize; k++) {
        printf("_"); // Top border
        if (k == gridSize-1) {
            printf("__\n");
        }
    }
    for (int i = 0; i < gridSize; i++) {
        printf("|"); // Side border
        for (int j = 0; j < gridSize; j++) {
            printf("%c", a[i][j]);
        }
        printf("|\n"); // Side border
    }
    for(int k = 0; k < gridSize + 2; k++) {
        printf("-"); // Bottom border
    }
}

// Count how many neighbours are alive for inputted coordinate
int rules (char **grid, int x, int y) {
    int count = 0;
    if ((x + 1) < gridSize && grid[x + 1][y] == 'X') {
        count++;
    }
    if ((x - 1) >= 0 && grid[x - 1][y] == 'X') {
        count++;
    }
    if ((y + 1) < gridSize && grid[x][y + 1] == 'X') {
        count++;
    }
    if ((y - 1) >= 0 && grid[x][y - 1] == 'X') {
        count++;
    } 
    if ((x + 1) < gridSize && (y + 1) < gridSize && grid[x + 1][y + 1] == 'X') {
        count++;
    }
    if ((x - 1) >= 0 && (y + 1) < gridSize && grid[x - 1][y + 1] == 'X') {
        count++;
    }
    if ((x + 1) < gridSize && (y - 1) >= 0 && grid[x + 1][y - 1] == 'X') {
        count++;
    }
    if ((x - 1) >= 0 && (y - 1) >= 0 && grid[x - 1][y - 1] == 'X') {
        count++;
    }
    return count;
}

// Thread function for tLive thread - take from live queue and update board
void *setAlive (void *arg) {
    //pthread_mutex_lock(&mutexLive);
    //printf("hi alive\n");
    if (live->size == (gridSize * gridSize)) {
        // If the live queue is full (every element is alive), return a full board
        // & unlock the mutex
        for(int i = 0; i < gridSize; i++) {
            for (int j = 0; j < gridSize; j++) {
                pthread_mutex_lock(&mutexLive);
                wboard[i][j] = 'X';
                pthread_mutex_unlock(&mutexLive);
            }
        }
        //pthread_mutex_unlock(&mutexLive);
        return NULL;
    }
    // While there are elements in the queue, update those coordinates to be alive
    while (!isEmpty(live)) {
        pthread_mutex_lock(&mutexLive);
        int *val = pop(live);
        if (val[0] != -1 && val[1] != -1) {
            wboard[val[0]][val[1]] = 'X';
        }
        pthread_mutex_unlock(&mutexLive);
    }
    //printf("bye alive\n");
    //pthread_mutex_unlock(&mutexLive);
    return NULL;
}

// Thread function for tDead thread - take from dead queue and update board
void *setDead (void *args) {
    //pthread_mutex_lock(&mutexDead);
    //printf("hi dead\n");
    if (dead->size == (gridSize * gridSize)) {
        // If the dead queue is full (every element is dead), return an empty board
        // & unlock the mutex
        for(int i = 0; i < gridSize; i++) {
            for (int j = 0; j < gridSize; j++) {
                pthread_mutex_lock(&mutexDead);
                wboard[i][j] = ' ';
                pthread_mutex_unlock(&mutexDead);
            }
        }
        //pthread_mutex_unlock(&mutexDead);
        return NULL;
    }
    // While there are elements in the queue, update those coordinates to be empty
    while (!isEmpty(dead)) {
        pthread_mutex_lock(&mutexDead);
        int *val = pop(dead);
        //printf("dead writing to board %d %d\n", val[0], val[1]);
        if (val[0] != -1 && val[1] != -1) {
            wboard[val[0]][val[1]] = ' ';
        }
        pthread_mutex_unlock(&mutexDead);
       // wboard[val[0]][val[1]] = ' ';
    }
    //printf("bye dead\n");
    //pthread_mutex_unlock(&mutexDead);
    return NULL;
}

// Check cell's neighbours and append that cell to the live or dead queue
void *cgol (void *args) {
    int count = 0;
    for (int i = 0; i < gridSize; i++) {
        for (int j = 0; j < gridSize; j++) {
            count = rules(rboard, i, j);
            // Check neighbour count for each cell & add to the appropriate queue
            if (rboard[i][j] == 'X') {
                if (count < 2) {
                    // Add coordinate to DEAD queue
                    append(dead, i, j);
                }
                else if (count > 3) {
                    // Add coordinate to DEAD queue
                    append(dead, i, j);
                } else {
                    // Add coordinate to LIVE queue
                    append(live, i, j);
                } 
            } else if (rboard[i][j] == ' ') {
                if (count == 3) {
                    // Add coordinate to LIVE queue
                    append(live, i, j);
                } else {
                    // Add coordinate to DEAD queue
                    append(dead, i, j);
                }  
            }
            count = 0;
        }
    }
    return NULL;
}

int main (int argc, char *argv[]) {
    // TIMER
    double start, finish, elapsed;
    GET_TIME(start);
    
    if (argc < 3 || argc > 4) { // Error check command prompt
        printf("Usage: ./gol_data gridSize nIterations -d\n");
        exit(0);
    } 
    // Initialize variables
    gridSize = atoi(argv[1]);
    nIterations = atoi(argv[2]);
    printFlag = 0;
    pthread_t tCheck;
    pthread_t tLive;
    pthread_t tDead;
    srand(time(0));
    pthread_mutex_init(&mutexDead, NULL);
    pthread_mutex_init(&mutexLive, NULL);

    rboard = malloc(sizeof(char*) * gridSize);
    wboard = malloc(sizeof(char*) * gridSize);
    for (int i = 0; i < gridSize; i++) {
        rboard[i] = malloc(sizeof(char) * gridSize);
        wboard[i] = malloc(sizeof(char) * gridSize);
    }
    
    live = malloc(sizeof(Queue));
    live->q = malloc(sizeof(int *) * (gridSize * gridSize));
    for(int m = 0; m < (gridSize * gridSize); m++) {
        live->q[m] = malloc(sizeof(int) * 2);
    }
    // Initialize queue to -1
    for (int i = 0; i < gridSize; i++) {
        for (int j = 0; j < 2; j++) {
            live->q[i][j] = -1;
        }
    }
    live->size = 0;
    live->front = 0;
    live->rear = 0;

    dead = malloc(sizeof(Queue));
    dead->q = malloc(sizeof(int *) * (gridSize * gridSize));
    for(int n = 0; n < (gridSize * gridSize); n++) {
        dead->q[n] = malloc(sizeof(int) * 2);
    }
    // Initialize queue to -1
    for (int i = 0; i < gridSize; i++) {
        for (int j = 0; j < 2; j++) {
            dead->q[i][j] = -1;
        }
    }
    dead->size = 0;
    dead->front = 0;
    dead->rear = 0;

    if (nIterations < 0) { // Error check inputted numbers
        printf("Error - Invalid number of iterations.\n");
        exit(0);
    }
    if (gridSize < 0) { // Error check inputted numbers
        printf("Error - Invalid grid size.\n");
        exit(0);
    }

    // Randomize grid population
    for (int i = 0; i < gridSize; i++) {
        for (int j = 0; j < gridSize; j++) {
            int r1 = (rand() % ((gridSize-1) + 1));
            int r2 = (rand() % ((gridSize-1) + 1));
            if (j == r1) {
                rboard[i][j] = 'X';
                wboard[i][j] = 'X';
            } else if (j == r2) {
                rboard[i][j] = 'X';
                wboard[i][j] = 'X';
            } else {
                rboard[i][j] = ' ';
                wboard[i][j] = ' ';
            }
        }
    }

    // Set print flag if there is a '-d' flag in the command line
    if (argc == 4 && strcmp(argv[3], "-d") == 0) {
        printFlag = 1;
    } else {
        printFlag = 0;
    }

    for (int i = 0; i < nIterations; i++) {
        if (printFlag == 1) {
            // Print final board of grid
            printArray(wboard);
            printf("%d\n", i+1);
        }
        pthread_create(&tCheck, NULL, cgol, NULL);
        pthread_create(&tLive, NULL, setAlive, NULL);
        pthread_create(&tDead, NULL, setDead, NULL);
        
        pthread_join(tCheck, NULL);
        pthread_join(tLive, NULL);
        pthread_join(tDead, NULL);

        // Update read grid to new written grid
        for (int k = 0; k < gridSize; k++) {
            for (int p = 0; p < gridSize; p++) {
                rboard[k][p] = wboard[k][p];
            }
        }
        // Reset queue values
        dead->rear = 0;
        live->rear = 0;
        dead->size = 0;
        live->size = 0;
    }

    // Print final board of grid
    if (printFlag == 1) {
        printf("Final board:\n");
        printArray(wboard);
        printf("\n");
    }

    // Destroy mutexes
    pthread_mutex_destroy(&mutexDead);
    pthread_mutex_destroy(&mutexLive);

    // Free variables
    for(int i = 0; i < gridSize; i++) {
        free(rboard[i]);
        free(wboard[i]);
    }
    free(rboard);
    free(wboard);
    for(int i = 0; i < (gridSize * gridSize); i++) {
        free(live->q[i]);
        free(dead->q[i]);
    }
    free(live->q);
    free(dead->q);
    free(live);
    free(dead);
    GET_TIME(finish);
    elapsed = finish - start;
    printf("The code to be timed took %e seconds\n", elapsed);

    return 0;
}

