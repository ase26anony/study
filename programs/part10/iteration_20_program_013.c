/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure loop body remains intact for DDG analysis */
__attribute__((noinline, noclone))
double compute_loop(double* arr, int* indices, int size, int* counter) {
    double sum = 0.0;
    double temp1, temp2;
    volatile double mem_barrier; /* Prevent reordering across volatile access */
    
    /* Complex loop with multiple dependency types */
    for (int i = 1; i < size; i++) {
        /* 1. TRUE DEPENDENCY (RAW) - carried across iterations */
        /* arr[i] depends on arr[i-1] from previous iteration */
        arr[i] = arr[i-1] * 1.5 + (double)i;
        
        /* 2. ANTI-DEPENDENCY (WAR) - read then write same location */
        temp1 = arr[i];                /* Read arr[i] */
        arr[i] = temp1 * 0.75;         /* Write arr[i] - WAR with previous read */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - multiple writes to same location */
        arr[i] = arr[i] + 2.0;         /* Second write to arr[i] - WAW */
        
        /* 4. MEMORY DEPENDENCY with pointer aliasing */
        /* Use pointer arithmetic to create ambiguous memory dependencies */
        double* ptr = &arr[indices[i] % size];
        *ptr = *ptr * 1.1;             /* Could alias with arr[i] */
        
        /* 5. MIXED DATA TYPES in dependency chain */
        /* Integer operation feeding floating point */
        int int_val = (*counter)++;    /* Integer dependency chain */
        double fp_val = (double)int_val * 0.5;
        
        /* 6. DIFFERENT LATENCY OPERATIONS */
        /* Floating multiply (higher latency) */
        fp_val = fp_val * fp_val * 3.14159;
        
        /* Integer add (lower latency) */
        int_val = int_val + (i % 16);
        
        /* Memory load/store with potential aliasing */
        temp2 = arr[(i * 7) % size];   /* Complex addressing */
        arr[(i * 3) % size] = temp2 + fp_val;
        
        /* 7. CONTROL DEPENDENCY-like pattern */
        /* Conditional that creates data flow */
        if (int_val % 4 == 0) {
            arr[i] = arr[i] * 2.0;
        } else {
            arr[i] = arr[i] * 0.5;
        }
        
        /* Volatile access to prevent reordering */
        mem_barrier = arr[i];
        
        /* Accumulate result to prevent dead code elimination */
        sum += arr[i] + fp_val;
    }
    
    return sum;
}

/* Another function with nested loops for more complex DDG */
__attribute__((noinline, noclone))
void nested_loop_deps(int* a, int* b, int* c, int n) {
    for (int i = 1; i < n; i++) {
        /* Cross-iteration dependencies in nested loop */
        for (int j = 1; j < n; j++) {
            /* 2D stencil computation with multiple dependencies */
            a[i*n + j] = b[(i-1)*n + j] +    /* RAW - vertical dependency */
                        b[i*n + (j-1)] +     /* RAW - horizontal dependency */
                        c[i*n + j];          /* Independent load */
            
            /* Anti-dependency pattern */
            int temp = b[i*n + j];
            b[i*n + j] = a[i*n + j] * 2;
            c[i*n + j] = temp + 1;          /* Use temp after b is written */
        }
    }
}

int main() {
    const int SIZE = 256;
    double* array = (double*)malloc(SIZE * sizeof(double));
    int* indices = (int*)malloc(SIZE * sizeof(int));
    int counter = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        array[i] = (double)i * 0.1;
        indices[i] = (i * 13) % SIZE;  /* Non-linear access pattern */
    }
    
    /* Call the function with complex dependencies */
    double result = compute_loop(array, indices, SIZE, &counter);
    
    /* Create data for nested loop test */
    int n = 32;
    int* a = (int*)malloc(n * n * sizeof(int));
    int* b = (int*)malloc(n * n * sizeof(int));
    int* c = (int*)malloc(n * n * sizeof(int));
    
    for (int i = 0; i < n*n; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = i * 3;
    }
    
    nested_loop_deps(a, b, c, n);
    
    /* Use results to prevent optimization */
    volatile double print_result = result + a[n*n-1] * 0.001;
    
    printf("Result: %f\n", print_result);
    
    free(array);
    free(indices);
    free(a);
    free(b);
    free(c);
    
    return 0;
}
