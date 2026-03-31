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
    /* Loop with floating-point recurrence (distance=1) */
    for (int i = 1; i < n; i++) {
        arr[i] = arr[i-1] * factor + (double)i * 1.5;
        sum += arr[i];
    }
    return sum;
}

__attribute__((noinline))
static int test2_mixed_latencies(int* arr, double* farr, int n) {
    int total = 0;
    /* Mixed integer/FP recurrence with memory accesses */
    for (int i = 1; i < n; i++) {
        farr[i] = farr[i-1] * 1.01 + (double)arr[i];
        arr[i] = (int)farr[i] * 3 + arr[i-1];
        total += arr[i];
    }
    return total;
}

__attribute__((noinline))
static float test3_pointer_chase(float** ptrs, float* data, int n) {
    float sum = 0.0f;
    /* Pointer-based recurrence simulating linked list traversal */
    for (int i = 0; i < n-1; i++) {
        *ptrs[i+1] = *ptrs[i] * 2.0f + data[i];
        sum += *ptrs[i];
    }
    return sum;
}

__attribute__((noinline))
static long test4_complex_dependence_chain(long* a, long* b, long* c, int n) {
    long result = 0;
    /* Multiple interleaved recurrences with different distances */
    for (int i = 2; i < n; i++) {
        a[i] = a[i-1] * b[i-2] + c[i];      /* Distance 1 and 2 */
        b[i] = b[i-1] + a[i] * 7;           /* Distance 1 */
        c[i] = c[i-1] - a[i-1] * 3;         /* Distance 1 */
        result += a[i] + b[i] - c[i];
    }
    return result;
}

__attribute__((noinline))
static double test5_memory_aliasing(double* arr1, double* arr2, int n) {
    double acc = 0.0;
    /* Potential aliasing creates conservative dependences */
    for (int i = 1; i < n; i++) {
        arr1[i] = arr1[i-1] * 0.99 + arr2[i] * 1.1;
        arr2[i] = arr1[i] * 0.5 + arr2[i-1];
        acc += arr1[i] + arr2[i];
    }
    return acc;
}

int main(int argc, char** argv) {
    /* Use volatile to prevent constant propagation */
    volatile int size = (argc > 1) ? atoi(argv[1]) : SIZE;
    int n = size;
    
    /* Allocate and initialize arrays */
    double* arr_fp = (double*)malloc(n * sizeof(double));
    int* arr_int = (int*)malloc(n * sizeof(int));
    float* arr_float = (float*)malloc(n * sizeof(float));
    float** ptr_array = (float**)malloc(n * sizeof(float*));
    long* arr_a = (long*)malloc(n * sizeof(long));
    long* arr_b = (long*)malloc(n * sizeof(long));
    long* arr_c = (long*)malloc(n * sizeof(long));
    double* arr1 = (double*)malloc(n * sizeof(double));
    double* arr2 = (double*)malloc(n * sizeof(double));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < n; i++) {
        arr_fp[i] = (double)(i % 100) * 0.1;
        arr_int[i] = i % 50;
        arr_float[i] = (float)i * 0.25f;
        ptr_array[i] = &arr_float[i];
        arr_a[i] = i * 2L;
        arr_b[i] = i * 3L;
        arr_c[i] = i * 5L;
        arr1[i] = (double)i * 0.33;
        arr2[i] = (double)i * 0.66;
    }
    
    double total = 0.0;
    
    /* Call test functions to trigger modulo scheduling analysis */
    total += test1_recurrence_fp(arr_fp, n, 1.05);
    total += (double)test2_mixed_latencies(arr_int, arr_fp, n);
    total += (double)test3_pointer_chase(ptr_array, arr_float, n);
    total += (double)test4_complex_dependence_chain(arr_a, arr_b, arr_c, n);
    total += test5_memory_aliasing(arr1, arr2, n);
    
    printf("Total result: %f\n", total);
    
    /* Cleanup */
    free(arr_fp);
    free(arr_int);
    free(arr_float);
    free(ptr_array);
    free(arr_a);
    free(arr_b);
    free(arr_c);
    free(arr1);
    free(arr2);
    
    return 0;
}
