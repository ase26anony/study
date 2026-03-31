/* Test program to trigger modulo scheduling edge printing in GCC */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 100
#define ITERS 1000

/* Global volatile to prevent optimization */
volatile int global_sink = 0;

/* Function 1: Loop with carried dependency and mixed operations */
int func1_carried_dep(volatile int* arr, int n) {
    int sum = arr[0];
    for (int i = 1; i < n; i++) {
        /* Mixed integer operations with carried dependency */
        int temp = arr[i-1] * 3 + 7;
        arr[i] = temp ^ (arr[i] + 1);
        sum += arr[i];
        
        /* Memory barrier to preserve operations */
        asm volatile("" : : "r"(arr[i]) : "memory");
    }
    return sum;
}

/* Function 2: Loop with floating-point operations and recurrence */
float func2_fp_recurrence(float* arr, int n) {
    float acc = arr[0];
    for (int i = 1; i < n; i++) {
        /* Floating-point operations with different latencies */
        float fp_temp = acc * 1.5f + arr[i];
        
        /* Integer operations mixed in */
        int idx = (i * 7) % n;
        acc = fp_temp + (float)(idx % 3);
        
        /* Conditional to create complex control flow */
        if (acc > 100.0f) {
            acc *= 0.9f;
        }
    }
    return acc;
}

/* Function 3: Pointer chasing pattern with memory accesses */
int func3_pointer_chase(int* base, int n) {
    int* p = base;
    int sum = 0;
    for (int i = 0; i < n; i++) {
        /* Pointer chasing with arithmetic */
        sum += *p;
        p = base + ((p - base) * 2 + 1) % n;
        
        /* Additional operations to increase loop body complexity */
        sum ^= (i << 3);
    }
    return sum;
}

/* Function 4: Reduction with multiple dependencies */
long func4_multi_dep(long* arr, int n) {
    long prod = 1;
    long sum = 0;
    for (int i = 0; i < n; i++) {
        /* Multiple interleaved dependencies */
        prod *= (arr[i] + 1);
        sum += prod;
        
        /* Floating-point in the middle */
        float ftemp = (float)prod * 0.5f;
        sum += (long)ftemp;
        
        /* Compiler barrier */
        asm volatile("" : : "r"(prod), "r"(sum) : "memory");
    }
    return sum;
}

/* Function 5: Nested loops with conditional inner logic */
int func5_nested_conditional(int* arr, int n, int outer_iters) {
    int total = 0;
    for (int iter = 0; iter < outer_iters; iter++) {
        /* Outer loop with conditional execution */
        if (iter % 3 == 0) {
            /* Inner loop with carried dependency */
            int local_acc = arr[0];
            for (int i = 1; i < n; i++) {
                /* Complex dependency chain */
                local_acc = (local_acc * arr[i]) + (arr[i-1] ^ i);
                arr[i] = local_acc % 100;
                
                /* Mix of operations */
                float ftemp = (float)local_acc * 0.25f;
                local_acc += (int)ftemp;
            }
            total += local_acc;
        } else {
            /* Alternative path to prevent dead code elimination */
            for (int i = 0; i < n; i++) {
                arr[i] += iter;
            }
        }
    }
    return total;
}

/* Main driver that ensures all loops are executed */
int main() {
    /* Initialize data arrays */
    volatile int arr1[SIZE];
    float arr2[SIZE];
    int arr3[SIZE];
    long arr4[SIZE];
    int arr5[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (i * 3 + 7) % 100;
        arr2[i] = (float)(i * 2 + 1) * 0.5f;
        arr3[i] = (i * 5 + 3) % 50;
        arr4[i] = (i + 1) * 2;
        arr5[i] = i;
    }
    
    /* Execute all functions to ensure loops are present */
    int result1 = func1_carried_dep(arr1, SIZE);
    float result2 = func2_fp_recurrence(arr2, SIZE);
    int result3 = func3_pointer_chase(arr3, SIZE);
    long result4 = func4_multi_dep(arr4, SIZE);
    int result5 = func5_nested_conditional(arr5, SIZE, 10);
    
    /* Use results to prevent dead code elimination */
    global_sink = result1 + (int)result2 + result3 + (int)result4 + result5;
    
    /* Print minimal output to verify execution */
    printf("Result checksum: %d\n", global_sink);
    
    return 0;
}
