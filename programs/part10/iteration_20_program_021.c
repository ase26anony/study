/* test_ddg.c - Program to trigger DDG edge creation in GCC scheduler */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure loop body remains intact for DDG analysis */
__attribute__((noinline, noclone))
double compute_loop(double* arr, int* indices, int size) {
    double sum = 0.0;
    double temp = 0.0;
    double* ptr = arr;
    int i;
    
    /* Complex loop with multiple dependency types */
    for (i = 1; i < size; i++) {
        /* 1. TRUE DEPENDENCY (RAW) - carried across iterations */
        double val1 = arr[i-1] * 1.5;      /* Read arr[i-1] */
        arr[i] = val1 + (double)i;         /* Write arr[i] - depends on previous iteration */
        
        /* 2. ANTI-DEPENDENCY (WAR) - reuse of same location */
        temp = arr[indices[i] % size];     /* Read arr[...] */
        arr[indices[i] % size] = temp * 2.0; /* Write same location - anti-dependency */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - multiple writes to same location */
        double* alias_ptr = &arr[i % 8];   /* Potential aliasing */
        *alias_ptr = (double)i * 3.14;     /* First write */
        if (i % 3 == 0) {
            *alias_ptr = *alias_ptr * 0.5; /* Second write to same location - output dep */
        }
        
        /* 4. MIXED DATA TYPES creating different latency operations */
        int int_val = (int)arr[i];         /* Type conversion - different latency */
        float float_val = (float)int_val * 1.1f; /* Float operation */
        arr[i % 16] = (double)float_val;   /* Store back as double */
        
        /* 5. POINTER ARITHMETIC with ambiguous memory dependencies */
        ptr = &arr[i % 32];
        *ptr = *ptr + (double)(i % 4);     /* Memory dependency through pointer */
        
        /* 6. ACCUMULATOR with loop-carried dependency */
        sum = sum + arr[i] * 0.1;          /* Another true dependency chain */
        
        /* 7. CONTROL DEPENDENCY (via conditional) */
        if (sum > 1000.0) {
            sum = sum * 0.99;              /* Control-dependent operation */
        }
    }
    
    return sum + temp;
}

/* Another function with nested loops for additional DDG complexity */
__attribute__((noinline, noclone))
void nested_loop_deps(int* matrix, int rows, int cols) {
    int i, j;
    
    /* Nested loop with stride access pattern */
    for (i = 1; i < rows; i++) {
        for (j = 1; j < cols; j++) {
            /* 2D stencil computation with multiple dependencies */
            int idx = i * cols + j;
            
            /* True dependencies in both dimensions */
            matrix[idx] = matrix[idx - 1] +           /* Left neighbor */
                          matrix[idx - cols] +        /* Top neighbor */
                          matrix[idx - cols - 1];     /* Diagonal neighbor */
            
            /* Anti-dependency through temporary */
            int old_val = matrix[idx];
            matrix[idx] = (old_val * 3) / 2;
            
            /* Output dependency through conditional */
            if (matrix[idx] > 100) {
                matrix[idx] = matrix[idx] % 100;
            }
        }
    }
}

int main() {
    const int SIZE = 256;
    double array[SIZE];
    int indices[SIZE];
    int matrix[32][32];
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        array[i] = (double)i * 0.5;
        indices[i] = (i * 13) % SIZE;  /* Pseudo-random pattern */
    }
    
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            matrix[i][j] = i + j;
        }
    }
    
    /* Force computation to prevent dead code elimination */
    volatile double result = compute_loop(array, indices, SIZE);
    nested_loop_deps(&matrix[0][0], 32, 32);
    
    /* Use results to prevent optimization */
    printf("Result: %f\n", result);
    printf("Matrix[10][10] = %d\n", matrix[10][10]);
    
    return 0;
}
