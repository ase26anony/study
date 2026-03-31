/* test_ddg.c - Program to trigger DDG edge creation in GCC scheduler */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure loop body remains intact for DDG analysis */
__attribute__((noinline, noclone))
int compute_loop(int* arr, double* darr, float* farr, int size) {
    int i;
    int temp_int;
    double temp_double;
    float temp_float;
    int* ptr = arr;
    int sum = 0;
    
    /* Complex loop with multiple dependency types */
    for (i = 1; i < size; i++) {
        /* 1. TRUE DEPENDENCY (RAW) - Integer chain */
        arr[i] = arr[i-1] + i * 2;           /* True dep: arr[i-1] -> arr[i] */
        
        /* 2. ANTI-DEPENDENCY (WAR) - Reuse of same location */
        temp_int = arr[i];                   /* Read arr[i] */
        arr[i] = temp_int * 3 - i;           /* Write arr[i] - anti-dep with previous read */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - Multiple writes */
        arr[i] = arr[i] + ptr[i % 8];        /* First write */
        if (i % 3 == 0) {
            arr[i] = arr[i] * 2;             /* Second write to same location - output dep */
        }
        
        /* 4. MEMORY DEPENDENCY with pointer aliasing */
        ptr = &arr[i & 7];                   /* Pointer may alias with arr */
        *ptr = *ptr + 1;                     /* Creates ambiguous memory dependency */
        
        /* 5. FLOATING POINT dependencies - different data type */
        darr[i] = darr[i-1] * 1.5 + i;       /* True dep: darr[i-1] -> darr[i] */
        temp_double = darr[i];
        darr[i] = temp_double / 2.0;         /* Anti-dep */
        
        /* 6. FLOAT dependencies - another data type */
        farr[i] = farr[i-1] + temp_float;    /* True dep with float */
        temp_float = farr[i] * 0.5f;
        farr[i] = temp_float + i * 0.1f;     /* Anti-dep */
        
        /* 7. MIXED TYPE dependencies causing different latencies */
        sum += arr[i] + (int)darr[i] + (int)farr[i]; /* Combines int, double, float */
        
        /* 8. CONTROL DEPENDENCY - conditional creates control flow */
        if (sum > 1000) {
            sum = sum / 2;                   /* Control-dependent operation */
        }
        
        /* 9. COMPLEX ADDRESSING with potential dependencies */
        arr[(i + arr[i-1]) % size] = sum;    /* Address depends on previous computation */
    }
    
    return sum;
}

/* Another function with nested loops for more complex DDG */
__attribute__((noinline, noclone))
int nested_loop_compute(int* matrix, int n) {
    int i, j;
    int total = 0;
    volatile int sink; /* Prevent optimization */
    
    /* Nested loop with carried dependencies */
    for (i = 1; i < n; i++) {
        for (j = 1; j < n; j++) {
            /* 2D stencil computation with multiple dependencies */
            matrix[i*n + j] = matrix[(i-1)*n + j] +      /* Vertical dep */
                             matrix[i*n + (j-1)] +      /* Horizontal dep */
                             matrix[(i-1)*n + (j-1)];   /* Diagonal dep */
            
            /* Anti-dependency in nested context */
            int old_val = matrix[i*n + j];
            matrix[i*n + j] = old_val * (i + j);
            
            /* Accumulator with loop-carried dependency */
            total += matrix[i*n + j];
        }
        
        /* Loop-carried dependency across outer loop iterations */
        matrix[i*n] = total % 256;
    }
    
    sink = total; /* Ensure computation isn't eliminated */
    return total;
}

int main() {
    const int SIZE = 256;
    int i;
    
    /* Allocate and initialize arrays with different data types */
    int* int_array = (int*)malloc(SIZE * sizeof(int));
    double* double_array = (double*)malloc(SIZE * sizeof(double));
    float* float_array = (float*)malloc(SIZE * sizeof(float));
    int* matrix = (int*)malloc(SIZE * SIZE * sizeof(int));
    
    if (!int_array || !double_array || !float_array || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (i = 0; i < SIZE; i++) {
        int_array[i] = i + 1;
        double_array[i] = (double)i * 0.5;
        float_array[i] = (float)i * 0.25f;
    }
    
    for (i = 0; i < SIZE * SIZE; i++) {
        matrix[i] = i % 100;
    }
    
    /* Call functions with complex loops */
    int result1 = compute_loop(int_array, double_array, float_array, SIZE);
    int result2 = nested_loop_compute(matrix, 16); /* 16x16 matrix */
    
    /* Use results to prevent dead code elimination */
    printf("Result 1: %d\n", result1);
    printf("Result 2: %d\n", result2);
    
    /* Final computation using all arrays */
    volatile int final_check = 0;
    for (i = 0; i < SIZE; i++) {
        final_check += int_array[i] + (int)double_array[i] + (int)float_array[i];
    }
    printf("Final check: %d\n", final_check);
    
    /* Cleanup */
    free(int_array);
    free(double_array);
    free(float_array);
    free(matrix);
    
    return 0;
}
