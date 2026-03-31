/* tm_loop_test.c - Test program for GCC transactional memory load/store loop transformation */
#include <stdio.h>
#include <stdlib.h>

/* Shared global variables for TM operations */
volatile int g_shared_array[1024];
volatile long g_shared_long[512];
int* g_shared_ptr;
volatile int g_loop_bound = 100;
volatile int g_nested_bound = 50;

/* Prevent optimization and inlining */
__attribute__((noinline, noipa, used))
void tm_loop_transform1(int start, int end) {
    /* Transaction with array processing loop */
    __transaction_atomic {
        for (volatile int i = start; i < end; i++) {
            g_shared_array[i] = g_shared_array[i] * 2 + 1;
            g_shared_long[i % 512] += g_shared_array[i];
        }
    }
    
    /* Additional relaxed transaction */
    __transaction_relaxed {
        int j = 0;
        while (j < g_loop_bound) {
            g_shared_array[j] ^= 0xAAAAAAAA;
            j++;
        }
    }
}

__attribute__((noinline, noipa, used))
void tm_loop_transform2(int* data, int size) {
    volatile int local_bound = size;
    
    /* Transaction with pointer-based loop */
    __transaction_atomic {
        for (int i = 0; i < local_bound; i++) {
            data[i] = data[i] + g_shared_array[i % 1024];
            if (data[i] > 1000) {
                data[i] = 0;
            }
        }
    }
    
    /* Nested loops in transaction */
    __transaction_relaxed {
        for (int i = 0; i < g_nested_bound; i++) {
            for (int j = 0; j < 10; j++) {
                g_shared_array[i * 10 + j] += i * j;
            }
        }
    }
}

__attribute__((noinline, noipa, used))
void tm_loop_transform3(int threshold) {
    /* Transaction with conditional control flow */
    __transaction_atomic {
        int sum = 0;
        for (int i = 0; i < g_loop_bound; i++) {
            if (g_shared_array[i] > threshold) {
                g_shared_array[i] = threshold;
                sum++;
            }
        }
        
        /* Transaction retry logic */
        if (sum > 50) {
            __transaction_cancel;
        }
    }
    
    /* Another transaction after potential cancel */
    __transaction_atomic {
        for (int i = g_loop_bound - 1; i >= 0; i--) {
            g_shared_long[i % 512] -= g_shared_array[i];
        }
    }
}

__attribute__((noinline, noipa, used))
void tm_loop_transform4(void) {
    /* Complex loop with multiple arrays */
    volatile int counter = 0;
    
    __transaction_atomic {
        do {
            g_shared_array[counter] = (g_shared_array[counter] << 1) | 1;
            g_shared_long[counter % 512] ^= g_shared_array[counter];
            counter++;
        } while (counter < g_nested_bound);
    }
    
    /* Mixed transaction types */
    __transaction_relaxed {
        for (volatile int i = 0; i < 75; i++) {
            for (int j = 0; j < 5; j++) {
                g_shared_array[i * 5 + j] %= 256;
            }
        }
    }
}

__attribute__((noinline, noipa, used))
void init_shared_data(void) {
    for (int i = 0; i < 1024; i++) {
        g_shared_array[i] = i % 256;
    }
    for (int i = 0; i < 512; i++) {
        g_shared_long[i] = i * 2;
    }
    
    /* Allocate and initialize pointer-based data */
    g_shared_ptr = (int*)malloc(256 * sizeof(int));
    for (int i = 0; i < 256; i++) {
        g_shared_ptr[i] = i * 3;
    }
}

__attribute__((noinline, noipa, used))
long compute_checksum(void) {
    long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += g_shared_array[i];
    }
    for (int i = 0; i < 512; i++) {
        checksum += g_shared_long[i];
    }
    if (g_shared_ptr) {
        for (int i = 0; i < 256; i++) {
            checksum += g_shared_ptr[i];
        }
    }
    return checksum;
}

int main(void) {
    /* Initialize shared data */
    init_shared_data();
    
    /* Call test functions with varying parameters */
    tm_loop_transform1(0, g_loop_bound);
    tm_loop_transform1(50, 150);
    
    tm_loop_transform2(g_shared_ptr, 256);
    
    /* Vary loop bounds dynamically */
    g_loop_bound = 75;
    g_nested_bound = 25;
    
    tm_loop_transform3(128);
    tm_loop_transform3(64);
    
    tm_loop_transform4();
    
    /* More complex scenarios */
    for (int iter = 0; iter < 3; iter++) {
        __transaction_atomic {
            for (int i = iter * 100; i < (iter + 1) * 100; i++) {
                g_shared_array[i] = (g_shared_array[i] + g_shared_long[i % 512]) % 1024;
            }
        }
    }
    
    /* Final checksum computation and output */
    long final_checksum = compute_checksum();
    printf("TM Loop Test Checksum: %ld\n", final_checksum);
    
    /* Cleanup */
    if (g_shared_ptr) {
        free(g_shared_ptr);
    }
    
    return 0;
}
