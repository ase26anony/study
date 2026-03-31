#ifdef _OPENMP
#include <omp.h>
#endif
#include <stdio.h>
#include <stdlib.h>

/* Graceful compilation without OpenMP */
#ifndef _OPENMP
#define _OPENMP_TARGET 0
#else
#ifdef _OPENMP_TARGET
/* Already defined */
#else
#define _OPENMP_TARGET 1
#endif
#endif

/* Test function with noinline to prevent optimization */
__attribute__((noinline))
int simt_test(int n, int use_gpu, int *a, int *b, int *c) {
    int checksum = 0;
    
#if _OPENMP_TARGET
    /* This is the key construct to trigger SIMT transformation */
    #pragma omp target simd if(use_gpu) map(tofrom: a[0:n], b[0:n], c[0:n])
    for (int i = 0; i < n; i++) {
        /* Simple non-eliminable computation */
        c[i] = a[i] + b[i];
        
        /* Diagnostic output inside target region */
        if (use_gpu) {
            /* Use __builtin_printf to avoid external function calls in SIMD loop */
            if (i == 0) {
                #pragma omp critical
                __builtin_printf("Offloaded loop executed with use_gpu=%d\n", use_gpu);
            }
        }
    }
#endif
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < n; i++) {
        checksum += c[i];
    }
    
    return checksum;
}

int main() {
    const int N = 128;
    int a[N], b[N], c[N];
    int checksum1, checksum2;
    
    /* Initialize arrays with simple patterns */
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = 2 * i;
        c[i] = 0;
    }
    
#if _OPENMP_TARGET
    printf("OpenMP target offloading is supported\n");
    
    /* First call: with GPU offloading (use_gpu=1) */
    checksum1 = simt_test(N, 1, a, b, c);
    
    /* Reinitialize for second call */
    for (int i = 0; i < N; i++) {
        c[i] = 0;
    }
    
    /* Second call: without GPU offloading (use_gpu=0) */
    checksum2 = simt_test(N, 0, a, b, c);
    
    printf("Checksum with GPU offloading: %d\n", checksum1);
    printf("Checksum without GPU offloading: %d\n", checksum2);
    
    /* Verify correctness */
    int expected = 0;
    for (int i = 0; i < N; i++) {
        expected += i + 2 * i;
    }
    
    if (checksum1 == expected && checksum2 == expected) {
        printf("Test PASSED\n");
    } else {
        printf("Test FAILED\n");
        return 1;
    }
#else
    printf("OpenMP target offloading not supported - skipping test\n");
#endif
    
    return 0;
}
