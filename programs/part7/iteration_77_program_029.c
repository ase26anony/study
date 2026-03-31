/* tm_loop_coverage.c
 * Designed to trigger default_builtin_tm_load_store_loop_transform in targhooks.cc
 * Compile with: gcc -O1 -fgnu-tm -o tm_test tm_loop_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Shared global variables for TM operations */
volatile int g_tm_counter = 0;
int g_shared_array[256];
long g_shared_data[128];
volatile int g_loop_bound = 64;

/* Prevent optimization of critical functions */
__attribute__((noinline, noipa, used))
void tm_loop_transform1(int start, int end) {
    /* Transaction with array processing loop */
    __transaction_atomic {
        for (int i = start; i < end; i++) {
            /* Load-store pattern that may trigger transformation */
            int val = g_shared_array[i];
            g_shared_array[i] = val * 2 + 1;
            g_tm_counter++;
        }
    }
}

__attribute__((noinline, noipa, used))
void tm_loop_transform2(int *data, int size) {
    /* Nested loops with pointer arithmetic */
    __transaction_relaxed {
        int *ptr = data;
        int i = 0;
        while (i < size) {
            /* Complex load-store pattern */
            long temp = g_shared_data[i % 128];
            *ptr = (int)(temp & 0xFFFFFFFF);
            ptr++;
            g_shared_data[i % 128] = temp >> 32;
            i++;
        }
        
        /* Additional loop with volatile bound */
        volatile int limit = g_loop_bound;
        for (int j = 0; j < limit; j += 2) {
            g_shared_array[j] ^= g_shared_array[j + 1];
            g_shared_array[j + 1] ^= g_shared_array[j];
            g_shared_array[j] ^= g_shared_array[j + 1];
        }
    }
}

__attribute__((noinline, noipa, used))
void tm_loop_transform3(int rows, int cols) {
    /* Multi-dimensional array access with conditional TM */
    if (rows > 0 && cols > 0) {
        __transaction_atomic {
            /* Nested loops for 2D transformation */
            for (int i = 0; i < rows; i++) {
                int base = i * 16;
                for (int j = 0; j < cols; j++) {
                    /* Load-modify-store pattern */
                    int idx = (base + j) % 256;
                    int old = g_shared_array[idx];
                    g_shared_array[idx] = old + i - j + g_tm_counter;
                    
                    /* Potential cancellation point */
                    if (g_shared_array[idx] < 0) {
                        __transaction_cancel;
                    }
                }
            }
        }
    }
}

__attribute__((noinline, noipa, used))
void tm_loop_transform4(void) {
    /* Mixed transaction types with complex control flow */
    int retry_count = 3;
    
    while (retry_count-- > 0) {
        __transaction_relaxed {
            /* Loop with volatile variable in condition */
            volatile int n = g_loop_bound;
            int i = n - 1;
            
            do {
                /* Memory transformation with dependency chain */
                g_shared_data[i % 128] = 
                    g_shared_data[(i + 1) % 128] * 3 +
                    g_shared_data[(i + 2) % 128] / 2;
                
                /* Interleaved array access */
                if (i < 256) {
                    g_shared_array[i] = (int)g_shared_data[i % 128];
                }
                
                i--;
            } while (i >= 0);
            
            /* Early commit on condition */
            if (g_tm_counter > 1000) {
                break;
            }
        }
        
        /* Small non-transactional section between retries */
        g_loop_bound = (g_loop_bound + 7) % 128 + 1;
    }
}

__attribute__((noinline, noipa, used))
void tm_loop_transform5(int *src, int *dst, int len) {
    /* Transaction with pointer-based loop and function calls */
    __transaction_atomic {
        /* Copy with transformation */
        for (int i = 0; i < len; i++) {
            dst[i] = src[i] + g_tm_counter;
            
            /* Store to shared global with stride */
            if (i % 4 == 0) {
                g_shared_array[i / 4] = dst[i];
            }
        }
        
        /* Second loop with different access pattern */
        int j = len - 1;
        while (j >= 0) {
            g_shared_data[j % 128] += dst[j];
            j -= 2;
        }
    }
}

/* Initialize test data */
void init_test_data(void) {
    for (int i = 0; i < 256; i++) {
        g_shared_array[i] = i * 3 + 7;
    }
    
    for (int i = 0; i < 128; i++) {
        g_shared_data[i] = i * 5 - 13;
    }
    
    g_tm_counter = 42;
    g_loop_bound = 64;
}

/* Compute checksum to verify execution */
long compute_checksum(void) {
    long sum = 0;
    
    for (int i = 0; i < 256; i++) {
        sum += g_shared_array[i];
    }
    
    for (int i = 0; i < 128; i++) {
        sum += g_shared_data[i];
    }
    
    sum += g_tm_counter;
    sum += g_loop_bound;
    
    return sum;
}

int main(void) {
    /* Local arrays for pointer-based tests */
    int src_array[100];
    int dst_array[100];
    
    /* Initialize all test data */
    init_test_data();
    
    for (int i = 0; i < 100; i++) {
        src_array[i] = i * 11 - 23;
        dst_array[i] = 0;
    }
    
    /* Execute TM functions with varied parameters */
    printf("Starting TM loop transformation tests...\n");
    
    /* Test 1: Basic array transformation */
    tm_loop_transform1(0, 128);
    
    /* Test 2: Pointer-based loops with mixed access */
    tm_loop_transform2(&g_shared_array[64], 32);
    
    /* Test 3: 2D access pattern with potential cancel */
    tm_loop_transform3(8, 8);
    
    /* Test 4: Retry loop with volatile bounds */
    tm_loop_transform4();
    
    /* Test 5: Pointer arrays with stride access */
    tm_loop_transform5(src_array, dst_array, 100);
    
    /* Additional mixed calls to increase coverage */
    for (int iter = 0; iter < 5; iter++) {
        tm_loop_transform1(iter * 16, iter * 16 + 16);
        g_loop_bound = (g_loop_bound * 13 + 17) % 96 + 16;
    }
    
    /* Compute and print checksum */
    long final_checksum = compute_checksum();
    printf("Final checksum: %ld\n", final_checksum);
    printf("TM counter: %d\n", g_tm_counter);
    printf("Loop bound: %d\n", g_loop_bound);
    
    return 0;
}
