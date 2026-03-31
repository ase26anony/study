/* test_ddg.c - Program to trigger DDG edge creation in GCC scheduler */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure loop body remains intact for DDG analysis */
__attribute__((noinline, noclone))
int compute_loop(int* arr, double* darr, float* farr, int size) {
    int i;
    int sum = 0;
    volatile int vol_var = 0; /* Prevent some optimizations */
    
    /* Complex loop with multiple dependency types */
    for (i = 1; i < size; i++) {
        /* 1. TRUE DEPENDENCY (RAW) - Integer chain */
        int temp1 = arr[i-1];          /* Read arr[i-1] */
        arr[i] = temp1 + i + vol_var;  /* Write arr[i] depends on arr[i-1] */
        
        /* 2. ANTI-DEPENDENCY (WAR) - Reuse same array location */
        int temp2 = arr[i];            /* Read arr[i] */
        arr[i] = temp2 * 2;            /* Write arr[i] - anti-dep on previous read */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - Multiple writes */
        arr[i] = arr[i] + 1;           /* Another write to arr[i] */
        
        /* 4. Memory aliasing with pointer arithmetic */
        int* ptr = &arr[i];
        *ptr = *ptr + arr[i-1];        /* Could alias with arr[i-1] */
        
        /* 5. Floating point with different latency operations */
        double dtemp = darr[i-1];      /* Read double */
        darr[i] = dtemp * 1.5 + i;     /* FP multiply has different latency */
        
        /* 6. Mixed float/int dependencies */
        float ftemp = farr[i-1];       /* Read float */
        farr[i] = ftemp + arr[i];      /* Depends on integer computation */
        
        /* 7. Control dependency-like pattern */
        if (arr[i] > 100) {
            vol_var = 1;               /* Volatile write creates memory barrier */
        }
        
        /* 8. Complex expression with multiple dependencies */
        sum += arr[i] + (int)darr[i] + (int)farr[i];
        
        /* 9. Pointer chasing for potential memory dep */
        int* indirect = arr + (i % 16);
        *indirect = *indirect + 1;     /* May alias with other arr elements */
    }
    
    return sum;
}

/* Another function with nested loops for more complex DDG */
__attribute__((noinline, noclone))
int nested_loop_compute(int* matrix, int rows, int cols) {
    int i, j;
    int total = 0;
    
    /* Nested loops create more complex dependency graphs */
    for (i = 1; i < rows; i++) {
        for (j = 1; j < cols; j++) {
            /* Cross-iteration dependencies in both dimensions */
            int idx = i * cols + j;
            int prev_row = (i-1) * cols + j;
            int prev_col = i * cols + (j-1);
            
            /* Multiple true dependencies */
            matrix[idx] = matrix[prev_row] + matrix[prev_col];
            
            /* Anti-dependency with computation */
            int temp = matrix[idx];
            matrix[idx] = temp * 3 - matrix[idx-1];
            
            total += matrix[idx];
        }
    }
    
    return total;
}

int main() {
    const int SIZE = 1024;
    int* int_array = (int*)malloc(SIZE * sizeof(int));
    double* double_array = (double*)malloc(SIZE * sizeof(double));
    float* float_array = (float*)malloc(SIZE * sizeof(float));
    int* matrix = (int*)malloc(64 * 64 * sizeof(int));
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i % 100;
        double_array[i] = i * 0.5;
        float_array[i] = i * 0.25f;
    }
    
    for (int i = 0; i < 64*64; i++) {
        matrix[i] = i % 50;
    }
    
    /* Call functions to trigger DDG construction */
    int result1 = compute_loop(int_array, double_array, float_array, SIZE);
    int result2 = nested_loop_compute(matrix, 64, 64);
    
    /* Use results to prevent dead code elimination */
    volatile int final_result = result1 + result2;
    
    printf("Results: %d, %d, Final: %d\n", result1, result2, final_result);
    
    /* Cleanup */
    free(int_array);
    free(double_array);
    free(float_array);
    free(matrix);
    
    return 0;
}
