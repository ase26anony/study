/* tm_loop_coverage.c
 * Designed to trigger default_builtin_tm_load_store_loop_transform
 * in GCC's targhooks.cc, specifically lines 981-990
 */

#include <stdio.h>
#include <stdlib.h>

/* Shared global variables for TM operations */
volatile int g_tm_counter = 0;
int g_shared_array[1024];
long g_large_buffer[2048];
volatile int g_loop_bound = 100;

/* Prevent optimization of TM functions */
#define TM_NOOPT __attribute__((noinline, noipa, used))

/* Test function 1: Simple array processing in atomic transaction */
TM_NOOPT
void tm_loop_transform1(int start, int end) {
    __transaction_atomic {
        for (int i = start; i < end; i++) {
            /* Load-store pattern with arithmetic */
            int val = g_shared_array[i];
            val = val * 2 + 1;
            g_shared_array[i] = val;
            g_tm_counter++;
        }
    }
}

/* Test function 2: Nested loops with relaxed transaction */
TM_NOOPT
void tm_loop_transform2(int rows, int cols) {
    /* Use volatile to prevent constant propagation */
    volatile int v_rows = rows;
    volatile int v_cols = cols;
    
    __transaction_relaxed {
        for (int i = 0; i < v_rows; i++) {
            for (int j = 0; j < v_cols; j++) {
                /* Multi-dimensional access pattern */
                int idx = i * cols + j;
                if (idx < 1024) {
                    g_shared_array[idx] += g_tm_counter;
                }
            }
        }
    }
}

/* Test function 3: Pointer-based loop with transaction cancellation */
TM_NOOPT
void tm_loop_transform3(int* data, int size) {
    int attempts = 0;
    
    while (attempts < 3) {
        __transaction_atomic {
            for (int i = 0; i < size; i++) {
                /* Complex load-store with condition */
                long temp = g_large_buffer[i];
                if (temp > 1000) {
                    data[i % 256] = (int)(temp % 100);
                    g_tm_counter += 2;
                } else {
                    data[i % 256] = (int)temp;
                    g_tm_counter++;
                }
            }
            
            /* Occasionally cancel to test retry logic */
            if (attempts == 1 && (g_tm_counter % 7) == 0) {
                __transaction_cancel;
            }
        }
        attempts++;
    }
}

/* Test function 4: Mixed control flow with TM */
TM_NOOPT
void tm_loop_transform4(int threshold) {
    volatile int local_bound = g_loop_bound;
    
    if (threshold > 50) {
        __transaction_atomic {
            int i = 0;
            while (i < local_bound) {
                /* While loop variant */
                g_shared_array[i % 1024] ^= 0xAA;
                g_large_buffer[i] = g_shared_array[i % 1024] * 3;
                i += (g_tm_counter % 3) + 1;  /* Non-uniform stride */
            }
        }
    } else {
        __transaction_relaxed {
            for (int i = local_bound - 1; i >= 0; i--) {
                /* Reverse loop */
                g_shared_array[i] = g_large_buffer[i * 2] / 2;
            }
        }
    }
}

/* Test function 5: Complex nested transactions */
TM_NOOPT
void tm_loop_transform5(int iterations) {
    for (int iter = 0; iter < iterations; iter++) {
        __transaction_atomic {
            /* Outer transaction */
            int chunk = 64;
            for (int i = 0; i < chunk; i++) {
                g_shared_array[i] += iter;
            }
            
            /* Inner relaxed transaction */
            __transaction_relaxed {
                for (int j = chunk; j < chunk * 2; j++) {
                    if (j < 1024) {
                        g_shared_array[j] -= iter;
                    }
                }
            }
            
            /* More operations after nested transaction */
            for (int k = chunk * 2; k < chunk * 3; k++) {
                if (k < 1024) {
                    g_shared_array[k] *= 2;
                }
            }
        }
    }
}

/* Initialize data */
void init_data(void) {
    for (int i = 0; i < 1024; i++) {
        g_shared_array[i] = i % 256;
    }
    for (int i = 0; i < 2048; i++) {
        g_large_buffer[i] = i * 3;
    }
    g_tm_counter = 0;
    g_loop_bound = 100;
}

/* Compute checksum to verify execution */
int compute_checksum(void) {
    int sum = 0;
    for (int i = 0; i < 1024; i++) {
        sum = (sum + g_shared_array[i]) % 1000000;
    }
    for (int i = 0; i < 2048; i++) {
        sum = (sum + (int)(g_large_buffer[i] % 1000)) % 1000000;
    }
    return sum + g_tm_counter;
}

int main(void) {
    int local_data[256];
    
    /* Initialize local data */
    for (int i = 0; i < 256; i++) {
        local_data[i] = i * 2;
    }
    
    /* Initialize shared data */
    init_data();
    
    printf("Starting TM loop coverage test...\n");
    
    /* Execute all test functions with varying parameters */
    tm_loop_transform1(0, 200);
    tm_loop_transform1(150, 300);
    
    tm_loop_transform2(16, 32);
    tm_loop_transform2(8, 64);
    
    tm_loop_transform3(local_data, 500);
    tm_loop_transform3(local_data + 128, 400);
    
    tm_loop_transform4(75);
    tm_loop_transform4(25);
    
    tm_loop_transform5(3);
    tm_loop_transform5(2);
    
    /* Additional mixed test */
    for (int repeat = 0; repeat < 2; repeat++) {
        __transaction_atomic {
            for (int i = 0; i < g_loop_bound; i += 2) {
                g_shared_array[i] = g_large_buffer[i * 3] + repeat;
            }
        }
        
        __transaction_relaxed {
            int j = g_loop_bound - 1;
            while (j >= 0) {
                g_large_buffer[j] = g_shared_array[j % 1024] - repeat;
                j -= (g_tm_counter % 5) + 1;
            }
        }
    }
    
    /* Compute and print checksum */
    int checksum = compute_checksum();
    printf("Final checksum: %d\n", checksum);
    printf("TM counter: %d\n", g_tm_counter);
    
    return checksum != 0 ? 0 : 1;
}
