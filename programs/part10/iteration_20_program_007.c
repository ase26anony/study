/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure loop body remains intact for DDG analysis */
__attribute__((noinline, noclone))
int compute_loop(int* arr, double* darr, float* farr, int size) {
    int i;
    int sum = 0;
    volatile int vol_var = 0;  /* Prevent some optimizations */
    
    /* Complex loop with multiple dependency types */
    for (i = 1; i < size; i++) {
        /* 1. TRUE DEPENDENCY (RAW) - carried across iterations */
        int temp = arr[i-1];           /* Read arr[i-1] */
        arr[i] = temp + i + vol_var;   /* Write arr[i] depends on arr[i-1] */
        
        /* 2. ANTI-DEPENDENCY (WAR) - reuse of same location */
        float f_temp = farr[i];        /* Read farr[i] */
        farr[i] = f_temp * 1.5f + i;   /* Write farr[i] after read */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - multiple writes to same location */
        darr[i] = (double)i * 0.5;     /* First write to darr[i] */
        darr[i] = darr[i] * 2.0;       /* Second write to darr[i] */
        
        /* 4. MEMORY DEPENDENCY with pointer aliasing */
        int* ptr1 = &arr[i];
        int* ptr2 = &arr[size - i - 1];
        /* Compiler can't know if ptr1 == ptr2, creates memory dependency */
        *ptr1 = *ptr1 + *ptr2;
        
        /* 5. MIXED DATA TYPES in dependency chain */
        double d_temp = darr[i-1];     /* Read double */
        int int_from_double = (int)d_temp; /* Convert to int */
        arr[i] += int_from_double;     /* Use in int array */
        
        /* 6. LOOP-CARRIED DEPENDENCY with different latencies */
        /* Integer add (low latency) */
        sum += arr[i];
        /* Floating multiply (higher latency) */
        farr[i] = farr[i] * farr[i-1];
        
        /* 7. CONTROL DEPENDENCY-like pattern */
        if (arr[i] > 1000) {
            vol_var = 1;  /* Volatile prevents elimination */
        }
    }
    
    /* Ensure result depends on all iterations */
    return sum + (int)farr[size-1] + (int)darr[size-1];
}

/* Another function with nested loops for more complex DDG */
__attribute__((noinline, noclone))
int nested_loop_compute(int* matrix, int n) {
    int i, j;
    int total = 0;
    
    /* Nested loops create more complex dependency graphs */
    for (i = 1; i < n; i++) {
        for (j = 1; j < n; j++) {
            /* 2D array with multiple dependency directions */
            int idx = i * n + j;
            int prev_row = (i-1) * n + j;
            int prev_col = i * n + (j-1);
            
            /* Cross-iteration dependencies in both dimensions */
            matrix[idx] = matrix[prev_row] + matrix[prev_col] + matrix[idx];
            
            /* Anti-dependency with computation */
            int temp = matrix[idx];
            matrix[idx] = temp * 2 - matrix[prev_row];
            
            total += matrix[idx];
        }
    }
    
    return total;
}

int main(void) {
    const int SIZE = 256;
    int* int_array = (int*)malloc(SIZE * sizeof(int));
    double* double_array = (double*)malloc(SIZE * sizeof(double));
    float* float_array = (float*)malloc(SIZE * sizeof(float));
    int* matrix = (int*)malloc(SIZE * SIZE * sizeof(int));
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i % 10;
        double_array[i] = i * 0.1;
        float_array[i] = i * 0.5f;
    }
    
    for (int i = 0; i < SIZE * SIZE; i++) {
        matrix[i] = i % 20;
    }
    
    /* Call functions with dependency-rich loops */
    int result1 = compute_loop(int_array, double_array, float_array, SIZE);
    int result2 = nested_loop_compute(matrix, 16);  /* 16x16 matrix */
    
    /* Use results to prevent dead code elimination */
    volatile int final_result = result1 + result2;
    
    printf("Result: %d\n", final_result);
    
    /* Cleanup */
    free(int_array);
    free(double_array);
    free(float_array);
    free(matrix);
    
    return 0;
}
