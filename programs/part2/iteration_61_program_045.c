/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler edge logging (lines 596-606 in modulo-sched.cc)
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c -mtune=itanium
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Prevent optimization and ensure loops aren't unrolled */
static volatile int g_volatile_size = SIZE;

/* Test functions marked noinline to prevent merging */
__attribute__((noinline, noipa))
double test1_recurrence_fp(double* arr, int n) {
    double sum = 0.0;
    /* Loop-carried FP recurrence with distance=1 */
    for (int i = 1; i < n; i++) {
        arr[i] = arr[i-1] * 1.01 + arr[i] * 0.99;  /* Distance-1 dependence on arr[i-1] */
        sum += arr[i];
    }
    return sum;
}

__attribute__((noinline, noipa))
int test2_mixed_latency(int* arr, double* farr, int n) {
    int sum = 0;
    /* Mixed integer/FP with memory aliasing and recurrence */
    for (int i = 1; i < n; i++) {
        farr[i] = farr[i-1] * 1.5 + i * 0.1;      /* FP recurrence, distance=1 */
        arr[i] = arr[i-1] + (int)farr[i];         /* Integer recurrence, distance=1 */
        sum += arr[i] * (i % 8);                  /* Integer multiply for latency */
    }
    return sum;
}

__attribute__((noinline, noipa))
float test3_pointer_chase(float** ptrs, float* data, int n) {
    float sum = 0.0f;
    /* Pointer-based recurrence simulating linked list traversal */
    for (int i = 0; i < n-1; i++) {
        *ptrs[i+1] = *ptrs[i] * 0.8f + data[i];   /* Distance-1 through pointers */
        sum += *ptrs[i];
    }
    return sum;
}

__attribute__((noinline, noipa))
long test4_complex_dependence(long* arr, double* darr, int n) {
    long acc = 0;
    /* Multiple interleaved dependences with different latencies */
    for (int i = 2; i < n; i++) {
        double temp = darr[i-1] * darr[i-2];      /* FP multiply, distance=1 and 2 */
        darr[i] = temp + 1.0;
        arr[i] = arr[i-1] + (long)(temp * 100);  /* Integer recurrence */
        acc += arr[i] * i;                        /* Integer multiply */
        
        /* Additional memory op with potential aliasing */
        if (i % 3 == 0) {
            darr[i-1] = darr[i] * 0.5;           /* Another FP op */
        }
    }
    return acc;
}

__attribute__((noinline, noipa))
double test5_nested_recurrence(int* iarr, double* darr, int n) {
    double sum = 0.0;
    /* Multiple recurrence chains */
    for (int i = 3; i < n; i++) {
        /* Chain 1: FP recurrence */
        darr[i] = darr[i-1] + darr[i-2] * 0.3 + darr[i-3] * 0.1;
        
        /* Chain 2: Integer recurrence with FP conversion */
        iarr[i] = iarr[i-1] + (int)(darr[i] * 10);
        
        /* Chain 3: Mixed operations */
        double temp = darr[i] * iarr[i];
        sum += temp + i;
        
        /* Memory store with computed index (creates address dependence) */
        if (i % 4 == 0) {
            iarr[i/2] = (int)temp;
        }
    }
    return sum;
}

int main(int argc, char** argv) {
    /* Use volatile to prevent constant propagation */
    int size = g_volatile_size;
    if (argc > 1) size = atoi(argv[1]);
    if (size < 10) size = 100;
    
    /* Allocate and initialize arrays */
    double* arr1 = (double*)malloc(size * sizeof(double));
    int* arr2 = (int*)malloc(size * sizeof(int));
    double* arr3 = (double*)malloc(size * sizeof(double));
    float** ptrs = (float**)malloc(size * sizeof(float*));
    float* data = (float*)malloc(size * sizeof(float));
    long* arr4 = (long*)malloc(size * sizeof(long));
    double* arr5 = (double*)malloc(size * sizeof(double));
    int* iarr5 = (int*)malloc(size * sizeof(int));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < size; i++) {
        arr1[i] = i * 0.5;
        arr2[i] = i;
        arr3[i] = i * 0.25;
        data[i] = i * 0.1f;
        ptrs[i] = &data[(i * 7) % size];  /* Create pointer chain */
        arr4[i] = i * 2L;
        arr5[i] = i * 0.33;
        iarr5[i] = i % 100;
    }
    
    double total = 0.0;
    
    /* Call test functions - each designed to trigger modulo scheduling */
    total += test1_recurrence_fp(arr1, size);
    total += test2_mixed_latency(arr2, arr3, size);
    total += test3_pointer_chase(ptrs, data, size);
    total += test4_complex_dependence(arr4, arr5, size);
    total += test5_nested_recurrence(iarr5, arr5, size);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %f\n", total);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(ptrs);
    free(data);
    free(arr4);
    free(arr5);
    free(iarr5);
    
    return 0;
}
