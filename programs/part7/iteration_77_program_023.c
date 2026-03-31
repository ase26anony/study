/* tm_loop_coverage.c
 * Designed to trigger default_builtin_tm_load_store_loop_transform
 * in GCC's targhooks.cc, specifically lines 981-990
 */

#include <stdio.h>
#include <stdlib.h>

/* Shared global variables for TM operations */
volatile int g_start = 0;
volatile int g_end = 100;
int g_array[1000];
long g_matrix[10][10];
volatile int *g_ptr = g_array;

/* Prevent optimization of TM functions */
#define TM_NOOPT __attribute__((noinline, noipa, used))

/* Test function 1: Simple array processing in transactional loop */
TM_NOOPT
void tm_loop_transform1(int start, int end) {
    __transaction_atomic {
        for (int i = start; i < end; i++) {
            /* Load and store pattern that should trigger TM transformation */
            int val = g_array[i];
            g_array[i] = val * 2 + 1;
        }
    }
}

/* Test function 2: Nested loops with multi-dimensional array */
TM_NOOPT
void tm_loop_transform2(int n) {
    __transaction_atomic {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                /* Complex access pattern */
                g_matrix[i][j] = g_matrix[i][j] * 3 + g_matrix[j][i];
            }
        }
    }
}

/* Test function 3: While loop with pointer arithmetic */
TM_NOOPT
void tm_loop_transform3(int count) {
    volatile int *ptr = g_ptr;
    __transaction_relaxed {
        int i = 0;
        while (i < count) {
            /* Pointer-based load/store */
            int old = *ptr;
            *ptr = old + i;
            ptr++;
            i++;
        }
    }
}

/* Test function 4: Loop with conditional TM operations */
TM_NOOPT
void tm_loop_transform4(int threshold) {
    __transaction_atomic {
        for (int i = g_start; i < g_end; i++) {
            if (g_array[i] > threshold) {
                g_array[i] = g_array[i] / 2;
            } else {
                g_array[i] = g_array[i] * 2;
            }
        }
    }
}

/* Test function 5: Nested transaction with cancel/retry logic */
TM_NOOPT
void tm_loop_transform5(int iterations) {
    int retries = 3;
    
    while (retries-- > 0) {
        __transaction_atomic {
            for (int i = 0; i < iterations; i++) {
                /* Create potential conflict */
                g_array[i] = g_array[i] + g_array[iterations - i - 1];
            }
            
            /* Simulate condition that might cause retry */
            if (g_array[0] % 7 == 0) {
                __transaction_cancel;
            }
        }
    }
}

/* Test function 6: Mixed relaxed and atomic transactions */
TM_NOOPT
void tm_loop_transform6(int size) {
    /* Outer relaxed transaction */
    __transaction_relaxed {
        for (int i = 0; i < size; i += 2) {
            /* Inner atomic transaction */
            __transaction_atomic {
                g_array[i] = g_array[i] * g_array[i + 1];
                g_array[i + 1] = g_array[i] - g_array[i + 1];
            }
        }
    }
}

/* Initialize test data */
void init_data(void) {
    for (int i = 0; i < 1000; i++) {
        g_array[i] = i % 100;
    }
    
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            g_matrix[i][j] = i * 10 + j;
        }
    }
}

/* Compute checksum to verify execution and prevent optimization */
long compute_checksum(void) {
    long sum = 0;
    for (int i = 0; i < 1000; i++) {
        sum += g_array[i];
    }
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            sum += g_matrix[i][j];
        }
    }
    return sum;
}

int main(void) {
    /* Initialize shared data */
    init_data();
    
    /* Call all TM test functions with varying parameters */
    tm_loop_transform1(0, 100);
    tm_loop_transform2(10);
    tm_loop_transform3(50);
    tm_loop_transform4(50);
    tm_loop_transform5(20);
    tm_loop_transform6(100);
    
    /* Additional calls with different parameters */
    tm_loop_transform1(100, 200);
    tm_loop_transform3(25);
    tm_loop_transform4(75);
    
    /* Compute and print checksum to ensure execution */
    long checksum = compute_checksum();
    printf("TM Loop Transformation Test\n");
    printf("Final checksum: %ld\n", checksum);
    
    return 0;
}
