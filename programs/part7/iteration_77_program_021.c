/* tm_loop_coverage.c
 * Designed to trigger default_builtin_tm_load_store_loop_transform in targhooks.cc
 * Compile with: gcc -O1 -fgnu-tm -o tm_test tm_loop_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Shared global variables for TM operations */
volatile int g_start = 0;
volatile int g_end = 100;
int shared_array[1000];
long shared_matrix[10][10];
volatile int *volatile shared_ptr = NULL;

/* Prevent optimization and inlining */
__attribute__((noinline, noipa))
void tm_loop_transform1(int start, int end) {
    __transaction_atomic {
        /* Loop with array accesses - candidate for load/store transformation */
        for (int i = start; i < end; i++) {
            shared_array[i] = shared_array[i] * 2 + 1;
        }
        
        /* Nested loop with multi-dimensional array */
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                shared_matrix[i][j] += i * j;
            }
        }
    }
}

__attribute__((noinline, noipa))
void tm_loop_transform2(int *data, int size) {
    int local_end = size;
    if (local_end > 1000) local_end = 1000;
    
    __transaction_relaxed {
        /* While loop with pointer arithmetic */
        int *ptr = data;
        int counter = 0;
        while (counter < local_end) {
            *ptr = (*ptr) ^ 0xAAAAAAAA;
            ptr++;
            counter++;
        }
        
        /* Another for loop with volatile bound */
        for (int i = g_start; i < g_end && i < local_end; i++) {
            data[i] = data[i] << 1;
        }
    }
}

__attribute__((noinline, noipa))
void tm_loop_transform3(int iterations) {
    int retry_count = 0;
    
transaction_retry:
    __transaction_atomic {
        /* Complex loop with conditional inside transaction */
        for (int i = 0; i < iterations; i++) {
            if (i % 3 == 0) {
                shared_array[i] = -shared_array[i];
            } else if (i % 3 == 1) {
                shared_array[i] = shared_array[i] / 2;
            } else {
                shared_array[i] = shared_array[i] + i;
            }
            
            /* Nested transaction attempt */
            if (i == iterations / 2 && retry_count < 3) {
                __transaction_cancel;
                retry_count++;
                goto transaction_retry;
            }
        }
    }
}

__attribute__((noinline, noipa))
void tm_loop_transform4(void) {
    volatile int dynamic_bound = g_end - g_start;
    
    __transaction_relaxed {
        /* Loop with function call inside - may create complex TM regions */
        for (int i = 0; i < dynamic_bound; i += 2) {
            shared_array[i] = shared_array[i] * shared_array[i + 1];
        }
        
        /* Access through volatile pointer */
        if (shared_ptr) {
            for (int i = 0; i < 50; i++) {
                shared_ptr[i] = i * i;
            }
        }
    }
}

__attribute__((noinline, noipa))
void tm_mixed_loops(int seed) {
    /* Multiple transactional regions in one function */
    __transaction_atomic {
        int limit = seed % 50 + 10;
        for (int i = 0; i < limit; i++) {
            shared_matrix[i % 10][i / 10] += seed;
        }
    }
    
    __transaction_relaxed {
        int j = 0;
        do {
            shared_array[j] = shared_array[j] - seed;
            j++;
        } while (j < 100);
    }
}

/* Main test driver */
int main(void) {
    int i, j;
    
    /* Initialize shared data */
    for (i = 0; i < 1000; i++) {
        shared_array[i] = i;
    }
    
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            shared_matrix[i][j] = i * 10 + j;
        }
    }
    
    shared_ptr = (int*)shared_array;
    
    /* Execute various TM loop transformations */
    tm_loop_transform1(0, 100);
    tm_loop_transform1(100, 200);  /* Different bounds */
    
    tm_loop_transform2(shared_array, 500);
    
    g_start = 50;
    g_end = 150;
    tm_loop_transform3(200);
    
    tm_loop_transform4();
    
    for (i = 0; i < 5; i++) {
        tm_mixed_loops(i * 17);
    }
    
    /* Calculate checksum to prevent optimization and verify execution */
    long checksum = 0;
    for (i = 0; i < 1000; i++) {
        checksum += shared_array[i];
    }
    
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            checksum += shared_matrix[i][j];
        }
    }
    
    printf("TM Loop Transformation Test Complete\n");
    printf("Checksum: %ld\n", checksum);
    
    return 0;
}
