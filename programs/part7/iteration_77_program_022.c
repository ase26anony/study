/* tm_loop_test.c - Test program for GCC TM load/store loop transformation */
#include <stdio.h>
#include <stdlib.h>

/* Shared global variables for TM operations */
volatile int g_tm_counter = 0;
int g_shared_array[1024];
long g_shared_matrix[32][32];
volatile int g_loop_bound = 32;

/* Prevent optimization and inlining */
#define NOOPT __attribute__((noinline, noipa, used))

/* Test function 1: Simple array processing in TM region */
NOOPT void tm_loop_transform1(int start, int end) {
    __transaction_atomic {
        for (int i = start; i < end; i++) {
            g_shared_array[i] = g_shared_array[i] * 2 + 1;
            g_tm_counter++;
        }
    }
}

/* Test function 2: Nested loops with multi-dimensional array */
NOOPT void tm_loop_transform2(int size) {
    volatile int local_bound = size; /* Non-constant bound */
    
    __transaction_relaxed {
        for (int i = 0; i < local_bound; i++) {
            for (int j = 0; j < local_bound; j++) {
                g_shared_matrix[i][j] = g_shared_matrix[i][j] + 
                                       (i * local_bound + j);
                if (g_shared_matrix[i][j] > 1000) {
                    g_shared_matrix[i][j] = 1000;
                }
            }
        }
    }
}

/* Test function 3: While loop with pointer arithmetic */
NOOPT void tm_loop_transform3(int* data, int length) {
    int* ptr = data;
    int* end = data + length;
    
    __transaction_atomic {
        while (ptr < end) {
            *ptr = (*ptr ^ 0x55AA) + g_tm_counter;
            ptr++;
            g_tm_counter += 2;
        }
    }
}

/* Test function 4: Complex control flow with transaction cancel */
NOOPT void tm_loop_transform4(int threshold) {
    int retry_count = 0;
    
transaction_retry:
    __transaction_atomic {
        int sum = 0;
        
        /* Loop that might cause transaction abort */
        for (int i = 0; i < g_loop_bound; i++) {
            sum += g_shared_array[i];
            g_shared_array[i] = (g_shared_array[i] + i) % 256;
            
            /* Simulate condition that might cause abort */
            if (sum > threshold && retry_count < 3) {
                __transaction_cancel;
                retry_count++;
                goto transaction_retry;
            }
        }
        
        g_tm_counter += sum;
    }
}

/* Test function 5: Mixed TM regions with different attributes */
NOOPT void tm_loop_transform5(void) {
    /* Outer relaxed transaction */
    __transaction_relaxed {
        int temp[64];
        
        /* Initialize temp array */
        for (int i = 0; i < 64; i++) {
            temp[i] = i * 3;
        }
        
        /* Inner atomic transaction */
        __transaction_atomic {
            for (int i = 0; i < 64; i += 2) {
                g_shared_array[i] = temp[i] + g_shared_array[i+1];
                g_shared_array[i+1] = temp[i+1] - g_shared_array[i];
            }
        }
        
        /* Another loop in relaxed region */
        int j = 0;
        while (j < 32) {
            g_shared_matrix[j][j] = g_shared_array[j] * 2;
            j++;
        }
    }
}

/* Helper to initialize data */
void initialize_data(void) {
    for (int i = 0; i < 1024; i++) {
        g_shared_array[i] = i % 256;
    }
    
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            g_shared_matrix[i][j] = i * 32 + j;
        }
    }
    
    g_tm_counter = 0;
    g_loop_bound = 32;
}

/* Calculate checksum to verify execution */
long calculate_checksum(void) {
    long checksum = g_tm_counter;
    
    for (int i = 0; i < 1024; i++) {
        checksum = (checksum * 31 + g_shared_array[i]) % 1000000007;
    }
    
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            checksum = (checksum * 17 + g_shared_matrix[i][j]) % 1000000007;
        }
    }
    
    return checksum;
}

int main(void) {
    /* Initialize test data */
    initialize_data();
    
    /* Execute TM test functions with various patterns */
    
    /* Test 1: Simple array loop */
    tm_loop_transform1(0, 256);
    
    /* Test 2: Nested matrix loops */
    tm_loop_transform2(16);
    
    /* Test 3: Pointer-based loop */
    int local_data[128];
    for (int i = 0; i < 128; i++) {
        local_data[i] = i * 7;
    }
    tm_loop_transform3(local_data, 128);
    
    /* Test 4: Transaction with potential cancel/retry */
    tm_loop_transform4(5000);
    
    /* Test 5: Mixed TM regions */
    tm_loop_transform5();
    
    /* Additional TM regions to increase coverage */
    __transaction_atomic {
        for (volatile int i = 0; i < g_loop_bound; i++) {
            g_shared_array[i * 2] += g_tm_counter;
        }
    }
    
    __transaction_relaxed {
        int k = 0;
        do {
            g_shared_matrix[k % 16][k % 16] -= k;
            k++;
        } while (k < 100);
    }
    
    /* Calculate and print checksum */
    long final_checksum = calculate_checksum();
    printf("TM Loop Test Checksum: %ld\n", final_checksum);
    printf("Final TM Counter: %d\n", g_tm_counter);
    
    return 0;
}
