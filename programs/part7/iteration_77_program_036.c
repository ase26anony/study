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
void tm_loop_transform1(int start, int end, int increment) {
    __transaction_atomic {
        /* Loop with array access - candidate for load/store transformation */
        for (int i = start; i < end; i += increment) {
            g_data_array[i] = g_data_array[i] * 2 + 1;
            g_shared_counter += g_data_array[i] & 0xF;
        }
        
        /* Nested loop with pointer arithmetic */
        int *ptr = g_data_array + (end % 256);
        while (ptr < g_data_array + 512) {
            *ptr = (*ptr ^ 0xAA) + g_shared_counter;
            ptr += increment;
        }
    }
}

__attribute__((noinline, noipa, used))
void tm_loop_transform2(long iterations, int mod_value) {
    volatile int local_seed = iterations;
    
    __transaction_relaxed {
        /* Multi-dimensional access pattern */
        for (long i = 0; i < iterations; i++) {
            int idx = (i * 7 + local_seed) % 2048;
            g_large_buffer[idx] = (g_large_buffer[idx] << 3) | (i & 0x7);
            
            /* Inner loop with conditional */
            for (int j = 0; j < (i % 8); j++) {
                g_data_array[j * 64 + (idx % 64)] += g_large_buffer[idx] % 256;
            }
        }
        
        /* Transaction with potential cancellation */
        if (g_shared_counter > 1000) {
            __transaction_cancel;
        }
    }
}

__attribute__((noinline, noipa, used))
void tm_loop_transform3(int *external_data, int size) {
    if (size <= 0) return;
    
    __transaction_atomic {
        /* Complex loop with volatile pointer */
        g_volatile_ptr = external_data;
        int sum = 0;
        
        for (int i = 0; i < size; i++) {
            volatile int *elem = g_volatile_ptr + i;
            *elem = (*elem * 3) / 2;
            sum += *elem;
            
            /* Conditional transaction inside loop */
            if (sum & 0x100) {
                __transaction_atomic {
                    g_data_array[i % 1024] = sum;
                }
            }
        }
        
        /* Store result with relaxed transaction */
        __transaction_relaxed {
            g_large_buffer[0] = sum;
        }
    }
}

__attribute__((noinline, noipa, used))
void tm_nested_transforms(int depth, int width) {
    if (depth <= 0) return;
    
    __transaction_atomic {
        /* Recursive pattern with loops */
        for (int d = 0; d < depth; d++) {
            int base = d * width;
            
            __transaction_relaxed {
                for (int w = 0; w < width; w++) {
                    int idx = (base + w) % 1024;
                    g_data_array[idx] = (g_data_array[idx] + g_shared_counter) ^ idx;
                }
            }
            
            /* Alternate between atomic and relaxed */
            if (d % 2 == 0) {
                __transaction_atomic {
                    g_shared_counter += d;
                }
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
        g_large_buffer[i] = i ^ 0x5A5A;
    }
    
    g_shared_counter = 42;
}

/* Compute checksum to verify execution */
long compute_checksum(void) {
    long checksum = g_shared_counter;
    
    for (int i = 0; i < 1024; i += 64) {
        checksum = (checksum * 31 + g_data_array[i]) & 0xFFFFFF;
    }
    
    for (int i = 0; i < 2048; i += 128) {
        checksum ^= g_large_buffer[i];
    }
    
    return checksum;
}

int main(int argc, char **argv) {
    /* Initialize test data */
    init_test_data();
    
    /* Dynamic loop bounds to prevent compile-time optimization */
    int loop_bound1 = argc > 1 ? atoi(argv[1]) : 100;
    int loop_bound2 = argc > 2 ? atoi(argv[2]) : 50;
    int loop_bound3 = argc > 3 ? atoi(argv[3]) : 200;
    
    if (loop_bound1 <= 0) loop_bound1 = 100;
    if (loop_bound2 <= 0) loop_bound2 = 50;
    if (loop_bound3 <= 0) loop_bound3 = 200;
    
    /* Execute TM functions with various loop patterns */
    tm_loop_transform1(0, loop_bound1, 1 + (loop_bound1 % 5));
    tm_loop_transform1(loop_bound1 / 2, loop_bound1, 2);
    
    tm_loop_transform2(loop_bound2 * 3L, 17 + (loop_bound2 % 13));
    
    int dynamic_array[500];
    for (int i = 0; i < 500; i++) {
        dynamic_array[i] = i * i - i;
    }
    tm_loop_transform3(dynamic_array, loop_bound3 % 500);
    
    tm_nested_transforms(3 + (loop_bound1 % 4), 25 + (loop_bound2 % 20));
    
    /* Additional relaxed transaction with loop */
    __transaction_relaxed {
        for (volatile int i = 0; i < (loop_bound1 % 20); i++) {
            g_shared_counter = (g_shared_counter * 1103515245 + 12345) & 0x7FFFFFFF;
        }
    }
    
    /* Compute and print checksum */
    long final_checksum = compute_checksum();
    printf("TM Loop Test Checksum: %ld\n", final_checksum);
    
    return (int)(final_checksum % 256);
}
