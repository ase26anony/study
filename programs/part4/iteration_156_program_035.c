/* Test program to trigger modulo scheduling edge printing in GCC */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define ITERS 100

/* Global volatile to prevent optimization */
volatile int global_sink = 0;

/* Function 1: Loop with carried dependency and mixed operations */
int func1_carried_dep(volatile int* arr, int n, int scalar) {
    int sum = 0;
    /* Loop with distance-1 carried dependency */
    for (int i = 1; i < n; i++) {
        /* Mixed integer operations with dependency chain */
        int temp = arr[i-1] * scalar;
        temp = temp + (temp >> 3);  /* Bitwise operation */
        arr[i] = temp + arr[i] * 7; /* Another multiplication */
        sum += arr[i];
        
        /* Fake dependency barrier */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Function 2: Reduction loop with floating point operations */
float func2_fp_reduction(float* arr, int n, float coeff) {
    float acc = arr[0];
    /* Loop with floating point operations */
    for (int i = 1; i < n; i++) {
        /* FP operations with different latencies */
        float t1 = arr[i] * coeff;
        float t2 = t1 + 1.5f;
        acc = acc * 0.99f + t2;
        arr[i] = acc;
        
        /* Prevent dead code elimination */
        asm volatile("" : "+f"(acc) : : "memory");
    }
    return acc;
}

/* Function 3: Pointer chasing pattern */
int func3_pointer_chase(int* base, int n) {
    int* p = base;
    int sum = 0;
    /* Pointer chasing creates unpredictable dependencies */
    for (int i = 0; i < n; i++) {
        int val = *p;
        val = (val * 13 + 17) & 0xFF;
        *p = val;
        sum += val;
        
        /* Move pointer with wrap-around */
        p = base + ((val * 31) % n);
        if (p >= base + n) p = base;
    }
    return sum;
}

/* Function 4: Multiple independent statements with final dependency */
void func4_multi_dep(int* a, int* b, int* c, int n) {
    /* Outer loop to encourage modulo scheduling */
    for (int iter = 0; iter < 5; iter++) {
        /* Inner loop with complex dependency pattern */
        for (int i = 2; i < n - 2; i++) {
            /* Independent computations */
            int t1 = a[i-2] * 3;
            int t2 = b[i-1] + 7;
            int t3 = c[i] >> 2;
            
            /* Dependent computation */
            int result = (t1 + t2) * t3 - a[i+1];
            
            /* Store with dependency */
            a[i] = result + b[i+1];
            b[i] = result - t1;
            
            /* Memory barrier */
            asm volatile("" : : "r"(result) : "memory");
        }
    }
}

/* Function 5: Nested loops with conditional inner logic */
int func5_nested_conditional(int* arr, int n, int threshold) {
    int total = 0;
    /* Outer loop */
    for (int outer = 0; outer < 3; outer++) {
        /* Conditional inner loop */
        if (threshold > 0) {
            /* Inner loop with carried dependency */
            for (int i = 1; i < n; i++) {
                /* Complex dependency chain */
                int val = arr[i-1] + arr[i] * 2;
                val = (val * 3) / 2;
                val = val ^ (val << 4);
                arr[i] = val;
                total += val;
            }
        } else {
            /* Alternative path */
            for (int i = 0; i < n; i++) {
                arr[i] = arr[i] + 1;
            }
        }
    }
    return total;
}

/* Main driver */
int main() {
    /* Initialize data arrays */
    volatile int arr1[SIZE];
    float arr2[SIZE];
    int arr3[SIZE];
    int arr4_a[SIZE], arr4_b[SIZE], arr4_c[SIZE];
    int arr5[SIZE];
    
    /* Initialize with non-zero values */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (i * 3 + 7) & 0xFF;
        arr2[i] = (float)(i * 2 + 1) * 0.5f;
        arr3[i] = (i * 5 + 11) & 0xFF;
        arr4_a[i] = i;
        arr4_b[i] = i * 2;
        arr4_c[i] = i * 3;
        arr5[i] = (i * 7 + 13) & 0xFF;
    }
    
    /* Call all functions to ensure they're compiled */
    int result1 = func1_carried_dep(arr1, SIZE, 3);
    float result2 = func2_fp_reduction(arr2, SIZE, 1.25f);
    int result3 = func3_pointer_chase(arr3, SIZE / 4);
    func4_multi_dep(arr4_a, arr4_b, arr4_c, SIZE);
    int result5 = func5_nested_conditional(arr5, SIZE, 10);
    
    /* Use results to prevent dead code elimination */
    global_sink = result1 + (int)result2 + result3 + result5;
    
    /* Print minimal output to ensure execution */
    printf("Result checksum: %d\n", global_sink);
    
    return 0;
}
