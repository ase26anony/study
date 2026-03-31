/* tm_loop_coverage.c - Program to trigger TM load/store loop transformation */
#include <stdio.h>
#include <stdlib.h>

/* Shared global variables for TM operations */
volatile int g_start = 0;
volatile int g_end = 100;
int shared_array[1000];
long shared_matrix[50][50];
volatile int *volatile shared_ptr = NULL;

/* Prevent optimization of TM functions */
#define TM_NOOPT __attribute__((noinline, noipa, used))

/* Test function 1: Simple array processing in transactional region */
TM_NOOPT
void tm_loop_transform1(int start, int end) {
    __transaction_atomic {
        for (int i = start; i < end; i++) {
            /* Load and store pattern */
            int val = shared_array[i];
            shared_array[i] = val * 2 + 1;
        }
    }
}

/* Test function 2: Nested loops with multi-dimensional array */
TM_NOOPT
void tm_loop_transform2(int rows, int cols) {
    __transaction_atomic {
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                /* Complex load/store with dependency */
                long old = shared_matrix[i][j];
                shared_matrix[i][j] = old + i * cols + j;
            }
        }
    }
}

/* Test function 3: While loop with pointer arithmetic */
TM_NOOPT
void tm_loop_transform3(int *data, int count) {
    __transaction_relaxed {
        int i = 0;
        while (i < count) {
            /* Pointer-based load/store */
            int temp = data[i];
            data[i] = temp ^ 0xABCD;
            i++;
        }
    }
}

/* Test function 4: Transaction with conditional retry logic */
TM_NOOPT
void tm_loop_transform4(int threshold) {
    int attempts = 0;
    
    __transaction_atomic {
        /* Loop with potential transformation */
        for (int i = g_start; i < g_end; i += 2) {
            shared_array[i] = shared_array[i] * 3;
        }
        
        /* Conditional transaction cancel */
        if (attempts++ < 3 && threshold > 100) {
            __transaction_cancel;
        }
    }
}

/* Test function 5: Mixed transaction types with complex control flow */
TM_NOOPT
void tm_loop_transform5(int mode) {
    if (mode & 1) {
        __transaction_atomic {
            /* Reverse loop */
            for (int i = g_end - 1; i >= g_start; i--) {
                int val = shared_array[i];
                shared_array[i] = val / 2;
            }
        }
    } else {
        __transaction_relaxed {
            /* Strided access pattern */
            for (int i = g_start; i < g_end; i += 3) {
                shared_array[i] = shared_array[i] + shared_array[i+1];
            }
        }
    }
}

/* Test function 6: Nested transactions with volatile variables */
TM_NOOPT
void tm_loop_transform6(void) {
    volatile int counter = 0;
    
    __transaction_atomic {
        /* Outer transaction */
        for (int i = 0; i < 50; i++) {
            counter++;
            
            __transaction_relaxed {
                /* Inner transaction - may trigger special handling */
                for (int j = 0; j < 50; j++) {
                    shared_matrix[i][j] = counter * j;
                }
            }
        }
    }
}

/* Test function 7: Function pointer call within TM region */
typedef void (*transform_func_t)(int*, int);
TM_NOOPT
static void helper_transform(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] << 1;
    }
}

TM_NOOPT
void tm_loop_transform7(void) {
    transform_func_t func = helper_transform;
    
    __transaction_atomic {
        /* Call function pointer in loop */
        for (int section = 0; section < 10; section++) {
            func(&shared_array[section * 10], 10);
        }
    }
}

/* Initialize test data */
void init_data(void) {
    for (int i = 0; i < 1000; i++) {
        shared_array[i] = i % 256;
    }
    
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 50; j++) {
            shared_matrix[i][j] = i * 100 + j;
        }
    }
    
    shared_ptr = shared_array;
}

/* Main function - drives all test cases */
int main(void) {
    init_data();
    
    /* Execute all TM test functions with varying parameters */
    tm_loop_transform1(0, 100);
    tm_loop_transform1(100, 200);  /* Different range */
    
    tm_loop_transform2(25, 25);
    tm_loop_transform2(10, 40);    /* Different dimensions */
    
    tm_loop_transform3(shared_array + 200, 150);
    tm_loop_transform3(shared_array + 500, 100);
    
    tm_loop_transform4(50);   /* Below threshold - no cancel */
    tm_loop_transform4(150);  /* Above threshold - may cancel */
    
    tm_loop_transform5(0);    /* Relaxed mode */
    tm_loop_transform5(1);    /* Atomic mode */
    
    tm_loop_transform6();
    tm_loop_transform7();
    
    /* Compute checksum to verify execution and prevent optimization */
    long checksum = 0;
    for (int i = 0; i < 1000; i++) {
        checksum += shared_array[i];
    }
    
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 50; j++) {
            checksum += shared_matrix[i][j];
        }
    }
    
    printf("TM Loop Transformation Test Complete\n");
    printf("Checksum: %ld\n", checksum);
    
    return 0;
}
