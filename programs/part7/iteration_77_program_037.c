/* tm_loop_test.c - Test program for GCC TM load/store loop transformation */
#include <stdio.h>
#include <stdlib.h>

/* Shared global variables for TM operations */
volatile int g_counter = 0;
int g_array[1024];
long g_matrix[32][32];
volatile int g_bound = 32;

/* Prevent optimization and inlining */
#define NOOPT __attribute__((noinline, noipa, used))

/* Function 1: Simple array processing in transactional region */
NOOPT void tm_loop_transform1(int start, int end) {
    __transaction_atomic {
        for (int i = start; i < end; i++) {
            g_array[i] = g_array[i] * 2 + g_counter;
            g_counter++;
        }
    }
}

/* Function 2: Nested loops with multi-dimensional array */
NOOPT void tm_loop_transform2(int n) {
    volatile int local_bound = n; /* Non-constant bound */
    
    __transaction_atomic {
        for (int i = 0; i < local_bound; i++) {
            for (int j = 0; j < local_bound; j++) {
                g_matrix[i][j] = g_matrix[i][j] + (i * j) - g_counter;
            }
        }
    }
}

/* Function 3: While loop with pointer arithmetic */
NOOPT void tm_loop_transform3(int *data, int size) {
    __transaction_relaxed {
        int *ptr = data;
        int i = 0;
        while (i < size) {
            *ptr = (*ptr ^ 0xAAAA) + g_counter;
            ptr++;
            i++;
            if (i % 16 == 0) {
                __transaction_cancel; /* Test transaction cancellation */
            }
        }
    }
}

/* Function 4: Complex control flow with conditional TM */
NOOPT void tm_loop_transform4(int threshold) {
    volatile int flag = g_counter % 3;
    
    if (flag > 0) {
        __transaction_atomic {
            for (int i = 0; i < g_bound; i += 2) {
                g_array[i] = (g_array[i] << 1) | (g_array[i] >> 31);
                g_counter += i;
            }
        }
    } else {
        __transaction_relaxed {
            int j = g_bound - 1;
            while (j >= 0) {
                g_matrix[j][0] = g_matrix[j][0] * 3 - j;
                j--;
            }
        }
    }
}

/* Function 5: Mixed load/store patterns with function calls */
NOOPT int helper(int x, int y) {
    return (x ^ y) & 0xFF;
}

NOOPT void tm_loop_transform5(int iterations) {
    __transaction_atomic {
        for (int i = 0; i < iterations; i++) {
            /* Complex addressing pattern */
            int idx = (i * 7 + 3) % 1024;
            g_array[idx] = helper(g_array[idx], g_counter);
            
            /* Store to matrix */
            int row = i % 32;
            int col = (i * 3) % 32;
            g_matrix[row][col] = g_matrix[row][col] + g_array[idx];
            
            /* Volatile update */
            g_counter = g_counter + 1;
        }
    }
}

/* Initialize test data */
void init_data(void) {
    for (int i = 0; i < 1024; i++) {
        g_array[i] = i * 3 + 1;
    }
    
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            g_matrix[i][j] = i * 100 + j;
        }
    }
    
    g_counter = 0;
    g_bound = 32;
}

/* Compute checksum to verify execution */
long compute_checksum(void) {
    long sum = g_counter;
    
    for (int i = 0; i < 1024; i++) {
        sum = (sum * 31 + g_array[i]) & 0xFFFFFF;
    }
    
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            sum = (sum + g_matrix[i][j]) & 0xFFFFFF;
        }
    }
    
    return sum;
}

int main(void) {
    /* Initialize shared data */
    init_data();
    
    /* Execute various TM loop patterns */
    tm_loop_transform1(0, 256);
    tm_loop_transform1(256, 512);
    
    tm_loop_transform2(16);
    tm_loop_transform2(24);
    
    int local_data[64];
    for (int i = 0; i < 64; i++) local_data[i] = i * 5;
    tm_loop_transform3(local_data, 64);
    
    tm_loop_transform4(100);
    tm_loop_transform4(200);
    
    tm_loop_transform5(128);
    tm_loop_transform5(256);
    
    /* Additional relaxed transaction */
    __transaction_relaxed {
        for (int i = 512; i < 768; i++) {
            g_array[i] = g_array[i] ^ g_array[i-512];
        }
    }
    
    /* Compute and print checksum */
    long checksum = compute_checksum();
    printf("TM Loop Test Checksum: %ld\n", checksum);
    
    return 0;
}
