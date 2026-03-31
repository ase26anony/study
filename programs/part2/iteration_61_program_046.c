/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler with distance-1 dependences
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c -mtune=itanium
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Prevent optimizations and ensure loops remain */
static volatile int g_volatile_size = SIZE;

/* Test 1: Floating-point recurrence with mixed operations */
__attribute__((noinline))
double test_fp_recurrence(double* arr, double factor, int n) {
    double sum = 0.0;
    /* Distance-1 dependence: arr[i] depends on arr[i-1] */
    for (int i = 1; i < n; i++) {
        /* Multiple operations with different latencies */
        double temp = arr[i-1] * factor;      /* FP multiply - higher latency */
        temp += (double)i * 0.5;              /* FP multiply + add */
        arr[i] = temp + arr[i];               /* FP add */
        sum += arr[i];                        /* Accumulator */
    }
    return sum;
}

/* Test 2: Integer recurrence with memory aliasing */
__attribute__((noinline))
int test_int_recurrence(int* arr, int* brr, int n) {
    int sum = 0;
    /* Complex recurrence with pointer-based accesses */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence through memory */
        int val = arr[i-1] + brr[i];          /* Integer add */
        val *= 3;                             /* Integer multiply - higher latency */
        arr[i] = val ^ (val >> 2);            /* Bit operations */
        sum += arr[i] * brr[i-1];             /* Mixed operations */
    }
    return sum;
}

/* Test 3: Mixed float/int recurrence with conditional */
__attribute__((noinline))
float test_mixed_recurrence(float* farr, int* iarr, int n) {
    float total = 0.0f;
    /* Multiple interleaved recurrences */
    for (int i = 2; i < n; i++) {
        /* Two separate distance-1 dependences */
        float f1 = farr[i-1] * 1.01f;         /* FP multiply */
        float f2 = farr[i-2] * 0.99f;         /* Distance-2 for variety */
        farr[i] = f1 + f2 + (float)iarr[i];   /* FP add with int conversion */
        
        int ival = iarr[i-1] * 2;             /* Integer multiply */
        iarr[i] = ival + (int)farr[i];        /* Mixed type operation */
        
        total += farr[i] * (float)iarr[i];    /* FP multiply */
    }
    return total;
}

/* Test 4: Pointer-chasing recurrence */
__attribute__((noinline))
double test_pointer_chase(double** ptrs, double* data, int n) {
    double result = 0.0;
    /* Simulate pointer chasing pattern */
    for (int i = 1; i < n; i++) {
        /* Distance-1 through pointer indirection */
        double* current = ptrs[i-1];
        double val = *current * 2.0;          /* Load + FP multiply */
        data[i] = val + data[i-1];            /* Distance-1 FP add */
        ptrs[i] = &data[i];                   /* Update pointer */
        result += data[i];
    }
    return result;
}

/* Test 5: Complex recurrence chain */
__attribute__((noinline))
int test_complex_chain(int* arr, int n) {
    int a = arr[0], b = arr[1], c = arr[2];
    int sum = a + b + c;
    
    /* Multiple inter-dependent recurrences */
    for (int i = 3; i < n; i++) {
        /* Three separate distance-1 chains */
        int new_a = (a * 7) ^ (b << 2);       /* Multiply + shift */
        int new_b = (b + c) * 3;              /* Add + multiply */
        int new_c = (c ^ a) + i;              /* XOR + add */
        
        a = new_a;
        b = new_b;
        c = new_c;
        
        arr[i] = a + b - c;
        sum += arr[i] * i;                    /* Multiply for latency */
    }
    return sum;
}

int main(int argc, char** argv) {
    /* Use volatile to prevent constant propagation */
    volatile int use_size = g_volatile_size;
    int n = (argc > 1) ? atoi(argv[1]) : use_size;
    if (n < 10) n = 100;  /* Ensure minimum size */
    
    /* Allocate and initialize arrays */
    double* darr1 = (double*)malloc(n * sizeof(double));
    double* darr2 = (double*)malloc(n * sizeof(double));
    int* iarr1 = (int*)malloc(n * sizeof(int));
    int* iarr2 = (int*)malloc(n * sizeof(int));
    float* farr = (float*)malloc(n * sizeof(float));
    double** ptrs = (double**)malloc(n * sizeof(double*));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < n; i++) {
        darr1[i] = (double)(i % 100) * 0.1;
        darr2[i] = (double)(i % 50) * 0.2;
        iarr1[i] = i * 3;
        iarr2[i] = i * 7 + 1;
        farr[i] = (float)i * 0.3f;
        ptrs[i] = &darr1[i];
    }
    
    double total = 0.0;
    
    /* Call test functions - each with different recurrence patterns */
    total += test_fp_recurrence(darr1, 1.05, n);
    total += (double)test_int_recurrence(iarr1, iarr2, n);
    total += (double)test_mixed_recurrence(farr, iarr1, n);
    total += test_pointer_chase(ptrs, darr2, n);
    total += (double)test_complex_chain(iarr2, n);
    
    /* Use results to prevent dead code elimination */
    printf("Total result: %f\n", total);
    
    /* Cleanup */
    free(darr1);
    free(darr2);
    free(iarr1);
    free(iarr2);
    free(farr);
    free(ptrs);
    
    return (int)total % 256;
}
