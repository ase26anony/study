/* Test program to trigger modulo scheduling edge printing in GCC */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define ITERS 100

/* Global volatile to prevent optimization */
volatile int global_sink = 0;

/* Function 1: Loop with carried dependency and mixed operations */
int func1_carried_dep(volatile int* arr, int n, int scalar) {
    int result = 0;
    /* Loop with carried dependency (distance 1) */
    for (int i = 1; i < n; i++) {
        /* Mix integer and floating point operations */
        int temp = arr[i-1] * scalar;
        float ftemp = (float)temp * 1.5f;
        arr[i] = (int)ftemp + arr[i] + (i & 0xF); /* Add bitwise op */
        result += arr[i];
    }
    /* Compiler barrier to preserve operations */
    asm volatile("" : : "r"(result) : "memory");
    return result;
}

/* Function 2: Reduction loop with pointer chasing pattern */
int func2_reduction(volatile int* arr, int n) {
    int sum = arr[0];
    volatile int* ptr = arr;
    
    /* Loop with memory dependencies */
    for (int i = 1; i < n; i++) {
        int val = *ptr;
        ptr = &arr[i];
        sum = sum * 3 + val + (sum >> 2); /* Mix arithmetic and shift */
        
        /* Floating point in the mix */
        if (i % 4 == 0) {
            float fsum = (float)sum * 0.25f;
            sum = (int)fsum;
        }
    }
    
    /* Force dependency chain */
    asm volatile("" : "+r"(sum) : : "memory");
    return sum;
}

/* Function 3: Multiple independent statements with dependent store */
int func3_multidep(volatile int* arr, volatile float* farr, int n) {
    int acc1 = 1, acc2 = 2, acc3 = 3;
    
    for (int i = 0; i < n; i++) {
        /* Independent computations */
        int t1 = arr[i] * 7;
        int t2 = t1 ^ 0xAAAA; /* Bitwise XOR */
        float t3 = (float)t2 * 3.14159f;
        
        /* Dependent store with carried dependency */
        if (i > 0) {
            acc1 = acc1 * 2 + arr[i-1];
        }
        acc2 = acc2 + t1;
        acc3 = acc3 ^ t2;
        
        /* Store with dependency on previous iteration */
        farr[i] = t3 + (float)acc1;
        arr[i] = acc2 + acc3;
    }
    
    /* Combine results */
    int result = acc1 + acc2 + acc3;
    asm volatile("" : : "r"(result) : "memory");
    return result;
}

/* Function 4: Nested loop with conditional inner logic */
int func4_nested(volatile int* arr, int n, int threshold) {
    int total = 0;
    
    /* Outer loop to encourage loop analysis */
    for (int outer = 0; outer < 3; outer++) {
        int inner_sum = 0;
        
        /* Inner loop with conditional */
        if (threshold > 0) {
            for (int i = 1; i < n; i++) {
                /* Complex carried dependency */
                int base = arr[i-1];
                arr[i] = base * (i + outer) - (arr[i] >> 1);
                
                /* Mix operations */
                float fval = (float)arr[i] * 0.333f;
                if (i % 8 == 0) {
                    fval = fval * 2.0f;
                }
                
                inner_sum += (int)fval + (base & 0xFF);
            }
        }
        total += inner_sum;
    }
    
    asm volatile("" : "+r"(total) : : "memory");
    return total;
}

/* Function 5: Loop with varying dependency distances */
int func5_varying_dist(volatile int* arr, int n) {
    int sum = 0;
    
    for (int i = 4; i < n; i++) {
        /* Dependency with distance 4 */
        int val1 = arr[i-4] * 11;
        
        /* Dependency with distance 2 */
        int val2 = arr[i-2] + (val1 & 0xFF);
        
        /* Dependency with distance 1 */
        arr[i] = val2 + arr[i-1] * 3;
        
        /* Floating point chain */
        float fsum = (float)sum * 1.01f;
        sum = (int)fsum + arr[i] + (i % 16);
    }
    
    asm volatile("" : "+r"(sum) : : "memory");
    return sum;
}

int main() {
    /* Initialize data arrays */
    volatile int arr1[SIZE];
    volatile int arr2[SIZE];
    volatile float farr[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (i * 3 + 1) & 0xFF;
        arr2[i] = (i * 5 + 2) & 0xFF;
        farr[i] = (float)i * 0.5f;
    }
    
    /* Call all functions to ensure they're compiled */
    int result = 0;
    
    result += func1_carried_dep(arr1, ITERS, 7);
    result += func2_reduction(arr2, ITERS);
    result += func3_multidep(arr1, farr, ITERS);
    result += func4_nested(arr2, ITERS, 1);
    result += func5_varying_dist(arr1, ITERS);
    
    /* Use result to prevent dead code elimination */
    global_sink = result;
    
    /* Print minimal output */
    printf("Result checksum: %d\n", global_sink);
    
    return 0;
}
