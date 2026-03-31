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
volatile int *g_volatile_ptr;

/* Prevent optimization and inlining */
#define NOOPT __attribute__((noinline, noipa, used))

/* Test function 1: Simple array processing in transactional region */
NOOPT void tm_loop_transform1(int start, int end) {
    __transaction_atomic {
        /* Loop with non-constant bounds for load/store transformation */
        for (int i = start; i < end; ++i) {
            g_data_array[i] = g_data_array[i] * 2 + 1;
            g_shared_counter += g_data_array[i];
        }
        
        /* Additional loop with pointer arithmetic */
        int *ptr = g_data_array + start;
        for (int j = 0; j < (end - start); ++j) {
            *ptr = (*ptr ^ 0x55AA) + j;
            ptr++;
        }
    }
}

/* Test function 2: Nested loops with multi-dimensional access pattern */
NOOPT void tm_loop_transform2(int rows, int cols) {
    /* Create 2D access pattern using 1D array */
    __transaction_atomic {
        for (int i = 0; i < rows; ++i) {
            int base = i * cols;
            for (int j = 0; j < cols; ++j) {
                int idx = base + j;
                if (idx < 1024) {
                    g_data_array[idx] = (g_data_array[idx] + i) * (j + 1);
                }
            }
        }
    }
    
    /* Second transaction with relaxed semantics */
    __transaction_relaxed {
        int k = 0;
        while (k < rows * cols && k < 1024) {
            g_data_array[k] ^= g_large_buffer[k % 2048];
            k += (rows % 7) + 1;  /* Non-uniform stride */
        }
    }
}

/* Test function 3: Complex control flow with transaction cancellation */
NOOPT void tm_loop_transform3(int threshold) {
    volatile int attempt_count = 0;
    
    /* Transaction with potential cancellation */
    __transaction_atomic {
        attempt_count++;
        
        /* Loop with conditional store */
        for (int i = 0; i < 256; ++i) {
            int val = g_data_array[i] + g_shared_counter;
            g_large_buffer[i] = val;
            
            /* Condition that might cause transaction abort */
            if (val > threshold && attempt_count < 3) {
                __transaction_cancel;
            }
        }
        
        /* Another loop after potential cancellation point */
        int sum = 0;
        for (int j = 100; j < 200; ++j) {
            sum += g_large_buffer[j];
            g_data_array[j] = sum % 1000;
        }
    }
}

/* Test function 4: Mixed transaction types and pointer chasing */
NOOPT void tm_loop_transform4(int iterations) {
    int local_buffer[100];
    
    /* Initialize local buffer */
    for (int i = 0; i < 100; ++i) {
        local_buffer[i] = i * 3;
    }
    
    /* Atomic transaction with pointer-based loop */
    __transaction_atomic {
        int *src = local_buffer;
        int *dst = g_data_array;
        
        for (int i = 0; i < iterations && i < 100; ++i) {
            *dst++ = *src++ + g_shared_counter;
            g_shared_counter += (i % 5);
        }
    }
    
    /* Relaxed transaction with while loop */
    __transaction_relaxed {
        int count = 0;
        volatile int *vp = &g_shared_counter;
        
        while (count < iterations && count < 50) {
            g_large_buffer[count] = *vp + count;
            *vp = (*vp + 1) & 0xFF;
            count++;
        }
    }
}

/* Test function 5: Recursive-like pattern using multiple transactions */
NOOPT void tm_loop_transform5(int depth, int idx) {
    if (depth <= 0 || idx >= 1024) return;
    
    __transaction_atomic {
        /* Transform a segment of the array */
        int segment_size = 32;
        for (int i = idx; i < idx + segment_size && i < 1024; ++i) {
            g_data_array[i] = (g_data_array[i] << 1) | (g_data_array[i] >> 31);
        }
        
        g_shared_counter += segment_size;
    }
    
    /* Recursive call in relaxed transaction */
    __transaction_relaxed {
        tm_loop_transform5(depth - 1, idx + 64);
    }
}

/* Main function that drives all tests */
int main(int argc, char **argv) {
    /* Initialize shared data */
    for (int i = 0; i < 1024; ++i) {
        g_data_array[i] = i;
    }
    
    for (int i = 0; i < 2048; ++i) {
        g_large_buffer[i] = i * 2;
    }
    
    g_volatile_ptr = &g_shared_counter;
    
    /* Call test functions with varying parameters to create
     * different loop patterns for TM transformation */
    
    /* Function 1: Simple array loops */
    tm_loop_transform1(0, 512);
    tm_loop_transform1(100, 600);
    
    /* Function 2: Nested loops */
    tm_loop_transform2(16, 32);
    tm_loop_transform2(8, 64);
    
    /* Function 3: Transaction with cancellation */
    tm_loop_transform3(1000000);
    tm_loop_transform3(500000);
    
    /* Function 4: Mixed transactions */
    tm_loop_transform4(75);
    tm_loop_transform4(120);
    
    /* Function 5: Recursive pattern */
    tm_loop_transform5(3, 0);
    tm_loop_transform5(5, 128);
    
    /* Calculate checksum to verify execution and prevent optimization */
    long long checksum = 0;
    for (int i = 0; i < 1024; ++i) {
        checksum += g_data_array[i];
    }
    
    for (int i = 0; i < 2048; ++i) {
        checksum += g_large_buffer[i];
    }
    
    checksum += g_shared_counter;
    
    printf("TM Loop Transformation Test Complete\n");
    printf("Checksum: %lld\n", checksum);
    printf("Shared counter: %d\n", g_shared_counter);
    
    return 0;
}
