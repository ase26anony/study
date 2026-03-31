/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler with distance-1 dependences
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c -mtune=itanium
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Prevent inlining to keep loop structure intact */
__attribute__((noinline))
static double test1_recurrence_fp(double* arr, int n, double factor) {
    double sum = 0.0;
    /* Loop with FP recurrence (distance=1) */
    for (int i = 1; i < n; i++) {
        arr[i] = arr[i-1] * factor + (double)i * 1.5;
        sum += arr[i];
    }
    return sum;
}

__attribute__((noinline))
static int test2_mixed_latencies(int* arr, int* brr, int n, int factor) {
    int total = 0;
    /* Mixed operations with memory and arithmetic */
    for (int i = 1; i < n; i++) {
        int temp = arr[i-1] * factor;      /* Multiplication latency */
        brr[i] = temp + brr[i] * 3;        /* Another multiplication */
        arr[i] = brr[i-1] + i;             /* Distance-1 dependence */
        total += arr[i] + brr[i];
    }
    return total;
}

__attribute__((noinline))
static float test3_pointer_chase(float** ptrs, float* data, int n) {
    float sum = 0.0f;
    /* Pointer-based recurrence */
    for (int i = 0; i < n-1; i++) {
        *ptrs[i+1] = *ptrs[i] * 2.0f + data[i];
        sum += *ptrs[i+1];
    }
    return sum;
}

__attribute__((noinline))
static double test4_complex_chain(double* a, double* b, double* c, int n) {
    double acc = 0.0;
    /* Multiple interleaved dependences */
    for (int i = 1; i < n; i++) {
        a[i] = b[i-1] * c[i-1] + (double)i;  /* Distance-1, FP mult */
        b[i] = a[i] * 0.5 + b[i-1];          /* Distance-1, FP mult */
        c[i] = a[i-1] + b[i] * c[i];         /* Distance-1, FP mult */
        acc += a[i] + b[i] + c[i];
    }
    return acc;
}

__attribute__((noinline))
static int test5_integer_recurrence(int* arr, int n, int mod) {
    int hash = 0;
    /* Integer recurrence with modulo (harder to optimize) */
    for (int i = 1; i < n; i++) {
        arr[i] = (arr[i-1] * 1103515245 + 12345) & 0x7fffffff;
        hash ^= arr[i] % mod;
    }
    return hash;
}

int main(int argc, char** argv) {
    /* Use volatile to prevent constant propagation */
    volatile int size = (argc > 1) ? atoi(argv[1]) : SIZE;
    if (size < 10) size = SIZE;
    
    /* Allocate and initialize arrays */
    double* arr1 = (double*)malloc(size * sizeof(double));
    int* arr2 = (int*)malloc(size * sizeof(int));
    int* arr3 = (int*)malloc(size * sizeof(int));
    float** ptrs = (float**)malloc(size * sizeof(float*));
    float* data = (float*)malloc(size * sizeof(float));
    double* a = (double*)malloc(size * sizeof(double));
    double* b = (double*)malloc(size * sizeof(double));
    double* c = (double*)malloc(size * sizeof(double));
    int* arr5 = (int*)malloc(size * sizeof(int));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < size; i++) {
        arr1[i] = (double)(i % 100) * 0.1;
        arr2[i] = i * 2;
        arr3[i] = i * 3;
        data[i] = (float)(i % 50) * 0.2f;
        a[i] = (double)(i % 30);
        b[i] = (double)(i % 40);
        c[i] = (double)(i % 20);
        arr5[i] = i;
        
        /* Create pointer array with some aliasing */
        ptrs[i] = &data[(i * 7) % size];
    }
    
    double result = 0.0;
    
    /* Call test functions to trigger modulo scheduling */
    result += test1_recurrence_fp(arr1, size, 1.01);
    result += (double)test2_mixed_latencies(arr2, arr3, size, 7);
    result += (double)test3_pointer_chase(ptrs, data, size);
    result += test4_complex_chain(a, b, c, size);
    result += (double)test5_integer_recurrence(arr5, size, 997);
    
    printf("Final result: %f\n", result);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(ptrs);
    free(data);
    free(a);
    free(b);
    free(c);
    free(arr5);
    
    return 0;
}
