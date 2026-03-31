/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure loop body remains intact for DDG analysis */
__attribute__((noinline, noclone))
int compute_loop(int* arr, int* brr, float* farr, double* darr, int n) {
    int sum = 0;
    float fsum = 0.0f;
    double dsum = 0.0;
    int* ptr = arr;
    
    /* Complex loop with multiple dependency types */
    for (int i = 1; i < n; i++) {
        /* 1. TRUE DEPENDENCY (RAW) - carried across iterations */
        int temp = arr[i-1];          /* Read arr[i-1] */
        arr[i] = temp + i;            /* Write arr[i] depends on arr[i-1] */
        
        /* 2. ANTI-DEPENDENCY (WAR) - reuse of same location */
        float ftemp = farr[i];        /* Read farr[i] */
        farr[i] = ftemp * 1.5f + i;   /* Write farr[i] after read */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - multiple writes */
        darr[i] = dsum * 0.5;         /* First write to darr[i] */
        darr[i] = darr[i] * 2.0;      /* Second write to same location */
        
        /* 4. MEMORY DEPENDENCY with pointer aliasing */
        *ptr = *ptr + 1;              /* ptr may alias with arr/brr */
        ptr = &arr[i % n];            /* Change pointer target */
        
        /* 5. MIXED DATA TYPES creating different latency edges */
        brr[i] = brr[i-1] * 2;        /* Integer multiplication */
        fsum += farr[i] * 0.3f;       /* Float multiplication */
        dsum = darr[i] / 1.7;         /* Double division */
        
        /* 6. CONTROL DEPENDENCY-like pattern */
        if (arr[i] > 100) {
            brr[i] = brr[i] / 2;      /* Creates additional dependencies */
        }
        
        /* 7. COMPLEX ADDRESSING with dependencies */
        int idx = (i + arr[i-1]) % n;
        arr[idx] = brr[i] + arr[idx];
        
        /* Update sums with cross-type dependencies */
        sum += arr[i] + brr[i];
        fsum = fsum + (float)arr[i];
        dsum = dsum * (1.0 + 0.01 * i);
    }
    
    /* Combine results to prevent dead code elimination */
    return sum + (int)fsum + (int)dsum + arr[0] + brr[0];
}

/* Another loop with different characteristics */
__attribute__((noinline, noclone))
void nested_loop_deps(int* a, int* b, int n) {
    /* Nested loop with carried dependencies */
    for (int i = 1; i < n; i++) {
        for (int j = 1; j < n; j++) {
            /* 2D true dependency */
            a[i*n + j] = a[(i-1)*n + j] + a[i*n + (j-1)];
            
            /* Anti-dependency in inner loop */
            int tmp = b[i*n + j];
            b[i*n + j] = tmp + a[i*n + j];
            
            /* Output dependency */
            a[i*n + j] = a[i*n + j] * 2;
            a[i*n + j] = a[i*n + j] - 1;
        }
    }
}

int main() {
    const int N = 512;
    
    /* Allocate and initialize arrays with different data types */
    int* arr = (int*)malloc(N * sizeof(int));
    int* brr = (int*)malloc(N * sizeof(int));
    float* farr = (float*)malloc(N * sizeof(float));
    double* darr = (double*)malloc(N * sizeof(double));
    
    /* Initialize with pattern */
    for (int i = 0; i < N; i++) {
        arr[i] = i % 37;
        brr[i] = i % 23;
        farr[i] = (float)i * 0.7f;
        darr[i] = (double)i * 1.3;
    }
    
    /* Volatile to ensure computation isn't optimized away */
    volatile int result;
    
    /* Call functions with dependency-heavy loops */
    result = compute_loop(arr, brr, farr, darr, N);
    
    /* Second loop for additional DDG coverage */
    nested_loop_deps(arr, brr, N/2);
    
    /* Use results to prevent elimination */
    printf("Result 1: %d\n", result);
    printf("Final values: arr[%d]=%d, brr[%d]=%d\n", 
           N-1, arr[N-1], N-1, brr[N-1]);
    
    free(arr);
    free(brr);
    free(farr);
    free(darr);
    
    return 0;
}
