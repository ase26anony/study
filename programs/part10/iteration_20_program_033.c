/* test_ddg.c - Program to trigger DDG edge creation in GCC scheduler */
#include <stdio.h>
#include <stdlib.h>

/* Non-inlined function to ensure loop body is analyzed for DDG */
__attribute__((noinline, noclone))
int compute_loop(int* arr, int* brr, float* farr, double* darr, int n) {
    int i;
    int sum = 0;
    float ftemp = 1.0f;
    double dtemp = 1.0;
    int* ptr = arr;
    
    /* Complex loop with multiple dependency types */
    for (i = 1; i < n; i++) {
        /* 1. TRUE DEPENDENCY (RAW) - Integer chain */
        int temp = arr[i-1];          /* Read arr[i-1] */
        arr[i] = temp + i;            /* Write arr[i] depends on temp (RAW) */
        
        /* 2. ANTI-DEPENDENCY (WAR) - Reusing same array location */
        int anti_temp = brr[i];       /* Read brr[i] */
        brr[i] = anti_temp * 2;       /* Write brr[i] after read (WAR) */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - Multiple writes to same location */
        farr[i] = ftemp * 0.5f;       /* First write to farr[i] */
        farr[i] = farr[i] * 2.0f;     /* Second write to farr[i] (WAW) */
        
        /* 4. MEMORY DEPENDENCY with pointer aliasing */
        *ptr = *ptr + 1;              /* ptr may alias arr (ambiguous dependency) */
        ptr = &arr[i];                /* Change pointer target */
        
        /* 5. MIXED DATA TYPES with different latencies */
        dtemp = darr[i] * 3.14159;    /* FP multiply (higher latency) */
        darr[i] = dtemp + darr[i-1];  /* FP add with carried dependency */
        
        /* 6. CONTROL DEPENDENCY created by conditional */
        if (arr[i] > 100) {
            brr[i] = brr[i] / 2;      /* Creates control flow */
        }
        
        /* 7. ACCUMULATOR with loop-carried dependency */
        sum += arr[i] + brr[i];       /* sum has loop-carried dependency */
        
        /* 8. FLOATING-POINT chain with different operations */
        ftemp = farr[i] * ftemp;      /* FP multiplication chain */
    }
    
    /* Final computation using all dependencies */
    return sum + (int)ftemp + (int)dtemp;
}

/* Another function with nested loops for more complex DDG */
__attribute__((noinline, noclone))
void nested_loop_deps(int* matrix, int rows, int cols) {
    int i, j;
    
    /* Nested loop with stride access patterns */
    for (i = 1; i < rows; i++) {
        for (j = 1; j < cols; j++) {
            /* 2D stencil computation with multiple dependencies */
            int up = matrix[(i-1)*cols + j];      /* RAW dependency */
            int left = matrix[i*cols + (j-1)];    /* RAW dependency */
            int diag = matrix[(i-1)*cols + (j-1)]; /* RAW dependency */
            
            /* Multiple writes creating WAW and WAR */
            int temp = matrix[i*cols + j];
            matrix[i*cols + j] = (up + left + diag) / 3;
            
            /* Anti-dependency through temp variable */
            matrix[(i-1)*cols + j] = temp;        /* WAR */
        }
    }
}

int main() {
    const int SIZE = 256;
    const int ROWS = 64;
    const int COLS = 64;
    
    /* Allocate and initialize arrays with different data types */
    int* arr = (int*)malloc(SIZE * sizeof(int));
    int* brr = (int*)malloc(SIZE * sizeof(int));
    float* farr = (float*)malloc(SIZE * sizeof(float));
    double* darr = (double*)malloc(SIZE * sizeof(double));
    int* matrix = (int*)malloc(ROWS * COLS * sizeof(int));
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i % 100;
        brr[i] = i % 50;
        farr[i] = (float)i * 0.1f;
        darr[i] = (double)i * 0.01;
    }
    
    for (int i = 0; i < ROWS * COLS; i++) {
        matrix[i] = i % 100;
    }
    
    /* Volatile to prevent optimization of results */
    volatile int result1, result2;
    
    /* Call functions with complex dependency patterns */
    result1 = compute_loop(arr, brr, farr, darr, SIZE);
    nested_loop_deps(matrix, ROWS, COLS);
    
    /* Compute checksum to ensure all iterations executed */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += arr[i] + brr[i];
    }
    
    result2 = checksum;
    
    /* Print results to prevent dead code elimination */
    printf("Result1: %d, Result2: %d\n", result1, result2);
    
    /* Clean up */
    free(arr);
    free(brr);
    free(farr);
    free(darr);
    free(matrix);
    
    return 0;
}
