/* test_ddg.c - Program to trigger DDG edge creation in GCC scheduler */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure loop body is analyzed separately */
__attribute__((noinline, noclone))
int compute_loop(int* arr, float* farr, double* darr, int size) {
    int i;
    int temp_int;
    float temp_float;
    double temp_double;
    int* ptr;
    int result = 0;
    
    /* Initialize arrays with some values */
    for (i = 0; i < size; i++) {
        arr[i] = i;
        farr[i] = i * 0.5f;
        darr[i] = i * 0.25;
    }
    
    /* 
     * Main loop with complex dependency patterns
     * Designed to create various DDG edges
     */
    for (i = 1; i < size - 1; i++) {
        /* 1. TRUE DEPENDENCY (RAW) with different data types */
        temp_int = arr[i - 1];           /* Load */
        arr[i] = temp_int + i;           /* Store with dependency on previous iteration */
        
        /* 2. ANTI-DEPENDENCY (WAR) */
        temp_float = farr[i];            /* Read before write */
        farr[i] = temp_float * 1.1f + arr[i]; /* Write after read */
        
        /* 3. OUTPUT DEPENDENCY (WAW) */
        darr[i] = temp_float * 0.5;      /* First write */
        darr[i] = darr[i] * 2.0;         /* Second write to same location */
        
        /* 4. MEMORY DEPENDENCY with pointer aliasing */
        ptr = &arr[i];
        *ptr = *ptr + arr[i - 1];        /* Potential memory dependency */
        
        /* 5. MIXED OPERATIONS with different latencies */
        temp_double = darr[i] * 3.14159; /* FP multiply */
        result += (int)temp_double;      /* Integer add with type conversion */
        
        /* 6. CONTROL DEPENDENCY (implicit through loop) */
        if (result > 1000) {
            result = result / 2;         /* Creates control flow */
        }
        
        /* 7. ADDITIONAL TRUE DEPENDENCY chain */
        arr[i + 1] = arr[i] + result;    /* Forward dependency to next iteration */
    }
    
    /* Final computation with dependencies */
    temp_int = arr[size - 2];
    temp_float = farr[size - 2];
    result = result + temp_int + (int)temp_float;
    
    return result;
}

/* Another function with nested loops for more complex DDG */
__attribute__((noinline, noclone))
int nested_loop_compute(int* matrix, int rows, int cols) {
    int i, j;
    int sum = 0;
    volatile int sink; /* Prevent optimization */
    
    /* Nested loop with carried dependencies */
    for (i = 1; i < rows; i++) {
        for (j = 1; j < cols; j++) {
            /* 2D stencil computation with multiple dependencies */
            int idx = i * cols + j;
            int prev_row = (i - 1) * cols + j;
            int prev_col = i * cols + (j - 1);
            
            /* Multiple true dependencies */
            matrix[idx] = matrix[prev_row] + matrix[prev_col] + matrix[idx];
            
            /* Anti-dependency */
            int temp = matrix[idx];
            matrix[idx] = temp * 2 - matrix[idx - 1];
            
            sum += matrix[idx];
        }
    }
    
    sink = sum; /* Force computation */
    return sum;
}

int main() {
    const int SIZE = 256;
    int* int_array;
    float* float_array;
    double* double_array;
    int result;
    
    /* Allocate arrays */
    int_array = (int*)malloc(SIZE * sizeof(int));
    float_array = (float*)malloc(SIZE * sizeof(float));
    double_array = (double*)malloc(SIZE * sizeof(double));
    
    if (!int_array || !float_array || !double_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Call the function with complex dependencies */
    result = compute_loop(int_array, float_array, double_array, SIZE);
    
    /* Use volatile to prevent dead code elimination */
    volatile int volatile_result = result;
    
    /* Print result to ensure execution */
    printf("Result from compute_loop: %d\n", volatile_result);
    
    /* Test with nested loops */
    const int ROWS = 64;
    const int COLS = 64;
    int* matrix = (int*)malloc(ROWS * COLS * sizeof(int));
    
    if (matrix) {
        int nested_result = nested_loop_compute(matrix, ROWS, COLS);
        printf("Result from nested_loop_compute: %d\n", nested_result);
        free(matrix);
    }
    
    /* Clean up */
    free(int_array);
    free(float_array);
    free(double_array);
    
    return 0;
}
