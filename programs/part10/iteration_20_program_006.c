/* test_ddg.c - Program to trigger DDG edge creation in GCC scheduler */

#include <stdio.h>
#include <stdlib.h>

/* Non-inlined function containing complex dependency patterns */
__attribute__((noinline, noclone))
double compute_loop(double* arr, int* indices, float* farr, int size) {
    double sum = 0.0;
    double temp = 0.0;
    int i;
    
    /* Initialize arrays with some values */
    for (i = 0; i < size; i++) {
        arr[i] = (double)(i + 1);
        farr[i] = (float)(i * 0.5);
        indices[i] = i;
    }
    
    /* Main loop with multiple carried dependencies */
    for (i = 1; i < size; i++) {
        /* 1. TRUE DEPENDENCY (RAW) - carried across iterations */
        double prev = arr[i-1];           /* Read */
        arr[i] = prev * 1.5 + (double)i;  /* Write - depends on previous iteration */
        
        /* 2. ANTI-DEPENDENCY (WAR) - within same iteration */
        temp = arr[i];                    /* Read */
        arr[i] = temp + farr[i];          /* Write - reuses same location */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - within same iteration */
        arr[i] = arr[i] * 2.0;            /* Second write to same location */
        
        /* 4. MEMORY DEPENDENCY with pointer aliasing potential */
        int idx = indices[i];
        double* ptr = &arr[idx];
        *ptr = *ptr + 1.0;                /* Could alias with arr[i] */
        
        /* 5. MIXED DATA TYPES creating different latency operations */
        float fval = farr[i];
        farr[i] = fval * 3.14f;           /* Float multiply */
        
        /* 6. INTEGER DEPENDENCY CHAIN */
        indices[i] = indices[i-1] + 1;    /* Integer RAW dependency */
        
        /* 7. ACCUMULATOR with loop-carried dependency */
        sum = sum + arr[i] + farr[i];     /* Multiple RAW dependencies */
        
        /* 8. CONTROL DEPENDENCY (via conditional) */
        if (sum > 1000.0) {
            sum = sum * 0.9;              /* Creates control flow */
        }
    }
    
    /* Additional output dependency */
    arr[0] = sum;                         /* WAW with initialization */
    
    return sum;
}

/* Another function with nested loops for more complex DDG */
__attribute__((noinline, noclone))
void nested_loop_deps(int* matrix, int n) {
    int i, j;
    
    /* Nested loops with multiple dependency patterns */
    for (i = 1; i < n; i++) {
        for (j = 1; j < n; j++) {
            /* 2D stencil computation with multiple dependencies */
            int up = matrix[(i-1)*n + j];     /* RAW - from previous row */
            int left = matrix[i*n + (j-1)];   /* RAW - from previous column */
            
            /* Multiple writes creating WAW and WAR */
            int temp = matrix[i*n + j];       /* WAR - read before write */
            matrix[i*n + j] = (up + left) / 2;
            matrix[i*n + j] += temp;          /* WAW - second write */
            
            /* Pointer-based memory dependency */
            int* ptr = &matrix[i*n + j];
            *ptr = *ptr + matrix[(i-1)*n + (j-1)]; /* Potential aliasing */
        }
    }
}

/* Function with pointer chasing creating memory dependencies */
__attribute__((noinline, noclone))
double pointer_chasing(double* data, int* next, int size) {
    double result = 0.0;
    int idx = 0;
    int i;
    
    for (i = 0; i < size; i++) {
        /* Pointer chasing through indirect indexing */
        result += data[idx];
        idx = next[idx];  /* Data-dependent control flow */
        
        /* Memory dependency through pointer arithmetic */
        double* dptr = data + idx;
        *dptr = *dptr * 1.1;
    }
    
    return result;
}

int main(void) {
    const int SIZE = 512;
    const int MATRIX_SIZE = 64;
    
    /* Allocate and initialize arrays with different data types */
    double* arr = (double*)malloc(SIZE * sizeof(double));
    float* farr = (float*)malloc(SIZE * sizeof(float));
    int* indices = (int*)malloc(SIZE * sizeof(int));
    int* matrix = (int*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
    int* next_ptr = (int*)malloc(SIZE * sizeof(int));
    
    /* Initialize next_ptr for pointer chasing */
    for (int i = 0; i < SIZE; i++) {
        next_ptr[i] = (i + 1) % SIZE;
    }
    
    /* Initialize matrix */
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
        matrix[i] = i % 100;
    }
    
    /* Force computation and prevent dead code elimination */
    volatile double result1 = compute_loop(arr, indices, farr, SIZE);
    nested_loop_deps(matrix, MATRIX_SIZE);
    volatile double result2 = pointer_chasing(arr, next_ptr, SIZE);
    
    /* Use results to prevent optimization */
    printf("Result 1: %f\n", (double)result1);
    printf("Result 2: %f\n", (double)result2);
    
    /* Compute checksum to ensure all computations matter */
    double checksum = 0.0;
    for (int i = 0; i < SIZE; i++) {
        checksum += arr[i] + farr[i];
    }
    printf("Checksum: %f\n", checksum);
    
    free(arr);
    free(farr);
    free(indices);
    free(matrix);
    free(next_ptr);
    
    return 0;
}
