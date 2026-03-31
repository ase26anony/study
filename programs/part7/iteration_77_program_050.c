/* tm_loop_coverage.c
 * Designed to trigger default_builtin_tm_load_store_loop_transform
 * in targhooks.cc lines 981-990
 */

#include <stdio.h>
#include <stdlib.h>

/* Shared global variables for TM operations */
volatile int g_shared_array[1024];
volatile long g_shared_counter = 0;
int g_data_buffer[2048];
volatile int* g_volatile_ptr = NULL;

/* Prevent optimization and inlining */
#define NOOPT __attribute__((noinline, noipa, used))

/* Test function 1: Simple array processing in atomic transaction */
NOOPT void tm_loop_transform1(int start, int end) {
    __transaction_atomic {
        for (int i = start; i < end; i++) {
            /* Load-store pattern with arithmetic */
            int val = g_shared_array[i % 1024];
            g_shared_array[(i + 1) % 1024] = val * 2 + 1;
            g_shared_counter++;
        }
    }
}

/* Test function 2: Nested loops with relaxed transaction */
NOOPT void tm_loop_transform2(int rows, int cols) {
    __transaction_relaxed {
        for (int i = 0; i < rows; i++) {
            int base = i * cols;
            for (int j = 0; j < cols; j++) {
                /* Multi-dimensional access pattern */
                int idx = (base + j) % 2048;
                int old = g_data_buffer[idx];
                g_data_buffer[(idx + cols) % 2048] = old ^ 0x55AA;
                
                /* Volatile pointer access */
                if (g_volatile_ptr) {
                    *g_volatile_ptr += old;
                }
            }
        }
    }
}

/* Test function 3: Conditional transaction with loop */
NOOPT void tm_loop_transform3(int limit, int threshold) {
    if (limit > threshold) {
        __transaction_atomic {
            int i = 0;
            while (i < limit) {
                /* Complex load-store with condition */
                int idx1 = (i * 7) % 1024;
                int idx2 = (i * 13) % 1024;
                
                int temp = g_shared_array[idx1];
                g_shared_array[idx2] = temp + g_shared_counter;
                
                if (temp % 3 == 0) {
                    g_data_buffer[i % 2048] = temp >> 2;
                }
                
                i += (limit % 5) + 1; /* Non-constant increment */
            }
        }
    } else {
        /* Fallback non-transactional path */
        for (int i = 0; i < limit; i++) {
            g_shared_array[i % 1024] = i;
        }
    }
}

/* Test function 4: Transaction with cancellation and retry */
NOOPT void tm_loop_transform4(int attempts) {
    for (int try = 0; try < attempts; try++) {
        __transaction_atomic {
            /* Loop that might cancel */
            for (int i = 0; i < 100; i++) {
                g_shared_array[i] = g_shared_array[i] + try;
                
                /* Cancel transaction on certain condition */
                if (try > 2 && i == 50) {
                    __transaction_cancel;
                }
            }
            
            /* This won't execute if transaction cancelled */
            g_shared_counter += 100;
        }
    }
}

/* Test function 5: Pointer-chasing loop in transaction */
NOOPT void tm_loop_transform5(int* data, int size, int iterations) {
    volatile int* current = data;
    int sum = 0;
    
    __transaction_relaxed {
        for (int iter = 0; iter < iterations; iter++) {
            /* Pointer-chasing pattern */
            for (int i = 0; i < size; i++) {
                sum += *current;
                current = data + ((current - data + 1) % size);
                
                /* Store back modified value */
                *(data + i) = sum % 256;
            }
        }
    }
    
    /* Use sum to prevent elimination */
    g_shared_array[0] = sum % 1024;
}

/* Initialize data with non-zero values */
void initialize_data(void) {
    for (int i = 0; i < 1024; i++) {
        g_shared_array[i] = i * 3 + 1;
    }
    
    for (int i = 0; i < 2048; i++) {
        g_data_buffer[i] = i ^ 0x1234;
    }
    
    g_volatile_ptr = &g_shared_array[0];
}

/* Main function that orchestrates all tests */
int main(void) {
    /* Initialize test data */
    initialize_data();
    
    /* Dynamic bounds to prevent constant folding */
    volatile int bound1 = 500;
    volatile int bound2 = 32;
    volatile int bound3 = 150;
    volatile int bound4 = 5;
    volatile int bound5 = 100;
    
    /* Execute all test functions with varying parameters */
    tm_loop_transform1(bound1 % 256, bound1 % 256 + 300);
    tm_loop_transform2(bound2 % 16 + 4, bound2 % 8 + 8);
    tm_loop_transform3(bound3 % 200 + 50, 100);
    tm_loop_transform4(bound4 % 10 + 1);
    tm_loop_transform5(g_data_buffer, bound5 % 128 + 64, 3);
    
    /* Compute checksum to verify execution and prevent dead code elimination */
    long checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum += g_shared_array[i];
        checksum += g_data_buffer[i];
    }
    checksum += g_shared_counter;
    
    printf("TM Loop Test Checksum: %ld\n", checksum);
    
    /* Additional volatile store to ensure all transactions complete */
    volatile int final_check = checksum % 1000;
    g_shared_array[1023] = final_check;
    
    return final_check != 0;
}
