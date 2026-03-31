/* test_ddg.c - Program to trigger DDG edge creation in GCC scheduler */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure loop body remains intact for DDG analysis */
__attribute__((noinline, noclone))
int compute_loop(int* arr, double* darr, float* farr, int size) {
    int i;
    int temp_int;
    double temp_double;
    float temp_float;
    int* ptr;
    int result = 0;
    
    /* Initialize some values */
    int prev_int = arr[0];
    double prev_double = darr[0];
    float prev_float = farr[0];
    
    /* Complex loop with multiple dependency types */
    for (i = 1; i < size; i++) {
        /* 1. TRUE DEPENDENCY (RAW) - Integer chain */
        temp_int = arr[i-1] + i;          /* Read arr[i-1] */
        arr[i] = temp_int * 2;            /* Write arr[i] - depends on temp_int */
        
        /* 2. ANTI-DEPENDENCY (WAR) - Reusing same array location */
        temp_int = arr[i];                /* Read arr[i] */
        arr[i] = temp_int + prev_int;     /* Write arr[i] again - anti-dep on previous read */
        prev_int = temp_int;
        
        /* 3. OUTPUT DEPENDENCY (WAW) - Multiple writes to same location */
        darr[i] = prev_double * 1.5;      /* First write to darr[i] */
        darr[i] = darr[i] * 2.0;          /* Second write to darr[i] - output dep */
        
        /* 4. Memory dependencies with pointer aliasing */
        ptr = &arr[i];
        *ptr = *ptr + arr[i-1];           /* Potential memory dep through pointer */
        
        /* 5. Mixed data type operations with different latencies */
        temp_float = (float)arr[i] * 0.5f;        /* Integer to float conversion */
        temp_double = (double)temp_float * 1.25;  /* Float to double conversion */
        darr[i-1] = temp_double + darr[i];        /* Double precision operation */
        
        /* 6. Floating-point chain with true dependency */
        prev_float = farr[i-1] * 1.1f;
        farr[i] = prev_float + (float)i * 0.01f;
        
        /* 7. Complex expression with multiple dependencies */
        result += arr[i] + (int)darr[i] + (int)farr[i];
        
        /* Update previous values for next iteration */
        prev_double = darr[i] * 0.8;
    }
    
    /* Additional loop with carried dependency */
    for (i = size - 2; i >= 0; i--) {
        /* Reverse direction carried dependency */
        arr[i] = arr[i+1] - i;
        darr[i] = darr[i+1] / 2.0;
        
        /* Memory dependency through different arrays (potential aliasing) */
        if (i % 2 == 0) {
            farr[i] = farr[i+1] + (float)arr[i];
        }
        
        result += arr[i];
    }
    
    return result;
}

/* Another function with nested loops */
__attribute__((noinline, noclone))
int nested_loop_computation(int* matrix, int rows, int cols) {
    int i, j;
    int sum = 0;
    int prev_row_val = 0;
    
    /* Nested loops with cross-iteration dependencies */
    for (i = 0; i < rows; i++) {
        int prev_col_val = matrix[i * cols];
        
        for (j = 1; j < cols; j++) {
            /* True dependency within inner loop */
            int current = matrix[i * cols + j - 1] + j;
            matrix[i * cols + j] = current * 3;
            
            /* Anti-dependency: read then write */
            int temp = matrix[i * cols + j];
            matrix[i * cols + j] = temp + prev_col_val;
            prev_col_val = temp;
            
            /* Cross-iteration dependency in outer loop */
            if (i > 0) {
                matrix[i * cols + j] += matrix[(i-1) * cols + j] / 2;
            }
            
            sum += matrix[i * cols + j];
        }
        
        /* Outer loop carried dependency */
        prev_row_val = matrix[i * cols + cols - 1];
    }
    
    return sum;
}

int main() {
    const int SIZE = 256;
    const int ROWS = 16;
    const int COLS = 16;
    
    /* Allocate and initialize arrays */
    int* arr = (int*)malloc(SIZE * sizeof(int));
    double* darr = (double*)malloc(SIZE * sizeof(double));
    float* farr = (float*)malloc(SIZE * sizeof(float));
    int* matrix = (int*)malloc(ROWS * COLS * sizeof(int));
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i + 1;
        darr[i] = (double)i * 0.5;
        farr[i] = (float)i * 0.25f;
    }
    
    for (int i = 0; i < ROWS * COLS; i++) {
        matrix[i] = i % 10;
    }
    
    /* Volatile to prevent optimization of results */
    volatile int result1, result2;
    
    /* Execute loops to trigger DDG construction */
    result1 = compute_loop(arr, darr, farr, SIZE);
    result2 = nested_loop_computation(matrix, ROWS, COLS);
    
    /* Use results to prevent dead code elimination */
    printf("Result 1: %d\n", result1);
    printf("Result 2: %d\n", result2);
    
    /* Additional volatile store */
    volatile int final_result = result1 + result2;
    
    /* Clean up */
    free(arr);
    free(darr);
    free(farr);
    free(matrix);
    
    return 0;
}
