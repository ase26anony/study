/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */

#include <stdio.h>
#include <stdlib.h>

/* Non-inlined function containing the complex dependency loop */
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
    
    /* Complex loop with multiple dependency types */
    for (i = 1; i < size; i++) {
        /* 1. TRUE DEPENDENCY (RAW) - Integer chain */
        int temp1 = arr[i-1];          /* Read arr[i-1] */
        arr[i] = temp1 + i;            /* Write arr[i] depends on arr[i-1] */
        
        /* 2. ANTI-DEPENDENCY (WAR) - Reusing same array location */
        double temp2 = darr[i];        /* Read darr[i] */
        darr[i] = darr[i-1] * 2.0;     /* Write darr[i] after read (WAR) */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - Multiple writes */
        farr[i] = farr[i-1] + 1.0f;    /* First write to farr[i] */
        farr[i] = farr[i] * 1.5f;      /* Second write to farr[i] (WAW) */
        
        /* 4. Memory aliasing with pointers (ambiguous dependencies) */
        int* ptr1 = &arr[i];
        int* ptr2 = &arr[(i * 7) % size];  /* Could alias with arr[i] */
        *ptr1 = *ptr1 + *ptr2;             /* Potential memory dependency */
        
        /* 5. Mixed data type operations with different latencies */
        sum += arr[i];                     /* Integer add (low latency) */
        dsum += darr[i] * 3.14159;         /* FP multiply (higher latency) */
        fsum = fsum + farr[i] / 2.0f;      /* FP divide (high latency) */
        
        /* 6. Control flow to create control dependencies */
        if (sum > 1000) {
            dsum = dsum * 0.9;             /* Control-dependent operation */
        }
    }
    
    /* Combine results to prevent dead code elimination */
    volatile int final_result = (int)(sum + dsum + fsum);
    return final_result;
}

/* Another function with nested loops for more complex DDG */
__attribute__((noinline, noclone))
int nested_loop_compute(int* matrix, int rows, int cols) {
    int i, j;
    int total = 0;
    
    /* Nested loops with carried dependencies in both dimensions */
    for (i = 1; i < rows; i++) {
        for (j = 1; j < cols; j++) {
            /* 2D true dependency - depends on previous row and column */
            int prev_row = matrix[(i-1) * cols + j];
            int prev_col = matrix[i * cols + (j-1)];
            
            /* Output dependency with multiple writes */
            matrix[i * cols + j] = prev_row + prev_col;
            matrix[i * cols + j] = matrix[i * cols + j] * 2;
            
            /* Anti-dependency through temporary */
            int temp = matrix[i * cols + j];
            matrix[i * cols + j] = temp + i + j;
            
            total += matrix[i * cols + j];
        }
    }
    
    return total;
}

int main(void) {
    const int SIZE = 256;
    int* int_array = (int*)malloc(SIZE * sizeof(int));
    double* double_array = (double*)malloc(SIZE * sizeof(double));
    float* float_array = (float*)malloc(SIZE * sizeof(float));
    
    /* Matrix for nested loop */
    const int ROWS = 64;
    const int COLS = 64;
    int* matrix = (int*)malloc(ROWS * COLS * sizeof(int));
    
    if (!int_array || !double_array || !float_array || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Call the function with complex dependencies */
    int result1 = compute_loop(int_array, double_array, float_array, SIZE);
    
    /* Call nested loop function */
    int result2 = nested_loop_compute(matrix, ROWS, COLS);
    
    /* Use results to prevent optimization */
    volatile int final_output = result1 + result2;
    printf("Result: %d\n", final_output);
    
    /* Cleanup */
    free(int_array);
    free(double_array);
    free(float_array);
    free(matrix);
    
    return 0;
}
