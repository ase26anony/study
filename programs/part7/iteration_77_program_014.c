/* tm_loop_coverage.c
 * Designed to trigger default_builtin_tm_load_store_loop_transform
 * in targhooks.cc lines 981-990
 */

#include <stdio.h>
#include <stdlib.h>

/* Shared global variables for TM operations */
volatile int g_shared_counter = 0;
int g_data_array[1024];
long g_large_buffer[2048];
volatile int g_loop_bound = 100;

/* Prevent optimization of TM functions */
#define TM_NOOPT __attribute__((noinline, noipa, used))

/* Test function 1: Simple array processing in atomic transaction */
TM_NOOPT
void tm_loop_transform1(int start, int end) {
    __transaction_atomic {
        for (int i = start; i < end; i++) {
            /* Load, modify, store pattern */
            int val = g_data_array[i];
            val = val * 2 + 1;
            g_data_array[i] = val;
            g_shared_counter++;
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
        int index = 0;
        for (int i = 0; i < v_rows; i++) {
            for (int j = 0; j < v_cols; j++) {
                /* Complex access pattern */
                g_large_buffer[index] = g_large_buffer[index] ^ 
                                       (g_data_array[i] * g_data_array[j]);
                index++;
            }
        }
    }
}

/* Test function 3: While loop with transaction cancellation */
TM_NOOPT
void tm_loop_transform3(int limit) {
    int attempts = 0;
    
    while (attempts < 3) {
        __transaction_atomic {
            int i = 0;
            while (i < limit) {
                /* Conditional store */
                if (g_data_array[i] > 1000) {
                    g_data_array[i] = g_data_array[i] / 2;
                    g_shared_counter += 2;
                } else {
                    g_data_array[i] = g_data_array[i] * 3;
                    g_shared_counter--;
                }
                i++;
            }
            
            /* Sometimes cancel to test restart logic */
            if (attempts == 1 && g_shared_counter > 50) {
                __transaction_cancel;
            }
        }
        attempts++;
    }
}

/* Test function 4: Mixed control flow with TM */
TM_NOOPT
void tm_loop_transform4(int *input, int size) {
    if (size <= 0) return;
    
    __transaction_atomic {
        /* Loop with pointer arithmetic */
        int *ptr = input;
        int *end = input + size;
        
        while (ptr < end) {
            *ptr = (*ptr << 2) | 0x1;
            ptr++;
            g_shared_counter += *ptr;
        }
        
        /* Another loop in same transaction */
        for (int i = 0; i < size; i += 2) {
            g_data_array[i] = input[i] + input[size - i - 1];
        }
    }
}

/* Test function 5: Complex nested transactions */
TM_NOOPT
void tm_loop_transform5(int iterations) {
    volatile int vi = iterations;
    
    for (int outer = 0; outer < 2; outer++) {
        __transaction_relaxed {
            int sum = 0;
            for (int i = 0; i < vi; i++) {
                /* Access multiple arrays */
                sum += g_data_array[i % 1024];
                g_large_buffer[i] = sum;
                
                /* Inner atomic region */
                __transaction_atomic {
                    g_data_array[i % 1024] = (g_data_array[i % 1024] + 1) % 100;
                }
            }
            g_shared_counter += sum;
        }
    }
}

/* Initialize test data */
void init_test_data(void) {
    for (int i = 0; i < 1024; i++) {
        g_data_array[i] = i * 3 + 7;
    }
    
    for (int i = 0; i < 2048; i++) {
        g_large_buffer[i] = i ^ 0xABCD;
    }
    
    g_shared_counter = 0;
    g_loop_bound = 100;
}

/* Compute checksum to verify execution */
long compute_checksum(void) {
    long checksum = g_shared_counter;
    
    for (int i = 0; i < 1024; i++) {
        checksum = checksum * 31 + g_data_array[i];
    }
    
    for (int i = 0; i < 2048; i += 64) {
        checksum = checksum ^ g_large_buffer[i];
    }
    
    return checksum;
}

int main(void) {
    /* Initialize test data */
    init_test_data();
    
    /* Dynamic loop bounds to prevent constant folding */
    int bound1 = g_loop_bound;
    int bound2 = g_loop_bound / 2;
    int bound3 = g_loop_bound * 3 / 4;
    
    /* Local array for function 4 */
    int local_array[200];
    for (int i = 0; i < 200; i++) {
        local_array[i] = i * i - i;
    }
    
    /* Execute all test functions with varying parameters */
    printf("Starting TM loop coverage tests...\n");
    
    tm_loop_transform1(0, bound1);
    tm_loop_transform2(16, 16);
    tm_loop_transform3(bound2);
    tm_loop_transform4(local_array, 200);
    tm_loop_transform5(bound3);
    
    /* Additional calls with different parameters */
    tm_loop_transform1(bound1, bound1 * 2);
    tm_loop_transform2(8, 32);
    tm_loop_transform3(bound3);
    
    /* Compute and print checksum */
    long final_checksum = compute_checksum();
    printf("Final checksum: %ld\n", final_checksum);
    printf("Shared counter: %d\n", g_shared_counter);
    
    return 0;
}
