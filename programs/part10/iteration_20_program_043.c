/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */
#include <stdio.h>
#include <stdlib.h>

/* Non-inlined function containing complex dependency patterns */
__attribute__((noinline, noclone))
double compute_loop(double* arr, int* indices, int size) {
    double sum = 0.0;
    double temp = 0.0;
    double* ptr = arr;
    int i;
    
    /* Complex loop with multiple dependency types */
    for (i = 1; i < size; i++) {
        /* 1. TRUE DEPENDENCY (RAW) - carried across iterations */
        double prev = arr[i-1];          /* Read arr[i-1] */
        arr[i] = prev * 1.5 + i;         /* Write arr[i] depends on arr[i-1] */
        
        /* 2. ANTI-DEPENDENCY (WAR) - reuse of register/variable */
        temp = arr[i];                   /* Read arr[i] */
        arr[i] = temp * 0.75;            /* Write arr[i] after reading it */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - multiple writes to same location */
        arr[i] = arr[i] + 0.1;           /* Second write to arr[i] */
        
        /* 4. MEMORY DEPENDENCY with pointer aliasing */
        *ptr = *ptr * 2.0;               /* ptr may alias arr */
        ptr = &arr[i % 16];              /* Change pointer target */
        
        /* 5. MIXED DATA TYPES creating different latency edges */
        int int_val = (int)arr[i];       /* Floating point to integer conversion */
        double dbl_val = int_val * 1.618; /* Integer to floating point */
        
        /* 6. COMPLEX ADDRESSING with potential dependencies */
        int idx = indices[i];            /* Load from index array */
        double* alias_ptr = arr + (idx % 8);
        *alias_ptr = *alias_ptr + dbl_val; /* Memory op with unknown aliasing */
        
        /* 7. ACCUMULATOR with loop-carried dependency */
        sum = sum + arr[i] + dbl_val;    /* True dependency on sum */
        
        /* 8. CONTROL DEPENDENCY-like pattern */
        if (arr[i] > 100.0) {
            arr[i] = arr[i] * 0.5;       /* Creates additional dependencies */
        }
    }
    
    return sum;
}

/* Second function with different patterns */
__attribute__((noinline, noclone))
float compute_nested_loop(float* farr, int size) {
    float total = 0.0f;
    int i, j;
    
    /* Nested loop with stride access pattern */
    for (i = 0; i < size; i++) {
        float accum = farr[i];
        for (j = 0; j < 8; j++) {
            /* Inter-iteration dependency in inner loop */
            accum = accum * 1.1f + j;
            
            /* Memory dependency with different stride */
            farr[(i + j) % size] = farr[(i + j) % size] + accum;
        }
        total += accum;
    }
    
    return total;
}

int main(void) {
    const int SIZE = 1024;
    double* arr = (double*)malloc(SIZE * sizeof(double));
    int* indices = (int*)malloc(SIZE * sizeof(int));
    float* farr = (float*)malloc(SIZE * sizeof(float));
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = (double)(i % 100);
        indices[i] = (i * 7) % SIZE;  /* Non-linear access pattern */
        farr[i] = (float)(i % 50);
    }
    
    /* Force compiler to consider pointer aliasing */
    double* volatile alias_check = arr;
    
    /* Compute with complex dependencies */
    double result1 = compute_loop(arr, indices, SIZE);
    float result2 = compute_nested_loop(farr, SIZE);
    
    /* Use volatile to prevent dead code elimination */
    volatile double final_result = result1 + result2;
    
    /* Print to ensure execution */
    printf("Result: %f\n", (double)final_result);
    
    /* Cleanup */
    free(arr);
    free(indices);
    free(farr);
    
    return 0;
}
