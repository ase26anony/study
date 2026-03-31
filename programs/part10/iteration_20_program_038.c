/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure DDG is built for this function */
__attribute__((noinline, noclone))
int compute_loop(int* arr, float* farr, double* darr, int size) {
    int i;
    int temp_int;
    float temp_float;
    double temp_double;
    int* ptr;
    int result = 0;
    
    /* Initialize arrays */
    for (i = 0; i < size; i++) {
        arr[i] = i;
        farr[i] = i * 0.5f;
        darr[i] = i * 0.25;
    }
    
    /* Main loop with complex dependency patterns */
    for (i = 1; i < size - 1; i++) {
        /* 1. TRUE DEPENDENCY (RAW) - Integer chain */
        arr[i] = arr[i-1] + i * 2;          /* Read arr[i-1], write arr[i] */
        
        /* 2. ANTI-DEPENDENCY (WAR) - Reuse of same location */
        temp_int = arr[i];                  /* Read arr[i] */
        arr[i] = temp_int * 3 - i;          /* Write arr[i] - anti-dep with previous read */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - Multiple writes to same location */
        arr[i] = arr[i] / 2;                /* Another write to arr[i] - output dep */
        
        /* 4. TRUE DEPENDENCY with different data type - Float */
        temp_float = farr[i-1];             /* Read farr[i-1] */
        farr[i] = temp_float * 1.5f + i;    /* Write farr[i] - float RAW */
        
        /* 5. ANTI-DEPENDENCY with float */
        temp_float = farr[i];               /* Read farr[i] */
        farr[i] = temp_float - 0.5f;        /* Write farr[i] - float WAR */
        
        /* 6. TRUE DEPENDENCY with double - higher latency operation */
        temp_double = darr[i-1] * darr[i-2]; /* Read two previous, multiply */
        darr[i] = temp_double + 0.1;        /* Write darr[i] - double RAW with multiply */
        
        /* 7. Pointer aliasing creating memory dependencies */
        ptr = &arr[i % 16];                 /* Pointer with modulo - creates aliasing */
        *ptr = *ptr + 1;                    /* Read-modify-write through pointer */
        
        /* 8. Mixed-type dependency chain */
        arr[i] = arr[i] + (int)farr[i];     /* Integer + float cast dependency */
        
        /* 9. Control dependency-like pattern */
        if (arr[i] > 100) {                 /* Conditional creates control flow */
            farr[i] = farr[i] * 2.0f;
        }
        
        /* 10. Accumulator with loop-carried dependency */
        result += arr[i];                   /* Loop-carried true dependency on result */
    }
    
    /* Final computation with all dependencies */
    result = result + (int)farr[size-2] + (int)darr[size-2];
    return result;
}

/* Another function with nested loops for more complex DDG */
__attribute__((noinline, noclone))
void nested_loop_deps(int* matrix, int n) {
    int i, j;
    
    /* Nested loops with multiple dependency patterns */
    for (i = 1; i < n; i++) {
        for (j = 1; j < n; j++) {
            /* True dependency in both dimensions */
            matrix[i*n + j] = matrix[(i-1)*n + j] + matrix[i*n + (j-1)];
            
            /* Anti-dependency pattern */
            int temp = matrix[i*n + j];
            matrix[i*n + j] = temp * matrix[(i-1)*n + (j-1)];
            
            /* Output dependency */
            matrix[i*n + j] = matrix[i*n + j] % 256;
        }
    }
}

int main() {
    const int SIZE = 256;
    int* int_array = (int*)malloc(SIZE * sizeof(int));
    float* float_array = (float*)malloc(SIZE * sizeof(float));
    double* double_array = (double*)malloc(SIZE * sizeof(double));
    int* matrix = (int*)malloc(SIZE * SIZE * sizeof(int));
    
    if (!int_array || !float_array || !double_array || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Force computation to prevent dead code elimination */
    volatile int result1 = compute_loop(int_array, float_array, double_array, SIZE);
    nested_loop_deps(matrix, 16);
    
    /* Use results to prevent optimization */
    printf("Result 1: %d\n", result1);
    printf("Matrix[100]: %d\n", matrix[100]);
    
    /* Additional volatile store to ensure all computations complete */
    volatile int final_check = int_array[SIZE-2] + (int)float_array[SIZE-2];
    printf("Final check: %d\n", final_check);
    
    free(int_array);
    free(float_array);
    free(double_array);
    free(matrix);
    
    return 0;
}
