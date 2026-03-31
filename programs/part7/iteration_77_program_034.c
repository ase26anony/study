/* tm_loop_test.c - Test program for GCC transactional memory loop transformation */
#include <stdio.h>
#include <stdlib.h>

/* Shared global variables for TM operations */
volatile int g_shared_array[1024];
volatile long g_shared_long[512];
int* g_shared_ptr = NULL;
volatile int g_loop_bound = 100;

/* Prevent optimization of TM functions */
#define TM_NOOPT __attribute__((noinline, noipa, used))

/* Test function 1: Simple array processing in transactional atomic region */
TM_NOOPT
void tm_loop_transform1(int start, int end) {
    /* Transaction with array load/store loop */
    __transaction_atomic {
        for (int i = start; i < end; i++) {
            /* Mixed load/store pattern */
            int val = g_shared_array[i];
            g_shared_array[i] = val * 2 + i;
            
            /* Additional store to create transformation opportunity */
            if (i % 2 == 0) {
                g_shared_long[i % 512] = val;
            }
        }
    }
}

/* Test function 2: Nested loops with relaxed transaction */
TM_NOOPT
void tm_loop_transform2(int rows, int cols) {
    volatile int local_bound = cols;
    
    __transaction_relaxed {
        /* Nested loop accessing 2D pattern */
        for (int i = 0; i < rows; i++) {
            int base = i * 64;
            for (int j = 0; j < local_bound; j++) {
                /* Complex addressing pattern */
                int idx = (base + j) % 1024;
                int old = g_shared_array[idx];
                
                /* Multiple stores with condition */
                g_shared_array[idx] = old ^ 0x5A5A;
                if (j % 3 == 0) {
                    g_shared_array[(idx + 1) % 1024] = old;
                }
            }
        }
    }
}

/* Test function 3: Pointer-based loop with transaction cancellation */
TM_NOOPT
void tm_loop_transform3(int* data, int size) {
    int retry_count = 0;
    
    /* Transaction with potential cancellation */
    __transaction_atomic {
        /* Loop with pointer arithmetic */
        int* ptr = data;
        for (int i = 0; i < size; i++) {
            *ptr = *ptr + g_shared_array[i % 1024];
            ptr++;
            
            /* Conditional transaction cancel */
            if (retry_count < 3 && *ptr == 0xDEADBEEF) {
                __transaction_cancel;
            }
        }
        retry_count++;
    }
}

/* Test function 4: Complex control flow with multiple TM regions */
TM_NOOPT
void tm_loop_transform4(int threshold) {
    volatile int dynamic_bound = g_loop_bound;
    
    /* Outer transaction */
    __transaction_atomic {
        /* First loop */
        for (int i = 0; i < dynamic_bound; i += 2) {
            g_shared_array[i] = g_shared_long[i % 512] + i;
        }
        
        /* Conditional inner transaction */
        if (threshold > 50) {
            __transaction_relaxed {
                /* Different loop structure */
                int j = dynamic_bound - 1;
                while (j >= 0) {
                    g_shared_long[j % 512] = g_shared_array[j] - threshold;
                    j -= 3;
                }
            }
        }
        
        /* Another loop in outer transaction */
        for (int k = 0; k < dynamic_bound / 2; k++) {
            /* Access with volatile to prevent optimization */
            volatile int* addr = &g_shared_array[k * 2];
            *addr = *addr ^ k;
        }
    }
}

/* Test function 5: Mixed TM operations with function calls */
TM_NOOPT
static int helper_compute(int x, int y) {
    return (x * y) & 0xFF;
}

TM_NOOPT
void tm_loop_transform5(int iterations) {
    __transaction_atomic {
        /* Loop with function call inside */
        for (int i = 0; i < iterations; i++) {
            int idx = i % 1024;
            int computed = helper_compute(g_shared_array[idx], i);
            
            /* Store with address computation */
            g_shared_array[(idx + 256) % 1024] = computed;
            g_shared_long[idx % 512] = computed >> 4;
        }
    }
}

/* Initialize shared data */
void init_shared_data(void) {
    for (int i = 0; i < 1024; i++) {
        g_shared_array[i] = i * 3 + 1;
    }
    for (int i = 0; i < 512; i++) {
        g_shared_long[i] = i * 5 - 2;
    }
    
    /* Allocate and initialize pointer-based data */
    g_shared_ptr = (int*)malloc(256 * sizeof(int));
    for (int i = 0; i < 256; i++) {
        g_shared_ptr[i] = i * 7;
    }
}

/* Compute checksum to verify execution */
int compute_checksum(void) {
    int sum = 0;
    for (int i = 0; i < 1024; i++) {
        sum = (sum + g_shared_array[i]) & 0xFFFF;
    }
    for (int i = 0; i < 512; i++) {
        sum = (sum + (int)g_shared_long[i]) & 0xFFFF;
    }
    if (g_shared_ptr) {
        for (int i = 0; i < 256; i++) {
            sum = (sum + g_shared_ptr[i]) & 0xFFFF;
        }
    }
    return sum;
}

int main(void) {
    /* Initialize test data */
    init_shared_data();
    
    /* Execute TM test functions with varying parameters */
    printf("Starting TM loop transformation tests...\n");
    
    /* Test 1: Basic array loop */
    tm_loop_transform1(0, g_loop_bound);
    
    /* Test 2: Nested loops */
    tm_loop_transform2(16, 32);
    
    /* Test 3: Pointer-based with cancellation */
    tm_loop_transform3(g_shared_ptr, 128);
    
    /* Test 4: Complex control flow */
    tm_loop_transform4(75);
    
    /* Test 5: Mixed operations */
    tm_loop_transform5(200);
    
    /* Additional calls with different bounds */
    for (int run = 0; run < 3; run++) {
        g_loop_bound = 80 + run * 20;
        tm_loop_transform1(10, g_loop_bound - 10);
        tm_loop_transform4(g_loop_bound);
    }
    
    /* Verify and output results */
    int checksum = compute_checksum();
    printf("TM tests completed. Checksum: 0x%04X\n", checksum);
    
    /* Cleanup */
    free(g_shared_ptr);
    
    return 0;
}
