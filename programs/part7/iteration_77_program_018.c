/* tm_loop_coverage.c
 * Designed to trigger default_builtin_tm_load_store_loop_transform
 * in targhooks.cc lines 981-990
 */

#include <stdio.h>
#include <stdlib.h>

/* Shared global variables for TM operations */
volatile int g_shared_array[1024];
volatile long g_shared_long[512];
int g_shared_data[2048];
volatile int g_counter = 0;
volatile int g_bound = 100;

/* Prevent optimization of TM functions */
#define TM_NOOPT __attribute__((noinline, noipa, used))

/* Test function 1: Simple array processing in atomic transaction */
TM_NOOPT
void tm_loop_transform1(int start, int end) {
    __transaction_atomic {
        for (int i = start; i < end; i++) {
            /* Load-store pattern with arithmetic */
            int val = g_shared_array[i % 1024];
            g_shared_array[(i + 1) % 1024] = val * 2 + 1;
            g_shared_data[i % 2048] += val;
        }
    }
}

/* Test function 2: Nested loops with relaxed transaction */
TM_NOOPT
void tm_loop_transform2(int rows, int cols) {
    __transaction_relaxed {
        int idx = 0;
        while (idx < rows * cols) {
            /* Multi-dimensional access pattern */
            int row = idx / cols;
            int col = idx % cols;
            long old_val = g_shared_long[(row * 32 + col) % 512];
            g_shared_long[(row * 32 + (col + 1) % cols) % 512] = old_val ^ 0xABCD;
            idx++;
        }
    }
}

/* Test function 3: Complex control flow with TM cancellation */
TM_NOOPT
void tm_loop_transform3(int limit) {
    volatile int attempts = 0;
    
    while (attempts < 3) {
        __transaction_atomic {
            attempts++;
            
            /* Conditional transaction with loop */
            if (g_counter < limit) {
                for (int i = g_counter; i < g_counter + 10; i++) {
                    int idx = i % 1024;
                    int load_val = g_shared_array[idx];
                    g_shared_array[(idx + 5) % 1024] = load_val + g_shared_data[idx % 2048];
                    g_shared_data[idx % 2048] = load_val >> 1;
                }
                g_counter += 5;
            } else {
                /* Cancel transaction on condition */
                __transaction_cancel;
            }
            
            /* Another inner loop */
            int j = 0;
            do {
                g_shared_long[j % 512] = (g_shared_long[j % 512] * 3) - 1;
                j++;
            } while (j < g_bound);
        }
    }
}

/* Test function 4: Pointer-based access in transaction */
TM_NOOPT
void tm_loop_transform4(int* data, int size) {
    __transaction_relaxed {
        int* ptr = data;
        int* end = data + size;
        
        while (ptr < end) {
            /* Load-modify-store with pointer arithmetic */
            int val = *ptr;
            *(ptr + 1) = val ^ 0xFF;
            ptr += 2;
        }
        
        /* Reverse loop */
        for (int i = size - 1; i >= 0; i -= 2) {
            data[i] = data[i] + data[i - 1];
        }
    }
}

/* Test function 5: Mixed transaction types */
TM_NOOPT
void tm_loop_transform5(void) {
    /* Outer atomic transaction */
    __transaction_atomic {
        /* Process first half */
        for (int i = 0; i < 512; i++) {
            g_shared_array[i] = i * 3;
        }
        
        /* Inner relaxed transaction */
        __transaction_relaxed {
            int j = 511;
            while (j >= 0) {
                g_shared_long[j] = g_shared_array[j] + g_shared_long[j];
                j--;
            }
        }
        
        /* More processing */
        for (int k = 0; k < 256; k += 2) {
            g_shared_data[k] = g_shared_array[k] | g_shared_long[k / 2];
        }
    }
}

/* Initialize shared data */
void init_shared_data(void) {
    for (int i = 0; i < 1024; i++) {
        g_shared_array[i] = i % 256;
    }
    for (int i = 0; i < 512; i++) {
        g_shared_long[i] = i * 2;
    }
    for (int i = 0; i < 2048; i++) {
        g_shared_data[i] = 0;
    }
}

/* Compute checksum to verify execution */
int compute_checksum(void) {
    int sum = 0;
    for (int i = 0; i < 1024; i++) {
        sum += g_shared_array[i];
    }
    for (int i = 0; i < 512; i++) {
        sum += (int)(g_shared_long[i] % 1000);
    }
    for (int i = 0; i < 2048; i++) {
        sum += g_shared_data[i];
    }
    return sum + g_counter;
}

int main(void) {
    /* Initialize test data */
    init_shared_data();
    
    /* Dynamic bounds to prevent compile-time optimization */
    volatile int bound1 = 500;
    volatile int bound2 = 50;
    volatile int bound3 = 75;
    
    /* Local array for pointer test */
    int local_array[200];
    for (int i = 0; i < 200; i++) {
        local_array[i] = i * 7;
    }
    
    /* Execute TM functions with various patterns */
    tm_loop_transform1(0, bound1);
    tm_loop_transform2(bound2, 10);
    tm_loop_transform3(bound3);
    tm_loop_transform4(local_array, 200);
    tm_loop_transform5();
    
    /* Additional calls with different parameters */
    for (int iter = 0; iter < 3; iter++) {
        tm_loop_transform1(iter * 100, iter * 100 + 50);
        tm_loop_transform2(5 + iter, 8 + iter);
        g_bound = 30 + iter * 10;
        tm_loop_transform3(40 + iter * 5);
    }
    
    /* Verify and output results */
    int checksum = compute_checksum();
    printf("TM Loop Transformation Test Complete\n");
    printf("Checksum: %d\n", checksum);
    printf("Final counter: %d\n", g_counter);
    
    return 0;
}
