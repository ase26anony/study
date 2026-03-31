/* test_ddg.c - Program to trigger DDG edge creation in GCC scheduler */

#include <stdio.h>
#include <stdlib.h>

/* Non-inlined function containing complex dependency loops */
__attribute__((noinline, noclone))
double compute_loop(double* arr_d, int* arr_i, float* arr_f, int size) {
    volatile double sum = 0.0;  /* Prevent optimization of final result */
    
    /* Loop with multiple carried dependencies */
    for (int i = 1; i < size; i++) {
        /* True Data Dependency (RAW) - double precision */
        double temp_d = arr_d[i-1] * 1.5;      /* FP multiply has latency */
        arr_d[i] = temp_d + arr_d[i];          /* FP add has different latency */
        
        /* Anti-dependency (WAR) - integer */
        int temp_i = arr_i[i];                 /* Read before write */
        arr_i[i] = temp_i * 2 + i;             /* Integer arithmetic */
        
        /* Output Dependency (WAW) - float */
        arr_f[i] = (float)i * 0.5f;            /* First write */
        arr_f[i] = arr_f[i] * 2.0f;            /* Second write to same location */
        
        /* Memory dependency through pointer aliasing */
        double* ptr = &arr_d[i];
        *ptr = *ptr * (*ptr - 1.0);            /* Complex FP operation */
        
        /* Mixed-type dependency chain */
        sum += (double)arr_i[i] + arr_d[i] + (double)arr_f[i];
    }
    
    /* Additional loop with different patterns */
    for (int i = size-2; i >= 0; i--) {
        /* Reverse carried dependency */
        arr_d[i] = arr_d[i+1] * 0.9 + (double)arr_i[i];
        
        /* Memory dependency with offset */
        arr_i[i] = arr_i[i+1] + arr_i[i];
        
        /* Control-like dependency through conditional */
        if (arr_f[i] > 0.0f) {
            arr_f[i] = arr_f[i] * arr_f[i];
        }
        
        sum -= arr_d[i];
    }
    
    return sum;
}

/* Another function with nested loops */
__attribute__((noinline, noclone))
int nested_deps(int* matrix, int n) {
    int total = 0;
    
    /* Nested loops with carried dependencies in both dimensions */
    for (int i = 1; i < n; i++) {
        for (int j = 1; j < n; j++) {
            /* 2D carried dependencies */
            int idx = i * n + j;
            int idx_up = (i-1) * n + j;
            int idx_left = i * n + (j-1);
            
            /* Multiple dependencies: from above and left */
            matrix[idx] = matrix[idx_up] + matrix[idx_left] * 2;
            
            /* Anti-dependency with computation reuse */
            int temp = matrix[idx];
            matrix[idx] = (temp * 3) / 2;
            
            total += matrix[idx];
        }
    }
    
    return total;
}

int main(void) {
    const int SIZE = 256;
    
    /* Allocate and initialize arrays with different data types */
    double* arr_d = (double*)malloc(SIZE * sizeof(double));
    int* arr_i = (int*)malloc(SIZE * sizeof(int));
    float* arr_f = (float*)malloc(SIZE * sizeof(float));
    int* matrix = (int*)malloc(SIZE * SIZE * sizeof(int));
    
    if (!arr_d || !arr_i || !arr_f || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-zero values to avoid constant propagation */
    for (int i = 0; i < SIZE; i++) {
        arr_d[i] = (double)(i % 10) + 0.5;
        arr_i[i] = i * 2;
        arr_f[i] = (float)(i % 5) * 0.7f;
    }
    
    for (int i = 0; i < SIZE * SIZE; i++) {
        matrix[i] = i % 7;
    }
    
    /* Call functions with complex dependency patterns */
    double result1 = compute_loop(arr_d, arr_i, arr_f, SIZE);
    int result2 = nested_deps(matrix, 16);  /* Use smaller size for matrix */
    
    /* Use results to prevent dead code elimination */
    volatile double final_result = result1 + (double)result2;
    
    /* Print something to ensure execution */
    printf("Results: %f (double loop), %d (nested)\n", result1, result2);
    printf("Final: %f\n", (double)final_result);
    
    /* Cleanup */
    free(arr_d);
    free(arr_i);
    free(arr_f);
    free(matrix);
    
    return 0;
}
