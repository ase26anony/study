/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */
#include <stdio.h>
#include <stdlib.h>

/* Non-inlined function to ensure loop body remains intact for DDG analysis */
__attribute__((noinline, noclone))
int compute_loop(int* arr, int* brr, float* farr, double* darr, int size) {
    int i;
    int sum = 0;
    float ftemp = 1.0f;
    double dtemp = 1.0;
    int* ptr = arr;
    
    /* Complex loop with multiple dependency types */
    for (i = 1; i < size; i++) {
        /* 1. TRUE DEPENDENCY (RAW) - carried across iterations */
        int temp = arr[i-1];          /* Read arr[i-1] */
        arr[i] = temp + i + brr[i];   /* Write arr[i] depends on arr[i-1] */
        
        /* 2. ANTI-DEPENDENCY (WAR) - reuse of same array location */
        float fval = farr[i];         /* Read farr[i] */
        farr[i] = fval * ftemp + i;   /* Write farr[i] after read */
        ftemp = farr[i] * 0.5f;       /* New read of farr[i] */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - multiple writes to same location */
        darr[i] = dtemp * i;          /* First write to darr[i] */
        dtemp = darr[i] + 1.0;        /* Read darr[i] */
        darr[i] = dtemp * 2.0;        /* Second write to darr[i] - WAW with first */
        
        /* 4. MEMORY DEPENDENCY with pointer aliasing */
        *ptr = *ptr + 1;              /* ptr points to arr, creates memory dep */
        ptr = &arr[i % 4];            /* Change pointer target */
        
        /* 5. MIXED DATA TYPES in dependency chain */
        brr[i] = (int)(farr[i]) + (int)(darr[i]) + arr[i];
        
        /* 6. ACCUMULATOR with loop-carried dependency */
        sum = sum + arr[i] + brr[i];  /* sum has true dependency across iterations */
        
        /* 7. CONTROL DEPENDENCY-like pattern */
        if (sum > 1000) {
            brr[i] = brr[i] / 2;      /* Creates additional data flow */
        }
    }
    
    /* Final computation with all dependencies */
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
            int temp = matrix[i*cols + j];        /* Read current - WAR */
            matrix[i*cols + j] = (up + left + diag) / 3; /* Write current */
            
            /* Cross-iteration dependency through temp variable */
            matrix[(i-1)*cols + (j-1)] = temp + diag; /* Write to previous */
        }
    }
}

int main() {
    const int SIZE = 1024;
    int i;
    
    /* Allocate and initialize arrays with different data types */
    int* arr = (int*)malloc(SIZE * sizeof(int));
    int* brr = (int*)malloc(SIZE * sizeof(int));
    float* farr = (float*)malloc(SIZE * sizeof(float));
    double* darr = (double*)malloc(SIZE * sizeof(double));
    int* matrix = (int*)malloc(SIZE * SIZE * sizeof(int));
    
    /* Initialize arrays */
    for (i = 0; i < SIZE; i++) {
        arr[i] = i % 100;
        brr[i] = (i * 2) % 100;
        farr[i] = (float)i * 0.5f;
        darr[i] = (double)i * 0.25;
    }
    
    for (i = 0; i < SIZE * SIZE; i++) {
        matrix[i] = i % 256;
    }
    
    /* Volatile to prevent optimization of results */
    volatile int result1, result2;
    
    /* Execute loops to trigger DDG construction */
    result1 = compute_loop(arr, brr, farr, darr, SIZE);
    
    /* Execute nested loop */
    nested_loop_deps(matrix, 32, 32);
    
    /* Compute checksum to ensure all loops executed */
    result2 = 0;
    for (i = 0; i < SIZE; i++) {
        result2 += arr[i] + brr[i];
    }
    
    /* Print results to prevent dead code elimination */
    printf("Result 1: %d\n", result1);
    printf("Result 2: %d\n", result2);
    printf("Sample values: arr[10]=%d, farr[20]=%.2f\n", arr[10], farr[20]);
    
    free(arr);
    free(brr);
    free(farr);
    free(darr);
    free(matrix);
    
    return 0;
}
