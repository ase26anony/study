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
        /* 1. TRUE DEPENDENCY (RAW) - Flow dependence */
        temp1 = arr[i-1] * 1.5;      /* Read arr[i-1] */
        arr[i] = temp1 + i;          /* Write arr[i] depends on temp1 */
        
        /* 2. ANTI-DEPENDENCY (WAR) - Anti dependence */
        temp2 = arr[i];              /* Read arr[i] */
        arr[i] = temp2 * 0.75;       /* Write arr[i] after read */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - Output dependence */
        arr[i] = arr[i] + 2.0;       /* Second write to arr[i] */
        
        /* 4. MEMORY DEPENDENCY with pointer aliasing */
        /* Use indices to create ambiguous memory dependencies */
        int idx = indices[i];
        double* ptr = &arr[idx];
        *ptr = *ptr * 1.1;           /* Could alias with arr[i] */
        
        /* 5. MIXED DATA TYPES creating different latency edges */
        sum += (double)(i % 3) * arr[i];  /* Integer to float conversion */
        
        /* 6. CONTROL DEPENDENCY-like pattern */
        if (arr[i] > 100.0) {
            arr[i] = 100.0;          /* Creates conditional dependency */
        }
    }
    
    /* Additional loop with carried dependency chain */
    for (i = 2; i < size; i++) {
        /* Longer dependency chain */
        double chain1 = arr[i-2] * 0.5;
        double chain2 = chain1 + arr[i-1];
        arr[i] = chain2 * arr[i];
        
        /* Cross-iteration dependency with different distances */
        sum = sum * 0.99 + arr[i];
    }
    
    return sum;
}

/* Another function with nested loops */
__attribute__((noinline, noclone))
void nested_loop_deps(float* farr, int* iarr, int n) {
    int i, j;
    
    for (i = 1; i < n; i++) {
        /* Inter-iteration dependency */
        farr[i] = farr[i-1] * 1.1f + (float)iarr[i];
        
        for (j = 1; j < 10; j++) {
            /* Inner loop with dependencies */
            float tmp = farr[i] * (float)j;
            iarr[j] = (int)(tmp * 0.5f);
            farr[i] = tmp - (float)iarr[j];
        }
        
        /* Memory dependency through pointer */
        float* fptr = &farr[i % 5];
        *fptr = *fptr + 0.5f;
    }
}

/* Function with pointer chasing creating memory deps */
__attribute__((noinline, noclone))
int pointer_chase(int** ptrs, int size) {
    int sum = 0;
    int i;
    
    for (i = 0; i < size - 1; i++) {
        /* Pointer-based dependencies */
        *ptrs[i+1] = *ptrs[i] + i;
        sum += *ptrs[i];
    }
    
    return sum;
}

int main(void) {
    const int SIZE = 256;
    double arr[SIZE];
    int indices[SIZE];
    float farr[SIZE];
    int iarr[SIZE];
    int* ptrs[SIZE];
    int buffer[SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = (double)(i % 10);
        indices[i] = (i * 7) % SIZE;  /* Create aliasing pattern */
        farr[i] = (float)i * 0.5f;
        iarr[i] = i;
        buffer[i] = i * 2;
        ptrs[i] = &buffer[i];
    }
    
    /* Volatile to prevent optimization */
    volatile double result = 0.0;
    volatile int int_result = 0;
    
    /* Call functions with dependency patterns */
    result = compute_loop(arr, indices, SIZE);
    
    nested_loop_deps(farr, iarr, SIZE/2);
    
    /* Rearrange pointers to create complex aliasing */
    for (int i = 0; i < SIZE/2; i++) {
        ptrs[i] = &buffer[SIZE - i - 1];
    }
    
    int_result = pointer_chase(ptrs, SIZE/2);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %f, %d\n", (double)result, int_result);
    
    /* Additional volatile store */
    volatile double check = arr[SIZE-1] + farr[SIZE/2-1];
    (void)check;  /* Prevent unused variable warning */
    
    return 0;
}
