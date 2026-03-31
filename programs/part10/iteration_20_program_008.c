/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */
#include <stdio.h>
#include <stdlib.h>

/* Non-inlined function containing complex dependency patterns */
__attribute__((noinline, noclone))
int compute_loop(int* arr, double* darr, float* farr, int size) {
    int i;
    int sum_int = 0;
    double prod_double = 1.0;
    float acc_float = 0.0f;
    
    /* Use volatile for specific variables to prevent over-optimization */
    volatile int barrier = 0;
    
    /* Complex loop with multiple carried dependencies */
    for (i = 1; i < size; i++) {
        /* ===== TRUE DATA DEPENDENCIES (RAW) ===== */
        /* Integer chain: true dependency across iterations */
        int temp1 = arr[i-1];          /* Read previous iteration's value */
        arr[i] = temp1 + i + barrier;  /* Write depends on previous read (RAW) */
        
        /* Floating-point chain with different data types */
        double dtemp = darr[i-1];      /* Read previous double */
        darr[i] = dtemp * 1.01;        /* Write depends on previous (RAW) */
        
        /* ===== ANTI-DEPENDENCIES (WAR) ===== */
        /* Reuse same array location: read after write in same iteration */
        float fread = farr[i];         /* Read current value */
        farr[i] = fread * 1.5f + i;    /* Write to same location (WAR) */
        
        /* ===== OUTPUT DEPENDENCIES (WAW) ===== */
        /* Multiple writes to same location */
        arr[i] = arr[i] * 2;           /* First write */
        arr[i] = arr[i] + 1;           /* Second write to same location (WAW) */
        
        /* ===== MEMORY ALIASING DEPENDENCIES ===== */
        /* Pointer aliasing creates ambiguous dependencies */
        int* ptr1 = &arr[i];
        int* ptr2 = &arr[(i * 7) % size];  /* May alias with ptr1 */
        *ptr1 = *ptr1 + *ptr2;             /* Potential memory dependency */
        
        /* ===== MIXED DATA TYPE OPERATIONS ===== */
        /* Operations with different latencies */
        sum_int += arr[i];                 /* Integer add (low latency) */
        prod_double *= darr[i];            /* FP multiply (higher latency) */
        acc_float += farr[i];              /* FP add (medium latency) */
        
        /* ===== CONTROL DEPENDENCY-LIKE PATTERN ===== */
        /* Conditional that creates data flow */
        if (arr[i] > 1000) {
            barrier = 1;                    /* Volatile write */
        }
        
        /* ===== POINTER ARITHMETIC DEPENDENCIES ===== */
        /* Pointer-based computation chain */
        int* p = arr + i;
        *p = *p + *(p-1);                   /* Depends on previous element */
    }
    
    /* Combine results to prevent dead code elimination */
    return sum_int + (int)prod_double + (int)acc_float;
}

/* Another function with nested loops for additional DDG complexity */
__attribute__((noinline, noclone))
void nested_loop_deps(int* matrix, int rows, int cols) {
    int i, j;
    
    /* Nested loops with carried dependencies in both dimensions */
    for (i = 1; i < rows; i++) {
        for (j = 1; j < cols; j++) {
            /* 2D stencil computation with multiple dependencies */
            int up = matrix[(i-1)*cols + j];      /* RAW: previous row */
            int left = matrix[i*cols + (j-1)];    /* RAW: previous column */
            int diag = matrix[(i-1)*cols + (j-1)]; /* RAW: diagonal */
            
            /* Complex computation with all dependencies */
            matrix[i*cols + j] = (up + left - diag) * 2 + i + j;
            
            /* Anti-dependency within same iteration */
            int temp = matrix[i*cols + j];
            matrix[i*cols + j] = temp % 256;
        }
    }
}

int main() {
    const int SIZE = 256;
    const int ROWS = 64;
    const int COLS = 64;
    
    /* Allocate and initialize arrays with different data types */
    int* int_arr = (int*)malloc(SIZE * sizeof(int));
    double* double_arr = (double*)malloc(SIZE * sizeof(double));
    float* float_arr = (float*)malloc(SIZE * sizeof(float));
    int* matrix = (int*)malloc(ROWS * COLS * sizeof(int));
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        int_arr[i] = i + 1;
        double_arr[i] = (double)(i + 1) / 2.0;
        float_arr[i] = (float)(i + 1) * 0.5f;
    }
    
    for (int i = 0; i < ROWS * COLS; i++) {
        matrix[i] = i % 100;
    }
    
    /* Execute loops to trigger DDG construction */
    int result1 = compute_loop(int_arr, double_arr, float_arr, SIZE);
    nested_loop_deps(matrix, ROWS, COLS);
    
    /* Compute verification value to prevent optimization */
    int verify = 0;
    for (int i = 0; i < SIZE; i++) {
        verify += int_arr[i] + (int)double_arr[i] + (int)float_arr[i];
    }
    
    for (int i = 0; i < ROWS * COLS; i++) {
        verify += matrix[i];
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d, Verification: %d\n", result1, verify);
    
    /* Cleanup */
    free(int_arr);
    free(double_arr);
    free(float_arr);
    free(matrix);
    
    return 0;
}
