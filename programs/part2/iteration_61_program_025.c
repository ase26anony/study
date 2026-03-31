/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler debug output for distance-1 dependences
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c -mtune=itanium
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Prevent inlining to keep loop structure intact */
__attribute__((noinline, optimize("no-unroll-loops")))
double test1_recurrence_fp(double* arr, int n, double factor) {
    double sum = 0.0;
    /* Loop with floating-point recurrence (distance=1) */
    for (int i = 1; i < n; i++) {
        arr[i] = arr[i-1] * factor + (double)i * 1.5;
        /* Additional operations to create more scheduling opportunities */
        arr[i] = arr[i] / 1.1 + arr[i-1] * 0.5;
        sum += arr[i];
    }
    return sum;
}

__attribute__((noinline, optimize("no-unroll-loops")))
int test2_mixed_latency(int* arr, double* farr, int n) {
    int total = 0;
    /* Mixed integer/float recurrence with memory aliasing */
    for (int i = 1; i < n; i++) {
        /* Integer recurrence with multiplication (higher latency) */
        arr[i] = arr[i-1] * 3 + i;
        
        /* Floating-point recurrence */
        farr[i] = farr[i-1] * 1.01 + farr[i] * 0.99;
        
        /* Cross-type dependence */
        farr[i] += (double)arr[i-1] * 0.25;
        
        /* Memory store with potential aliasing */
        if (i > 2) {
            arr[i-2] = (int)farr[i] + arr[i-1];
        }
        
        total += arr[i] + (int)farr[i];
    }
    return total;
}

__attribute__((noinline, optimize("no-unroll-loops")))
double test3_pointer_chasing(double** ptrs, double* data, int n) {
    double result = 0.0;
    /* Pointer-based recurrence simulating linked list traversal */
    for (int i = 0; i < n-1; i++) {
        /* Load from pointer set in previous iteration */
        double val = *ptrs[i];
        
        /* Compute new value with FP operations */
        val = val * 1.05 + data[i] * 2.0;
        
        /* Store and set pointer for next iteration */
        data[i+1] = val;
        ptrs[i+1] = &data[i+1];
        
        result += val;
    }
    return result;
}

__attribute__((noinline, optimize("no-unroll-loops")))
int test4_complex_dependence_chain(int* a, int* b, double* c, int n) {
    int sum = 0;
    /* Multiple interleaved dependence chains */
    for (int i = 2; i < n; i++) {
        /* Chain 1: distance=1 integer recurrence */
        a[i] = a[i-1] * a[i-2] + i;
        
        /* Chain 2: distance=1 floating-point recurrence */
        c[i] = c[i-1] * 0.97 + c[i-2] * 0.03;
        
        /* Chain 3: cross-chain dependence */
        b[i] = (int)(c[i-1] * 100.0) + a[i-1];
        
        /* Additional operation mixing both chains */
        a[i] += b[i-1] / 2;
        
        /* Memory operations with potential aliasing */
        if (i % 3 == 0) {
            b[i-1] = a[i] + b[i-2];
        }
        
        sum += a[i] + b[i] + (int)c[i];
    }
    return sum;
}

/* Use volatile to prevent constant propagation */
volatile int g_size = SIZE;

int main(int argc, char** argv) {
    /* Use command line argument or volatile to prevent loop unrolling */
    int size = (argc > 1) ? atoi(argv[1]) : g_size;
    if (size <= 10) size = SIZE;
    
    /* Allocate and initialize arrays */
    double* arr1 = (double*)malloc(size * sizeof(double));
    int* arr2 = (int*)malloc(size * sizeof(int));
    double* arr3 = (double*)malloc(size * sizeof(double));
    double** ptrs = (double**)malloc(size * sizeof(double*));
    
    for (int i = 0; i < size; i++) {
        arr1[i] = (double)(i % 100) * 0.1;
        arr2[i] = i % 50;
        arr3[i] = (double)(i % 75) * 0.3;
        ptrs[i] = &arr3[i];
    }
    
    double total = 0.0;
    
    /* Call test functions to trigger modulo scheduling analysis */
    total += test1_recurrence_fp(arr1, size, 1.05);
    
    int int_result = test2_mixed_latency(arr2, arr3, size);
    total += (double)int_result;
    
    total += test3_pointer_chasing(ptrs, arr3, size);
    
    int_result = test4_complex_dependence_chain(arr2, (int*)arr1, arr3, size);
    total += (double)int_result;
    
    printf("Total result: %f\n", total);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(ptrs);
    
    return (int)total % 100;
}
