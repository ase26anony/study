/* test_ddg.c - Program to trigger GCC Data Dependency Graph edge creation */
#include <stdio.h>
#include <stdlib.h>

/* Non-inlined function to ensure DDG is built for the loop body */
__attribute__((noinline, noclone))
double compute_loop(double* arr, int* indices, float* farr, int size) {
    volatile double sum = 0.0;  /* Prevent elimination, but not entire loop */
    double temp1, temp2;
    int i;
    
    /* Complex loop with multiple carried dependencies */
    for (i = 1; i < size; i++) {
        /* 1. TRUE DEPENDENCY (RAW) with floating point - affects latency */
        temp1 = arr[i-1] * 1.5;          /* FP multiply */
        
        /* 2. ANTI-DEPENDENCY (WAR) - reuse of arr[i] */
        temp2 = arr[i];                  /* Read arr[i] before writing */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - multiple writes to same location */
        arr[i] = temp1 + (double)farr[i]; /* First write to arr[i] */
        
        /* 4. MEMORY DEPENDENCY with pointer aliasing potential */
        /* indices[i] could theoretically point to arr[0..size-1] */
        int idx = indices[i] % size;
        arr[i] = arr[i] + arr[idx];      /* Second write to arr[i] (WAW) */
        
        /* 5. INTEGER TRUE DEPENDENCY with loop-carried value */
        indices[i] = indices[i-1] + i;   /* Integer RAW */
        
        /* 6. MIXED TYPE OPERATIONS affecting data_type field */
        farr[i] = (float)(arr[i] * 0.25) + (float)i; /* FP conversion */
        
        /* 7. ACCUMULATOR with true dependency across iterations */
        sum = sum + arr[i] + (double)farr[i];
    }
    
    return sum;
}

/* Another function with nested loops for additional DDG complexity */
__attribute__((noinline, noclone))
int nested_loop_deps(int* a, int* b, double* c, int n) {
    int i, j;
    volatile int total = 0;
    
    for (i = 1; i < n; i++) {
        /* Loop-carried dependency in outer loop */
        a[i] = a[i-1] + b[i];
        
        for (j = 1; j < n; j++) {
            /* Inner loop with multiple dependency types */
            /* Memory dependency with potential aliasing */
            b[j] = b[j-1] + a[i];          /* RAW on b, RAW on a */
            
            /* Anti-dependency through c */
            double temp = c[j];            /* Read c[j] */
            c[j] = (double)(a[i] + b[j]);  /* Write c[j] - WAR */
            c[j] = c[j] * 2.0;             /* Another write - WAW */
            
            /* Pointer-based memory dependency */
            int* ptr = &b[j % 10];
            *ptr = *ptr + 1;               /* Ambiguous memory dep */
            
            total += b[j] + (int)c[j];
        }
    }
    
    return total;
}

/* Function with control dependencies */
__attribute__((noinline, noclone))
void control_dep_loop(int* data, int size, int limit) {
    int i;
    volatile int count = 0;
    
    for (i = 1; i < size; i++) {
        /* Data-dependent control flow */
        if (data[i-1] > limit) {
            /* Branch creates control dependencies */
            data[i] = data[i] * 2;      /* WAR on data[i] */
            count++;
        } else {
            data[i] = data[i-1] + i;    /* RAW on data[i-1] */
        }
        
        /* Output dependency */
        data[i] = data[i] + 1;          /* WAW on data[i] */
        
        /* Memory operation with unknown aliasing */
        int* alias = data + (i % 5);
        *alias = *alias + data[i];
    }
    
    /* Use volatile to prevent elimination */
    volatile int dummy = count;
    (void)dummy;
}

int main(void) {
    const int SIZE = 512;
    double* arr = (double*)malloc(SIZE * sizeof(double));
    float* farr = (float*)malloc(SIZE * sizeof(float));
    int* indices = (int*)malloc(SIZE * sizeof(int));
    int* data_a = (int*)malloc(SIZE * sizeof(int));
    int* data_b = (int*)malloc(SIZE * sizeof(int));
    double* data_c = (double*)malloc(SIZE * sizeof(double));
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = (double)(i % 100);
        farr[i] = (float)(i % 50);
        indices[i] = (i * 3) % SIZE;
        data_a[i] = i;
        data_b[i] = SIZE - i;
        data_c[i] = (double)(i * 2);
    }
    
    /* Call functions to trigger DDG construction */
    double result1 = compute_loop(arr, indices, farr, SIZE);
    int result2 = nested_loop_deps(data_a, data_b, data_c, SIZE / 2);
    control_dep_loop(data_a, SIZE, 100);
    
    /* Use results to prevent dead code elimination */
    volatile double print_me = result1 + result2;
    printf("Result: %f\n", print_me);
    
    /* Cleanup */
    free(arr);
    free(farr);
    free(indices);
    free(data_a);
    free(data_b);
    free(data_c);
    
    return 0;
}
