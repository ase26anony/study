/* test_ddg.c - Program to trigger DDG edge creation in GCC scheduler */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure loop body remains intact for DDG analysis */
__attribute__((noinline, noclone))
double compute_loop(double* arr, int* int_arr, int size, int* counter) {
    double sum = 0.0;
    double temp = 0.0;
    double* ptr = arr;
    int* int_ptr = int_arr;
    
    /* Complex loop with multiple dependency types */
    for (int i = 1; i < size; i++) {
        /* 1. TRUE DEPENDENCY (RAW) - carried across iterations */
        double prev = arr[i-1];           /* Read */
        arr[i] = prev * 1.5 + i;         /* Write depending on previous read */
        
        /* 2. ANTI-DEPENDENCY (WAR) - reuse of registers/memory */
        temp = int_arr[i];                /* Read */
        int_arr[i] = i * 2;               /* Write to same location */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - multiple writes */
        double intermediate = prev * 0.5;
        arr[i] = intermediate + temp;     /* Second write to arr[i] */
        
        /* 4. MEMORY DEPENDENCY with pointer aliasing */
        *ptr += 0.1;                      /* May alias with arr[i] */
        ptr = &arr[i % 3];                /* Change pointer target */
        
        /* 5. MIXED DATA TYPES creating different latency edges */
        int int_val = int_arr[i-1];       /* Integer load */
        double fp_val = arr[i] * 2.0;     /* FP multiply (higher latency) */
        
        /* 6. CONTROL DEPENDENCY-like pattern */
        if (int_val > 100) {
            fp_val /= 2.0;
        }
        
        /* 7. ACCUMULATOR with loop-carried dependency */
        sum += fp_val + int_val;          /* True dependency on sum */
        
        /* 8. POINTER ARITHMETIC creating address dependencies */
        int_ptr = int_arr + (i % 5);
        *int_ptr += 1;
        
        /* Update counter to prevent dead code elimination */
        (*counter)++;
    }
    
    return sum;
}

/* Another function with nested loops for more complex DDG */
__attribute__((noinline, noclone))
void nested_loop_deps(float* farr, double* darr, int n) {
    for (int i = 1; i < n; i++) {
        /* Cross-iteration dependencies */
        darr[i] = darr[i-1] * 1.1 + farr[i];
        
        /* Inner loop with dependencies */
        float inner_sum = 0.0f;
        for (int j = 0; j < 4; j++) {
            /* Memory dependency with computed index */
            int idx = (i + j) % n;
            inner_sum += farr[idx];
            farr[idx] = inner_sum * 0.9f;  /* WAR dependency */
        }
        
        /* Output dependency */
        farr[i] = inner_sum;
        farr[i] = farr[i] * 2.0f;         /* WAW on farr[i] */
    }
}

int main(void) {
    const int SIZE = 1024;
    
    /* Arrays with different data types */
    double* dbl_arr = (double*)malloc(SIZE * sizeof(double));
    int* int_arr = (int*)malloc(SIZE * sizeof(int));
    float* flt_arr = (float*)malloc(SIZE * sizeof(float));
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        dbl_arr[i] = i * 0.5;
        int_arr[i] = i;
        flt_arr[i] = i * 0.25f;
    }
    
    /* Volatile counter to prevent optimization */
    volatile int counter = 0;
    
    /* Call functions with dependency-rich loops */
    double result1 = compute_loop(dbl_arr, int_arr, SIZE, (int*)&counter);
    
    /* Use volatile to ensure computation isn't eliminated */
    volatile double vol_result = result1;
    
    nested_loop_deps(flt_arr, dbl_arr, SIZE);
    
    /* Compute checksum to use results */
    double checksum = 0.0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dbl_arr[i] + int_arr[i] + flt_arr[i];
    }
    
    /* Print to prevent dead code elimination */
    printf("Result: %f, Checksum: %f, Iterations: %d\n", 
           result1, checksum, counter);
    
    free(dbl_arr);
    free(int_arr);
    free(flt_arr);
    
    return 0;
}
