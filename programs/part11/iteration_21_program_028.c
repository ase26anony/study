#ifdef _OPENMP
#include <omp.h>
#endif
#include <stdio.h>
#include <stdlib.h>

#define N 128

/* Prevent inlining to ensure the SIMT transformation isn't optimized away */
__attribute__((noinline,noipa))
int simt_test(int n, int use_gpu) {
    int a[N], b[N], c[N];
    int checksum = 0;
    
    /* Initialize arrays with simple patterns */
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = 2 * i;
        c[i] = 0;
    }
    
#ifdef _OPENMP_TARGET
    /* The key construct: target simd with conditional offloading */
    #pragma omp target simd if(use_gpu) map(tofrom: a[0:n], b[0:n], c[0:n])
    for (int i = 0; i < n; i++) {
        /* Diagnostic output to verify execution */
        if (use_gpu) {
            /* Use builtin to avoid external function calls in offloaded code */
            __builtin_printf("Offloaded loop executed with use_gpu=%d, i=%d\n", 
                           use_gpu, i);
        }
        c[i] = a[i] + b[i];
    }
#else
    /* Fallback for non-offloading compilation */
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
#endif
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < n; i++) {
        checksum += c[i];
    }
    
    return checksum;
}

int main() {
    int result1 = 0, result2 = 0;
    
    printf("Testing SIMT transformation with GPU offloading...\n");
    
#ifdef _OPENMP_TARGET
    /* First call: with GPU offloading enabled */
    result1 = simt_test(N, 1);
    printf("Result with use_gpu=1: %d\n", result1);
    
    /* Second call: with GPU offloading disabled */
    result2 = simt_test(N, 0);
    printf("Result with use_gpu=0: %d\n", result2);
#else
    /* Fallback for systems without offloading support */
    printf("OpenMP target offloading not supported in this configuration.\n");
    result1 = simt_test(N, 0);
    printf("Result (host-only): %d\n", result1);
#endif
    
    /* Expected checksum: sum(i + 2*i) for i=0..127 = sum(3*i) = 3 * (127*128/2) */
    int expected = 3 * (127 * 128 / 2);
    printf("Expected checksum: %d\n", expected);
    
    return 0;
}
