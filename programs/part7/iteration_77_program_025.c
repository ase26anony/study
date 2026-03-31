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
volatile int *g_volatile_ptr = NULL;

/* Prevent optimization and inlining */
#define NOOPT __attribute__((noinline, noipa, used))

/* Test function 1: Simple array processing in transactional region */
NOOPT void tm_loop_transform1(int start, int end) {
    /* Transaction with array processing loop */
    __transaction_atomic {
        for (int i = start; i < end; i++) {
            g_data_array[i] = g_data_array[i] * 2 + 1;
            g_shared_counter++;
        }
    }
}

/* Test function 2: Nested loops with multi-dimensional access pattern */
NOOPT void tm_loop_transform2(int rows, int cols) {
    volatile int local_bound = cols; /* Non-constant bound */
    
    __transaction_atomic {
        /* Process as if 2D array */
        for (int i = 0; i < rows; i++) {
            int base = i * 32; /* Simulate row offset */
            for (int j = 0; j < local_bound; j++) {
                int idx = base + j;
                if (idx < 1024) {
                    g_data_array[idx] += g_shared_counter;
                    g_large_buffer[idx % 2048] = g_data_array[idx];
                }
            }
        }
    }
}

/* Test function 3: Pointer-based loop with transaction retry logic */
NOOPT void tm_loop_transform3(int *data, int count) {
    int retries = 3;
    
    while (retries-- > 0) {
        __transaction_atomic {
            /* Pointer chasing loop */
            int *ptr = data;
            int sum = 0;
            
            for (int i = 0; i < count; i++) {
                sum += *ptr;
                *ptr = sum % 256;
                ptr++;
                
                /* Volatile access to prevent optimization */
                if (g_volatile_ptr) {
                    sum += *g_volatile_ptr;
                }
            }
            
            g_shared_counter += sum;
            
            /* Conditional transaction cancel */
            if (sum < 0) {
                __transaction_cancel;
            }
        }
        
        if (g_shared_counter > 1000) break;
    }
}

/* Test function 4: Mixed relaxed and atomic transactions */
NOOPT void tm_loop_transform4(int iterations) {
    volatile int dynamic_bound = iterations;
    
    /* First a relaxed transaction */
    __transaction_relaxed {
        int temp = 0;
        for (int i = 0; i < dynamic_bound && i < 512; i++) {
            temp += g_data_array[i];
            g_data_array[i] = temp;
        }
    }
    
    /* Then an atomic transaction with while loop */
    __transaction_atomic {
        int j = dynamic_bound - 1;
        while (j >= 0) {
            g_large_buffer[j] = g_data_array[j % 1024] + j;
            j--;
            
            /* Access volatile global */
            if (g_shared_counter > 100) {
                g_large_buffer[j + 1024] = g_shared_counter;
            }
        }
    }
}

/* Test function 5: Complex control flow with TM in branches */
NOOPT void tm_loop_transform5(int mode, int limit) {
    volatile int condition = mode;
    
    if (condition > 0) {
        __transaction_atomic {
            /* Forward loop */
            for (int i = 0; i < limit; i += 2) {
                g_data_array[i] = g_large_buffer[i] ^ 0xFF;
            }
        }
    } else {
        __transaction_relaxed {
            /* Backward loop */
            for (int i = limit - 1; i >= 0; i -= 3) {
                g_large_buffer[i] = g_data_array[i] | 0x55;
            }
        }
    }
    
    /* Another transaction in all cases */
    __transaction_atomic {
        int k = 0;
        do {
            g_data_array[k] += g_large_buffer[k % 2048];
            k++;
        } while (k < 256 && k < limit);
    }
}

/* Helper to initialize data */
void initialize_data(void) {
    for (int i = 0; i < 1024; i++) {
        g_data_array[i] = i % 128;
    }
    
    for (int i = 0; i < 2048; i++) {
        g_large_buffer[i] = i * 2;
    }
    
    g_volatile_ptr = &g_shared_counter;
}

/* Main function that orchestrates all tests */
int main(void) {
    int local_data[256];
    
    /* Initialize test data */
    initialize_data();
    
    for (int i = 0; i < 256; i++) {
        local_data[i] = i * 3;
    }
    
    /* Execute all TM test functions with varying parameters */
    tm_loop_transform1(0, 512);           /* Simple array loop */
    tm_loop_transform2(16, 32);           /* Nested loops */
    tm_loop_transform3(local_data, 256);  /* Pointer-based with retry */
    tm_loop_transform4(768);              /* Mixed transaction types */
    tm_loop_transform5(1, 1024);          /* Complex control flow */
    
    /* Also test with different parameters */
    tm_loop_transform5(-1, 512);
    tm_loop_transform1(512, 1024);
    tm_loop_transform2(8, 64);
    
    /* Calculate checksum to verify execution and prevent elimination */
    long long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += g_data_array[i];
    }
    for (int i = 0; i < 2048; i++) {
        checksum += g_large_buffer[i];
    }
    checksum += g_shared_counter;
    
    printf("TM Loop Transformation Test Complete\n");
    printf("Checksum: %lld\n", checksum);
    
    return (checksum > 0) ? 0 : 1;
}
