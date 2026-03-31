/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler with distance-1 dependences
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c -mtune=itanium
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Prevent inlining to keep loop structure intact */
__attribute__((noinline))
double test1_fp_recurrence(double *arr, int n, double factor) {
    double sum = 0.0;
    /* Distance-1 recurrence: arr[i] depends on arr[i-1] */
    for (int i = 1; i < n; i++) {
        arr[i] = arr[i-1] * factor + (double)i * 0.5;
        sum += arr[i];
    }
    return sum;
}

__attribute__((noinline))
float test2_mixed_latency(float *fa, int *ia, int n, float f1, float f2) {
    float total = 0.0f;
    /* Mixed operations with different latencies */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence through fa[] */
        fa[i] = fa[i-1] * f1 + fa[i] * f2;
        /* Integer multiplication (higher latency than add) */
        ia[i] = ia[i-1] * 3 + i * 7;
        /* Additional FP operation for more scheduling complexity */
        total += fa[i] * (float)ia[i];
    }
    return total;
}

__attribute__((noinline))
int test3_pointer_chasing(int **ptr_arr, int *data, int n) {
    int sum = 0;
    /* Pointer-based recurrence with memory latency */
    for (int i = 0; i < n-1; i++) {
        /* Load from address computed in previous iteration */
        int val = *ptr_arr[i];
        /* Store to next pointer's target */
        *ptr_arr[i+1] = val + data[i];
        sum += val;
    }
    return sum;
}

__attribute__((noinline))
double test4_complex_chain(double *a, double *b, double *c, int n) {
    double acc = 0.0;
    /* Multiple interleaved dependence chains */
    for (int i = 1; i < n; i++) {
        /* Chain 1: distance-1 through a[] */
        a[i] = a[i-1] * 1.01 + b[i];
        
        /* Chain 2: distance-1 through b[] with FP multiply latency */
        b[i] = b[i-1] * 0.99 + c[i] * 2.5;
        
        /* Chain 3: intra-iteration dependence on a[i] */
        c[i] = a[i] * 0.5 + (double)i;
        
        /* Accumulator with mixed operations */
        acc += a[i] * b[i] - c[i];
    }
    return acc;
}

__attribute__((noinline))
int test5_integer_recurrence(int *arr, int n, int mod) {
    int hash = 0;
    /* Integer recurrence with multiplication latency */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence with 32-bit multiply */
        arr[i] = (arr[i-1] * 1103515245 + 12345) & 0x7fffffff;
        
        /* Additional operation to create more scheduling opportunities */
        hash ^= arr[i] % mod;
    }
    return hash;
}

int main(int argc, char *argv[]) {
    /* Use volatile to prevent constant propagation */
    volatile int size = (argc > 1) ? atoi(argv[1]) : SIZE;
    int n = size;
    
    /* Allocate and initialize arrays */
    double *arr1 = (double*)malloc(n * sizeof(double));
    float *fa = (float*)malloc(n * sizeof(float));
    int *ia = (int*)malloc(n * sizeof(int));
    double *arr2 = (double*)malloc(n * sizeof(double));
    double *arr3 = (double*)malloc(n * sizeof(double));
    int *arr4 = (int*)malloc(n * sizeof(int));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < n; i++) {
        arr1[i] = (double)(i % 100) * 0.1;
        fa[i] = (float)(i % 50) * 0.2f;
        ia[i] = i % 30;
        arr2[i] = (double)(i % 80) * 0.3;
        arr3[i] = (double)(i % 70) * 0.4;
        arr4[i] = i % 60;
    }
    
    /* Set up pointer chasing test */
    int **ptr_arr = (int**)malloc(n * sizeof(int*));
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        data[i] = i * 3;
        ptr_arr[i] = &data[(i * 7) % n];
    }
    
    double result = 0.0;
    
    /* Call test functions to trigger modulo scheduling analysis */
    result += test1_fp_recurrence(arr1, n, 1.05);
    result += test2_mixed_latency(fa, ia, n, 1.1f, 0.9f);
    result += test3_pointer_chasing(ptr_arr, data, n);
    result += test4_complex_chain(arr1, arr2, arr3, n);
    result += test5_integer_recurrence(arr4, n, 997);
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %f\n", result);
    
    /* Cleanup */
    free(arr1);
    free(fa);
    free(ia);
    free(arr2);
    free(arr3);
    free(arr4);
    free(ptr_arr);
    free(data);
    
    return 0;
}
