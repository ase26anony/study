#ifdef _OPENMP
#include <omp.h>
#endif
#include <stdio.h>
#include <stdlib.h>

/* Graceful compilation without OpenMP offloading support */
#ifdef _OPENMP
#ifdef _OPENMP_TARGET

/* Prevent inlining to ensure the function isn't optimized away */
__attribute__((noinline))
void simt_test(int n, int use_gpu) {
    int a[128], b[128], c[128];
    
    /* Initialize arrays with simple patterns */
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = 2 * i;
        c[i] = 0;
    }
    
    /* The key construct: target simd with if clause */
    #pragma omp target simd if(use_gpu) map(tofrom: a[0:n], b[0:n], c[0:n])
    for (int i = 0; i < n; i++) {
        /* Diagnostic output inside target region */
        if (use_gpu) {
            /* Use __builtin_printf to avoid host/device printf issues */
            __builtin_printf("Offloaded loop executed with use_gpu=%d\n", use_gpu);
        }
        c[i] = a[i] + b[i];
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += c[i];
    }
    
    /* Return checksum through global to avoid optimization */
    static int result[2];
    result[use_gpu] = checksum;
    
    printf("Test completed with use_gpu=%d, checksum=%d\n", use_gpu, checksum);
}

#endif /* _OPENMP_TARGET */
#endif /* _OPENMP */

int main() {
#ifdef _OPENMP
#ifdef _OPENMP_TARGET
    const int n = 128;
    
    printf("Testing SIMT transformation with GPU offloading...\n");
    
    /* First call: with GPU offloading (use_gpu=1) */
    printf("\nCall 1: use_gpu=1 (should trigger SIMT path)\n");
    simt_test(n, 1);
    
    /* Second call: without GPU offloading (use_gpu=0) */
    printf("\nCall 2: use_gpu=0 (should use host fallback)\n");
    simt_test(n, 0);
    
    printf("\nSIMT test completed.\n");
#else
    printf("OpenMP target offloading not supported in this configuration.\n");
#endif
#else
    printf("OpenMP not supported.\n");
#endif
    
    return 0;
}
