/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler with distance-1 dependences
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c -mtune=itanium
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Prevent inlining to keep loop structures intact */
__attribute__((noinline, cold))
double test1_fp_recurrence(double *arr, int n, double factor) {
    double sum = 0.0;
    /* Loop with floating-point recurrence (distance=1) */
    for (int i = 1; i < n; i++) {
        arr[i] = arr[i-1] * factor + arr[i] * 0.5;  /* Distance-1 dependence */
        sum += arr[i];
    }
    return sum;
}

__attribute__((noinline, cold))
int test2_mixed_latency(int *arr, double *farr, int n, int step) {
    int total = 0;
    double acc = farr[0];
    
    /* Mixed integer/FP operations with loop-carried dependence */
    for (int i = 1; i < n; i++) {
        /* Integer recurrence with multiplication (higher latency) */
        arr[i] = arr[i-1] * 3 + i;  /* Distance-1 dependence */
        
        /* Floating-point recurrence */
        acc = acc * 1.01 + farr[i];  /* Another distance-1 dependence */
        farr[i] = acc;
        
        /* Memory access with potential aliasing */
        total += arr[i] + (int)farr[i];
    }
    return total;
}

__attribute__((noinline, cold))
float test3_pointer_chase(float **ptrs, float *data, int n) {
    float result = 0.0f;
    
    /* Set up pointer chain */
    for (int i = 0; i < n; i++) {
        ptrs[i] = &data[i];
    }
    
    /* Pointer chasing with loop-carried dependence */
    for (int i = 1; i < n-1; i++) {
        /* Load through pointer from previous iteration */
        float val = *ptrs[i-1];  /* Distance-1 memory dependence */
        
        /* FP operation with latency */
        val = val * 1.5f + data[i];
        
        /* Store result for next iteration */
        *ptrs[i] = val;
        
        result += val;
    }
    return result;
}

__attribute__((noinline, cold))
long test4_complex_dependence(long *a, long *b, double *c, int n) {
    long sum = 0;
    double d_acc = c[0];
    long l_acc = a[0];
    
    /* Multiple recurrence chains with different latencies */
    for (int i = 1; i < n; i++) {
        /* Integer multiplication chain (higher latency) */
        l_acc = l_acc * 7 + b[i];  /* Distance-1 */
        a[i] = l_acc;
        
        /* Floating-point division chain (high latency) */
        d_acc = d_acc / 1.03 + c[i];  /* Distance-1 */
        c[i] = d_acc;
        
        /* Cross-type operation */
        sum += a[i] + (long)(c[i] * 100);
    }
    return sum;
}

__attribute__((noinline, cold))
int test5_nested_dependence(int *arr1, int *arr2, int n) {
    int total = 0;
    
    /* Multiple interleaved dependences */
    for (int i = 2; i < n; i++) {
        /* Two separate recurrence chains */
        arr1[i] = arr1[i-1] * 2 + arr1[i-2];  /* Distance-1 and distance-2 */
        arr2[i] = arr2[i-1] + arr1[i] * 3;    /* Distance-1 with cross-chain */
        
        /* Additional computation to increase ILP opportunity */
        int temp = arr1[i] * arr2[i];
        total += temp / (i + 1);
    }
    return total;
}

int main(int argc, char **argv) {
    /* Use volatile to prevent constant propagation */
    volatile int size = (argc > 1) ? atoi(argv[1]) : SIZE;
    if (size < 10) size = SIZE;
    
    /* Allocate and initialize arrays */
    double *fp_arr = (double*)malloc(size * sizeof(double));
    int *int_arr = (int*)malloc(size * sizeof(int));
    float *float_data = (float*)malloc(size * sizeof(float));
    float **ptrs = (float**)malloc(size * sizeof(float*));
    long *long_arr1 = (long*)malloc(size * sizeof(long));
    long *long_arr2 = (long*)malloc(size * sizeof(long));
    double *double_arr = (double*)malloc(size * sizeof(double));
    int *arr1 = (int*)malloc(size * sizeof(int));
    int *arr2 = (int*)malloc(size * sizeof(int));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < size; i++) {
        fp_arr[i] = (i % 7) * 1.1;
        int_arr[i] = i * 3;
        float_data[i] = i * 0.7f;
        long_arr1[i] = i * 5L;
        long_arr2[i] = i * 11L;
        double_arr[i] = i * 2.5;
        arr1[i] = i;
        arr2[i] = i * 2;
    }
    
    double result1 = 0.0;
    int result2 = 0;
    float result3 = 0.0f;
    long result4 = 0L;
    int result5 = 0;
    
    /* Call test functions multiple times to ensure execution */
    for (int iter = 0; iter < 3; iter++) {
        result1 += test1_fp_recurrence(fp_arr, size, 1.05);
        result2 += test2_mixed_latency(int_arr, fp_arr, size, 2);
        result3 += test3_pointer_chase(ptrs, float_data, size);
        result4 += test4_complex_dependence(long_arr1, long_arr2, double_arr, size);
        result5 += test5_nested_dependence(arr1, arr2, size);
        
        /* Modify inputs slightly between iterations */
        fp_arr[0] += 0.5;
        int_arr[0] += 1;
        float_data[0] += 0.3f;
    }
    
    /* Aggregate results to prevent dead code elimination */
    long final_result = (long)result1 + result2 + (long)result3 + result4 + result5;
    printf("Final checksum: %ld\n", final_result);
    
    /* Cleanup */
    free(fp_arr);
    free(int_arr);
    free(float_data);
    free(ptrs);
    free(long_arr1);
    free(long_arr2);
    free(double_arr);
    free(arr1);
    free(arr2);
    
    return (int)(final_result % 1000);
}
