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

/* Prevent optimization of TM functions */
#define TM_NOOPT __attribute__((noinline, noipa, used))

/* Test function 1: Simple array processing loop in atomic transaction */
TM_NOOPT
void tm_loop_transform1(int start, int end) {
    /* Transaction with array load/store loop */
    __transaction_atomic {
        for (int i = start; i < end; i++) {
            /* Load, modify, store pattern */
            int val = g_data_array[i];
            val = val * 2 + 1;
            g_data_array[i] = val;
            
            /* Volatile access to prevent optimization */
            g_shared_counter++;
        }
    }
}

/* Test function 2: Nested loops with multi-dimensional access pattern */
TM_NOOPT
void tm_loop_transform2(int rows, int cols) {
    /* Treat linear array as 2D for nested loop access */
    __transaction_atomic {
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                int idx = i * cols + j;
                if (idx < 1024) {
                    /* Complex load/store with condition */
                    g_data_array[idx] = g_data_array[idx] ^ (i * j);
                }
            }
        }
    }
    
    /* Additional relaxed transaction */
    __transaction_relaxed {
        int i = 0;
        while (i < rows * 2) {
            if (i < 2048) {
                g_large_buffer[i] = g_large_buffer[i] + g_shared_counter;
            }
            i++;
        }
    }
}

/* Test function 3: Pointer-based loop with transaction cancellation */
TM_NOOPT
void tm_loop_transform3(int *buffer, int size, int threshold) {
    int retries = 3;
    
    while (retries-- > 0) {
        __transaction_atomic {
            /* Pointer chasing loop */
            int *ptr = buffer;
            int sum = 0;
            
            for (int i = 0; i < size; i++) {
                sum += *ptr;
                *ptr = sum % 256;
                ptr++;
            }
            
            /* Conditional transaction cancel */
            if (sum > threshold) {
                __transaction_cancel;
            } else {
                g_shared_counter = sum;
                break; /* Success */
            }
        }
    }
}

/* Test function 4: Mixed control flow with TM in branches */
TM_NOOPT
void tm_loop_transform4(int mode, int iterations) {
    volatile int local_seed = mode * 100;
    
    if (mode & 1) {
        __transaction_atomic {
            /* Do-while loop variant */
            int count = 0;
            do {
                int idx = (local_seed + count) % 1024;
                g_data_array[idx] = g_data_array[idx] * 3 - 2;
                count++;
            } while (count < iterations);
        }
    } else {
        __transaction_relaxed {
            /* For loop with stride */
            for (int i = 0; i < iterations; i += 2) {
                if (i < 2048) {
                    g_large_buffer[i] = g_large_buffer[i] | 0xFF00FF00;
                }
            }
        }
    }
}

/* Test function 5: Complex loop with function calls inside TM */
TM_NOOPT
static int helper_transform(int x, int y) {
    return (x ^ y) + (x & y);
}

TM_NOOPT
void tm_loop_transform5(int limit) {
    __transaction_atomic {
        /* Loop with function call - may create complex TM regions */
        for (int i = 0; i < limit; i++) {
            int idx1 = i % 1024;
            int idx2 = (i * 7) % 1024;
            
            int temp = helper_transform(g_data_array[idx1], g_data_array[idx2]);
            g_data_array[idx1] = temp;
            
            /* Access through volatile pointer */
            if (g_volatile_ptr) {
                *g_volatile_ptr = i;
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
        g_large_buffer[i] = i * 5 - 2;
    }
    
    g_volatile_ptr = &g_shared_counter;
}

/* Compute checksum to verify execution */
long compute_checksum(void) {
    long checksum = g_shared_counter;
    
    for (int i = 0; i < 1024; i++) {
        checksum = checksum * 31 + g_data_array[i];
    }
    
    for (int i = 0; i < 2048; i += 64) { /* Sample every 64th */
        checksum = checksum ^ g_large_buffer[i];
    }
    
    return checksum;
}

int main(void) {
    /* Initialize test data */
    init_test_data();
    
    printf("Starting TM loop transformation tests...\n");
    
    /* Execute various TM patterns to trigger the hook */
    tm_loop_transform1(0, 512);           /* Simple array loop */
    tm_loop_transform1(256, 768);         /* Overlapping region */
    
    tm_loop_transform2(32, 32);           /* Nested loops */
    tm_loop_transform2(16, 64);           /* Different shape */
    
    int local_buffer[256];
    for (int i = 0; i < 256; i++) {
        local_buffer[i] = i * 11 % 97;
    }
    tm_loop_transform3(local_buffer, 256, 5000); /* With cancellation */
    
    tm_loop_transform4(1, 100);           /* Do-while in atomic */
    tm_loop_transform4(0, 200);           /* For loop in relaxed */
    
    tm_loop_transform5(300);              /* With helper function */
    
    /* Additional mixed transaction */
    __transaction_atomic {
        for (int i = 100; i < 200; i++) {
            g_data_array[i] = g_data_array[i] + g_data_array[i-50];
        }
    }
    
    /* Compute and print checksum */
    long final_checksum = compute_checksum();
    printf("Final checksum: %ld\n", final_checksum);
    printf("Shared counter: %d\n", g_shared_counter);
    
    return 0;
}
