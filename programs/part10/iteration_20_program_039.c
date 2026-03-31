/* test_ddg.c - Program to trigger DDG edge creation in GCC scheduler */

#include <stdio.h>
#include <stdlib.h>

/* Non-inlined function to ensure loop body remains intact for DDG analysis */
__attribute__((noinline, noclone))
double compute_loop(double* arr, int* indices, int size, double init) {
    double sum = init;
    double temp1, temp2;
    volatile double mem_barrier; /* Prevent over-optimization */
    
    /* Complex loop with multiple dependency types */
    for (int i = 1; i < size; i++) {
        /* 1. TRUE DEPENDENCY (RAW) - carried across iterations */
        temp1 = arr[i-1] * 1.5;      /* FP multiply with latency */
        
        /* 2. ANTI-DEPENDENCY (WAR) - reuse of arr[i] */
        temp2 = arr[i];              /* Read arr[i] before overwriting */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - multiple writes to same location */
        arr[i] = temp1 + (double)i;  /* First write to arr[i] */
        
        /* Memory dependency with pointer aliasing possibility */
        arr[indices[i]] = temp2 * 0.75; /* May alias with arr[i] */
        
        /* 4. Another TRUE DEPENDENCY with different data type */
        sum += (int)arr[i];          /* Type conversion: double -> int */
        
        /* 5. Control dependency-like structure */
        if (sum > 1000.0) {
            sum *= 0.99;             /* FP operation with potential latency */
        }
        
        /* 6. Output dependency on sum (WAW through different paths) */
        sum = sum - (double)(i % 3); /* Second write to sum in same iteration */
        
        /* Memory barrier to prevent reordering across iterations */
        mem_barrier = sum;
    }
    
    /* Additional output dependency across loop iterations */
    arr[0] = sum;  /* WAW with potential previous writes to arr[0] */
    
    return sum;
}

/* Second loop with integer dependencies and pointer arithmetic */
__attribute__((noinline, noclone))
int compute_loop_int(int* data, int** ptrs, int size, int init) {
    int result = init;
    int* local_ptr = data;
    
    /* Loop with pointer-based dependencies */
    for (int i = 0; i < size - 1; i++) {
        /* Pointer arithmetic creating address dependencies */
        int* next_ptr = local_ptr + 1;
        
        /* True dependency through pointer dereference */
        int val = *local_ptr;
        
        /* Anti-dependency: read before write through pointer */
        int old_val = *next_ptr;
        
        /* Output dependency: write through pointer */
        *local_ptr = val + result;
        
        /* Memory dependency with potential aliasing */
        *ptrs[i] = old_val * 2;
        
        /* Integer operation with carry dependency */
        result = (result * 1103515245 + 12345) & 0x7fffffff;
        
        /* Update pointer for next iteration (carried dependency) */
        local_ptr = next_ptr;
    }
    
    return result;
}

/* Loop with mixed dependencies and function calls */
__attribute__((noinline, noclone))
float mixed_dependencies(float* farr, int* iarr, int size) {
    float accum = 0.0f;
    float temp[3];
    
    for (int i = 1; i < size; i++) {
        /* Chain of true dependencies */
        temp[0] = farr[i-1] * 3.14f;
        temp[1] = temp[0] + (float)iarr[i];
        temp[2] = temp[1] / 2.0f;
        
        /* Anti-dependency on farr[i] */
        float old = farr[i];
        
        /* Output dependency on farr[i] */
        farr[i] = temp[2];
        
        /* Memory dependency with different type */
        iarr[i] = (int)(old * 100.0f);
        
        /* Accumulator with true dependency chain */
        accum = accum + farr[i] - (float)iarr[i-1];
        
        /* Conditional creating control dependency */
        if (accum > 1000.0f) {
            accum = accum * 0.9f;
        } else {
            accum = accum * 1.1f;
        }
    }
    
    return accum;
}

int main() {
    const int SIZE = 256;
    
    /* Allocate and initialize arrays */
    double* darr = (double*)malloc(SIZE * sizeof(double));
    int* iarr = (int*)malloc(SIZE * sizeof(int));
    int* indices = (int*)malloc(SIZE * sizeof(int));
    int** ptrs = (int**)malloc(SIZE * sizeof(int*));
    float* farr = (float*)malloc(SIZE * sizeof(float));
    
    for (int i = 0; i < SIZE; i++) {
        darr[i] = (double)i * 0.5;
        iarr[i] = i * 2;
        indices[i] = (i * 3) % SIZE;
        ptrs[i] = &iarr[(i * 7) % SIZE];
        farr[i] = (float)i * 0.25f;
    }
    
    /* Force computation to prevent dead code elimination */
    volatile double result1 = 0.0;
    volatile int result2 = 0;
    volatile float result3 = 0.0f;
    
    /* Execute loops to trigger DDG construction */
    result1 = compute_loop(darr, indices, SIZE, 1.0);
    result2 = compute_loop_int(iarr, ptrs, SIZE / 2, 42);
    result3 = mixed_dependencies(farr, iarr, SIZE);
    
    /* Use results to prevent optimization */
    printf("Results: %f, %d, %f\n", result1, result2, result3);
    
    /* Cleanup */
    free(darr);
    free(iarr);
    free(indices);
    free(ptrs);
    free(farr);
    
    return 0;
}
