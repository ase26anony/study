/* Test program to trigger modulo scheduling edge printing in GCC */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define ITERS 100

/* Global volatile sink to prevent optimization */
volatile int sink = 0;

/* Function 1: Loop with carried dependency (distance 1) */
int func1_carried_dep(volatile int* arr, int n) {
    int sum = 0;
    /* Loop with recurrence: arr[i] depends on arr[i-1] */
    for (int i = 1; i < n; i++) {
        arr[i] = arr[i-1] * 3 + arr[i];
        sum += arr[i];
    }
    /* Mix integer and bitwise operations */
    for (int i = 0; i < n; i++) {
        arr[i] = (arr[i] << 2) | (arr[i] >> 30);
        sum ^= arr[i];
    }
    return sum;
}

/* Function 2: Mixed integer/float operations with dependencies */
double func2_mixed_ops(double* darr, int* iarr, int n) {
    double acc = 1.0;
    /* Floating-point recurrence */
    for (int i = 1; i < n; i++) {
        darr[i] = darr[i-1] * 1.5 + darr[i] * 0.5;
        acc *= darr[i];
    }
    /* Integer operations with memory dependencies */
    for (int i = 0; i < n-1; i++) {
        iarr[i] = iarr[i] * iarr[i+1] + i;
        acc += iarr[i];
    }
    return acc;
}

/* Function 3: Pointer chasing pattern */
int func3_pointer_chase(int* base, int n) {
    int* p = base;
    int sum = 0;
    /* Pointer chasing creates carried dependency */
    for (int i = 0; i < n; i++) {
        sum += *p;
        /* Create dependency chain */
        p = base + (*p % n);
        /* Insert compiler barrier */
        asm volatile("" : : "r"(p) : "memory");
    }
    return sum;
}

/* Function 4: Reduction with multiple dependencies */
long func4_complex_reduction(int* arr1, int* arr2, int n) {
    long total = 0;
    /* Multiple independent statements followed by dependent store */
    for (int i = 1; i < n; i++) {
        int t1 = arr1[i-1] * 7;
        int t2 = arr2[i] + arr2[i-1];
        int t3 = t1 ^ t2;
        arr1[i] = t3 + i;
        arr2[i] = arr1[i] >> 1;
        total += t1 + t2 + t3;
    }
    return total;
}

/* Function 5: Nested loops with conditional inner logic */
int func5_nested_conditional(int* arr, int n, int flag) {
    int result = 0;
    /* Outer loop */
    for (int outer = 0; outer < 10; outer++) {
        /* Conditional inner loop */
        if (flag || outer > 0) {
            /* Inner loop with carried dependency */
            for (int i = 1; i < n; i++) {
                arr[i] = (arr[i-1] + arr[i]) * (i % 8 + 1);
                result += arr[i];
            }
        }
        /* Additional operation to prevent over-simplification */
        result ^= outer;
    }
    return result;
}

/* Wrapper functions to create different compilation contexts */
void run_test_suite(void) {
    /* Initialize data arrays */
    volatile int arr1[SIZE];
    int arr2[SIZE];
    double darr[SIZE];
    int arr3[SIZE];
    int arr4[SIZE];
    int arr5[SIZE];
    
    /* Initialize with non-zero values */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (i * 3 + 7) % 97;
        arr2[i] = (i * 5 + 11) % 101;
        darr[i] = (i * 0.7 + 1.3);
        arr3[i] = (i * 13 + 17) % 103;
        arr4[i] = (i * 19 + 23) % 107;
        arr5[i] = (i * 29 + 31) % 109;
    }
    
    /* Call all test functions multiple times */
    for (int iter = 0; iter < ITERS; iter++) {
        sink += func1_carried_dep(arr1, SIZE - iter % 32);
        
        double dresult = func2_mixed_ops(darr, arr2, SIZE - iter % 16);
        sink += (int)dresult;
        
        sink += func3_pointer_chase(arr3, SIZE - iter % 24);
        
        long lresult = func4_complex_reduction(arr4, arr5, SIZE - iter % 28);
        sink += (int)lresult;
        
        sink += func5_nested_conditional(arr3, SIZE - iter % 20, iter & 1);
        
        /* Modify arrays slightly between iterations */
        for (int i = 0; i < SIZE; i++) {
            arr1[i] += iter;
            arr2[i] ^= iter;
            darr[i] += iter * 0.1;
        }
    }
}

int main(void) {
    /* Run the test suite */
    run_test_suite();
    
    /* Use the sink value to prevent dead code elimination */
    printf("Result checksum: %d\n", sink);
    
    return 0;
}
