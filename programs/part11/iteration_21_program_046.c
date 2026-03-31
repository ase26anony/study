#ifdef _OPENMP
#include <omp.h>
#endif
#include <stdio.h>
#include <stdlib.h>

#define N 128

/* Prevent inlining to ensure the function isn't optimized away */
__attribute__((noinline))
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
    /* This is the key construct that should trigger SIMT transformation */
    #pragma omp target simd if(use_gpu) map(tofrom: a[0:n], b[0:n], c[0:n])
    for (int i = 0; i < n; i++) {
        /* Diagnostic output to verify execution */
        if (use_gpu) {
            /* Use __builtin_printf to avoid potential issues with stdio in device code */
            __builtin_printf("Offloaded loop executed with use_gpu=%d, i=%d\n", use_gpu, i);
        }
        c[i] = a[i] + b[i];
    }
#else
    /* Fallback for non-offloading compilation */
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
    int result1, result2;
    
    printf("Testing SIMT transformation with GPU offloading...\n");
    
#ifdef _OPENMP_TARGET
    /* First call with GPU enabled - should trigger SIMT path */
    result1 = simt_test(N, 1);
    printf("Result with use_gpu=1: %d\n", result1);
    
    /* Second call with GPU disabled - should take fallback path */
    result2 = simt_test(N, 0);
    printf("Result with use_gpu=0: %d\n", result2);
    
    /* Verify results match (both should compute the same checksum) */
    if (result1 == result2) {
        printf("Results match - test passed!\n");
    } else {
        printf("Results differ - test failed!\n");
    }
#else
    printf("OpenMP target offloading not supported in this configuration.\n");
    printf("Compile with -fopenmp and appropriate -foffload flag.\n");
#endif
    
    return 0;
}
