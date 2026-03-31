/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler with distance-1 dependences
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c -mtune=itanium
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Prevent inlining to keep loop structure intact */
__attribute__((noinline))
double test1_fp_recurrence(double* arr, int n, double factor) {
    double sum = 0.0;
    /* Loop with floating-point recurrence (distance=1) */
    for (int i = 1; i < n; i++) {
        arr[i] = arr[i-1] * factor + (double)i * 1.5;
        /* Additional operations to create more scheduling opportunities */
        arr[i] = arr[i] * 0.99 + arr[i-1] * 0.01;
        sum += arr[i];
    }
    return sum;
}

__attribute__((noinline))
float test2_mixed_recurrence(float* arr, int* indices, int n) {
    float total = 0.0f;
    /* Mixed integer/float recurrence with memory aliasing */
    for (int i = 1; i < n; i++) {
        int idx = indices[i-1] % n;
        float temp = arr[idx] * 2.5f;
        arr[i] = temp + arr[i-1] * 1.1f;
        indices[i] = (int)(arr[i] * 10.0f);
        total += arr[i] + (float)indices[i];
    }
    return total;
}

__attribute__((noinline))
int test3_pointer_chasing(int** ptr_arr, int* data, int n) {
    int sum = 0;
    /* Pointer-chasing recurrence (distance=1) */
    for (int i = 0; i < n-1; i++) {
        int val = *ptr_arr[i];
        data[i+1] = val + i * 3;
        ptr_arr[i+1] = &data[i+1];
        sum += val + data[i+1];
    }
    return sum;
}

__attribute__((noinline))
double test4_complex_dependence(double* a, double* b, double* c, int n) {
    double acc = 0.0;
    /* Multiple interleaved recurrences */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence on a */
        a[i] = a[i-1] * b[i] + c[i];
        
        /* Distance-1 dependence on b with different latency */
        b[i] = b[i-1] * 0.8 + a[i] * 0.2;
        
        /* Intra-iteration dependence only */
        c[i] = a[i] + b[i] * (double)i;
        
        /* Memory access with potential aliasing */
        acc += a[i] * b[i] - c[i];
    }
    return acc;
}

__attribute__((noinline))
int test5_integer_recurrence(int* arr, int n, int mod) {
    int result = 0;
    /* Integer recurrence with multiplication (higher latency) */
    for (int i = 1; i < n; i++) {
        arr[i] = arr[i-1] * 3 + i * 7;
        /* Additional operation to create scheduling pressure */
        arr[i] = (arr[i] % mod) ^ (arr[i-1] & 0xFF);
        result ^= arr[i];
    }
    return result;
}

int main(int argc, char** argv) {
    /* Use volatile to prevent constant propagation */
    volatile int size = (argc > 1) ? atoi(argv[1]) : SIZE;
    int n = size;
    
    /* Allocate and initialize arrays */
    double* arr1 = (double*)malloc(n * sizeof(double));
    float* arr2 = (float*)malloc(n * sizeof(float));
    int* arr3 = (int*)malloc(n * sizeof(int));
    int* indices = (int*)malloc(n * sizeof(int));
    int** ptr_arr = (int**)malloc(n * sizeof(int*));
    double* arr4a = (double*)malloc(n * sizeof(double));
    double* arr4b = (double*)malloc(n * sizeof(double));
    double* arr4c = (double*)malloc(n * sizeof(double));
    int* arr5 = (int*)malloc(n * sizeof(int));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < n; i++) {
        arr1[i] = (double)(i + 1) * 0.5;
        arr2[i] = (float)(i + 1) * 0.25f;
        arr3[i] = i * 2;
        indices[i] = i * 3 % n;
        ptr_arr[i] = &arr3[i];
        arr4a[i] = (double)i * 0.1;
        arr4b[i] = (double)i * 0.2;
        arr4c[i] = (double)i * 0.3;
        arr5[i] = i + 100;
    }
    
    /* Call test functions to trigger modulo scheduling */
    double sum1 = test1_fp_recurrence(arr1, n, 1.01);
    float sum2 = test2_mixed_recurrence(arr2, indices, n);
    int sum3 = test3_pointer_chasing(ptr_arr, arr3, n);
    double sum4 = test4_complex_dependence(arr4a, arr4b, arr4c, n);
    int sum5 = test5_integer_recurrence(arr5, n, 997);
    
    /* Aggregate results to prevent dead code elimination */
    double final_result = sum1 + (double)sum2 + (double)sum3 + sum4 + (double)sum5;
    
    printf("Result: %f\n", final_result);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(indices);
    free(ptr_arr);
    free(arr4a);
    free(arr4b);
    free(arr4c);
    free(arr5);
    
    return 0;
}
