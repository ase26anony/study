/* tm_loop_test.c - Test program for GCC transactional memory loop transformations */

#include <stdio.h>
#include <stdlib.h>

/* Shared global variables for TM operations */
volatile int g_shared_counter = 0;
int g_data_array[1024];
long g_large_buffer[2048];
volatile int* g_volatile_ptr = NULL;

/* Prevent optimization of TM functions */
#define TM_NOOPT __attribute__((noinline, noipa, used))

/* Test function 1: Simple array processing in atomic transaction */
TM_NOOPT
void tm_loop_transform1(int start, int end) {
    /* Transaction with array load/store loop */
    __transaction_atomic {
        for (int i = start; i < end; i++) {
            g_data_array[i] = g_data_array[i] * 2 + 1;
            g_shared_counter++;
        }
    }
}

/* Test function 2: Nested loops with multi-dimensional access pattern */
TM_NOOPT
void tm_loop_transform2(int rows, int cols) {
    volatile int local_seed = rows * cols;
    
    __transaction_atomic {
        /* Nested loop accessing array with stride */
        for (int i = 0; i < rows; i++) {
            int base = i * cols;
            for (int j = 0; j < cols; j++) {
                int idx = base + j;
                g_data_array[idx] ^= local_seed;
                local_seed = (local_seed * 1103515245 + 12345) & 0x7fffffff;
            }
        }
    }
}

/* Test function 3: While loop with pointer arithmetic */
TM_NOOPT
void tm_loop_transform3(int* buffer, int size) {
    if (!buffer || size <= 0) return;
    
    __transaction_relaxed {
        int* ptr = buffer;
        int* end = buffer + size;
        volatile int temp = 0;
        
        while (ptr < end) {
            *ptr = *ptr + temp;
            temp = *ptr;
            ptr++;
            g_shared_counter--;
        }
    }
}

/* Test function 4: Complex control flow with transaction cancellation */
TM_NOOPT
void tm_loop_transform4(int threshold) {
    int retries = 3;
    
    while (retries-- > 0) {
        __transaction_atomic {
            /* Loop with conditional transaction cancel */
            for (int i = 0; i < 256; i++) {
                g_large_buffer[i] = g_large_buffer[i] * 3 - 1;
                
                if (g_large_buffer[i] > threshold && retries == 1) {
                    __transaction_cancel;
                }
            }
            
            /* Another loop in same transaction */
            int j = 512;
            while (j < 768) {
                g_large_buffer[j] = ~g_large_buffer[j];
                j += (g_shared_counter & 3) + 1;  /* Non-constant stride */
            }
        }
        
        if (retries == 0) break;
    }
}

/* Test function 5: Mixed relaxed and atomic transactions */
TM_NOOPT
void tm_loop_transform5(int iterations) {
    volatile int mod = iterations % 7;
    
    /* Outer relaxed transaction */
    __transaction_relaxed {
        for (int i = 0; i < iterations; i++) {
            g_data_array[i % 1024] += i;
            
            /* Inner atomic transaction */
            if (i % 13 == 0) {
                __transaction_atomic {
                    for (int k = 0; k < mod + 1; k++) {
                        g_large_buffer[k] = g_data_array[k] | g_large_buffer[k];
                    }
                }
            }
        }
    }
}

/* Test function 6: Loop with function calls (potential for TM transformation) */
TM_NOOPT
static int helper_transform(int x, int y) {
    return (x ^ y) + (x & y);
}

TM_NOOPT
void tm_loop_transform6(int limit) {
    __transaction_atomic {
        int acc = 0;
        for (int i = 0; i < limit; i++) {
            /* Loop with function call that might be inlined/transformed */
            g_data_array[i] = helper_transform(g_data_array[i], acc);
            acc = g_data_array[i] % 17;
            
            /* Volatile access to force memory ops */
            g_volatile_ptr = &g_data_array[i];
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
    
    g_shared_counter = 42;
    g_volatile_ptr = &g_data_array[0];
}

/* Compute checksum to verify execution and prevent dead code elimination */
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

int main(int argc, char** argv) {
    /* Use arguments to create non-constant loop bounds */
    int base_iter = (argc > 1) ? atoi(argv[1]) : 100;
    if (base_iter <= 0) base_iter = 100;
    
    init_test_data();
    
    /* Execute all TM test functions with varying parameters */
    tm_loop_transform1(base_iter % 512, base_iter % 512 + 128);
    tm_loop_transform2(16, 32);
    tm_loop_transform3(g_data_array, base_iter % 256 + 64);
    tm_loop_transform4(base_iter * 1000);
    tm_loop_transform5(base_iter % 200 + 50);
    tm_loop_transform6(base_iter % 400 + 100);
    
    /* Additional calls with different parameters */
    for (int repeat = 0; repeat < 3; repeat++) {
        tm_loop_transform1(repeat * 64, repeat * 64 + 32);
        tm_loop_transform5((repeat + 1) * 20);
    }
    
    long final_checksum = compute_checksum();
    printf("TM Loop Test Checksum: %ld\n", final_checksum);
    
    return (int)(final_checksum % 256);
}
