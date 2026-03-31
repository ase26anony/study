/* test_ddg.c - Program to trigger DDG edge creation in GCC scheduler */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure loop body remains intact for DDG analysis */
__attribute__((noinline, noclone))
int compute_loop(int* arr, float* farr, double* darr, int size) {
    int i;
    int temp_int;
    float temp_float;
    double temp_double;
    int* ptr;
    int result = 0;
    
    /* Initialize arrays with some values */
    for (i = 0; i < size; i++) {
        arr[i] = i;
        farr[i] = i * 0.5f;
        darr[i] = i * 0.25;
    }
    
    /* 
     * Main loop with complex dependency patterns
     * This should create various DDG edges when compiled with -O2/-O3
     */
    for (i = 1; i < size; i++) {
        /* 1. TRUE DEPENDENCY (RAW) with integer */
        temp_int = arr[i - 1];          /* Read arr[i-1] */
        arr[i] = temp_int + i;          /* Write arr[i] depends on arr[i-1] */
        
        /* 2. ANTI-DEPENDENCY (WAR) with float */
        temp_float = farr[i];           /* Read farr[i] */
        farr[i] = temp_float * 1.1f;    /* Write farr[i] - anti-dep on previous read */
        
        /* 3. OUTPUT DEPENDENCY (WAW) with double */
        darr[i] = temp_int * 0.33;      /* First write to darr[i] */
        darr[i] = darr[i] * 1.5;        /* Second write to darr[i] - output dep */
        
        /* 4. MEMORY DEPENDENCY with pointer aliasing */
        ptr = &arr[i];
        *ptr = *ptr + arr[i - 1];       /* Pointer access creates memory dep */
        
        /* 5. MIXED DATA TYPE OPERATIONS with carried dependency */
        temp_double = (double)temp_int + (double)temp_float;
        arr[i] = arr[i] + (int)temp_double; /* Mixed type operation */
        
        /* 6. LOOP-CARRIED DEPENDENCY with accumulation */
        result += arr[i];               /* True dep through 'result' */
        
        /* 7. CONTROL DEPENDENCY (through condition) */
        if (result > 1000) {
            temp_int = arr[i] / 2;      /* Control-dependent operation */
        }
        
        /* 8. MEMORY DEP with array index calculation */
        int idx = i % 10;
        arr[idx] = arr[idx] + 1;        /* Potential memory dep if idx == i */
    }
    
    /* Additional loop with different dependency pattern */
    for (i = size - 2; i >= 0; i--) {
        /* Reverse carried dependency */
        arr[i] = arr[i + 1] - 1;        /* True dep in reverse direction */
        
        /* Complex floating point operation with latency */
        farr[i] = farr[i] * farr[i] + farr[i + 1];
        
        /* Nested dependency chain */
        temp_double = darr[i] * 0.75;
        darr[i] = temp_double + darr[i + 1];
        result += (int)darr[i];
    }
    
    return result;
}

/* Another function with nested loops for more complex DDG */
__attribute__((noinline, noclone))
int nested_loop_deps(int* mat, int n) {
    int i, j;
    int sum = 0;
    
    /* Nested loops with carried dependencies */
    for (i = 1; i < n; i++) {
        for (j = 1; j < n; j++) {
            /* 2D carried dependencies */
            mat[i*n + j] = mat[(i-1)*n + j] + mat[i*n + (j-1)];
            
            /* Anti-dependency in inner loop */
            int temp = mat[i*n + j];
            mat[i*n + j] = temp * 2;
            
            /* Accumulator with output dependency */
            sum = sum + mat[i*n + j];
            sum = sum - 1;  /* WAW on sum */
        }
    }
    
    return sum;
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
    volatile int result2 = nested_loop_deps(matrix, 16);
    
    /* Use results to prevent optimization */
    printf("Result 1: %d\n", result1);
    printf("Result 2: %d\n", result2);
    
    /* Additional volatile store to ensure all writes happen */
    volatile int check = int_array[SIZE-1] + (int)float_array[SIZE-1];
    
    free(int_array);
    free(float_array);
    free(double_array);
    free(matrix);
    
    return 0;
}
