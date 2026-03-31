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
volatile int *g_volatile_ptr;

/* Prevent optimization and inlining */
#define NOOPT __attribute__((noinline, noipa, used))

/* Test function 1: Simple array processing loop in atomic transaction */
NOOPT void tm_loop_transform1(int start, int end) {
    __transaction_atomic {
        for (int i = start; i < end; i++) {
            /* Load-store pattern that may trigger loop transformation */
            int val = g_data_array[i];          /* Load */
            val = val * 2 + 1;                  /* Transformation */
            g_data_array[i] = val;              /* Store */
            g_shared_counter++;                 /* Volatile access */
        }
    }
}

/* Test function 2: Nested loops with multi-dimensional access pattern */
NOOPT void tm_loop_transform2(int rows, int cols) {
    /* Use volatile to prevent constant propagation */
    volatile int v_rows = rows;
    volatile int v_cols = cols;
    
    __transaction_atomic {
        for (int i = 0; i < v_rows; i++) {
            for (int j = 0; j < v_cols; j++) {
                /* Simulate 2D array access with 1D array */
                int idx = i * 32 + j;
                if (idx < 1024) {
                    long temp = g_large_buffer[idx];
                    g_large_buffer[idx] = temp ^ (i * j);  /* Store with computation */
                }
            }
        }
    }
}

/* Test function 3: While loop with pointer arithmetic in relaxed transaction */
NOOPT void tm_loop_transform3(int *ptr, int count) {
    __transaction_relaxed {
        int *current = ptr;
        int i = 0;
        
        while (i < count) {
            /* Complex load-store with pointer chasing */
            int value = *current;
            value = (value << 3) | (value >> 5);  /* Rotate */
            *current = value;
            
            current++;
            i++;
            
            /* Access volatile global */
            if (g_shared_counter > 1000) {
                *current = *current ^ 0xFF;
            }
        }
    }
}

/* Test function 4: Transaction with cancellation and retry logic */
NOOPT void tm_loop_transform4(int threshold) {
    int retries = 3;
    
    while (retries-- > 0) {
        __transaction_atomic {
            /* Process array with early exit condition */
            for (int i = 0; i < 256; i++) {
                g_data_array[i] += i;
                
                /* Conditional transaction cancel */
                if (g_data_array[i] > threshold && retries < 2) {
                    __transaction_cancel;
                }
            }
            
            /* Another inner loop */
            int j = 0;
            do {
                g_large_buffer[j] = g_large_buffer[j] * 2 - 1;
                j += 16;  /* Non-unit stride */
            } while (j < 512);
        }
        
        if (retries == 0) break;
    }
}

/* Test function 5: Mixed transaction types with complex control flow */
NOOPT void tm_loop_transform5(int mode) {
    volatile int dynamic_bound = mode * 64 + 32;
    
    if (mode & 1) {
        __transaction_atomic {
            /* Loop with break/continue */
            for (int i = 0; i < dynamic_bound; i++) {
                if (i % 7 == 0) continue;
                
                g_data_array[i] = g_data_array[i] ^ g_large_buffer[i % 256];
                
                if (g_data_array[i] < 0) {
                    g_data_array[i] = -g_data_array[i];
                    break;  /* Early exit from loop */
                }
            }
        }
    } else {
        __transaction_relaxed {
            int k = dynamic_bound;
            while (k > 0) {
                /* Reverse iteration */
                g_large_buffer[k] = g_large_buffer[k] + k;
                k -= 4;  /* Negative stride */
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
        g_large_buffer[i] = i ^ 0xABCD;
    }
    
    g_volatile_ptr = &g_shared_counter;
}

/* Compute checksum to verify execution and prevent elimination */
long compute_checksum(void) {
    long sum = 0;
    
    for (int i = 0; i < 1024; i++) {
        sum += g_data_array[i];
    }
    
    for (int i = 0; i < 2048; i++) {
        sum += g_large_buffer[i];
    }
    
    sum += g_shared_counter;
    
    return sum;
}

int main(void) {
    init_test_data();
    
    printf("Starting TM loop transformation tests...\n");
    
    /* Execute test functions with varying parameters */
    tm_loop_transform1(0, 512);
    tm_loop_transform1(256, 768);
    
    tm_loop_transform2(16, 32);
    tm_loop_transform2(8, 64);
    
    int local_array[128];
    for (int i = 0; i < 128; i++) local_array[i] = i * 2;
    tm_loop_transform3(local_array, 128);
    
    tm_loop_transform4(10000);
    tm_loop_transform4(5000);
    
    tm_loop_transform5(0);
    tm_loop_transform5(1);
    tm_loop_transform5(2);
    
    /* Additional mixed transaction regions */
    __transaction_atomic {
        for (int i = 100; i < 200; i++) {
            g_data_array[i] = g_data_array[i] * g_data_array[i - 50];
        }
    }
    
    __transaction_relaxed {
        int temp = 0;
        for (int i = 0; i < 100; i++) {
            temp += g_large_buffer[i * 10];
        }
        g_shared_counter += temp;
    }
    
    /* Nested transaction attempt */
    {
        int x = 42;
        __transaction_atomic {
            x = x * 2;
            __transaction_relaxed {
                for (int i = 0; i < 50; i++) {
                    g_data_array[i] += x;
                }
            }
        }
    }
    
    long final_checksum = compute_checksum();
    printf("Final checksum: %ld\n", final_checksum);
    printf("Shared counter: %d\n", g_shared_counter);
    
    return 0;
}
