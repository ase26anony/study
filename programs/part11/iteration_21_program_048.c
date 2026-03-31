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
    int i;
    
    /* Initialize arrays with simple patterns */
    for (i = 0; i < n; i++) {
        a[i] = i;
        b[i] = 2 * i;
        c[i] = 0;
    }
    
    int checksum = 0;
    
#ifdef _OPENMP_TARGET
    /* This is the key construct that should trigger the SIMT transformation */
    #pragma omp target simd if(use_gpu) map(tofrom: a[0:n], b[0:n], c[0:n])
    for (i = 0; i < n; i++) {
        /* Diagnostic output to confirm execution */
        if (use_gpu) {
            /* Use __builtin_printf to avoid potential issues with stdio in offload regions */
            __builtin_printf("Offloaded loop executed with use_gpu=%d, i=%d\n", use_gpu, i);
        }
        c[i] = a[i] + b[i];
    }
#else
    /* Fallback for non-offloading compilation */
    #pragma omp simd
    for (i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
#endif
    
    /* Compute checksum to prevent dead code elimination */
    for (i = 0; i < n; i++) {
        checksum += c[i];
    }
    
    return checksum;
}

int main() {
    int result1, result2;
    
    printf("Testing SIMT transformation with GPU offloading...\n");
    
#ifdef _OPENMP_TARGET
    /* Test both paths: with and without GPU offloading */
    printf("Calling with use_gpu=1 (should trigger SIMT path)...\n");
    result1 = simt_test(N, 1);
    
    printf("Calling with use_gpu=0 (should use fallback path)...\n");
    result2 = simt_test(N, 0);
#else
    /* Fallback for systems without offloading support */
    printf("Offloading not supported, using host-only execution...\n");
    result1 = simt_test(N, 0);
    result2 = result1;
#endif
    
    printf("Result with use_gpu=1: %d\n", result1);
    printf("Result with use_gpu=0: %d\n", result2);
    
    /* Expected checksum: sum_{i=0}^{127} (i + 2i) = sum_{i=0}^{127} 3i = 3 * (127*128/2) = 3 * 8128 = 24384 */
    printf("Expected result: 24384\n");
    
    return 0;
}
