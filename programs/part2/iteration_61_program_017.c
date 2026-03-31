/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler with distance-1 dependences
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c -mtune=itanium
 */

#include <stdlib.h>
#include <stdio.h>

#define SIZE 1024

/* Prevent inlining to keep loop structure intact */
__attribute__((noinline))
double test_recurrence_fp(double* arr, int n, double factor) {
    double sum = 0.0;
    /* Loop with floating-point recurrence (distance=1) */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence: arr[i] depends on arr[i-1] */
        arr[i] = arr[i-1] * factor + (double)i;
        /* Additional operations to create scheduling opportunities */
        arr[i] += arr[i] * 0.5;      /* FP multiply */
        sum += arr[i];               /* FP add */
    }
    return sum;
}

__attribute__((noinline))
int test_mixed_latency(int* arr, double* darr, int n) {
    int total = 0;
    /* Mixed integer/FP recurrence with memory aliasing */
    for (int i = 1; i < n; i++) {
        /* Distance-1 integer dependence */
        int temp = arr[i-1] * 3;     /* Integer multiply (higher latency) */
        
        /* FP operation with recurrence */
        darr[i] = darr[i-1] * 1.01 + temp;
        
        /* Store with potential aliasing */
        arr[i] = temp + i;
        
        /* Complex expression to prevent simplification */
        total += arr[i] + (int)(darr[i] * 0.5);
    }
    return total;
}

__attribute__((noinline))
float test_pointer_chase(float** ptrs, float* values, int n) {
    float sum = 0.0f;
    /* Pointer-based recurrence simulating linked list traversal */
    for (int i = 0; i < n-1; i++) {
        /* Load through pointer (memory latency) */
        float val = *ptrs[i];
        
        /* Distance-1 dependence through pointer update */
        ptrs[i+1] = &values[i+1];
        
        /* FP operation chain */
        values[i+1] = val * 1.1f + values[i] * 0.9f;
        
        /* Additional FP ops */
        sum += values[i+1] * 2.0f;
    }
    return sum;
}

__attribute__((noinline))
long test_multi_recurrence(long* a, long* b, long* c, int n) {
    long total = 0;
    /* Multiple interleaved recurrences */
    for (int i = 1; i < n; i++) {
        /* Three parallel recurrence chains */
        a[i] = a[i-1] * 7 + b[i-1];      /* Distance-1 on a and b */
        b[i] = b[i-1] * 3 + i;           /* Distance-1 on b */
        c[i] = c[i-1] + a[i] * b[i];     /* Distance-1 on c, uses a[i], b[i] */
        
        /* Complex reduction */
        total += a[i] + b[i] * 2 - c[i];
    }
    return total;
}

__attribute__((noinline))
double test_nested_deps(double* arr1, double* arr2, int n) {
    double sum = 0.0;
    /* Nested dependence chain within iteration */
    for (int i = 1; i < n; i++) {
        /* Chain of FP operations with loop-carried dependence */
        double t1 = arr1[i-1] * 1.5;     /* Distance-1 */
        double t2 = t1 + arr2[i];        /* Intra-iteration */
        double t3 = t2 * 0.8;            /* FP multiply */
        double t4 = t3 - arr1[i-1];      /* Distance-1 again */
        
        arr1[i] = t4 * 2.0;
        arr2[i] = arr2[i-1] + t1;        /* Another distance-1 */
        
        sum += arr1[i] + arr2[i];
    }
    return sum;
}

int main(int argc, char** argv) {
    /* Use volatile to prevent constant propagation */
    volatile int size = (argc > 1) ? atoi(argv[1]) : SIZE;
    if (size < 10) size = SIZE;
    
    /* Allocate and initialize arrays */
    double* arr_fp = (double*)malloc(size * sizeof(double));
    int* arr_int = (int*)malloc(size * sizeof(int));
    double* arr_dbl = (double*)malloc(size * sizeof(double));
    float* arr_flt = (float*)malloc(size * sizeof(float));
    float** ptrs = (float**)malloc(size * sizeof(float*));
    long* arr_long1 = (long*)malloc(size * sizeof(long));
    long* arr_long2 = (long*)malloc(size * sizeof(long));
    long* arr_long3 = (long*)malloc(size * sizeof(long));
    double* arr1 = (double*)malloc(size * sizeof(double));
    double* arr2 = (double*)malloc(size * sizeof(double));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < size; i++) {
        arr_fp[i] = (double)(i % 100) * 0.1;
        arr_int[i] = i % 50;
        arr_dbl[i] = (double)(i % 75) * 0.3;
        arr_flt[i] = (float)(i % 60) * 0.2f;
        ptrs[i] = &arr_flt[i];
        arr_long1[i] = i % 30;
        arr_long2[i] = i % 40;
        arr_long3[i] = i % 20;
        arr1[i] = (double)(i % 80) * 0.15;
        arr2[i] = (double)(i % 90) * 0.25;
    }
    
    double total = 0.0;
    
    /* Call test functions to trigger modulo scheduling analysis */
    total += test_recurrence_fp(arr_fp, size, 1.05);
    total += (double)test_mixed_latency(arr_int, arr_dbl, size);
    total += (double)test_pointer_chase(ptrs, arr_flt, size);
    total += (double)test_multi_recurrence(arr_long1, arr_long2, arr_long3, size);
    total += test_nested_deps(arr1, arr2, size);
    
    /* Print result to prevent dead code elimination */
    printf("Total checksum: %f\n", total);
    
    /* Cleanup */
    free(arr_fp);
    free(arr_int);
    free(arr_dbl);
    free(arr_flt);
    free(ptrs);
    free(arr_long1);
    free(arr_long2);
    free(arr_long3);
    free(arr1);
    free(arr2);
    
    return (int)(total / 1000.0);
}
