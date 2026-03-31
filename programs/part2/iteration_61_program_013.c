/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler with distance-1 dependences
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c -mtune=itanium
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024

/* Prevent inlining to keep loop structure intact */
__attribute__((noinline, optimize("no-unroll-loops")))
double test1_fp_recurrence(double *arr, int n, double factor) {
    double sum = 0.0;
    /* Loop-carried dependence: arr[i] depends on arr[i-1] */
    for (int i = 1; i < n; i++) {
        arr[i] = arr[i-1] * factor + (double)i;  /* Distance-1 recurrence */
        sum += arr[i];
    }
    return sum;
}

__attribute__((noinline, optimize("no-unroll-loops")))
float test2_mixed_latency(float *a, float *b, int n) {
    float acc = a[0];
    /* Multiple operations with different latencies */
    for (int i = 1; i < n; i++) {
        float t1 = a[i-1] * 1.5f;      /* FP multiply */
        float t2 = t1 + b[i];          /* FP add */
        float t3 = t2 * acc;           /* Another multiply with loop-carried dependence */
        a[i] = t3 + (float)(i & 0xFF); /* Integer-to-float conversion + add */
        acc = t3;                      /* Distance-1 dependence */
    }
    return acc;
}

__attribute__((noinline, optimize("no-unroll-loops")))
int test3_int_recurrence(int *arr, int n, int *coeffs) {
    int sum = 0;
    /* Integer recurrence with memory aliasing */
    for (int i = 1; i < n; i++) {
        int prev = arr[i-1];           /* Load with potential aliasing */
        int mult = prev * coeffs[i];   /* Integer multiply */
        int add = mult + i;            /* Integer add */
        arr[i] = add;                  /* Store */
        sum += add;                    /* Reduction */
    }
    return sum;
}

__attribute__((noinline, optimize("no-unroll-loops")))
double test4_pointer_chasing(double **ptrs, int n) {
    double sum = 0.0;
    /* Pointer-based recurrence simulating linked list traversal */
    for (int i = 0; i < n-1; i++) {
        double *current = ptrs[i];
        double *next = ptrs[i+1];
        *next = *current * 0.99 + (double)i;  /* Distance-1 through pointers */
        sum += *next;
    }
    return sum;
}

__attribute__((noinline, optimize("no-unroll-loops")))
int test5_complex_dependence(int *a, int *b, int *c, int n) {
    /* Multiple interleaved dependences */
    int x = a[0];
    int y = b[0];
    for (int i = 1; i < n; i++) {
        int t1 = x * y;                /* Depends on previous iteration's x,y */
        int t2 = a[i] + i;             /* Intra-iteration dependence */
        int t3 = t1 * t2;              /* Combine both */
        int t4 = t3 + c[i-1];          /* Distance-1 memory dependence */
        b[i] = t4;
        x = t3;                        /* Distance-1 register dependence */
        y = t4;
        c[i] = t4 * 2;
    }
    return x + y;
}

/* Use volatile to prevent constant propagation */
volatile int g_size = SIZE;

int main(int argc, char *argv[]) {
    /* Use command line or volatile to prevent compile-time optimization */
    int n = (argc > 1) ? atoi(argv[1]) : g_size;
    if (n > SIZE) n = SIZE;
    
    /* Allocate and initialize arrays */
    double *arr1 = (double*)malloc(SIZE * sizeof(double));
    float *arr2_f = (float*)malloc(SIZE * sizeof(float));
    float *arr3_f = (float*)malloc(SIZE * sizeof(float));
    int *arr4_i = (int*)malloc(SIZE * sizeof(int));
    int *coeffs = (int*)malloc(SIZE * sizeof(int));
    double **ptrs = (double**)malloc(SIZE * sizeof(double*));
    double *ptr_data = (double*)malloc(SIZE * sizeof(double));
    int *arr5_a = (int*)malloc(SIZE * sizeof(int));
    int *arr5_b = (int*)malloc(SIZE * sizeof(int));
    int *arr5_c = (int*)malloc(SIZE * sizeof(int));
    
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (double)(i % 100);
        arr2_f[i] = (float)(i % 50);
        arr3_f[i] = (float)(i % 30);
        arr4_i[i] = i;
        coeffs[i] = (i % 7) + 1;
        ptr_data[i] = (double)(i % 80);
        ptrs[i] = &ptr_data[i];
        arr5_a[i] = i * 2;
        arr5_b[i] = i * 3;
        arr5_c[i] = i * 5;
    }
    
    /* Call test functions to trigger modulo scheduling */
    double sum1 = test1_fp_recurrence(arr1, n, 1.01);
    float sum2 = test2_mixed_latency(arr2_f, arr3_f, n);
    int sum3 = test3_int_recurrence(arr4_i, n, coeffs);
    double sum4 = test4_pointer_chasing(ptrs, n);
    int sum5 = test5_complex_dependence(arr5_a, arr5_b, arr5_c, n);
    
    /* Use results to prevent dead code elimination */
    double total = sum1 + (double)sum2 + (double)sum3 + sum4 + (double)sum5;
    
    printf("Result: %f\n", total);
    
    /* Cleanup */
    free(arr1);
    free(arr2_f);
    free(arr3_f);
    free(arr4_i);
    free(coeffs);
    free(ptrs);
    free(ptr_data);
    free(arr5_a);
    free(arr5_b);
    free(arr5_c);
    
    return 0;
}
