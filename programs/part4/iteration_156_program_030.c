/* Test program to trigger modulo scheduling edge printing */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define ITERS 100

/* Global volatile sink to prevent elimination */
volatile int sink = 0;

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

/* Function 2: Reduction loop with floating point */
void func2_reduction(float* farr, int* iarr, int n) {
    float sum = farr[0];
    int isum = iarr[0];
    
    for (int i = 1; i < n; i++) {
        /* Floating point operations (higher latency) */
        sum = sum * 1.01f + farr[i];
        
        /* Integer operations in parallel */
        isum = (isum * 3 + iarr[i]) & 0xFFF;
        
        /* Cross-type dependency */
        farr[i] = sum + (float)isum;
    }
    
    /* Use results */
    sink += (int)sum + isum;
}

/* Function 3: Pointer chasing with conditional */
void func3_pointer_chase(int* arr, int n, int threshold) {
    int* p = arr;
    int count = 0;
    
    /* Loop with pointer chasing pattern */
    for (int i = 0; i < n && count < ITERS; i++) {
        /* Conditional inner logic */
        if (*p > threshold) {
            *p = (*p * 2) | 1;      /* Mixed mul and bitwise */
            count++;
        }
        
        /* Pointer chase with stride */
        p = arr + ((p - arr + 5) % n);
        
        /* Memory barrier */
        asm volatile("" : : "r"(p) : "memory");
    }
}

/* Function 4: Multiple independent statements with final dependency */
void func4_multi_dep(double* darr, int* iarr, int n) {
    for (int i = 1; i < n; i++) {
        /* Independent computations */
        double a = darr[i-1] * 1.5;
        int b = iarr[i] * 3;
        double c = a * 0.75;
        int d = b ^ 0xAA;
        
        /* Final dependent store with mixed types */
        darr[i] = a + c + (double)(b + d);
        iarr[i] = (int)(darr[i]) * 2;
    }
}

/* Function 5: Nested loop with outer condition */
void func5_nested(int* arr, int n, int outer_iters) {
    for (int iter = 0; iter < outer_iters; iter++) {
        /* Only pipeline inner loops with enough iterations */
        if (iter % 2 == 0) {
            /* Inner loop with carried dependency */
            int acc = arr[0];
            for (int i = 1; i < n; i++) {
                acc = (acc * 13 + arr[i]) % 1000;
                arr[i] = acc;
                
                /* Prevent vectorization */
                asm volatile("" : "+r"(acc) : : "memory");
            }
        } else {
            /* Different pattern to create varied scheduling */
            for (int i = n-1; i > 0; i--) {
                arr[i] = arr[i-1] + (arr[i] << 2);
            }
        }
    }
}

/* Main driver */
int main() {
    /* Initialize data */
    volatile int arr1[SIZE];
    float arr2[SIZE];
    int arr3[SIZE];
    double arr4[SIZE];
    int arr5[SIZE];
    
    /* Seed with non-zero values */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (i * 3 + 7) % 100;
        arr2[i] = (float)(i * 0.7 + 2.5);
        arr3[i] = (i * 11 + 13) % 255;
        arr4[i] = (double)(i * 0.3 + 1.2);
        arr5[i] = i * 5;
    }
    
    /* Call all functions to ensure they're compiled */
    func1_carried_dep(arr1, SIZE, 3);
    func2_reduction(arr2, arr3, SIZE);
    func3_pointer_chase(arr3, SIZE, 50);
    func4_multi_dep(arr4, arr3, SIZE);
    func5_nested(arr5, SIZE, 10);
    
    /* Use results to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += arr1[i] + (int)arr2[i] + arr3[i] + (int)arr4[i] + arr5[i];
        checksum &= 0xFFFF;
    }
    
    sink += checksum;
    
    /* Print minimal output */
    printf("Result: %d\n", sink & 1);
    
    return 0;
}
