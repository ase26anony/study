/* tm_loop_transform_test.c
 * Designed to trigger default_builtin_tm_load_store_loop_transform in targhooks.cc
 * Compile with: gcc -O1 -fgnu-tm -o tm_test tm_loop_transform_test.c
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

/* Test function 1: Simple array processing loop within transaction */
NOOPT void tm_loop_transform1(int start, int end, int increment) {
    __transaction_atomic {
        for (int i = start; i < end; i += increment) {
            /* Load-store pattern that may trigger loop transformation */
            int old_val = g_data_array[i];
            g_data_array[i] = old_val * 2 + i;
            g_shared_counter++;
        }
    }
}

/* Test function 2: Nested loops with multi-dimensional access pattern */
NOOPT void tm_loop_transform2(int rows, int cols) {
    /* Use relaxed transaction for variety */
    __transaction_relaxed {
        int index = 0;
        for (int i = 0; i < rows; i++) {
            /* Inner loop with pointer arithmetic */
            int *row_ptr = &g_data_array[i * cols];
            for (int j = 0; j < cols; j++) {
                /* Complex load-store pattern */
                row_ptr[j] = (row_ptr[j] + g_shared_counter) ^ (i * j);
                g_large_buffer[index++] = row_ptr[j] * 3;
            }
        }
    }
}

/* Test function 3: While loop with conditional transaction cancel */
NOOPT void tm_loop_transform3(int limit) {
    int attempts = 0;
    while (attempts < 3) {
        __transaction_atomic {
            int i = 0;
            while (i < limit) {
                /* Volatile pointer access to prevent optimization */
                if (g_volatile_ptr) {
                    g_data_array[i] += g_volatile_ptr[i % 16];
                }
                
                /* Load-modify-store with dependency */
                g_large_buffer[i] = g_large_buffer[i] * 2 - g_data_array[i];
                
                /* Conditional transaction cancel */
                if (g_data_array[i] > 1000000) {
                    __transaction_cancel;
                }
                i++;
            }
            break; /* Success */
        }
        attempts++;
    }
}

/* Test function 4: Complex control flow with TM in branches */
NOOPT void tm_loop_transform4(int threshold, int size) {
    volatile int local_seed = threshold;
    
    if (local_seed > 0) {
        __transaction_atomic {
            /* Loop with stride not known at compile time */
            for (int i = 0; i < size; i += (local_seed % 3) + 1) {
                /* Multiple memory operations */
                g_data_array[i] = (g_data_array[i] << 2) | (g_large_buffer[i] & 0x3);
                g_large_buffer[i] = g_data_array[i] ^ g_shared_counter;
                
                /* Function call within transaction (simulated) */
                if ((i % 7) == 0) {
                    g_shared_counter += (i * 2);
                }
            }
        }
    } else {
        __transaction_relaxed {
            int i = size - 1;
            while (i >= 0) {
                /* Reverse traversal */
                g_data_array[i] -= g_large_buffer[size - i - 1];
                i--;
            }
        }
    }
}

/* Test function 5: Multiple consecutive transactions */
NOOPT void tm_loop_transform5(int iterations) {
    for (int trans_num = 0; trans_num < 3; trans_num++) {
        __transaction_atomic {
            /* Loop with runtime-dependent bounds */
            int chunk = iterations / (trans_num + 1);
            for (int i = 0; i < chunk; i++) {
                /* Interleaved access pattern */
                int idx = (i * 17) % 1024;
                g_data_array[idx] += trans_num;
                g_large_buffer[i % 2048] -= g_data_array[idx];
                
                /* Volatile store to ensure visibility */
                g_shared_counter = g_shared_counter + 1;
            }
        }
        
        /* Small relaxed transaction between atomic ones */
        if (trans_num < 2) {
            __transaction_relaxed {
                for (int i = 0; i < 10; i++) {
                    g_data_array[i] = g_data_array[i] ^ 0xFF;
                }
            }
        }
    }
}

/* Initialize test data */
void init_test_data(void) {
    for (int i = 0; i < 1024; i++) {
        g_data_array[i] = i * 3 + 1;
    }
    
    for (int i = 0; i < 2048; i++) {
        g_large_buffer[i] = i - 1024;
    }
    
    static volatile int ptr_buffer[16];
    for (int i = 0; i < 16; i++) {
        ptr_buffer[i] = i * i;
    }
    g_volatile_ptr = ptr_buffer;
}

/* Main function that drives all tests */
int main(void) {
    /* Initialize shared data */
    init_test_data();
    
    printf("Starting TM loop transformation tests...\n");
    
    /* Execute test functions with varying parameters */
    tm_loop_transform1(0, 512, 1);           /* Simple stride 1 loop */
    tm_loop_transform1(100, 400, 3);         /* Different stride */
    
    tm_loop_transform2(32, 32);              /* 32x32 grid */
    tm_loop_transform2(16, 64);              /* Different aspect ratio */
    
    tm_loop_transform3(256);                 /* While loop with cancel */
    tm_loop_transform3(128);                 /* Smaller limit */
    
    tm_loop_transform4(5, 256);              /* Complex branch taken */
    tm_loop_transform4(-1, 128);             /* Else branch */
    
    tm_loop_transform5(300);                 /* Multiple transactions */
    tm_loop_transform5(150);                 /* Fewer iterations */
    
    /* Compute checksum to verify execution and prevent elimination */
    long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += g_data_array[i];
    }
    for (int i = 0; i < 2048; i++) {
        checksum += g_large_buffer[i];
    }
    checksum += g_shared_counter;
    
    printf("Final checksum: %ld\n", checksum);
    printf("Shared counter: %d\n", g_shared_counter);
    
    return 0;
}
