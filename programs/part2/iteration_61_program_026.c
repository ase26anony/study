/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler with distance-1 dependences
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c -mtune=itanium
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Prevent inlining to keep loop structure intact */
__attribute__((noinline))
static double test1_recurrence_fp(double *arr, int n, double factor) {
    double sum = 0.0;
    /* Loop with floating-point recurrence (distance=1) */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence: arr[i] depends on arr[i-1] */
        arr[i] = arr[i-1] * factor + (double)i;
        /* Additional operations to create scheduling opportunities */
        arr[i] += arr[i] * 0.5;
        arr[i] = arr[i] / 1.3;
        sum += arr[i];
    }
    return sum;
}

__attribute__((noinline))
static int test2_mixed_latency(int *arr, double *farr, int n) {
    int total = 0;
    /* Mixed integer/float recurrence with memory accesses */
    for (int i = 1; i < n; i++) {
        /* Distance-1 integer dependence */
        arr[i] = arr[i-1] * 3 + i;
        
        /* Distance-1 floating-point dependence */
        farr[i] = farr[i-1] * 1.01 + farr[i];
        
        /* Cross-type operations with potential latency */
        arr[i] += (int)(farr[i] * 100.0);
        
        /* Memory access with pointer arithmetic (potential aliasing) */
        int *ptr = &arr[i];
        *ptr += *(ptr - 1) & 0xFF;
        
        total += arr[i];
    }
    return total;
}

__attribute__((noinline))
static float test3_pointer_chase(float **ptrs, float *data, int n) {
    float result = 0.0f;
    /* Pointer-chasing recurrence (distance=1) */
    for (int i = 0; i < n - 1; i++) {
        /* Set up pointer to next iteration's data */
        ptrs[i+1] = &data[i+1];
        
        /* Distance-1 dependence through pointer */
        *ptrs[i+1] = *ptrs[i] * 1.5f + (float)i;
        
        /* Additional FP operations with different latencies */
        *ptrs[i+1] = *ptrs[i+1] / 1.2f;
        *ptrs[i+1] = *ptrs[i+1] + *ptrs[i+1] * 0.1f;
        
        result += *ptrs[i+1];
    }
    return result;
}

__attribute__((noinline))
static long test4_complex_chain(long *arr, double *darr, int n) {
    long acc = 0;
    /* Complex chain with multiple distance-1 dependences */
    for (int i = 2; i < n; i++) {
        /* Multiple interleaved recurrences */
        double temp = darr[i-1] * darr[i-2];  /* Distance 1 and 2 */
        darr[i] = temp + (double)arr[i-1];    /* Distance 1 */
        
        arr[i] = arr[i-1] + arr[i-2] + (long)darr[i]; /* Multiple distances */
        
        /* Memory operations that might alias */
        long *p1 = &arr[i];
        long *p2 = &arr[i-1];
        *p1 += *p2 >> 4;
        
        /* Integer multiply (higher latency) */
        arr[i] = arr[i] * 7;
        
        acc += arr[i];
    }
    return acc;
}

/* Use volatile to prevent constant propagation */
volatile int g_size = SIZE;

int main(int argc, char *argv[]) {
    /* Use command line or volatile to avoid constant loop bounds */
    int n = (argc > 1) ? atoi(argv[1]) : g_size;
    if (n < 10) n = SIZE;
    
    /* Allocate and initialize arrays */
    double *arr1 = (double*)malloc(n * sizeof(double));
    int *arr2 = (int*)malloc(n * sizeof(int));
    double *arr3 = (double*)malloc(n * sizeof(double));
    float **ptrs = (float**)malloc(n * sizeof(float*));
    float *data = (float*)malloc(n * sizeof(float));
    long *arr4 = (long*)malloc(n * sizeof(long));
    double *darr = (double*)malloc(n * sizeof(double));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < n; i++) {
        arr1[i] = (double)(i % 100) * 0.1;
        arr2[i] = i % 50;
        arr3[i] = (double)(i % 75) * 0.3;
        data[i] = (float)(i % 60) * 0.2f;
        ptrs[i] = &data[i];
        arr4[i] = i % 40;
        darr[i] = (double)(i % 30) * 0.4;
    }
    
    double result1 = 0.0;
    int result2 = 0;
    float result3 = 0.0f;
    long result4 = 0L;
    
    /* Call test functions multiple times to ensure execution */
    for (int iter = 0; iter < 3; iter++) {
        result1 += test1_recurrence_fp(arr1, n, 1.05 + iter * 0.01);
        result2 += test2_mixed_latency(arr2, arr3, n);
        result3 += test3_pointer_chase(ptrs, data, n);
        result4 += test4_complex_chain(arr4, darr, n);
    }
    
    /* Print results to prevent dead code elimination */
    printf("Results: %f, %d, %f, %ld\n", 
           result1, result2, result3, result4);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(ptrs);
    free(data);
    free(arr4);
    free(darr);
    
    return 0;
}
