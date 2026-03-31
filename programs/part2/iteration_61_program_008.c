/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler edge logging (lines 596-606 in modulo-sched.cc)
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c -mtune=itanium
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Prevent inlining to keep loop structures intact */
__attribute__((noinline))
static double test1_fp_recurrence(double *arr, int n, double factor) {
    double sum = 0.0;
    /* Loop with floating-point recurrence (distance=1) */
    for (int i = 1; i < n; i++) {
        arr[i] = arr[i-1] * factor + arr[i] * 0.5;
        sum += arr[i];
    }
    return sum;
}

__attribute__((noinline))
static int test2_int_mixed_recurrence(int *arr, int n, int factor) {
    int sum = 0;
    /* Mixed operations with recurrence and memory access */
    for (int i = 1; i < n; i++) {
        int temp = arr[i-1] * factor;      /* Multiplication with latency */
        arr[i] = temp + i;                 /* Integer add */
        sum += arr[i] * 2;                 /* Another multiplication */
    }
    return sum;
}

__attribute__((noinline))
static float test3_float_chain(float *a, float *b, int n) {
    float acc = 0.0f;
    /* Multiple dependent operations per iteration */
    for (int i = 1; i < n; i++) {
        float t1 = a[i-1] * 1.1f;          /* FP mult */
        float t2 = t1 + b[i];              /* FP add */
        a[i] = t2 * 0.9f;                  /* Another FP mult */
        b[i] = a[i-1] + t2;                /* Recurrence with FP add */
        acc += a[i] + b[i];
    }
    return acc;
}

__attribute__((noinline))
static double test4_pointer_chase(double **ptr_arr, double *data, int n) {
    double sum = 0.0;
    /* Pointer-based recurrence simulating linked list traversal */
    for (int i = 0; i < n-1; i++) {
        *ptr_arr[i+1] = *ptr_arr[i] * 2.0 + data[i];
        sum += *ptr_arr[i+1];
    }
    return sum;
}

__attribute__((noinline))
static int test5_complex_recurrence(int *arr1, int *arr2, int n) {
    int sum = 0;
    /* Complex recurrence with multiple distance-1 dependences */
    for (int i = 2; i < n; i++) {
        arr1[i] = arr1[i-1] + arr1[i-2];   /* Two-element recurrence */
        arr2[i] = arr2[i-1] * arr1[i];     /* Mixed recurrence */
        sum += arr1[i] * arr2[i];          /* High-latency multiplication */
    }
    return sum;
}

int main(int argc, char **argv) {
    /* Use volatile to prevent constant propagation */
    volatile int data_size = (argc > 1) ? atoi(argv[1]) : SIZE;
    int n = data_size;
    
    /* Allocate and initialize arrays */
    double *arr1 = (double*)malloc(n * sizeof(double));
    int *arr2 = (int*)malloc(n * sizeof(int));
    float *arr3 = (float*)malloc(n * sizeof(float));
    float *arr4 = (float*)malloc(n * sizeof(float));
    double **ptr_arr = (double**)malloc(n * sizeof(double*));
    double *data = (double*)malloc(n * sizeof(double));
    int *arr5 = (int*)malloc(n * sizeof(int));
    int *arr6 = (int*)malloc(n * sizeof(int));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < n; i++) {
        arr1[i] = (i % 10) * 0.1;
        arr2[i] = i % 7;
        arr3[i] = (i % 5) * 0.2f;
        arr4[i] = (i % 3) * 0.3f;
        data[i] = (i % 8) * 0.25;
        arr5[i] = i % 9;
        arr6[i] = i % 11;
    }
    
    /* Set up pointer array for pointer chasing test */
    for (int i = 0; i < n; i++) {
        ptr_arr[i] = &data[i];
    }
    
    double total = 0.0;
    
    /* Call test functions to trigger modulo scheduling analysis */
    total += test1_fp_recurrence(arr1, n, 1.01);
    total += test2_int_mixed_recurrence(arr2, n, 3);
    total += test3_float_chain(arr3, arr4, n);
    total += test4_pointer_chase(ptr_arr, data, n);
    total += test5_complex_recurrence(arr5, arr6, n);
    
    /* Print result to prevent dead code elimination */
    printf("Total checksum: %f\n", total);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    free(ptr_arr);
    free(data);
    free(arr5);
    free(arr6);
    
    return 0;
}
