/* tm_loop_coverage.c
 * Designed to trigger default_builtin_tm_load_store_loop_transform
 * in targhooks.cc lines 981-990
 */

#include <stdio.h>
#include <stdlib.h>

/* Shared global variables for TM operations */
volatile int g_counter = 0;
int g_array[1024];
long g_matrix[32][32];
volatile int g_loop_bound = 64;

/* Prevent optimization and inlining */
__attribute__((noinline, noipa, used))
void tm_loop_transform1(int n) {
    __transaction_atomic {
        /* Simple array processing loop */
        for (int i = 0; i < n; ++i) {
            g_array[i] = g_array[i] * 2 + 1;
            g_counter += g_array[i];
        }
    }
}

__attribute__((noinline, noipa, used))
void tm_loop_transform2(int rows, int cols) {
    /* Nested loops with 2D array access */
    __transaction_relaxed {
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                g_matrix[i][j] = g_matrix[i][j] + i - j;
                if (g_matrix[i][j] < 0) {
                    g_matrix[i][j] = 0;
                }
            }
        }
    }
}

__attribute__((noinline, noipa, used))
void tm_loop_transform3(int limit) {
    /* While loop with pointer arithmetic */
    int* ptr = g_array;
    int count = 0;
    
    __transaction_atomic {
        while (count < limit) {
            *ptr = (*ptr ^ 0x55AA) + count;
            ptr++;
            count++;
            
            /* Nested transaction for retry logic */
            if (count % 16 == 0) {
                __transaction_relaxed {
                    g_counter += *ptr;
                }
            }
        }
    }
}

__attribute__((noinline, noipa, used))
void tm_loop_transform4(int n, int condition) {
    /* Complex control flow with conditional TM */
    if (condition) {
        __transaction_atomic {
            /* Loop with stride */
            for (int i = 0; i < n; i += 2) {
                g_array[i] = g_array[i] * g_array[i + 1];
                g_array[i + 1] = g_array[i] - g_array[i + 1];
            }
        }
    } else {
        /* Alternative path with relaxed transaction */
        __transaction_relaxed {
            for (int i = n - 1; i >= 0; --i) {
                g_array[i] = g_array[i] / 2;
            }
        }
    }
}

__attribute__((noinline, noipa, used))
void tm_nested_transactions(int depth) {
    /* Nested transactional regions */
    for (int d = 0; d < depth; ++d) {
        __transaction_atomic {
            /* Loop that might be transformed */
            int bound = g_loop_bound >> d;
            for (int i = 0; i < bound; ++i) {
                g_array[i] += d;
                
                /* Inner relaxed transaction */
                if (i % 8 == 0) {
                    __transaction_relaxed {
                        g_matrix[d][i % 32] = g_array[i];
                    }
                }
            }
        }
    }
}

__attribute__((noinline, noipa, used))
void tm_with_cancel_retry(int iterations) {
    /* TM with cancel/retry logic */
    int retries = 3;
    
    while (retries-- > 0) {
        __transaction_atomic {
            for (int i = 0; i < iterations; ++i) {
                g_array[i] = (g_array[i] << 1) | 0x1;
                
                /* Conditional cancel */
                if (g_array[i] > 1000000 && retries > 0) {
                    __transaction_cancel;
                }
            }
            break; /* Success */
        }
    }
}

/* Initialize test data */
void init_data(void) {
    for (int i = 0; i < 1024; ++i) {
        g_array[i] = i % 256;
    }
    
    for (int i = 0; i < 32; ++i) {
        for (int j = 0; j < 32; ++j) {
            g_matrix[i][j] = i * 32 + j;
        }
    }
    
    g_counter = 0;
    g_loop_bound = 128;
}

/* Compute checksum to verify execution */
long compute_checksum(void) {
    long sum = g_counter;
    
    for (int i = 0; i < 256; ++i) {
        sum += g_array[i];
    }
    
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 16; ++j) {
            sum += g_matrix[i][j];
        }
    }
    
    return sum;
}

int main(void) {
    /* Initialize shared data */
    init_data();
    
    /* Execute various TM loop patterns */
    tm_loop_transform1(256);
    tm_loop_transform2(16, 16);
    tm_loop_transform3(128);
    tm_loop_transform4(64, 1);
    tm_nested_transactions(4);
    tm_loop_transform4(128, 0);
    tm_with_cancel_retry(32);
    
    /* Final verification */
    long checksum = compute_checksum();
    printf("TM Loop Transformation Test\n");
    printf("Final checksum: %ld\n", checksum);
    printf("g_counter: %d\n", g_counter);
    
    return 0;
}
