/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */
#include <stdio.h>
#include <stdlib.h>

/* Non-inlined function containing complex dependency loops */
__attribute__((noinline, noclone))
int compute_loop(int* arr, float* farr, double* darr, int size) {
    int i;
    int sum_int = 0;
    float sum_float = 0.0f;
    double sum_double = 0.0;
    int temp1, temp2;
    float ftemp1, ftemp2;
    double dtemp1, dtemp2;
    
    /* Complex loop with multiple carried dependencies */
    for (i = 1; i < size; i++) {
        /* ========== TRUE DATA DEPENDENCIES (RAW) ========== */
        /* Integer RAW: array[i] depends on array[i-1] */
        arr[i] = arr[i-1] + i * 2;          /* Line 1 */
        
        /* Floating-point RAW with different latency operations */
        ftemp1 = farr[i-1] * 1.5f;          /* Line 2 - FP multiply */
        farr[i] = ftemp1 + (float)i;        /* Line 3 - FP add */
        
        /* Double precision RAW with mixed operations */
        dtemp1 = darr[i-1] / 2.0;           /* Line 4 - FP divide (high latency) */
        darr[i] = dtemp1 * dtemp1;          /* Line 5 - FP multiply */
        
        /* ========== ANTI-DEPENDENCIES (WAR) ========== */
        /* Integer WAR: read then write to same location */
        temp1 = arr[i];                     /* Line 6 - Read arr[i] */
        arr[i] = temp1 + farr[i];           /* Line 7 - Write arr[i] (anti-dep on line 6) */
        
        /* Floating-point WAR */
        ftemp2 = farr[i];                   /* Line 8 - Read farr[i] */
        farr[i] = ftemp2 * 2.0f - 1.0f;     /* Line 9 - Write farr[i] */
        
        /* ========== OUTPUT DEPENDENCIES (WAW) ========== */
        /* Multiple writes to same location */
        darr[i] = (double)arr[i] * 0.5;     /* Line 10 - First write to darr[i] */
        darr[i] = darr[i] + 1.0;            /* Line 11 - Second write to darr[i] (output dep) */
        
        /* ========== MEMORY ALIASING DEPENDENCIES ========== */
        /* Pointer aliasing creates ambiguous dependencies */
        int* ptr1 = &arr[i];
        int* ptr2 = &arr[size - i - 1];
        
        /* Compiler can't determine if these access the same memory */
        *ptr1 = *ptr1 + *ptr2;              /* Line 12 - Could create memory dep edge */
        
        /* ========== CONTROL DEPENDENCIES ========== */
        /* Conditional creates control flow dependencies */
        if (arr[i] > 1000) {                /* Line 13 - Control dep */
            arr[i] = arr[i] % 1000;         /* Line 14 - Data dep on line 13 */
        }
        
        /* ========== ACCUMULATORS (PREVENT DEAD CODE) ========== */
        sum_int += arr[i];
        sum_float += farr[i];
        sum_double += darr[i];
        
        /* ========== CROSS-DATA TYPE DEPENDENCIES ========== */
        /* Mixed type operations create complex dependencies */
        temp2 = (int)farr[i];               /* Line 15 - FP to int conversion */
        arr[i] = arr[i] ^ temp2;            /* Line 16 - Integer XOR */
    }
    
    /* Final computation to ensure all dependencies matter */
    volatile int final_result = 0;
    final_result = sum_int + (int)sum_float + (int)sum_double;
    
    return final_result;
}

/* Another loop with different patterns */
__attribute__((noinline, noclone))
void nested_loop_deps(int* A, int* B, int* C, int n) {
    int i, j;
    
    /* Nested loop with carried dependencies in both dimensions */
    for (i = 1; i < n; i++) {
        for (j = 1; j < n; j++) {
            /* 2D stencil computation - multiple dependencies */
            int up = A[(i-1)*n + j];        /* RAW on previous row */
            int left = A[i*n + (j-1)];      /* RAW on previous column */
            int diag = A[(i-1)*n + (j-1)];  /* RAW on diagonal */
            
            /* Complex computation with multiple dependencies */
            B[i*n + j] = up + left - diag + i * j;
            
            /* Anti-dependency: read then modify */
            int temp = B[i*n + j];
            C[i*n + j] = temp * 2;
            B[i*n + j] = C[i*n + j] + 1;    /* WAR on B */
            
            /* Output dependency on C */
            C[i*n + j] = C[i*n + j] % 256;  /* WAW on C */
        }
    }
}

int main() {
    const int SIZE = 256;
    const int N = 64;
    
    /* Allocate and initialize arrays */
    int* arr_int = (int*)malloc(SIZE * sizeof(int));
    float* arr_float = (float*)malloc(SIZE * sizeof(float));
    double* arr_double = (double*)malloc(SIZE * sizeof(double));
    
    int* A = (int*)malloc(N * N * sizeof(int));
    int* B = (int*)malloc(N * N * sizeof(int));
    int* C = (int*)malloc(N * N * sizeof(int));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < SIZE; i++) {
        arr_int[i] = i + 1;
        arr_float[i] = (float)i * 0.5f;
        arr_double[i] = (double)i * 0.25;
    }
    
    for (int i = 0; i < N * N; i++) {
        A[i] = i % 100;
        B[i] = 0;
        C[i] = 0;
    }
    
    /* Call functions with complex dependency patterns */
    int result1 = compute_loop(arr_int, arr_float, arr_double, SIZE);
    nested_loop_deps(A, B, C, N);
    
    /* Use results to prevent optimization */
    volatile int final_check = result1 + A[N*N-1] + B[N*N-1] + C[N*N-1];
    
    printf("Result: %d\n", final_check);
    
    /* Cleanup */
    free(arr_int);
    free(arr_float);
    free(arr_double);
    free(A);
    free(B);
    free(C);
    
    return 0;
}
