#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lib.h"

#define HOT_LOOP_COUNT 1000
#define COLD_LOOP_COUNT 10

/* Function with varying execution frequency based on mode */
void hot_function(int mode) {
    int limit = (mode == 1) ? HOT_LOOP_COUNT : COLD_LOOP_COUNT;
    for (int i = 0; i < limit; i++) {
        /* Hot loop body - executed many times in mode 1 */
        volatile int x = i * 2;
    }
}

void cold_function(int mode) {
    if (mode == 1) {
        /* Rarely executed in mode 1 */
        printf("Cold function executed in mode 1\n");
    } else {
        /* Executed more in mode 2 */
        for (int i = 0; i < 5; i++) {
            printf("Cold function iteration %d in mode 2\n", i);
        }
    }
}

void mixed_function(int mode) {
    /* Always executed but with different paths */
    if (mode % 2 == 0) {
        /* Even modes take this path */
        for (int i = 0; i < 50; i++) {
            volatile int y = i * 3;
        }
    } else {
        /* Odd modes take this path */
        for (int i = 0; i < 20; i++) {
            volatile int z = i * 4;
        }
    }
}

void recursive_function(int depth, int mode) {
    if (depth <= 0) return;
    
    volatile int temp = depth * mode;
    recursive_function(depth - 1, mode);
}

void switch_based_function(int mode) {
    switch (mode % 4) {
        case 0:
            /* Frequently executed in mode 1 */
            for (int i = 0; i < 100; i++) {
                volatile int a = i * mode;
            }
            break;
        case 1:
            /* Less frequent */
            printf("Case 1 executed\n");
            break;
        case 2:
            /* Medium frequency */
            for (int i = 0; i < 30; i++) {
                volatile int b = i + mode;
            }
            break;
        case 3:
            /* Rare case */
            printf("Case 3 executed\n");
            break;
    }
}

int main(int argc, char *argv[]) {
    int mode = 1;  /* Default mode */
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    printf("Running in mode %d\n", mode);
    
    /* Call functions with different frequencies based on mode */
    hot_function(mode);
    cold_function(mode);
    mixed_function(mode);
    recursive_function(5, mode);
    switch_based_function(mode);
    
    /* Call library functions */
    lib_function_a(mode);
    lib_function_b(mode);
    lib_function_c(mode);
    lib_complex_function(mode);
    
    /* Vary execution based on mode */
    if (mode == 1) {
        /* Execute hot paths more */
        for (int i = 0; i < 3; i++) {
            hot_function(mode);
            lib_function_a(mode);
        }
    } else if (mode == 2) {
        /* Execute different mix */
        for (int i = 0; i < 2; i++) {
            cold_function(mode);
            lib_function_b(mode);
        }
    } else {
        /* Mode 3 or higher - different pattern */
        for (int i = 0; i < mode; i++) {
            mixed_function(mode + i);
            lib_function_c(mode);
        }
    }
    
    return 0;
}
