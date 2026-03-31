/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler edge logging (lines 596-606 in modulo-sched.cc)
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c -mtune=itanium
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Prevent inlining to keep loop structure intact */
__attribute__((noinline))
double test1_recurrence_fp(double* arr, int n) {
    double sum = 0.0;
    /* Loop with floating-point recurrence (distance=1) */
    for (int i = 1; i < n; i++) {
        arr[i] = arr[i-1] * 1.01 + (double)i * 0.5;  /* FP mult + add with recurrence */
        sum += arr[i];
    }
    return sum;
}

__attribute__((noinline))
int test2_mixed_latency(int* arr, double* farr, int n) {
    int total = 0;
    /* Mixed integer/FP operations with memory aliasing */
    for (int i = 1; i < n; i++) {
        farr[i] = farr[i-1] * 1.5;           /* FP recurrence */
        arr[i] = arr[i-1] + (int)farr[i];    /* Integer recurrence using FP result */
        total += arr[i] * i;                  /* Integer multiply with latency */
    }
    return total;
}

__attribute__((noinline))
float test3_pointer_chase(float** ptrs, float* data, int n) {
    float sum = 0.0f;
    /* Pointer-based recurrence simulating linked list traversal */
    for (int i = 0; i < n-1; i++) {
        *ptrs[i+1] = *ptrs[i] * 2.0f + data[i];  /* Load from prev, store to next */
        sum += *ptrs[i];
    }
    return sum;
}

__attribute__((noinline))
double test4_complex_dependence(double* a, double* b, double* c, int n) {
    double total = 0.0;
    /* Multiple interleaved dependences */
    for (int i = 2; i < n; i++) {
        a[i] = a[i-1] * b[i-2] + 1.0;    /* Distance 1 and 2 dependences */
        b[i] = b[i-1] * 0.99 + a[i];     /* Another distance 1 dependence */
        c[i] = c[i-1] + a[i] * b[i];     /* FP multiply-add chain */
        total += c[i];
    }
    return total;
}

__attribute__((noinline))
int test5_integer_recurrence(int* arr, int n) {
    int sum = 0;
    /* Integer recurrence with multiplication (higher latency than add) */
    for (int i = 3; i < n; i++) {
        arr[i] = arr[i-1] * arr[i-2] + arr[i-3];  /* Multiple distance dependences */
        sum += arr[i] % 7;  /* Modulo operation adds complexity */
    }
    return sum;
}

int main(int argc, char** argv) {
    /* Use volatile to prevent constant propagation */
    volatile int data_size = (argc > 1) ? atoi(argv[1]) : SIZE;
    int n = data_size;
    
    /* Allocate and initialize arrays */
    double* arr1 = (double*)malloc(n * sizeof(double));
    int* arr2 = (int*)malloc(n * sizeof(int));
    double* arr3 = (double*)malloc(n * sizeof(double));
    float** ptrs = (float**)malloc(n * sizeof(float*));
    float* data = (float*)malloc(n * sizeof(float));
    double* a = (double*)malloc(n * sizeof(double));
    double* b = (double*)malloc(n * sizeof(double));
    double* c = (double*)malloc(n * sizeof(double));
    int* arr5 = (int*)malloc(n * sizeof(int));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < n; i++) {
        arr1[i] = (double)(i % 10 + 1);
        arr2[i] = i % 5 + 1;
        arr3[i] = (double)(i % 7 + 1);
        data[i] = (float)(i % 3 + 1);
        ptrs[i] = &data[(i + 1) % n];
        a[i] = (double)(i % 4 + 1);
        b[i] = (double)(i % 6 + 1);
        c[i] = (double)(i % 8 + 1);
        arr5[i] = i % 9 + 1;
    }
    
    double result1 = 0.0;
    int result2 = 0;
    float result3 = 0.0f;
    double result4 = 0.0;
    int result5 = 0;
    
    /* Call test functions multiple times to ensure execution */
    for (int iter = 0; iter < 3; iter++) {
        result1 += test1_recurrence_fp(arr1, n);
        result2 += test2_mixed_latency(arr2, arr3, n);
        result3 += test3_pointer_chase(ptrs, data, n);
        result4 += test4_complex_dependence(a, b, c, n);
        result5 += test5_integer_recurrence(arr5, n);
    }
    
    /* Print results to prevent dead code elimination */
    printf("Results: %.2f, %d, %.2f, %.2f, %d\n", 
           result1, result2, result3, result4, result5);
    
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
