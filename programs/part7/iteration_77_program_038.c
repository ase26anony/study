/* tm_loop_transform_test.c
 * Designed to trigger default_builtin_tm_load_store_loop_transform
 * in targhooks.cc lines 981-990
 */

#include <stdio.h>
#include <stdlib.h>

/* Shared global variables for TM operations */
volatile int g_shared_counter = 0;
int g_data_array[1024];
long g_large_buffer[2048];
volatile int g_loop_bound = 100;

/* Prevent optimization of critical functions */
#define NOOPT __attribute__((noinline, noipa, used))

/* Test function 1: Simple array processing in atomic transaction */
NOOPT void tm_loop_transform1(int start, int end) {
    __transaction_atomic {
        for (int i = start; i < end; i++) {
            /* Load, modify, store pattern */
            int val = g_data_array[i];
            val = val * 2 + 1;
            g_data_array[i] = val;
            g_shared_counter++;
        }
    }
}

/* Test function 2: Nested loops with multi-dimensional access pattern */
NOOPT void tm_loop_transform2(int rows, int cols) {
    int local_matrix[10][10];
    
    __transaction_atomic {
        /* Initialize local matrix */
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                local_matrix[i][j] = i * cols + j;
            }
        }
        
        /* Process with dependency between iterations */
        for (int i = 1; i < rows - 1; i++) {
            for (int j = 1; j < cols - 1; j++) {
                /* Stencil computation - multiple loads */
                int sum = local_matrix[i-1][j] + local_matrix[i+1][j] +
                         local_matrix[i][j-1] + local_matrix[i][j+1];
                g_data_array[i * cols + j] = sum;
            }
        }
    }
}

/* Test function 3: While loop with volatile bound and pointer arithmetic */
NOOPT void tm_loop_transform3(int *buffer, volatile int size) {
    __transaction_relaxed {
        int *ptr = buffer;
        int count = 0;
        
        while (count < size) {
            /* Load-modify-store with pointer */
            int value = *ptr;
            value = (value << 3) | (value >> 5); /* Simple permutation */
            *ptr = value;
            
            ptr++;
            count++;
            
            /* Conditional store to global */
            if (count % 7 == 0) {
                g_shared_counter = g_shared_counter ^ value;
            }
        }
    }
}

/* Test function 4: Complex control flow with transaction cancellation */
NOOPT void tm_loop_transform4(int threshold) {
    int attempts = 0;
    
transaction_retry:
    __transaction_atomic {
        attempts++;
        
        /* Process array with early exit condition */
        for (int i = 0; i < g_loop_bound; i++) {
            g_large_buffer[i] = g_large_buffer[i] * 3;
            
            /* Simulate condition that might cause cancellation */
            if (g_large_buffer[i] > threshold && attempts < 3) {
                __transaction_cancel;
            }
        }
        
        /* Another loop after potential cancellation point */
        for (int i = g_loop_bound; i < g_loop_bound * 2; i++) {
            g_large_buffer[i] = g_large_buffer[i] + g_shared_counter;
        }
    }
}

/* Test function 5: Mixed relaxed and atomic transactions */
NOOPT void tm_loop_transform5(int iterations) {
    /* Outer relaxed transaction */
    __transaction_relaxed {
        int temp_sum = 0;
        
        for (int i = 0; i < iterations; i++) {
            /* Inner atomic transaction for critical section */
            __transaction_atomic {
                temp_sum += g_data_array[i];
                g_data_array[i] = temp_sum % 256;
            }
        }
        
        /* Final store in relaxed transaction */
        g_shared_counter = temp_sum;
    }
}

/* Test function 6: Loop with function call inside transaction */
NOOPT int helper_compute(int x, int y) {
    return (x * y) ^ (x + y);
}

NOOPT void tm_loop_transform6(int limit) {
    __transaction_atomic {
        for (int i = 0; i < limit; i++) {
            /* Function call inside loop inside transaction */
            int result = helper_compute(g_data_array[i], i);
            g_large_buffer[i] = result;
            
            /* Conditional store with side effect */
            if (result % 2 == 0) {
                g_data_array[i] = result;
            }
        }
    }
}

/* Initialize test data */
void init_test_data(void) {
    for (int i = 0; i < 1024; i++) {
        g_data_array[i] = i % 100;
    }
    
    for (int i = 0; i < 2048; i++) {
        g_large_buffer[i] = i * 2;
    }
    
    g_shared_counter = 42;
    g_loop_bound = 100;
}

/* Compute checksum to verify execution and prevent elimination */
long compute_checksum(void) {
    long checksum = g_shared_counter;
    
    for (int i = 0; i < 256; i++) {
        checksum = (checksum * 31 + g_data_array[i]) % 1000000007;
    }
    
    for (int i = 0; i < 512; i++) {
        checksum = (checksum * 17 + g_large_buffer[i]) % 1000000007;
    }
    
    return checksum;
}

int main(void) {
    /* Initialize test data */
    init_test_data();
    
    /* Execute various TM loop patterns */
    tm_loop_transform1(0, 200);
    tm_loop_transform2(8, 8);
    tm_loop_transform3(g_data_array + 300, 150);
    tm_loop_transform4(1000000);
    tm_loop_transform5(75);
    tm_loop_transform6(120);
    
    /* Additional calls with different parameters */
    for (int repeat = 0; repeat < 3; repeat++) {
        tm_loop_transform1(200 + repeat * 50, 200 + (repeat + 1) * 50);
        g_loop_bound = 80 + repeat * 10;
        tm_loop_transform4(500000 * (repeat + 1));
    }
    
    /* Compute and print checksum to ensure execution */
    long final_checksum = compute_checksum();
    printf("Transaction test checksum: %ld\n", final_checksum);
    
    return 0;
}
