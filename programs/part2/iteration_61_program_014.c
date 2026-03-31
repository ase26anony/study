/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler edge logging (lines 596-606 in modulo-sched.cc)
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c -mtune=itanium
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Prevent inlining to keep loop structure intact */
__attribute__((noinline, optimize("no-unroll-loops")))
double test1_recurrence_fp(double* arr, int n, double factor) {
    double sum = 0.0;
    /* Loop with floating-point recurrence (distance=1) */
    for (int i = 1; i < n; i++) {
        arr[i] = arr[i-1] * factor + (double)i * 1.5;  /* FP mult + add with recurrence */
        sum += arr[i];
    }
    return sum;
}

__attribute__((noinline, optimize("no-unroll-loops")))
int test2_mixed_latency(int* arr, double* farr, int n) {
    int total = 0;
    /* Mixed integer/FP operations with loop-carried dependence */
    for (int i = 1; i < n; i++) {
        farr[i] = farr[i-1] * 1.01 + farr[i];          /* FP recurrence */
        arr[i] = arr[i-1] * 3 + i;                     /* Integer recurrence */
        total += arr[i] + (int)farr[i];                /* Mixed type operation */
    }
    return total;
}

__attribute__((noinline, optimize("no-unroll-loops")))
float test3_pointer_chase(float** ptrs, float* data, int n) {
    float result = 0.0f;
    /* Pointer-based recurrence simulating linked list traversal */
    for (int i = 0; i < n-1; i++) {
        *ptrs[i+1] = *ptrs[i] * 2.5f + data[i];        /* Load-store recurrence */
        result += *ptrs[i];
    }
    return result;
}

__attribute__((noinline, optimize("no-unroll-loops")))
double test4_complex_dependence(double* a, double* b, int* c, int n) {
    double acc = 0.0;
    /* Multiple interleaved dependences with different latencies */
    for (int i = 2; i < n; i++) {
        /* Distance-2 dependence: a[i] depends on a[i-2] */
        a[i] = a[i-2] * 0.99 + b[i-1] * 1.1;           /* FP mult chain */
        
        /* Distance-1 dependence: b[i] depends on b[i-1] */
        b[i] = b[i-1] * 0.95 + (double)c[i];
        
        /* Integer recurrence with memory access */
        c[i] = c[i-1] + c[i-2] + i;                    /* Fibonacci-like */
        
        acc += a[i] + b[i] + c[i];
    }
    return acc;
}

__attribute__((noinline, optimize("no-unroll-loops")))
int test5_artificial_chain(int* arr, int n, int seed) {
    int x = seed;
    int y = seed * 2;
    int z = seed * 3;
    
    /* Artificial serial chain across iterations */
    for (int i = 0; i < n; i++) {
        x = x * 3 + arr[i];        /* Distance-1 dependence on x */
        y = y + x * 2;             /* Depends on x from same iteration */
        z = z * 5 + y;             /* Depends on y from same iteration */
        arr[i] = z;                /* Store result creates memory dependence */
    }
    
    return x + y + z;
}

int main(int argc, char** argv) {
    /* Use volatile to prevent constant propagation */
    volatile int size = (argc > 1) ? atoi(argv[1]) : SIZE;
    int n = size;
    
    /* Allocate and initialize arrays */
    double* arr1 = (double*)malloc(n * sizeof(double));
    int* arr2 = (int*)malloc(n * sizeof(int));
    double* arr3 = (double*)malloc(n * sizeof(double));
    float** ptrs = (float**)malloc(n * sizeof(float*));
    float* data = (float*)malloc(n * sizeof(float));
    double* a = (double*)malloc(n * sizeof(double));
    double* b = (double*)malloc(n * sizeof(double));
    int* c = (int*)malloc(n * sizeof(int));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < n; i++) {
        arr1[i] = (double)(i % 100) * 0.1;
        arr2[i] = i % 50;
        arr3[i] = (double)(i % 75) * 0.3;
        data[i] = (float)(i % 60) * 0.5f;
        ptrs[i] = &data[(i * 17) % n];  /* Create pointer aliasing */
        a[i] = (double)(i % 80) * 0.2;
        b[i] = (double)(i % 90) * 0.4;
        c[i] = i % 70;
    }
    
    double total = 0.0;
    
    /* Call test functions to trigger modulo scheduling analysis */
    total += test1_recurrence_fp(arr1, n, 1.05);
    total += (double)test2_mixed_latency(arr2, arr3, n);
    total += (double)test3_pointer_chase(ptrs, data, n);
    total += test4_complex_dependence(a, b, c, n);
    total += (double)test5_artificial_chain(arr2, n, 42);
    
    printf("Result: %f\n", total);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(ptrs);
    free(data);
    free(a);
    free(b);
    free(c);
    
    return 0;
}
