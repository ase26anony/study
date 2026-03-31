/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler with distance-1 dependences
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c -mtune=itanium
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Prevent inlining to keep loop structures intact */
__attribute__((noinline))
static double test1_fp_recurrence(double *arr, int n, double factor) {
    double sum = 0.0;
    /* Loop-carried FP dependence: arr[i] depends on arr[i-1] */
    for (int i = 1; i < n; i++) {
        arr[i] = arr[i-1] * factor + (double)i * 1.5;
        /* Additional operations to create scheduling opportunities */
        arr[i] = arr[i] / 1.1 + (double)(i % 8);
        sum += arr[i];
    }
    return sum;
}

__attribute__((noinline))
static int test2_mixed_latency(int *data, double *fp_data, int n) {
    int total = 0;
    /* Mixed integer/FP recurrence with memory aliasing */
    for (int i = 1; i < n; i++) {
        /* Integer recurrence with multiplication (higher latency) */
        data[i] = data[i-1] * 3 + i;
        
        /* FP recurrence with division (high latency) */
        fp_data[i] = fp_data[i-1] / 1.7 + data[i] * 0.5;
        
        /* Additional memory access with potential aliasing */
        total += data[i] + (int)fp_data[i];
        
        /* Another recurrence chain */
        if (i > 1) {
            data[i] += data[i-2] / 2;  /* Distance-2 dependence */
        }
    }
    return total;
}

__attribute__((noinline))
static float test3_pointer_chase(float **ptrs, float *values, int n) {
    float result = 0.0f;
    /* Pointer-based recurrence simulating linked list traversal */
    for (int i = 0; i < n - 1; i++) {
        /* Load through pointer from previous iteration */
        float val = *ptrs[i];
        
        /* Store to next pointer's target with computation */
        *ptrs[i+1] = val * 1.3f + values[i];
        
        /* Additional FP operations */
        values[i+1] = values[i] * 0.9f + val;
        
        result += val + values[i];
    }
    return result;
}

__attribute__((noinline))
static long test4_complex_dependence(long *a, long *b, double *c, int n) {
    long sum = 0;
    /* Multiple interleaved recurrence chains */
    for (int i = 2; i < n; i++) {
        /* Chain 1: distance-1 integer recurrence */
        a[i] = a[i-1] + b[i-2] * 2;
        
        /* Chain 2: distance-1 FP recurrence */
        c[i] = c[i-1] * 1.01 - c[i-2] * 0.5;
        
        /* Chain 3: mixed recurrence with memory */
        b[i] = (long)(c[i] * 100) + a[i-1] / 3;
        
        /* Complex expression with multiple uses */
        sum += a[i] * b[i-1] + (long)c[i];
        
        /* Additional operation to increase register pressure */
        c[i] = c[i] + (double)(i % 16) * 0.1;
    }
    return sum;
}

__attribute__((noinline))
static int test5_artificial_chain(int *arr, int n, int seed) {
    int x = seed;
    int y = seed * 2;
    int z = seed / 2;
    
    /* Artificial serial chain across iterations */
    for (int i = 0; i < n; i++) {
        /* Each iteration depends on all three values from previous iteration */
        x = x * 3 + arr[i];
        y = y + x / 5;
        z = z * 2 - y;
        arr[i] = x + y + z;
        
        /* Additional operations to create scheduling windows */
        if (i % 4 == 0) {
            x = x ^ (y << 2);
            y = y | (z & 0xFF);
        }
    }
    return x + y + z;
}

int main(int argc, char **argv) {
    /* Use volatile to prevent constant propagation */
    volatile int size = (argc > 1) ? atoi(argv[1]) : SIZE;
    if (size < 16) size = 16;
    if (size > 4096) size = 4096;
    
    /* Allocate and initialize arrays */
    double *arr1 = (double*)malloc(size * sizeof(double));
    int *arr2 = (int*)malloc(size * sizeof(int));
    double *arr3 = (double*)malloc(size * sizeof(double));
    float **ptrs = (float**)malloc(size * sizeof(float*));
    float *values = (float*)malloc(size * sizeof(float));
    long *arr4 = (long*)malloc(size * sizeof(long));
    long *arr5 = (long*)malloc(size * sizeof(long));
    double *arr6 = (double*)malloc(size * sizeof(double));
    int *arr7 = (int*)malloc(size * sizeof(int));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < size; i++) {
        arr1[i] = (double)(i % 100) * 0.1;
        arr2[i] = i * 2;
        arr3[i] = (double)(i % 50) * 0.3;
        values[i] = (float)(i % 25) * 0.4f;
        ptrs[i] = &values[(i + 1) % size];
        arr4[i] = i * 3L;
        arr5[i] = i * 5L;
        arr6[i] = (double)(i % 75) * 0.2;
        arr7[i] = i * 7;
    }
    
    double result1 = 0.0;
    int result2 = 0;
    float result3 = 0.0f;
    long result4 = 0L;
    int result5 = 0;
    
    /* Call test functions multiple times to ensure execution */
    for (int iter = 0; iter < 3; iter++) {
        result1 += test1_fp_recurrence(arr1, size, 1.05 + iter * 0.01);
        result2 += test2_mixed_latency(arr2, arr3, size);
        result3 += test3_pointer_chase(ptrs, values, size);
        result4 += test4_complex_dependence(arr4, arr5, arr6, size);
        result5 += test5_artificial_chain(arr7, size, 42 + iter);
    }
    
    /* Aggregate results to prevent dead code elimination */
    double final_result = result1 + result2 + result3 + result4 + result5;
    
    printf("Result: %f\n", final_result);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(ptrs);
    free(values);
    free(arr4);
    free(arr5);
    free(arr6);
    free(arr7);
    
    return (int)(final_result / 1000000.0);
}
