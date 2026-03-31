/* tm_loop_coverage.c
 * Designed to trigger default_builtin_tm_load_store_loop_transform in targhooks.cc
 * Compile with: gcc -O1 -fgnu-tm -o tm_test tm_loop_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Shared global variables for TM operations */
volatile int g_tm_counter = 0;
int g_shared_array[1024];
long g_shared_matrix[32][32];
volatile int *g_volatile_ptr;

/* Prevent optimization of TM functions */
#define TM_NOOPT __attribute__((noinline, noipa, used))

/* Test function 1: Simple array processing loop in atomic transaction */
TM_NOOPT
void tm_loop_transform1(int start, int end) {
    __transaction_atomic {
        for (int i = start; i < end; i++) {
            g_shared_array[i] = g_shared_array[i] * 2 + 1;
            g_tm_counter++;
        }
    }
}

/* Test function 2: Nested loops with multi-dimensional array */
TM_NOOPT
void tm_loop_transform2(int rows, int cols) {
    __transaction_atomic {
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                g_shared_matrix[i][j] = g_shared_matrix[i][j] + 
                                       (i * cols + j);
                if (g_shared_matrix[i][j] > 1000) {
                    g_shared_matrix[i][j] = 0;
                }
            }
        }
    }
}

/* Test function 3: While loop with pointer arithmetic */
TM_NOOPT
void tm_loop_transform3(int *data, int size) {
    volatile int local_counter = 0;
    
    __transaction_relaxed {
        int *ptr = data;
        int *end = data + size;
        
        while (ptr < end) {
            *ptr = (*ptr ^ 0x55AA) + local_counter;
            ptr++;
            local_counter++;
        }
        g_tm_counter += local_counter;
    }
}

/* Test function 4: Complex control flow with conditional transaction */
TM_NOOPT
void tm_loop_transform4(int threshold) {
    int temp_array[64];
    
    /* Initialize local array */
    for (int i = 0; i < 64; i++) {
        temp_array[i] = i;
    }
    
    if (g_tm_counter > threshold) {
        __transaction_atomic {
            for (int i = 0; i < 64; i += 2) {
                g_shared_array[i] = temp_array[i] + g_shared_array[i+1];
                /* Potential cancellation point */
                if (g_shared_array[i] < 0) {
                    __transaction_cancel;
                }
            }
        }
    } else {
        __transaction_relaxed {
            for (int i = 1; i < 64; i += 2) {
                g_shared_array[i] = temp_array[i] * 2;
            }
        }
    }
}

/* Test function 5: Mixed load/store patterns with volatile */
TM_NOOPT
void tm_loop_transform5(int iterations) {
    volatile int sync_var = 0;
    
    for (int outer = 0; outer < 3; outer++) {
        __transaction_atomic {
            int sum = 0;
            
            /* Load-intensive loop */
            for (int i = 0; i < iterations; i++) {
                sum += g_shared_array[i % 1024];
            }
            
            /* Store-intensive loop */
            for (int i = 0; i < iterations && i < 1024; i++) {
                g_shared_array[i] = (g_shared_array[i] + sum) & 0xFF;
            }
            
            sync_var = sum;
            g_tm_counter += outer;
        }
        
        /* Small relaxed transaction between atomic ones */
        __transaction_relaxed {
            g_shared_array[outer] = sync_var;
        }
    }
}

/* Test function 6: Loop with function call inside transaction */
TM_NOOPT
static int helper_compute(int x, int y) {
    return (x * y) / (x + y + 1);
}

TM_NOOPT
void tm_loop_transform6(int limit) {
    __transaction_atomic {
        for (int i = 0; i < limit; i++) {
            for (int j = 0; j < 16; j++) {
                g_shared_matrix[i % 32][j] = 
                    helper_compute(g_shared_array[i], g_shared_matrix[j][i % 32]);
            }
            
            /* Every 8th iteration, do a volatile write */
            if ((i & 7) == 0) {
                g_volatile_ptr = &g_shared_array[i];
            }
        }
    }
}

/* Main function that drives all test cases */
int main(int argc, char **argv) {
    /* Initialize shared data */
    for (int i = 0; i < 1024; i++) {
        g_shared_array[i] = i;
    }
    
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            g_shared_matrix[i][j] = i * 32 + j;
        }
    }
    
    g_volatile_ptr = &g_shared_array[0];
    
    /* Execute test functions with varying parameters */
    printf("Starting TM loop transformation tests...\n");
    
    /* Use non-constant bounds from argc to prevent compile-time optimization */
    int loop_bound = (argc > 1) ? atoi(argv[1]) : 100;
    if (loop_bound <= 0) loop_bound = 100;
    if (loop_bound > 1024) loop_bound = 1024;
    
    tm_loop_transform1(0, loop_bound);
    tm_loop_transform2(loop_bound / 32, 16);
    
    int *dynamic_data = (int*)malloc(loop_bound * sizeof(int));
    for (int i = 0; i < loop_bound; i++) {
        dynamic_data[i] = i * 3;
    }
    tm_loop_transform3(dynamic_data, loop_bound);
    
    tm_loop_transform4(loop_bound / 2);
    tm_loop_transform5(loop_bound);
    tm_loop_transform6(loop_bound / 4);
    
    free(dynamic_data);
    
    /* Compute checksum to verify execution and prevent dead code elimination */
    long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += g_shared_array[i];
    }
    
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            checksum += g_shared_matrix[i][j];
        }
    }
    
    checksum += g_tm_counter;
    
    printf("TM tests completed. Checksum: %ld\n", checksum);
    printf("Final counter: %d\n", g_tm_counter);
    
    return (checksum != 0) ? 0 : 1;
}
