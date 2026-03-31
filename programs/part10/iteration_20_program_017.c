/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */
#include <stdio.h>
#include <stdlib.h>

/* Non-inlined function containing complex dependency loops */
__attribute__((noinline, noclone))
int compute_loop(int* arr, int* brr, float* farr, double* darr, int n) {
    int i, sum = 0;
    float ftemp = 1.0f;
    double dtemp = 1.0;
    int* ptr = arr;
    
    /* Loop with multiple carried dependencies */
    for (i = 1; i < n; i++) {
        /* 1. True Data Dependency (RAW) with integer */
        int temp = arr[i-1];          /* Read arr[i-1] */
        arr[i] = temp + i + brr[i];   /* Write arr[i] depends on arr[i-1] */
        
        /* 2. Anti-dependency (WAR) with float */
        ftemp = farr[i];              /* Read farr[i] */
        farr[i] = ftemp * 2.5f + i;   /* Write farr[i] after read */
        
        /* 3. Output Dependency (WAW) with double */
        darr[i] = dtemp * 0.5;        /* First write to darr[i] */
        darr[i] = darr[i] * 3.0 + i;  /* Second write to same location */
        
        /* 4. Memory dependency through pointer aliasing */
        *ptr = *ptr + 1;              /* ptr may alias with arr/brr */
        ptr = &arr[i % 10];           /* Change pointer target */
        
        /* 5. Mixed-type operations creating complex dependencies */
        brr[i] = (int)(farr[i-1] * 2) + arr[i-1];
        
        /* 6. Loop-carried dependency with varying latency operations */
        dtemp = darr[i-1] * 1.618;    /* FP multiply with carried dependency */
        
        /* 7. Control dependency-like pattern */
        sum += (arr[i] > 0) ? brr[i] : -brr[i];
    }
    
    /* Final computation using all dependency chains */
    return sum + (int)ftemp + (int)dtemp + arr[n-1];
}

/* Another loop with nested dependencies */
__attribute__((noinline, noclone))
void nested_deps(int* matrix, int size) {
    int i, j;
    
    for (i = 1; i < size; i++) {
        for (j = 1; j < size; j++) {
            /* 2D stencil with multiple dependencies */
            int up = matrix[(i-1)*size + j];      /* RAW: read up */
            int left = matrix[i*size + (j-1)];    /* RAW: read left */
            int diag = matrix[(i-1)*size + (j-1)];/* RAW: read diagonal */
            
            /* Complex computation with all dependencies */
            matrix[i*size + j] = (up + left - diag) * 2 + i - j;
            
            /* Anti-dependency in same iteration */
            int temp = matrix[i*size + j];        /* WAR: read just written */
            matrix[i*size + j] = temp + (i * j);  /* WAR: write again */
        }
    }
}

/* Main function to drive the computation */
int main() {
    const int N = 512;
    const int M = 64;
    
    /* Allocate and initialize arrays with different data types */
    int* arr = (int*)malloc(N * sizeof(int));
    int* brr = (int*)malloc(N * sizeof(int));
    float* farr = (float*)malloc(N * sizeof(float));
    double* darr = (double*)malloc(N * sizeof(double));
    int* matrix = (int*)malloc(M * M * sizeof(int));
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr[i] = i % 100;
        brr[i] = (i * 3) % 100;
        farr[i] = i * 0.5f;
        darr[i] = i * 0.25;
    }
    
    for (int i = 0; i < M * M; i++) {
        matrix[i] = i % 50;
    }
    
    /* Volatile to prevent optimization of results */
    volatile int result1, result2;
    
    /* Execute loops with complex dependencies */
    result1 = compute_loop(arr, brr, farr, darr, N);
    
    /* Execute nested loop */
    nested_deps(matrix, M);
    
    /* Compute checksum from matrix */
    result2 = 0;
    for (int i = 0; i < M * M; i += M+1) {
        result2 += matrix[i];
    }
    
    /* Print results to prevent dead code elimination */
    printf("Result 1: %d\n", result1);
    printf("Result 2: %d\n", result2);
    printf("Final: %d\n", result1 + result2);
    
    /* Cleanup */
    free(arr);
    free(brr);
    free(farr);
    free(darr);
    free(matrix);
    
    return 0;
}
