/* tm_loop_test.c - Test program for GCC transactional memory loop transformation */
#include <stdio.h>
#include <stdlib.h>

/* Shared global variables for TM operations */
volatile int g_shared_counter = 0;
int g_data_array[1024];
long g_large_buffer[2048];
volatile int *g_volatile_ptr = NULL;

/* Prevent optimization and inlining */
__attribute__((noinline, noipa, used))
void tm_loop_transform1(int start, int end) {
    /* Transaction with array processing loop */
    __transaction_atomic {
        for (int i = start; i < end; i++) {
            g_data_array[i] = g_data_array[i] * 2 + g_shared_counter;
            g_shared_counter++;
        }
    }
}

__attribute__((noinline, noipa, used))
void tm_loop_transform2(int rows, int cols) {
    /* Nested loops with multi-dimensional access pattern */
    __transaction_relaxed {
        int idx = 0;
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                g_large_buffer[idx] = g_large_buffer[idx] ^ (i * j);
                idx++;
            }
        }
    }
    
    /* Another transaction with while loop */
    __transaction_atomic {
        int k = 0;
        while (k < rows * cols) {
            if (g_large_buffer[k] < 0) {
                g_large_buffer[k] = -g_large_buffer[k];
            }
            k += (g_shared_counter % 3) + 1; /* Non-constant stride */
        }
    }
}

__attribute__((noinline, noipa, used))
void tm_loop_transform3(int limit) {
    /* Complex control flow with transaction cancellation */
    int attempts = 0;
    
    while (attempts < 3) {
        __transaction_atomic {
            /* Loop with pointer arithmetic */
            int *ptr = g_data_array;
            for (int i = 0; i < limit; i++) {
                *(ptr + i) += i * i;
                
                /* Conditional transaction cancel */
                if (g_data_array[i] > 1000000) {
                    __transaction_cancel;
                }
            }
            
            /* Another loop in same transaction */
            volatile int *vptr = &g_shared_counter;
            for (int j = 0; j < limit / 2; j++) {
                *vptr = *vptr + g_data_array[j];
            }
        }
        attempts++;
    }
}

__attribute__((noinline, noipa, used))
void tm_loop_transform4(int seed) {
    /* Mixed transaction types with complex loop */
    if (seed % 2 == 0) {
        __transaction_atomic {
            int sum = 0;
            for (int i = 0; i < 256; i++) {
                sum += g_data_array[i];
                g_data_array[i] = sum;
            }
            g_shared_counter = sum;
        }
    } else {
        __transaction_relaxed {
            for (int i = 255; i >= 0; i--) {
                g_data_array[i] = g_data_array[i] / (seed + 1);
            }
        }
    }
}

__attribute__((noinline, noipa, used))
void tm_nested_transactions(int depth) {
    /* Nested transactional regions */
    if (depth <= 0) return;
    
    __transaction_atomic {
        for (int i = 0; i < 100; i++) {
            g_data_array[i % 1024] += depth;
        }
        
        if (depth > 1) {
            tm_nested_transactions(depth - 1);
        }
        
        /* Inner relaxed transaction */
        __transaction_relaxed {
            int temp = 0;
            for (int j = 0; j < 50; j++) {
                temp += g_large_buffer[j];
            }
            g_shared_counter += temp;
        }
    }
}

/* Main test driver */
int main(int argc, char **argv) {
    int loop_limit = 512;
    int rows = 32;
    int cols = 16;
    
    /* Initialize shared data */
    for (int i = 0; i < 1024; i++) {
        g_data_array[i] = i;
    }
    
    for (int i = 0; i < 2048; i++) {
        g_large_buffer[i] = i % 256;
    }
    
    g_volatile_ptr = &g_shared_counter;
    
    /* Execute various TM loop transformations */
    tm_loop_transform1(0, loop_limit);
    tm_loop_transform2(rows, cols);
    tm_loop_transform3(loop_limit / 2);
    tm_loop_transform4(argc > 1 ? atoi(argv[1]) : 42);
    tm_nested_transactions(3);
    
    /* Compute checksum to prevent optimization */
    long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += g_data_array[i];
    }
    for (int i = 0; i < 2048; i++) {
        checksum += g_large_buffer[i];
    }
    checksum += g_shared_counter;
    
    printf("TM Loop Test Checksum: %ld\n", checksum);
    return (int)(checksum % 256);
}
