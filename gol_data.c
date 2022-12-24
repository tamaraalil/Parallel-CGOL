#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "timer.h"

/*
 * Program: gol_data.c
 * Author: Tamara Alilovic
 */

// GLOBAL VARIABLES
int gridSize;
int nIterations;
int printFlag;
char **rboard;
char **wboard;

typedef struct thread {
    int start;
    int end;
} Thread;

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

// Thread function - change board based on alive and dead cells
void *cgol (void *t) {
    int count = 0;
    for (int j = 0; j < gridSize; j++) {
        for (int k = 0; k < gridSize; k++) {
            count = rules(rboard, j, k);
            int cur = ((j*gridSize) + k) + 1;
            // Check neighbour count for each cell & update the write board
            if (rboard[j][k] == 'X') {
                // Thread will only write in given part of grid
                if (cur >= ((Thread *)t)->start && cur <= ((Thread *)t)->end) {
                    if (count < 2) {
                        wboard[j][k] = ' ';
                    }
                    else if (count > 3) {
                        wboard[j][k] = ' ';
                    } else {
                        wboard[j][k] = 'X';
                    } 
                }
            } else if (rboard[j][k] == ' ') {
                if (cur >= ((Thread *)t)->start && cur <= ((Thread *)t)->end) {
                    if (count == 3) {
                        wboard[j][k] = 'X';
                    } else {
                        wboard[j][k] = ' ';
                    }
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

    if (argc < 4 || argc > 5) { // Error check command prompt
        printf("Usage: ./gol_data nThreads gridSize nIterations -d\n");
        exit(0);
    } 
    // Initialize variables
    int nThreads = atoi(argv[1]);
    gridSize = atoi(argv[2]);
    rboard = malloc(sizeof(char*) * gridSize);
    wboard = malloc(sizeof(char*) * gridSize);
    for (int i = 0; i < gridSize; i++) {
        rboard[i] = malloc(sizeof(char) * gridSize);
        wboard[i] = malloc(sizeof(char) * gridSize);
    }
    Thread *t = malloc(sizeof(Thread *) * nThreads);
    nIterations = atoi(argv[3]);
    pthread_t *threads = malloc(sizeof(pthread_t) * nThreads);
    printFlag = 0;
    int totalSize = gridSize * gridSize;
    int inc = totalSize / nThreads;
    int space = 0;
    srand(time(0));

    if (nThreads < 0) { // Error check inputted numbers
        printf("Error - Invalid number of threads.\n");
        exit(0);
    }
    if (nIterations < 0) { // Error check inputted numbers
        printf("Error - Invalid number of iterations.\n");
        exit(0);
    }
    if (gridSize < 0) { // Error check inputted numbers
        printf("Error - Invalid grid size.\n");
        exit(0);
    }
    if (nThreads > (gridSize * gridSize)) {
        printf("Error - Number of threads cannot be greater than the total grid size\n");
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

    // Divide boundaries among the threads
    for (int i = 0; i < nThreads; i++) {
        if ((totalSize % inc) != 0 && i == (nThreads-1)) {
            t[i].start = space;
            t[i].end = totalSize;
            //printf("Thread %d starts at %d ends at %d\n", i+1, t[i].start, t[i].end);
        } else {
            t[i].start = space;
            t[i].end = space + (inc - i);
            //printf("Thread %d starts at %d ends at %d\n", i+1, t[i].start, t[i].end);
        }
        space = space + inc + 1;
    }

    // Set print flag if there is a '-d' flag in the command line
    if (argc == 5 && strcmp(argv[4], "-d") == 0) {
        printFlag = 1;
    } else {
        printFlag = 0;
    }

    //threads = malloc(sizeof(pthread_t) * nThreads);
    for (int i = 0; i < nIterations; i++) {
        if (printFlag == 1) {
            // Print final board of grid
            printArray(wboard);
            printf("%d\n", i+1);
        }
        for (int j = 0; j < nThreads; j++) {
            pthread_create(&threads[j], NULL, cgol, &t[j]);
            pthread_join(threads[j], NULL);
        }
        // Update read grid to new written grid
        for (int k = 0; k < gridSize; k++) {
            for (int p = 0; p < gridSize; p++) {
                rboard[k][p] = wboard[k][p];
            }
        }
    }

    // Print final board of grid
    if (printFlag == 1) {
        printf("Final board:\n");
        printArray(wboard);
        printf("\n");
    }

    // Free variables
    for(int i = 0; i < gridSize; i++) {
        free(rboard[i]);
        free(wboard[i]);
    }
    free(rboard);
    free(wboard);

    GET_TIME(finish);
    elapsed = finish - start;
    printf("The code to be timed took %e seconds\n", elapsed);

    return 0;
}

