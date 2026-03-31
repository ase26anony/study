/* Test program to trigger modulo scheduling edge printing in GCC */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define ITERS 100

/* Global volatile sink to prevent optimization */
volatile int sink = 0;

/* Function 1: Loop with carried dependency and mixed operations */
void func1_carried_dep(int *arr, int n, int scalar) {
    volatile int *varr = (volatile int *)arr;
    
    /* Loop with carried dependency (distance 1) */
    for (int i = 1; i < n; i++) {
        varr[i] = varr[i-1] * scalar + varr[i];
        /* Mix in some bitwise operations */
        varr[i] ^= (varr[i] >> 3);
    }
    
    /* Add floating point operations in same loop */
    float farr[SIZE];
    for (int i = 1; i < n; i++) {
        farr[i] = farr[i-1] * 1.5f + farr[i];
        /* Memory barrier to preserve operations */
        asm volatile("" : : "r"(farr[i]) : "memory");
    }
}

/* Function 2: Reduction loop with pointer chasing pattern */
int func2_reduction_chase(int *data, int n) {
    volatile int sum = 0;
    int *p = data;
    
    for (int i = 0; i < n; i++) {
        sum += *p;
        /* Pointer chasing with dependency */
        p = data + (*p % n);
        /* Integer multiplication with latency */
        sum *= (sum & 0xFF);
    }
    
    /* Use result to prevent elimination */
    sink += sum;
    return sum;
}

/* Function 3: Loop with multiple independent then dependent operations */
void func3_mixed_ops(double *darr, int *iarr, int n) {
    volatile double acc = 1.0;
    
    for (int i = 1; i < n; i++) {
        /* Independent floating point ops */
        double temp1 = darr[i] * 2.5;
        double temp2 = darr[i-1] / 1.7;
        
        /* Dependent integer ops */
        iarr[i] = (int)(temp1 + temp2) * iarr[i-1];
        
        /* More mixed operations */
        acc = acc * 1.1 + darr[i];
        iarr[i] ^= (iarr[i] << 2) | 1;
    }
    
    /* Compiler barrier */
    asm volatile("" : : "r"(acc) : "memory");
}

/* Function 4: Nested loops with conditional inner logic */
void func4_nested_conditional(int *arr, int n, int threshold) {
    volatile int count = 0;
    
    /* Outer loop */
    for (int j = 0; j < 5; j++) {
        /* Inner loop with condition */
        if (threshold > 0) {
            for (int i = 1; i < n; i++) {
                /* Complex carried dependency chain */
                arr[i] = (arr[i-1] * arr[i]) + (arr[i] >> 2);
                
                /* Conditional operation inside loop */
                if (arr[i] > threshold) {
                    count++;
                    arr[i] -= threshold;
                }
                
                /* Floating point in integer loop */
                float ftemp = (float)arr[i] * 0.5f;
                asm volatile("" : : "r"(ftemp) : "memory");
            }
        }
    }
    
    sink += count;
}

/* Function 5: Loop with varying dependency distances */
void func5_varying_distances(int *arr, int n) {
    /* Distance 1 dependency */
    for (int i = 3; i < n; i++) {
        arr[i] = arr[i-1] + arr[i-2] + arr[i-3];
    }
    
    /* Distance 2 dependency */
    for (int i = 2; i < n; i++) {
        int tmp = arr[i] * arr[i-2];
        arr[i] = tmp ^ (tmp >> 4);
    }
}

/* Main driver that ensures all loops execute */
int main(int argc, char **argv) {
    /* Initialize data with non-zero values */
    int arr1[SIZE], arr2[SIZE];
    double darr[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (i * 3 + 1) & 0xFF;
        arr2[i] = (i * 5 + 2) & 0xFF;
        darr[i] = (i * 1.3) + 0.5;
    }
    
    /* Execute all functions to ensure loops are compiled */
    func1_carried_dep(arr1, ITERS, 7);
    int sum2 = func2_reduction_chase(arr2, ITERS);
    func3_mixed_ops(darr, arr1, ITERS);
    func4_nested_conditional(arr2, ITERS, 100);
    func5_varying_distances(arr1, ITERS);
    
    /* Use results to prevent dead code elimination */
    volatile int total = 0;
    for (int i = 0; i < SIZE; i++) {
        total += arr1[i] + arr2[i] + (int)darr[i];
    }
    
    total += sum2 + sink;
    
    /* Print minimal output */
    printf("Result checksum: %d\n", total % 1000);
    
    return 0;
}
