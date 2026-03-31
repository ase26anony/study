/* tm_loop_coverage.c
 * Designed to trigger default_builtin_tm_load_store_loop_transform in targhooks.cc
 * Compile with: gcc -O1 -fgnu-tm -o tm_test tm_loop_coverage.c
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

/* Function 1: Simple array processing loop in transactional region */
TM_NOOPT
void tm_loop_transform1(int start, int end) {
    __transaction_atomic {
        for (int i = start; i < end; i++) {
            /* Load and store pattern that should trigger TM transformation */
            int old_val = g_data_array[i];
            g_data_array[i] = old_val * 2 + i;
            g_shared_counter++;
        }
    }
}

/* Function 2: Nested loops with multi-dimensional access pattern */
TM_NOOPT
void tm_loop_transform2(int rows, int cols) {
    /* Use volatile to prevent constant propagation */
    volatile int local_bound = cols;
    
    __transaction_atomic {
        for (int i = 0; i < rows; i++) {
            int base_idx = i * 32;  /* Non-constant stride */
            for (int j = 0; j < local_bound; j++) {
                /* Complex load/store pattern */
                long temp = g_large_buffer[base_idx + j];
                g_large_buffer[base_idx + j] = temp ^ (i * j);
                
                /* Also modify the data array */
                if (j < 1024) {
                    g_data_array[j] += (int)temp;
                }
            }
        }
    }
}

/* Function 3: While loop with transaction retry logic */
TM_NOOPT
void tm_loop_transform3(int max_attempts) {
    int attempts = 0;
    
    while (attempts < max_attempts) {
        __transaction_atomic {
            int bound = g_loop_bound;  /* Volatile read */
            int sum = 0;
            
            /* Loop that processes data */
            for (int i = 0; i < bound; i++) {
                sum += g_data_array[i % 1024];
            }
            
            /* Store result with potential conflict */
            g_large_buffer[attempts % 2048] = sum;
            
            /* Conditional transaction cancel */
            if (attempts == 0 && sum > 1000000) {
                __transaction_cancel;
            }
        }
        attempts++;
    }
}

/* Function 4: Mixed relaxed and atomic transactions */
TM_NOOPT
void tm_loop_transform4(int iterations) {
    for (int iter = 0; iter < iterations; iter++) {
        /* Relaxed transaction for read-heavy phase */
        __transaction_relaxed {
            int local_sum = 0;
            for (int i = 0; i < 256; i++) {
                local_sum += g_data_array[i * 4];
            }
            g_shared_counter += (local_sum > 0);
        }
        
        /* Atomic transaction for write phase */
        __transaction_atomic {
            for (int i = 0; i < 128; i++) {
                int idx = (iter * 128 + i) % 1024;
                g_data_array[idx] = g_data_array[idx] ^ g_shared_counter;
            }
        }
    }
}

/* Function 5: Complex control flow with TM in branches */
TM_NOOPT
void tm_loop_transform5(int mode, int limit) {
    volatile int dynamic_limit = limit;
    
    if (mode == 0) {
        __transaction_atomic {
            /* Forward loop */
            for (int i = 0; i < dynamic_limit; i++) {
                g_data_array[i] = g_data_array[i] * 3 - 7;
            }
        }
    } else if (mode == 1) {
        __transaction_relaxed {
            /* Reverse loop */
            for (int i = dynamic_limit - 1; i >= 0; i--) {
                g_large_buffer[i] = g_data_array[i % 1024] + i;
            }
        }
    } else {
        /* Nested transaction with loop */
        __transaction_atomic {
            int step = 2;
            while (step < dynamic_limit) {
                g_data_array[step] += g_large_buffer[step];
                step *= 2;
            }
        }
    }
}

/* Initialize test data */
void init_test_data(void) {
    for (int i = 0; i < 1024; i++) {
        g_data_array[i] = i * 3 + 7;
    }
    
    for (int i = 0; i < 2048; i++) {
        g_large_buffer[i] = i * 5 - 3;
    }
    
    g_shared_counter = 42;
    g_loop_bound = 150;  /* Non-constant initial value */
}

/* Compute checksum to verify execution */
long compute_checksum(void) {
    long checksum = g_shared_counter;
    
    for (int i = 0; i < 1024; i++) {
        checksum = checksum * 31 + g_data_array[i];
    }
    
    for (int i = 0; i < 2048; i += 64) {
        checksum = checksum * 17 + g_large_buffer[i];
    }
    
    return checksum;
}

int main(void) {
    /* Initialize test data */
    init_test_data();
    
    printf("Starting TM loop transformation tests...\n");
    
    /* Execute various TM loop patterns */
    tm_loop_transform1(0, 200);
    tm_loop_transform1(100, 300);
    
    tm_loop_transform2(8, 32);
    tm_loop_transform2(4, 64);
    
    tm_loop_transform3(5);
    
    tm_loop_transform4(3);
    
    tm_loop_transform5(0, 80);
    tm_loop_transform5(1, 120);
    tm_loop_transform5(2, 200);
    
    /* Additional complex pattern */
    __transaction_atomic {
        volatile int outer = 3;
        for (int k = 0; k < outer; k++) {
            for (int i = 0; i < g_loop_bound; i += 16) {
                for (int j = 0; j < 16; j++) {
                    int idx = i + j;
                    if (idx < 1024) {
                        g_data_array[idx] = (g_data_array[idx] + g_large_buffer[idx]) % 1000;
                    }
                }
            }
        }
    }
    
    /* Compute and print checksum */
    long final_checksum = compute_checksum();
    printf("Final checksum: %ld\n", final_checksum);
    printf("Shared counter: %d\n", g_shared_counter);
    
    return 0;
}
