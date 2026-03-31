/* test_ddg.c - Program to trigger DDG edge creation in GCC scheduler */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure loop body is analyzed for DDG */
__attribute__((noinline, noclone))
int compute_loop(int* arr, double* darr, float* farr, int size) {
    int i;
    int sum = 0;
    double dsum = 0.0;
    float fsum = 0.0f;
    
    /* Initialize with some values */
    for (i = 0; i < size; i++) {
        arr[i] = i;
        darr[i] = i * 0.5;
        farr[i] = i * 0.25f;
    }
    
    /* Main loop with complex dependency patterns */
    for (i = 1; i < size; i++) {
        /* 1. TRUE DEPENDENCY (RAW) - Integer chain */
        int temp1 = arr[i-1];          /* Read arr[i-1] */
        arr[i] = temp1 + i;            /* Write arr[i] depends on arr[i-1] */
        
        /* 2. ANTI-DEPENDENCY (WAR) - Reuse same array location */
        double temp2 = darr[i];        /* Read darr[i] */
        darr[i] = darr[i-1] * 2.0;     /* Write darr[i] after read */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - Multiple writes */
        farr[i] = farr[i-1] + 0.1f;    /* First write to farr[i] */
        farr[i] = farr[i] * 1.5f;      /* Second write to farr[i] */
        
        /* 4. MIXED DATA TYPES with dependencies */
        /* Integer to float conversion dependency */
        float ftemp = (float)arr[i];
        fsum += ftemp;
        
        /* Float to double conversion dependency */
        dsum += (double)ftemp;
        
        /* 5. POINTER ALIASING for memory dependencies */
        int* ptr1 = &arr[i];
        int* ptr2 = &arr[i-1] + 1;  /* Could alias with &arr[i] */
        *ptr1 = *ptr1 + *ptr2;      /* Potential memory dependency */
        
        /* 6. LOOP-CARRIED DEPENDENCY with different latencies */
        /* Integer add (low latency) */
        sum += arr[i];
        
        /* Floating multiply (higher latency) */
        dsum = dsum * 1.0001;
        
        /* 7. CONTROL DEPENDENCY (via conditional) */
        if (arr[i] > 100) {
            fsum -= 1.0f;           /* Control-dependent operation */
        }
    }
    
    /* Combine results to prevent elimination */
    volatile int final_sum = sum + (int)dsum + (int)fsum;
    return final_sum;
}

/* Another function with nested loops for more complex DDG */
__attribute__((noinline, noclone))
int nested_loop_test(int* matrix, int n) {
    int i, j;
    int total = 0;
    
    /* Nested loop with stride access pattern */
    for (i = 1; i < n; i++) {
        for (j = 1; j < n; j++) {
            /* 2D array access with multiple dependencies */
            int idx = i * n + j;
            
            /* True dependency in both dimensions */
            matrix[idx] = matrix[idx - 1] + matrix[idx - n];
            
            /* Anti-dependency with temporary */
            int old_val = matrix[idx];
            matrix[idx] = old_val * 2;
            
            /* Output dependency */
            matrix[idx] = matrix[idx] + i + j;
            matrix[idx] = matrix[idx] - 1;
            
            total += matrix[idx];
        }
    }
    
    return total;
}

/* Function with pointer chasing for complex memory deps */
__attribute__((noinline, noclone))
int pointer_chase(int* data, int size) {
    int* current = &data[0];
    int sum = 0;
    
    for (int i = 0; i < size - 1; i++) {
        /* Pointer-based dependency chain */
        int* next = current + 1;
        *next = *current + i;
        current = next;
        sum += *current;
    }
    
    return sum;
}

int main() {
    const int SIZE = 256;
    
    /* Allocate arrays with different data types */
    int* int_arr = (int*)malloc(SIZE * sizeof(int));
    double* double_arr = (double*)malloc(SIZE * sizeof(double));
    float* float_arr = (float*)malloc(SIZE * sizeof(float));
    int* matrix = (int*)malloc(SIZE * SIZE * sizeof(int));
    
    if (!int_arr || !double_arr || !float_arr || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Call functions to trigger DDG construction */
    int result1 = compute_loop(int_arr, double_arr, float_arr, SIZE);
    
    /* Initialize matrix for nested loop */
    for (int i = 0; i < SIZE * SIZE; i++) {
        matrix[i] = i % 100;
    }
    int result2 = nested_loop_test(matrix, 16);  /* Use smaller size for matrix */
    
    int result3 = pointer_chase(int_arr, SIZE);
    
    /* Use results to prevent dead code elimination */
    volatile int final_result = result1 + result2 + result3;
    
    printf("Result: %d\n", final_result);
    
    /* Cleanup */
    free(int_arr);
    free(double_arr);
    free(float_arr);
    free(matrix);
    
    return 0;
}
