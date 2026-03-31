/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler with distance-1 dependences
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c -mtune=itanium
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024

/* Prevent inlining to keep loop structure intact */
__attribute__((noinline, optimize("no-unroll-loops")))
double test_recurrence_fp(double* arr, int n, double factor) {
    double sum = 0.0;
    /* Loop with floating-point recurrence (distance=1) */
    for (int i = 1; i < n; i++) {
        arr[i] = arr[i-1] * factor + (double)i * 0.5;  /* FP mult + add */
        sum += arr[i];
    }
    return sum;
}

__attribute__((noinline, optimize("no-unroll-loops")))
int test_mixed_latency(int* arr, double* farr, int n) {
    int sum = 0;
    /* Mixed integer/FP recurrence with memory accesses */
    for (int i = 1; i < n; i++) {
        farr[i] = farr[i-1] * 1.01 + (double)arr[i];  /* FP recurrence */
        arr[i] = (int)farr[i] * 3 + arr[i-1];         /* Integer recurrence */
        sum += arr[i];
    }
    return sum;
}

__attribute__((noinline, optimize("no-unroll-loops")))
double test_pointer_chase(double** ptrs, double* data, int n) {
    double sum = 0.0;
    /* Pointer-based recurrence with memory latency */
    for (int i = 0; i < n-1; i++) {
        *ptrs[i+1] = (*ptrs[i]) * 1.5 + data[i];  /* Distance-1 through pointers */
        sum += *ptrs[i];
    }
    return sum;
}

__attribute__((noinline, optimize("no-unroll-loops")))
int test_complex_dependence(int* a, int* b, double* c, int n) {
    int sum = 0;
    /* Multiple interleaved dependences */
    for (int i = 1; i < n; i++) {
        /* Three separate recurrence chains */
        a[i] = a[i-1] * 2 + i;           /* Integer recurrence */
        b[i] = b[i-1] + a[i] * 3;        /* Mixed recurrence */
        c[i] = c[i-1] * 0.99 + (double)b[i]; /* FP recurrence */
        
        /* Cross-iteration use with latency */
        sum += (int)(c[i] * 100.0) + a[i] + b[i];
    }
    return sum;
}

__attribute__((noinline, optimize("no-unroll-loops")))
double test_nested_dependences(double* arr1, double* arr2, int n) {
    double sum = 0.0;
    /* Multiple FP operations with dependences */
    for (int i = 1; i < n; i++) {
        double temp = arr1[i-1] * arr2[i-1];  /* FP multiplication latency */
        arr1[i] = temp + (double)i * 0.25;    /* FP addition */
        arr2[i] = arr1[i] * 0.9 - arr2[i-1];  /* FP recurrence */
        sum += arr1[i] + arr2[i];
    }
    return sum;
}

int main(int argc, char** argv) {
    /* Use volatile to prevent constant propagation */
    volatile int data_size = (argc > 1) ? atoi(argv[1]) : SIZE;
    int n = data_size;
    
    /* Allocate and initialize arrays */
    double* arr_fp1 = (double*)malloc(n * sizeof(double));
    double* arr_fp2 = (double*)malloc(n * sizeof(double));
    int* arr_int1 = (int*)malloc(n * sizeof(int));
    int* arr_int2 = (int*)malloc(n * sizeof(int));
    double** ptr_array = (double**)malloc(n * sizeof(double*));
    double* data = (double*)malloc(n * sizeof(double));
    
    /* Initialize with non-zero values */
    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        arr_fp1[i] = (double)(rand() % 100) * 0.1;
        arr_fp2[i] = (double)(rand() % 100) * 0.1;
        arr_int1[i] = rand() % 100;
        arr_int2[i] = rand() % 100;
        data[i] = (double)(rand() % 100) * 0.01;
        ptr_array[i] = &data[(i + 1) % n];  /* Create pointer chain */
    }
    
    double total = 0.0;
    
    /* Call test functions to trigger modulo scheduling analysis */
    total += test_recurrence_fp(arr_fp1, n, 1.05);
    total += (double)test_mixed_latency(arr_int1, arr_fp2, n);
    total += test_pointer_chase(ptr_array, data, n);
    total += (double)test_complex_dependence(arr_int1, arr_int2, arr_fp1, n);
    total += test_nested_dependences(arr_fp1, arr_fp2, n);
    
    /* Use results to prevent dead code elimination */
    printf("Total checksum: %f\n", total);
    
    /* Cleanup */
    free(arr_fp1);
    free(arr_fp2);
    free(arr_int1);
    free(arr_int2);
    free(ptr_array);
    free(data);
    
    return (int)total % 100;
}
