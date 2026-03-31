/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */
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
    int sum = 0;
    
    /* Initialize pointers for aliasing */
    ptr = arr;
    
    /* Complex loop with multiple dependency types */
    for (i = 1; i < size; i++) {
        /* 1. TRUE DEPENDENCY (RAW) - Integer chain */
        arr[i] = arr[i-1] + i;           /* RAW: read arr[i-1], write arr[i] */
        
        /* 2. ANTI-DEPENDENCY (WAR) - Reuse of same location */
        temp_int = arr[i];               /* WAR: read arr[i] before next write */
        arr[i] = temp_int * 2;           /* WAR: write arr[i] after read */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - Multiple writes */
        arr[i] = arr[i] + 1;             /* WAW: write arr[i] again */
        
        /* 4. Memory aliasing dependencies */
        *ptr = arr[i] + *ptr;            /* Potential memory dependency via ptr */
        ptr = &arr[i];                   /* Change pointer target */
        
        /* 5. Floating-point dependencies with different latencies */
        temp_double = darr[i-1] * 3.14159; /* FP multiply has latency */
        darr[i] = temp_double + darr[i];   /* FP add has latency */
        
        /* 6. Mixed-type dependencies */
        temp_float = (float)darr[i] + farr[i-1];
        farr[i] = temp_float * 2.0f;
        
        /* 7. Pointer arithmetic with dependency */
        int* arr_ptr = arr + i;
        *arr_ptr = *arr_ptr + *(arr_ptr - 1);
        
        /* 8. Control dependency simulation */
        if (arr[i] > 1000) {
            arr[i] = arr[i] % 1000;
        }
        
        /* Accumulate result to prevent dead code elimination */
        sum += arr[i] + (int)darr[i] + (int)farr[i];
    }
    
    return sum;
}

/* Another function with nested loops for more complex DDG */
__attribute__((noinline, noclone))
void nested_loop_deps(int* matrix, int rows, int cols) {
    int i, j;
    
    /* Nested loops with carried dependencies in both dimensions */
    for (i = 1; i < rows; i++) {
        for (j = 1; j < cols; j++) {
            /* 2D stencil computation with multiple dependencies */
            int up = matrix[(i-1)*cols + j];     /* RAW: read previous row */
            int left = matrix[i*cols + (j-1)];   /* RAW: read previous column */
            int diag = matrix[(i-1)*cols + (j-1)]; /* RAW: read diagonal */
            
            /* Multiple writes to same location */
            matrix[i*cols + j] = up + left;      /* WAW: first write */
            matrix[i*cols + j] = matrix[i*cols + j] - diag; /* WAW: second write */
            
            /* Anti-dependency through temporary */
            int temp = matrix[i*cols + j];       /* WAR: read after write */
            matrix[i*cols + j] = temp * 2;       /* WAR: write after read */
        }
    }
}

int main() {
    const int SIZE = 1024;
    const int ROWS = 64;
    const int COLS = 64;
    
    /* Allocate and initialize arrays with different data types */
    int* int_array = (int*)malloc(SIZE * sizeof(int));
    double* double_array = (double*)malloc(SIZE * sizeof(double));
    float* float_array = (float*)malloc(SIZE * sizeof(float));
    int* matrix = (int*)malloc(ROWS * COLS * sizeof(int));
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i % 100;
        double_array[i] = (double)i / 10.0;
        float_array[i] = (float)i / 5.0f;
    }
    
    for (int i = 0; i < ROWS * COLS; i++) {
        matrix[i] = i % 50;
    }
    
    /* Force computation to prevent optimization */
    volatile int result1 = compute_loop(int_array, double_array, float_array, SIZE);
    nested_loop_deps(matrix, ROWS, COLS);
    
    /* Use results to prevent dead code elimination */
    volatile int result2 = matrix[ROWS*COLS - 1];
    
    printf("Result 1: %d\n", result1);
    printf("Result 2: %d\n", result2);
    
    /* Cleanup */
    free(int_array);
    free(double_array);
    free(float_array);
    free(matrix);
    
    return 0;
}
