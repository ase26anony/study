/* test_ddg.c - Program to trigger DDG edge creation in GCC scheduler */
#include <stdio.h>
#include <stdlib.h>

/* Non-inlined function containing complex dependency patterns */
__attribute__((noinline, noclone))
double compute_loop(double* arr, int* indices, int size) {
    double sum = 0.0;
    double temp1, temp2;
    int i;
    
    /* Complex loop with multiple dependency types */
    for (i = 1; i < size; i++) {
        /* 1. TRUE DEPENDENCY (RAW) - carried across iterations */
        /* arr[i] depends on arr[i-1] from previous iteration */
        arr[i] = arr[i-1] * 1.5 + (double)i;
        
        /* 2. ANTI-DEPENDENCY (WAR) - reuse of same variable */
        temp1 = arr[i];                /* Read arr[i] */
        arr[i] = temp1 * 2.0 - 1.0;    /* Write arr[i] - anti-dep on previous read */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - multiple writes to same location */
        arr[i] = arr[i] + 0.5;         /* Second write to arr[i] */
        
        /* 4. MEMORY DEPENDENCY with pointer aliasing */
        /* Use indices array to create ambiguous memory dependencies */
        int idx = indices[i];
        double* ptr = &arr[idx];
        
        /* 5. MIXED DATA TYPES creating different latency operations */
        /* Integer operation with different latency */
        indices[i] = indices[i-1] + i * 3;
        
        /* Floating point operation (higher latency) */
        temp2 = (double)indices[i] * 0.25;
        
        /* 6. COMPLEX EXPRESSION with multiple operations */
        arr[i] = arr[i] + temp2 * (arr[i-1] / 2.0);
        
        /* 7. ACCUMULATOR with loop-carried dependency */
        sum = sum + arr[i] * (double)(i % 4);
    }
    
    /* Additional output dependency outside loop */
    arr[0] = sum;      /* Write to arr[0] */
    arr[0] = arr[0] * 0.9;  /* Second write to arr[0] - output dependency */
    
    return sum;
}

/* Another function with nested loops and different patterns */
__attribute__((noinline, noclone))
void nested_loop_pattern(float* farr, int* iarr, int n) {
    int i, j;
    
    for (i = 1; i < n; i++) {
        /* Pointer chasing creating memory dependencies */
        float* fptr = &farr[i];
        int* iptr = &iarr[i];
        
        /* Cross-iteration dependencies through multiple arrays */
        farr[i] = farr[i-1] * 1.1f + (float)iarr[i-1];
        iarr[i] = iarr[i-1] + (int)farr[i-1];
        
        /* Inner loop with reduction */
        float inner_sum = 0.0f;
        for (j = 0; j < 4; j++) {
            inner_sum += farr[i] * (float)j;
            /* Memory dependency through pointer */
            *fptr = *fptr + inner_sum * 0.5f;
        }
        
        /* Anti-dependency pattern */
        float temp = farr[i];
        farr[i] = (float)iarr[i] * 0.3f;
        iarr[i] = (int)(temp * 2.0f);
    }
}

int main() {
    const int SIZE = 256;
    double arr[SIZE];
    int indices[SIZE];
    float farr[SIZE];
    int iarr[SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = (double)(i + 1) * 0.5;
        indices[i] = i;
        farr[i] = (float)i * 0.25f;
        iarr[i] = i * 2;
    }
    
    /* Shuffle indices to create irregular memory access patterns */
    for (int i = 0; i < SIZE; i++) {
        int j = (i * 13) % SIZE;  /* Simple pseudo-shuffle */
        int temp = indices[i];
        indices[i] = indices[j];
        indices[j] = temp;
    }
    
    /* Volatile to prevent optimization of results */
    volatile double result1, result2;
    
    /* Call functions with complex dependency patterns */
    result1 = compute_loop(arr, indices, SIZE);
    
    /* Call second function */
    nested_loop_pattern(farr, iarr, SIZE);
    
    /* Compute a simple checksum from results */
    double checksum = result1;
    for (int i = 0; i < SIZE; i++) {
        checksum += arr[i] + (double)farr[i] + (double)iarr[i];
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result checksum: %f\n", checksum);
    
    return 0;
}
