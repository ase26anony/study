/* test_ddg.c - Program to trigger DDG edge initialization in GCC */
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
    
    /* Initialize with some values */
    for (i = 0; i < size; i++) {
        arr[i] = i;
        darr[i] = i * 0.5;
        farr[i] = i * 0.25f;
    }
    
    /* 
     * Complex loop with multiple dependency types to trigger DDG edge creation
     * This should create edges with different types, data types, and latencies
     */
    for (i = 1; i < size - 1; i++) {
        /* 1. TRUE DEPENDENCY (RAW) - Integer chain */
        temp_int = arr[i - 1] + i;          /* Read arr[i-1] */
        arr[i] = temp_int * 2;              /* Write arr[i] - depends on temp_int */
        
        /* 2. ANTI-DEPENDENCY (WAR) - Reuse of arr[i] */
        temp_int = arr[i] + 5;              /* Read arr[i] */
        arr[i] = temp_int - 3;              /* Write arr[i] - anti-dep on previous read */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - Multiple writes to same location */
        arr[i] = arr[i] * arr[i - 1];       /* Write arr[i] again */
        
        /* 4. TRUE DEPENDENCY with different data type - Floating point */
        temp_double = darr[i - 1] * 1.5;    /* Read darr[i-1] */
        darr[i] = temp_double + 0.1;        /* Write darr[i] - FP dependency */
        
        /* 5. ANTI-DEPENDENCY with float */
        temp_float = farr[i];               /* Read farr[i] */
        farr[i] = temp_float * 2.0f;        /* Write farr[i] - anti-dep */
        
        /* 6. Memory aliasing through pointers - creates conservative memory deps */
        ptr = &arr[i];
        *ptr = *ptr + *(ptr - 1);           /* Could alias with other accesses */
        
        /* 7. Mixed-type operations with different latencies */
        darr[i] = darr[i] + (double)arr[i] * 0.01;  /* FP multiply + add */
        
        /* 8. Control dependency-like pattern */
        if (arr[i] > 100) {
            temp_int = arr[i] / 2;          /* Creates data dep inside condition */
        } else {
            temp_int = arr[i] * 3;
        }
        
        /* 9. Accumulator with loop-carried dependency */
        result += temp_int + (int)darr[i];  /* True dep across iterations via result */
        
        /* 10. Pointer arithmetic with dependency */
        farr[i] = farr[i] + *(farr + i - 1); /* Pointer-based memory access */
    }
    
    /* Final computation to prevent elimination */
    arr[size - 1] = result;
    return result;
}

/* Another function with different patterns */
__attribute__((noinline, noclone))
void nested_loop_deps(int* matrix, int rows, int cols) {
    int i, j;
    
    /* Nested loops with carried dependencies in both dimensions */
    for (i = 1; i < rows; i++) {
        for (j = 1; j < cols; j++) {
            /* 2D stencil computation with multiple dependencies */
            int idx = i * cols + j;
            
            /* Horizontal true dependency */
            matrix[idx] = matrix[idx - 1] + matrix[idx];
            
            /* Vertical true dependency */
            matrix[idx] += matrix[idx - cols];
            
            /* Diagonal anti-dependency */
            int temp = matrix[idx - cols - 1];
            matrix[idx - cols - 1] = matrix[idx] * 2;
            matrix[idx] += temp;
        }
    }
}

int main(void) {
    const int SIZE = 256;
    int* int_array = (int*)malloc(SIZE * sizeof(int));
    double* double_array = (double*)malloc(SIZE * sizeof(double));
    float* float_array = (float*)malloc(SIZE * sizeof(float));
    int* matrix = (int*)malloc(SIZE * SIZE * sizeof(int));
    
    if (!int_array || !double_array || !float_array || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Force computation to prevent dead code elimination */
    volatile int result1 = compute_loop(int_array, double_array, float_array, SIZE);
    nested_loop_deps(matrix, 16, 16);
    
    /* Use results to prevent optimization */
    printf("Result 1: %d\n", result1);
    printf("Matrix[0]: %d\n", matrix[0]);
    
    /* Cleanup */
    free(int_array);
    free(double_array);
    free(float_array);
    free(matrix);
    
    return 0;
}
