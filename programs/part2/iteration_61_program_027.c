/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler edge logging (lines 596-606 in modulo-sched.cc)
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c -mtune=itanium
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Prevent inlining to keep loop structure intact */
__attribute__((noinline))
double test1_recurrence_fp(double* arr, int n, double factor) {
    double sum = 0.0;
    /* Loop with floating-point recurrence (distance=1) */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence: arr[i] depends on arr[i-1] */
        arr[i] = arr[i-1] * factor + (double)i;
        /* Additional operations to create scheduling opportunities */
        arr[i] += arr[i] * 0.5;      /* Intra-iteration dependence */
        sum += arr[i];               /* Reduction for side effect */
    }
    return sum;
}

__attribute__((noinline))
int test2_mixed_latency(int* arr, double* farr, int n) {
    int total = 0;
    /* Mixed integer/float with memory aliasing */
    for (int i = 1; i < n; i++) {
        /* Distance-1 integer recurrence */
        arr[i] = arr[i-1] * 3 + i;
        
        /* Floating-point with higher latency */
        farr[i] = farr[i-1] * 1.01 + farr[i];
        
        /* Cross-type dependence */
        farr[i] += (double)arr[i];
        
        /* Memory load with potential aliasing */
        total += arr[i] + (int)farr[i];
    }
    return total;
}

__attribute__((noinline))
float test3_pointer_chase(float** ptrs, float* data, int n) {
    float result = 0.0f;
    /* Pointer-based recurrence simulating linked list traversal */
    for (int i = 0; i < n-1; i++) {
        /* Distance-1 pointer dependence */
        *ptrs[i+1] = *ptrs[i] + data[i];
        
        /* Additional FP operations */
        data[i+1] = data[i] * 1.5f + *ptrs[i];
        
        /* Mixed operations to create varied latencies */
        result += *ptrs[i] * data[i];
    }
    return result;
}

__attribute__((noinline))
long test4_complex_chain(long* a, double* b, int* c, int n) {
    long acc = 0;
    /* Complex chain with multiple dependences */
    for (int i = 1; i < n; i++) {
        /* Chain of distance-1 dependences */
        a[i] = a[i-1] + i * 2;           /* Integer arithmetic */
        b[i] = b[i-1] * 1.1 + a[i];      /* FP using integer result */
        c[i] = c[i-1] + (int)b[i];       /* Integer using FP result */
        
        /* Intra-iteration operations */
        double temp = b[i] * 0.5;
        a[i] += (long)temp;
        c[i] ^= (c[i] << 3);
        
        acc += a[i] + c[i] + (long)b[i];
    }
    return acc;
}

/* Helper to initialize arrays with volatile to prevent optimization */
volatile int init_seed = 42;

void init_arrays(double* arr1, int* arr2, double* arr3, float* arr4, long* arr5) {
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (double)(i + init_seed);
        arr2[i] = i * 2;
        arr3[i] = (double)(i * 3);
        arr4[i] = (float)(i * 1.5f);
        arr5[i] = i * 10L;
    }
}

int main(int argc, char** argv) {
    /* Use argc to make loop bounds non-constant */
    int n = (argc > 1) ? atoi(argv[1]) : SIZE;
    if (n > SIZE) n = SIZE;
    if (n < 10) n = 10;  /* Ensure meaningful loop size */
    
    /* Allocate and initialize arrays */
    double* arr1 = (double*)malloc(SIZE * sizeof(double));
    int* arr2 = (int*)malloc(SIZE * sizeof(int));
    double* arr3 = (double*)malloc(SIZE * sizeof(double));
    float* arr4 = (float*)malloc(SIZE * sizeof(float));
    long* arr5 = (long*)malloc(SIZE * sizeof(long));
    float** ptrs = (float**)malloc(SIZE * sizeof(float*));
    
    init_arrays(arr1, arr2, arr3, arr4, arr5);
    
    /* Initialize pointer array */
    for (int i = 0; i < SIZE; i++) {
        ptrs[i] = &arr4[i];
    }
    
    /* Call test functions to trigger modulo scheduling analysis */
    double result1 = test1_recurrence_fp(arr1, n, 1.05);
    int result2 = test2_mixed_latency(arr2, arr3, n);
    float result3 = test3_pointer_chase(ptrs, arr4, n);
    long result4 = test4_complex_chain(arr5, arr3, arr2, n);
    
    /* Aggregate results to prevent dead code elimination */
    double final_result = result1 + result2 + result3 + result4;
    printf("Final result: %f\n", final_result);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    free(arr5);
    free(ptrs);
    
    return 0;
}
