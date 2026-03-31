/* tm_loop_test.c - Test program for GCC TM load/store loop transformation hooks */

#include <stdio.h>
#include <stdlib.h>

/* Shared global variables for TM operations */
volatile int g_tm_counter = 0;
int g_shared_array[1024];
long g_large_buffer[2048];
volatile int *g_volatile_ptr = NULL;

/* Prevent optimization and inlining */
__attribute__((noinline, noipa))
void tm_loop_transform1(int start, int end, int increment) {
    __transaction_atomic {
        /* Loop with array accesses - candidate for load/store transformation */
        for (int i = start; i < end; i += increment) {
            g_shared_array[i] = g_shared_array[i] * 2 + 1;
            g_tm_counter++;
        }
        
        /* Nested loop with pointer arithmetic */
        int *ptr = g_shared_array + start;
        for (int j = 0; j < (end - start); j++) {
            *ptr = (*ptr) ^ 0xAAAA;
            ptr += increment;
        }
    }
}

__attribute__((noinline, noipa))
void tm_loop_transform2(long *data, int size, int iterations) {
    volatile int local_seed = size;
    
    for (int iter = 0; iter < iterations; iter++) {
        __transaction_relaxed {
            /* Multi-dimensional access pattern */
            for (int i = 0; i < size; i++) {
                for (int j = 0; j < 4; j++) {
                    int idx = i * 4 + j;
                    if (idx < 2048) {
                        g_large_buffer[idx] = data[i] + (local_seed * j);
                    }
                }
            }
            
            /* Conditional transaction cancel */
            if (local_seed > 1000) {
                __transaction_cancel;
            }
        }
        
        /* Modify seed to vary loop behavior */
        local_seed = (local_seed * 1103515245 + 12345) & 0x7fffffff;
    }
}

__attribute__((noinline, noipa))
int tm_loop_transform3(int threshold, volatile int *control) {
    int result = 0;
    
    /* Transaction with complex control flow */
    if (*control > threshold) {
        __transaction_atomic {
            /* While loop with volatile condition */
            int k = 0;
            while (k < 1024 && *control > threshold) {
                g_shared_array[k] = g_shared_array[1023 - k] + k;
                g_large_buffer[k % 2048] = result;
                result += g_shared_array[k];
                k += (*control % 16) + 1;  /* Non-constant stride */
            }
            
            /* Early exit under certain conditions */
            if (result > 0xFFFFFF) {
                __transaction_cancel;
                return -1;
            }
        }
    } else {
        __transaction_relaxed {
            /* Different loop pattern */
            for (int i = threshold; i > 0; i--) {
                g_shared_array[i % 1024] ^= g_shared_array[(i * 7) % 1024];
                result++;
            }
        }
    }
    
    return result;
}

__attribute__((noinline, noipa))
void tm_nested_transactions(int depth, int base) {
    if (depth <= 0) return;
    
    __transaction_atomic {
        /* Loop with function call that might also have TM */
        for (int i = 0; i < base; i++) {
            g_shared_array[i] += depth;
            
            /* Nested transaction attempt */
            if (depth > 1 && (i % 3) == 0) {
                __transaction_relaxed {
                    for (int j = 0; j < 2; j++) {
                        g_large_buffer[i * 2 + j] = g_shared_array[i] * j;
                    }
                }
            }
        }
        
        /* Recursive call with reduced depth */
        tm_nested_transactions(depth - 1, base / 2);
    }
}

/* Main test driver */
int main(int argc, char **argv) {
    /* Initialize shared data */
    for (int i = 0; i < 1024; i++) {
        g_shared_array[i] = i * 3;
    }
    
    for (int i = 0; i < 2048; i++) {
        g_large_buffer[i] = i * 5L;
    }
    
    volatile int control_var = argc > 1 ? atoi(argv[1]) : 42;
    g_volatile_ptr = &control_var;
    
    printf("Starting TM loop transformation tests...\n");
    
    /* Test 1: Basic array transformation */
    tm_loop_transform1(0, 512, 2);
    printf("Test 1 complete: counter=%d, array[100]=%d\n", 
           g_tm_counter, g_shared_array[100]);
    
    /* Test 2: Multi-dimensional pattern */
    long test_data[256];
    for (int i = 0; i < 256; i++) test_data[i] = i * 7L;
    tm_loop_transform2(test_data, 256, 3);
    printf("Test 2 complete: buffer[500]=%ld\n", g_large_buffer[500]);
    
    /* Test 3: Conditional TM with volatile */
    int res3 = tm_loop_transform3(50, &control_var);
    printf("Test 3 complete: result=%d, counter=%d\n", res3, g_tm_counter);
    
    /* Test 4: Nested transactions */
    tm_nested_transactions(3, 64);
    printf("Test 4 complete: array[32]=%d, buffer[128]=%ld\n",
           g_shared_array[32], g_large_buffer[128]);
    
    /* Final checksum to verify all operations */
    long long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += g_shared_array[i];
    }
    for (int i = 0; i < 2048; i++) {
        checksum += g_large_buffer[i];
    }
    checksum += g_tm_counter;
    
    printf("Final checksum: %lld\n", checksum);
    printf("TM loop transformation tests completed.\n");
    
    return 0;
}
