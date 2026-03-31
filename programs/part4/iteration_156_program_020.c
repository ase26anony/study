/* test_modulo_sched.c - Program to trigger GCC's modulo scheduler edge printing */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 100
#define ITERS 1000

/* Global volatile sink to prevent dead code elimination */
volatile int global_sink = 0;

/* Function 1: Loop with carried dependency and mixed operations */
void func1_carried_dep(volatile int* arr, int n, int scalar) {
    /* Loop with distance-1 carried dependency */
    for (int i = 1; i < n; i++) {
        /* Mixed integer operations with dependency chain */
        int temp = arr[i-1] * scalar;
        temp = temp + (temp >> 3);      /* Bitwise operation */
        arr[i] = temp + arr[i] * 7;     /* Another multiplication */
        
        /* Fake dependency to prevent optimization */
        asm volatile("" : "+r"(temp) : : "memory");
    }
}

/* Function 2: Reduction loop with floating-point operations */
float func2_reduction(float* data, int n) {
    float sum = 0.0f;
    float prod = 1.0f;
    
    /* Loop with multiple dependency chains */
    for (int i = 0; i < n; i++) {
        /* Floating-point operations (different latencies) */
        float val = data[i];
        sum += val * 0.5f;              /* FP multiply-add pattern */
        prod *= val + 1.0f;             /* Carried dependency */
        
        /* Integer operation in the mix */
        int idx = (int)val & 0xFF;      /* Bitwise operation */
        asm volatile("" : "+r"(idx) : : "memory");
    }
    
    return sum + prod;
}

/* Function 3: Pointer chasing with conditional inner logic */
int func3_pointer_chase(int* base, int n) {
    int* p = base;
    int sum = 0;
    
    /* Outer loop to encourage modulo scheduling */
    for (int iter = 0; iter < 10; iter++) {
        /* Inner loop with pointer chasing */
        for (int i = 0; i < n; i++) {
            /* Conditional inside loop */
            if (p != NULL) {
                sum += *p;
                p = base + (*p % n);    /* Pointer chase with dependency */
            }
            
            /* Additional arithmetic */
            sum = (sum * 13 + 7) & 0xFFFF;
        }
        
        /* Reset pointer occasionally */
        if (iter % 3 == 0) {
            p = base;
        }
    }
    
    return sum;
}

/* Function 4: Multiple independent statements with final dependent store */
void func4_multi_ops(int* dst, const int* src1, const int* src2, int n) {
    for (int i = 0; i < n; i++) {
        /* Independent computations */
        int a = src1[i] * 3;
        int b = src2[i] / 2;
        int c = a ^ b;                  /* Bitwise XOR */
        int d = (src1[i] + src2[i]) << 1;
        
        /* Dependent store with carried dependency */
        if (i > 0) {
            dst[i] = dst[i-1] + a + b + c + d;
        } else {
            dst[i] = a + b + c + d;
        }
        
        /* Memory barrier */
        asm volatile("" : : "r"(a), "r"(b) : "memory");
    }
}

/* Function 5: Nested loops with mixed operations */
double func5_nested(double* arr, int n) {
    double result = 0.0;
    
    /* Outer loop */
    for (int j = 0; j < 5; j++) {
        double acc = 1.0;
        
        /* Inner loop - target for modulo scheduling */
        for (int i = 0; i < n; i++) {
            /* Mixed FP and integer operations */
            double val = arr[i] + (double)j;
            acc *= val * 0.75;          /* FP multiplication with carried dependency */
            
            /* Integer computation in parallel */
            int idx = (int)val % 32;
            result += (double)idx * 0.1;
            
            /* Compiler barrier */
            asm volatile("" : "+r"(idx) : : "memory");
        }
        
        result += acc;
    }
    
    return result;
}

int main() {
    /* Initialize data arrays */
    volatile int arr1[SIZE];
    float arr2[SIZE];
    int arr3[SIZE];
    int src1[SIZE], src2[SIZE], dst[SIZE];
    double arr5[SIZE];
    
    /* Initialize with non-zero values */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = i + 1;
        arr2[i] = (float)(i * 0.7);
        arr3[i] = (i * 3) % SIZE;
        src1[i] = i * 2;
        src2[i] = i * 3 + 1;
        arr5[i] = (double)i * 0.25;
    }
    
    /* Force initialization to be used */
    asm volatile("" : : "r"(arr1[0]), "r"(arr2[0]) : "memory");
    
    /* Call all functions to ensure they're compiled */
    func1_carried_dep(arr1, SIZE, 3);
    
    float f2_result = func2_reduction(arr2, SIZE);
    global_sink += (int)f2_result;
    
    int f3_result = func3_pointer_chase(arr3, SIZE / 2);
    global_sink += f3_result;
    
    func4_multi_ops(dst, src1, src2, SIZE);
    global_sink += dst[SIZE-1];
    
    double f5_result = func5_nested(arr5, SIZE);
    global_sink += (int)f5_result;
    
    /* Use results to prevent elimination */
    printf("Result checksum: %d\n", global_sink);
    
    return 0;
}
