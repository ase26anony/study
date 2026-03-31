/* Test program to trigger modulo scheduling edge printing */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define ITERS 100

/* Volatile sink to prevent dead code elimination */
volatile int sink = 0;

/* Function 1: Loop with carried dependency (distance 1) */
int func1_carried_dep(int *arr, int n, int scalar) {
    int sum = 0;
    /* Loop with recurrence: arr[i] depends on arr[i-1] */
    for (int i = 1; i < n; i++) {
        arr[i] = arr[i-1] * scalar + arr[i];
        /* Mix integer operations */
        sum += arr[i] & 0xFF;
    }
    return sum;
}

/* Function 2: Mixed integer/float operations with dependency */
float func2_mixed_ops(float *farr, int *iarr, int n) {
    float acc = farr[0];
    /* Loop with mixed operations and memory dependencies */
    for (int i = 1; i < n; i++) {
        /* Floating-point operation with higher latency */
        float temp = farr[i] * 1.5f + acc;
        /* Integer operation with dependency on previous iteration */
        iarr[i] = iarr[i-1] + (int)temp;
        /* Update accumulator with dependency */
        acc = temp * 0.9f;
    }
    return acc;
}

/* Function 3: Reduction with pointer chasing pattern */
int func3_reduction_chase(int *arr, int n) {
    int total = 0;
    int *p = arr;
    /* Pointer chasing with dependency */
    for (int i = 0; i < n; i++) {
        total += *p;
        /* Simple pointer arithmetic with dependency */
        p = arr + (total & 0xF);
        /* Barrier to prevent over-optimization */
        asm volatile("" : : "r"(p) : "memory");
    }
    return total;
}

/* Function 4: Nested loops with conditional inner logic */
int func4_nested_conditional(int *arr, int n, int threshold) {
    int result = 0;
    /* Outer loop */
    for (int outer = 0; outer < 5; outer++) {
        /* Conditional inner loop */
        if (threshold > 0) {
            /* Inner loop with carried dependency */
            for (int i = 1; i < n; i++) {
                /* Complex dependency chain */
                arr[i] = (arr[i-1] * 3 + arr[i]) / 2;
                /* Bitwise operations */
                result ^= arr[i];
            }
        }
        /* Update threshold to vary behavior */
        threshold = (threshold * 7 + 1) & 0xFF;
    }
    return result;
}

/* Function 5: Multiple independent statements with final dependent store */
int func5_multiple_stmts(int *a, int *b, int *c, int n) {
    int sum = 0;
    for (int i = 1; i < n; i++) {
        /* Independent computations */
        int t1 = a[i] * b[i];
        int t2 = a[i-1] + b[i-1];
        int t3 = c[i] & 0xFF;
        
        /* Dependent store with multiple uses */
        a[i] = t1 + t2 * t3;
        sum += a[i];
        
        /* Floating-point to introduce different latency */
        float ft = (float)t1 * 0.5f;
        b[i] += (int)ft;
    }
    return sum;
}

int main() {
    /* Initialize arrays with non-zero values */
    int arr1[SIZE], arr2[SIZE], arr3[SIZE];
    float farr[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = i + 1;
        arr2[i] = i * 2;
        arr3[i] = i * 3;
        farr[i] = (float)i * 1.1f;
    }
    
    /* Call all functions to ensure they're compiled */
    int res1 = func1_carried_dep(arr1, ITERS, 3);
    float res2 = func2_mixed_ops(farr, arr2, ITERS);
    int res3 = func3_reduction_chase(arr3, ITERS);
    int res4 = func4_nested_conditional(arr1, ITERS, 10);
    int res5 = func5_multiple_stmts(arr1, arr2, arr3, ITERS);
    
    /* Use results to prevent dead code elimination */
    sink = res1 + (int)res2 + res3 + res4 + res5;
    
    /* Print minimal output to ensure execution */
    printf("Result: %d\n", sink);
    
    return 0;
}
