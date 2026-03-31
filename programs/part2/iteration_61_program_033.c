/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler with loop-carried dependences
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c -mtune=itanium
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Prevent inlining to keep loop structures intact */
__attribute__((noinline))
double test_recurrence_fp(double* arr, int n, double factor) {
    double sum = 0.0;
    /* Loop with floating-point recurrence (distance=1) */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence: arr[i] depends on arr[i-1] */
        arr[i] = arr[i-1] * factor + (double)i;
        /* Additional operations to create scheduling opportunities */
        arr[i] += arr[i] * 0.5;      /* Intra-iteration dependence */
        sum += arr[i];               /* Reduction */
    }
    return sum;
}

__attribute__((noinline))
int test_mixed_latency(int* arr, double* farr, int n, double factor) {
    int total = 0;
    /* Mixed integer/floating-point with memory aliasing */
    for (int i = 1; i < n; i++) {
        /* Distance-1 integer recurrence */
        arr[i] = arr[i-1] * 3 + i;
        
        /* Floating-point recurrence with different latency */
        farr[i] = farr[i-1] * factor + arr[i];
        
        /* Memory load with potential aliasing */
        int temp = arr[i] + arr[i/2];
        
        /* More operations to increase DDG complexity */
        farr[i] = farr[i] * 1.01 - (double)temp;
        total += arr[i] + (int)farr[i];
    }
    return total;
}

__attribute__((noinline))
float test_pointer_chase(float** ptrs, float* data, int n) {
    float result = 0.0f;
    /* Pointer-based recurrence simulating linked list traversal */
    for (int i = 0; i < n-1; i++) {
        /* Distance-1 pointer dependence */
        *ptrs[i+1] = *ptrs[i] * 1.5f + data[i];
        
        /* Additional floating operations */
        data[i+1] = data[i] * 0.9f + *ptrs[i];
        result += *ptrs[i] + data[i];
    }
    return result;
}

__attribute__((noinline))
long test_complex_recurrence(long* A, long* B, double* C, int n) {
    long sum = 0;
    /* Complex recurrence with multiple dependence chains */
    for (int i = 2; i < n; i++) {
        /* Multiple distance-1 dependences */
        A[i] = A[i-1] + B[i-2] * 7;
        B[i] = B[i-1] - A[i-1] / 3;
        
        /* Floating-point with higher latency */
        C[i] = C[i-1] * 1.7 + C[i-2] * 0.3;
        
        /* Cross-type operations */
        C[i] += (double)(A[i] + B[i]);
        
        /* Memory operations that may alias */
        if (i % 3 == 0) {
            A[i] = B[i] + (long)C[i];
        }
        
        sum += A[i] + B[i] + (long)C[i];
    }
    return sum;
}

__attribute__((noinline))
double test_nested_dependences(double* arr1, double* arr2, int n) {
    double total = 0.0;
    /* Loop with nested recurrence patterns */
    for (int i = 3; i < n; i++) {
        /* Chain of distance-1 dependences */
        double t1 = arr1[i-1] * 2.5;
        double t2 = t1 + arr1[i-2] * 1.5;
        double t3 = t2 - arr1[i-3] * 0.5;
        
        /* Parallel recurrence chain */
        arr2[i] = arr2[i-1] * 1.2 + arr2[i-2] * 0.8;
        
        /* Combine chains with different latencies */
        arr1[i] = t3 * arr2[i] + (double)i;
        
        /* Additional FP operations */
        arr1[i] = arr1[i] / 1.1;
        arr2[i] = arr2[i] * 0.95;
        
        total += arr1[i] + arr2[i];
    }
    return total;
}

int main(int argc, char** argv) {
    /* Use volatile to prevent constant propagation */
    volatile int size = (argc > 1) ? atoi(argv[1]) : SIZE;
    if (size < 10) size = SIZE;
    
    /* Allocate and initialize arrays */
    double* arr_fp = (double*)malloc(size * sizeof(double));
    int* arr_int = (int*)malloc(size * sizeof(int));
    double* arr_fp2 = (double*)malloc(size * sizeof(double));
    long* arr_long1 = (long*)malloc(size * sizeof(long));
    long* arr_long2 = (long*)malloc(size * sizeof(long));
    double* arr_fp3 = (double*)malloc(size * sizeof(double));
    float** ptr_array = (float**)malloc(size * sizeof(float*));
    float* data = (float*)malloc(size * sizeof(float));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < size; i++) {
        arr_fp[i] = 1.0 + i * 0.1;
        arr_int[i] = i * 2;
        arr_fp2[i] = 2.0 + i * 0.05;
        arr_long1[i] = i * 3;
        arr_long2[i] = i * 5;
        arr_fp3[i] = 0.5 + i * 0.01;
        data[i] = (float)i * 0.25f;
        
        /* Create pointer array for chasing test */
        ptr_array[i] = &data[(i * 7) % size];
    }
    
    double result = 0.0;
    
    /* Call test functions to trigger modulo scheduling analysis */
    result += test_recurrence_fp(arr_fp, size, 1.05);
    
    int int_result = test_mixed_latency(arr_int, arr_fp2, size, 1.03);
    result += (double)int_result;
    
    result += (double)test_pointer_chase(ptr_array, data, size);
    
    long long_result = test_complex_recurrence(arr_long1, arr_long2, arr_fp3, size);
    result += (double)long_result;
    
    result += test_nested_dependences(arr_fp, arr_fp2, size);
    
    /* Print result to prevent optimization */
    printf("Final result: %f\n", result);
    
    /* Cleanup */
    free(arr_fp);
    free(arr_int);
    free(arr_fp2);
    free(arr_long1);
    free(arr_long2);
    free(arr_fp3);
    free(ptr_array);
    free(data);
    
    return 0;
}
