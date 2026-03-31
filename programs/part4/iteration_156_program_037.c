/* Test program to trigger modulo scheduling edge printing in GCC */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define ITERATIONS 100

/* Global volatile sink to prevent dead code elimination */
volatile int sink = 0;

/* Function 1: Loop with carried dependency and mixed operations */
void func1_carried_dep(volatile int* arr, int n) {
    if (n <= 1) return;
    
    /* Mixed integer operations with carried dependency */
    for (int i = 1; i < n; i++) {
        /* Create complex dependency chain */
        int temp = arr[i-1] * 3 + arr[i];
        arr[i] = (temp >> 2) + (arr[i] & 0xFF);
        
        /* Add floating point operations for varied latency */
        float f = (float)arr[i] * 1.5f;
        arr[i] = (int)f + (arr[i] % 7);
    }
}

/* Function 2: Reduction loop with pointer chasing pattern */
int func2_reduction(int* data, int n) {
    if (n <= 0) return 0;
    
    int sum = data[0];
    int* p = &data[0];
    
    for (int i = 1; i < n; i++) {
        /* Pointer chasing with arithmetic */
        p = &data[i];
        sum = sum * 2 + *p;
        
        /* Additional operations to increase complexity */
        sum = (sum << 3) | (sum >> 29);  /* Rotate */
        sum = sum ^ (sum * 3);
        
        /* Memory barrier to preserve ordering */
        asm volatile("" : : "r"(p) : "memory");
    }
    
    return sum;
}

/* Function 3: Loop with multiple independent statements and dependent store */
void func3_mixed_ops(double* darr, int* iarr, int n) {
    if (n <= 1) return;
    
    /* Initialize first element */
    darr[0] = 1.0;
    iarr[0] = 1;
    
    for (int i = 1; i < n; i++) {
        /* Independent floating point operations */
        double d1 = darr[i-1] * 2.5;
        double d2 = darr[i] * 1.7;
        
        /* Independent integer operations */
        int i1 = iarr[i-1] * 3;
        int i2 = iarr[i] + 7;
        
        /* Cross-type dependency */
        darr[i] = d1 + d2 + (double)i1;
        iarr[i] = i1 + i2 + (int)darr[i];
        
        /* Conditional to add control flow complexity */
        if (iarr[i] > 1000) {
            iarr[i] = iarr[i] % 1000;
        }
    }
}

/* Function 4: Nested loops with conditional inner logic */
void func4_nested(int* arr, int n, int m) {
    volatile int cond = 1;
    
    for (int outer = 0; outer < m; outer++) {
        /* Outer loop with simple operation */
        arr[outer % n] += outer;
        
        /* Inner loop with carried dependency, only if condition is true */
        if (cond) {
            for (int i = 1; i < n; i++) {
                /* Complex dependency chain with mixed ops */
                int val = arr[i-1] * arr[i];
                arr[i] = (val + i) ^ (arr[i] << 2);
                
                /* Floating point to increase latency diversity */
                float fval = (float)arr[i] / 3.14159f;
                arr[i] += (int)(fval * 100.0f);
            }
        }
        
        /* Compiler barrier */
        asm volatile("" : : "r"(cond) : "memory");
    }
}

/* Function 5: Loop with varying distance dependencies */
void func5_varying_dist(int* arr, int n) {
    if (n < 4) return;
    
    /* Initialize first few elements */
    for (int i = 0; i < 4; i++) {
        arr[i] = i + 1;
    }
    
    /* Loop with dependencies at different distances */
    for (int i = 4; i < n; i++) {
        /* Distance 1 dependency */
        int d1 = arr[i-1] * 2;
        
        /* Distance 2 dependency */
        int d2 = arr[i-2] / 3;
        
        /* Distance 3 dependency */
        int d3 = arr[i-3] + 5;
        
        /* Distance 4 dependency */
        int d4 = arr[i-4] - 2;
        
        /* Combine all dependencies */
        arr[i] = d1 + d2 * d3 - d4;
        
        /* Additional operation with immediate */
        arr[i] = (arr[i] << 1) | (arr[i] >> 31);
    }
}

int main() {
    /* Allocate and initialize data arrays */
    volatile int arr1[SIZE];
    int arr2[SIZE];
    double darr[SIZE];
    int iarr[SIZE];
    int arr4[SIZE];
    int arr5[SIZE];
    
    /* Initialize with non-zero values */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = i + 1;
        arr2[i] = (i * 3) % 97;
        darr[i] = (double)(i + 1) * 0.5;
        iarr[i] = i * 2;
        arr4[i] = i % 50;
        arr5[i] = i + 10;
    }
    
    /* Execute all functions multiple times */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        func1_carried_dep(arr1, SIZE);
        
        int sum = func2_reduction(arr2, SIZE);
        sink += sum;  /* Use result to prevent elimination */
        
        func3_mixed_ops(darr, iarr, SIZE);
        
        func4_nested(arr4, SIZE, 10);
        
        func5_varying_dist(arr5, SIZE);
        
        /* Mix data between iterations to create dependencies */
        arr1[0] = sum % 100;
        arr2[0] = iter;
    }
    
    /* Final computation to use all results */
    int final_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        final_sum += arr1[i] + arr2[i] + (int)darr[i] + iarr[i] + arr4[i] + arr5[i];
    }
    
    printf("Result checksum: %d\n", final_sum % 1000);
    
    return 0;
}
