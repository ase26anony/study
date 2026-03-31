/* tm_loop_coverage.c - Program to trigger TM load/store loop transformation */
#include <stdio.h>
#include <stdlib.h>

/* Shared global variables for TM operations */
volatile int g_start = 0;
volatile int g_end = 100;
int shared_array[1000];
long shared_matrix[50][50];
volatile int *volatile shared_ptr = NULL;

/* Prevent optimization of TM functions */
#define TM_NOOPT __attribute__((noinline, noipa, used))

/* Test function 1: Simple array processing in atomic transaction */
TM_NOOPT
void tm_loop_transform1(int start, int end) {
    __transaction_atomic {
        for (int i = start; i < end; i++) {
            shared_array[i] = shared_array[i] * 2 + 1;
        }
    }
}

/* Test function 2: Nested loops with relaxed transaction */
TM_NOOPT
void tm_loop_transform2(int rows, int cols) {
    __transaction_relaxed {
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                shared_matrix[i][j] = shared_matrix[i][j] + i - j;
            }
        }
    }
}

/* Test function 3: Pointer-based loop with conditional transaction */
TM_NOOPT
void tm_loop_transform3(int *data, int n) {
    if (n > 0) {
        __transaction_atomic {
            int *ptr = data;
            int i = 0;
            while (i < n) {
                *ptr = (*ptr) ^ 0xAAAA;
                ptr++;
                i++;
            }
        }
    }
}

/* Test function 4: Complex control flow with transaction retry */
TM_NOOPT
void tm_loop_transform4(int limit) {
    volatile int attempts = 0;
    
    while (attempts < 3) {
        __transaction_atomic {
            for (int i = 0; i < limit; i++) {
                shared_array[i] = shared_array[i] + shared_array[limit - i - 1];
            }
            
            /* Simulate potential conflict */
            if (attempts == 1) {
                __transaction_cancel;
            }
            attempts++;
        }
    }
}

/* Test function 5: Mixed transaction types in same function */
TM_NOOPT
void tm_loop_transform5(int size) {
    /* First atomic region */
    __transaction_atomic {
        for (int i = 0; i < size / 2; i++) {
            shared_array[i] = shared_array[i] * 3;
        }
    }
    
    /* Then relaxed region */
    __transaction_relaxed {
        int j = size / 2;
        while (j < size) {
            shared_array[j] = shared_array[j] / 2;
            j++;
        }
    }
}

/* Test function 6: Multi-dimensional access with volatile bounds */
TM_NOOPT
void tm_loop_transform6(volatile int *rows_ptr, volatile int *cols_ptr) {
    int rows = *rows_ptr;
    int cols = *cols_ptr;
    
    __transaction_atomic {
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                for (int k = 0; k < 5; k++) {
                    shared_matrix[i][j] += k * (i + j);
                }
            }
        }
    }
}

/* Initialize test data */
void init_data(void) {
    for (int i = 0; i < 1000; i++) {
        shared_array[i] = i % 100;
    }
    
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 50; j++) {
            shared_matrix[i][j] = i * 50 + j;
        }
    }
    
    shared_ptr = shared_array;
}

/* Calculate checksum to verify execution */
long calculate_checksum(void) {
    long sum = 0;
    for (int i = 0; i < 1000; i++) {
        sum += shared_array[i];
    }
    
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 50; j++) {
            sum += shared_matrix[i][j];
        }
    }
    
    return sum;
}

int main(void) {
    /* Initialize shared data */
    init_data();
    
    /* Get volatile bounds to prevent compile-time optimization */
    volatile int v1 = 100;
    volatile int v2 = 200;
    volatile int v3 = 50;
    
    /* Execute various TM loop transformations */
    tm_loop_transform1(g_start, g_end);                    /* Simple array loop */
    tm_loop_transform2(30, 30);                           /* Nested matrix loop */
    tm_loop_transform3(shared_array + 200, 150);          /* Pointer-based loop */
    tm_loop_transform4(75);                               /* Transaction with retry */
    tm_loop_transform5(300);                              /* Mixed transactions */
    tm_loop_transform6(&v3, &v3);                         /* Volatile bounds */
    
    /* Additional calls with different parameters */
    for (int i = 0; i < 5; i++) {
        tm_loop_transform1(i * 20, (i + 1) * 20);
    }
    
    /* Calculate and print checksum to prevent dead code elimination */
    long checksum = calculate_checksum();
    printf("TM Loop Transformation Test\n");
    printf("Final checksum: %ld\n", checksum);
    
    return 0;
}
