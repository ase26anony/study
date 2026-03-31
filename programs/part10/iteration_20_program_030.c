/* test_ddg.c - Program to trigger DDG edge creation in GCC scheduler */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure loop body remains intact for DDG analysis */
__attribute__((noinline, noclone))
int compute_loop(int* arr, float* farr, double* darr, int size) {
    int i;
    int temp_int;
    float temp_float;
    double temp_double;
    int* ptr;
    int sum = 0;
    
    /* Initialize arrays with some values */
    for (i = 0; i < size; i++) {
        arr[i] = i;
        farr[i] = i * 0.5f;
        darr[i] = i * 0.25;
    }
    
    /* 
     * Main loop with complex dependency patterns to trigger DDG edge creation
     * This loop contains multiple types of dependencies:
     */
    for (i = 1; i < size - 1; i++) {
        /* 1. TRUE DEPENDENCY (RAW) - Integer chain */
        arr[i] = arr[i-1] + i * 2;          /* RAW: arr[i-1] read, then arr[i] written */
        
        /* 2. ANTI-DEPENDENCY (WAR) - Reusing same location */
        temp_int = arr[i];                  /* Read arr[i] */
        arr[i] = temp_int * 3 - i;          /* Write arr[i] - WAR with previous read */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - Multiple writes to same location */
        arr[i] = arr[i] / 2;                /* Another write to arr[i] - WAW */
        
        /* 4. TRUE DEPENDENCY with different data type - Float */
        farr[i] = farr[i-1] * 1.5f + arr[i]; /* RAW on farr[i-1], RAW on arr[i] */
        
        /* 5. ANTI-DEPENDENCY with pointer aliasing (potential memory dependency) */
        ptr = &arr[i];
        temp_int = *ptr;                    /* Read through pointer */
        *ptr = temp_int + farr[i];          /* Write through same pointer - WAR */
        
        /* 6. TRUE DEPENDENCY - Double precision with mixed operations */
        darr[i] = darr[i-1] * 2.0 + farr[i] * 0.5; /* RAW on darr[i-1], RAW on farr[i] */
        
        /* 7. OUTPUT DEPENDENCY with different data type */
        temp_double = darr[i];              /* Read darr[i] */
        darr[i] = temp_double * 0.75;       /* Write darr[i] - WAW if scheduled poorly */
        
        /* 8. Control-dependent operation with data dependency */
        if (arr[i] > 100) {
            temp_float = farr[i];           /* Read farr[i] */
            farr[i] = temp_float * 0.9f;    /* Write farr[i] - WAR */
        }
        
        /* 9. Complex expression with multiple dependencies */
        arr[i+1] = (arr[i] + arr[i-1]) * (int)farr[i] - (int)darr[i];
        
        /* Accumulate result to prevent dead code elimination */
        sum += arr[i] + (int)farr[i] + (int)darr[i];
    }
    
    /* Use volatile to ensure computation isn't optimized away */
    volatile int final_sum = sum;
    return final_sum;
}

/* Another function with nested loops for additional DDG opportunities */
__attribute__((noinline, noclone))
int nested_loop_compute(int* matrix, int rows, int cols) {
    int i, j;
    int total = 0;
    
    /* Nested loop with carried dependencies in both dimensions */
    for (i = 1; i < rows; i++) {
        for (j = 1; j < cols; j++) {
            /* 2D true dependency (RAW) */
            matrix[i*cols + j] = matrix[(i-1)*cols + j] + matrix[i*cols + (j-1)];
            
            /* Anti-dependency in inner loop */
            int temp = matrix[i*cols + j];
            matrix[i*cols + j] = temp * 2 - matrix[(i-1)*cols + (j-1)];
            
            total += matrix[i*cols + j];
        }
    }
    
    volatile int vol_total = total;
    return vol_total;
}

int main() {
    const int SIZE = 1024;
    const int ROWS = 64;
    const int COLS = 64;
    
    /* Allocate arrays with different data types */
    int* int_array = (int*)malloc(SIZE * sizeof(int));
    float* float_array = (float*)malloc(SIZE * sizeof(float));
    double* double_array = (double*)malloc(SIZE * sizeof(double));
    int* matrix = (int*)malloc(ROWS * COLS * sizeof(int));
    
    if (!int_array || !float_array || !double_array || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize matrix */
    for (int i = 0; i < ROWS * COLS; i++) {
        matrix[i] = i % 100;
    }
    
    printf("Starting computation...\n");
    
    /* Call functions with complex dependency loops */
    int result1 = compute_loop(int_array, float_array, double_array, SIZE);
    int result2 = nested_loop_compute(matrix, ROWS, COLS);
    
    /* Use results to prevent optimization */
    printf("Result 1: %d\n", result1);
    printf("Result 2: %d\n", result2);
    
    /* Final computation mixing all arrays */
    volatile int final_check = 0;
    for (int i = 0; i < SIZE && i < ROWS*COLS; i++) {
        final_check += int_array[i] + (int)float_array[i] + (int)double_array[i];
        if (i < ROWS*COLS) {
            final_check += matrix[i];
        }
    }
    
    printf("Final check: %d\n", final_check);
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(matrix);
    
    return 0;
}
