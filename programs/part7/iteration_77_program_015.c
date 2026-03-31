/* tm_loop_test.c - Test program for GCC TM load/store loop transformation */
#include <stdio.h>
#include <stdlib.h>

/* Shared global variables for TM operations */
volatile int g_shared_counter = 0;
int g_data_array[1024];
volatile long g_volatile_array[512];
int* g_dynamic_ptr = NULL;

/* Prevent optimization attributes */
#define NOOPT __attribute__((noinline, noipa, used))

/* Test function 1: Simple array processing in transactional region */
NOOPT void tm_loop_transform1(int start, int end) {
    __transaction_atomic {
        for (int i = start; i < end; i++) {
            /* Load and store pattern that may trigger loop transformation */
            int val = g_data_array[i];
            g_data_array[i] = val * 2 + 1;
            g_shared_counter++;
        }
    }
}

/* Test function 2: Nested loops with multi-dimensional access pattern */
NOOPT void tm_loop_transform2(int rows, int cols) {
    /* Use volatile to prevent constant propagation */
    volatile int local_rows = rows;
    volatile int local_cols = cols;
    
    __transaction_atomic {
        for (int i = 0; i < local_rows; i++) {
            for (int j = 0; j < local_cols; j++) {
                /* Complex addressing pattern */
                int idx = i * cols + j;
                if (idx < 1024) {
                    g_data_array[idx] = g_data_array[idx] ^ (i * j);
                    g_volatile_array[j % 512] = i + j;
                }
            }
        }
    }
}

/* Test function 3: Pointer-based loop with transaction retry logic */
NOOPT void tm_loop_transform3(int* data, int size) {
    int attempts = 0;
    
    while (attempts < 3) {
        __transaction_atomic {
            for (int i = 0; i < size; i++) {
                /* Load-modify-store with pointer arithmetic */
                *(data + i) = *(data + i) * 3 - 1;
                g_shared_counter += (i % 7);
            }
            
            /* Conditional transaction cancel to test retry */
            if (attempts < 2 && g_shared_counter % 1000 < 500) {
                __transaction_cancel;
            }
        }
        attempts++;
    }
}

/* Test function 4: Mixed relaxed and atomic transactions */
NOOPT void tm_loop_transform4(int iterations) {
    volatile int iter = iterations;
    
    /* First a relaxed transaction */
    __transaction_relaxed {
        int sum = 0;
        for (int i = 0; i < iter && i < 256; i++) {
            sum += g_volatile_array[i];
            g_volatile_array[i] = sum % 100;
        }
    }
    
    /* Then an atomic transaction */
    __transaction_atomic {
        for (int i = 0; i < iter && i < 512; i++) {
            /* Transform with conditional store */
            long old = g_volatile_array[i];
            g_volatile_array[i] = (old > 0) ? old * 2 : -old;
        }
    }
}

/* Test function 5: Complex control flow with TM in branches */
NOOPT void tm_loop_transform5(int mode, int limit) {
    volatile int local_limit = limit;
    
    if (mode & 1) {
        __transaction_atomic {
            int i = 0;
            while (i < local_limit) {
                g_data_array[i % 1024] += i;
                g_volatile_array[i % 512] -= 1;
                i += (mode % 3) + 1;  /* Variable increment */
            }
        }
    } else {
        __transaction_relaxed {
            for (int j = local_limit - 1; j >= 0; j--) {
                /* Reverse loop with different access pattern */
                int idx = j * 3 % 1024;
                g_data_array[idx] = g_data_array[idx] / 2;
            }
        }
    }
}

/* Initialize test data */
void init_test_data(void) {
    for (int i = 0; i < 1024; i++) {
        g_data_array[i] = i * 3 + 7;
    }
    for (int i = 0; i < 512; i++) {
        g_volatile_array[i] = i * 2 - 255;
    }
    
    g_dynamic_ptr = (int*)malloc(256 * sizeof(int));
    for (int i = 0; i < 256; i++) {
        g_dynamic_ptr[i] = i * 5;
    }
}

/* Compute checksum to verify execution */
long compute_checksum(void) {
    long checksum = g_shared_counter;
    
    for (int i = 0; i < 1024; i++) {
        checksum = checksum * 31 + g_data_array[i];
    }
    
    for (int i = 0; i < 512; i++) {
        checksum = checksum * 17 + g_volatile_array[i];
    }
    
    if (g_dynamic_ptr) {
        for (int i = 0; i < 256; i++) {
            checksum = checksum * 13 + g_dynamic_ptr[i];
        }
    }
    
    return checksum;
}

int main(void) {
    init_test_data();
    
    /* Execute test functions with varying parameters */
    tm_loop_transform1(0, 100);
    tm_loop_transform1(50, 200);
    
    tm_loop_transform2(16, 32);
    tm_loop_transform2(8, 64);
    
    tm_loop_transform3(g_dynamic_ptr, 128);
    tm_loop_transform3(&g_data_array[300], 150);
    
    tm_loop_transform4(128);
    tm_loop_transform4(256);
    
    tm_loop_transform5(1, 75);
    tm_loop_transform5(0, 50);
    tm_loop_transform5(3, 100);
    
    /* Additional mixed transactional regions */
    __transaction_atomic {
        for (int i = 0; i < 50; i++) {
            g_data_array[i * 10] = g_volatile_array[i * 5] + g_shared_counter;
        }
    }
    
    __transaction_relaxed {
        int temp = 0;
        for (int i = 0; i < 200; i++) {
            temp += g_data_array[i];
        }
        g_shared_counter += temp % 1000;
    }
    
    long final_checksum = compute_checksum();
    printf("TM Loop Test Checksum: %ld\n", final_checksum);
    
    if (g_dynamic_ptr) {
        free(g_dynamic_ptr);
    }
    
    return 0;
}
