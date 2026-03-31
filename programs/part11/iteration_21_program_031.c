#ifdef _OPENMP
#include <omp.h>
#endif
#include <stdio.h>
#include <stdlib.h>

/* Graceful compilation for systems without offloading support */
#ifdef _OPENMP
#ifdef _OPENMP_TARGET
#define HAS_OFFLOAD 1
#else
#define HAS_OFFLOAD 0
#endif
#else
#define HAS_OFFLOAD 0
#endif

/* Prevent inlining to ensure the SIMT transformation isn't bypassed */
__attribute__((noinline))
int simt_test(int n, int use_gpu) {
    int a[128], b[128], c[128];
    int i;
    
    /* Initialize arrays with simple patterns */
    for (i = 0; i < n; i++) {
        a[i] = i;
        b[i] = 2 * i;
        c[i] = 0;
    }
    
    /* The key construct: target simd with conditional offloading */
    #if HAS_OFFLOAD
    #pragma omp target simd if(use_gpu) \
        map(tofrom: a[0:n], b[0:n], c[0:n])
    #else
    #pragma omp simd
    #endif
    for (i = 0; i < n; i++) {
        #if HAS_OFFLOAD
        /* Diagnostic output inside target region */
        if (use_gpu) {
            /* Use builtin to avoid external function calls in offloaded code */
            __builtin_printf("Offloaded loop executed with use_gpu=%d\n", use_gpu);
        }
        #endif
        c[i] = a[i] + b[i];
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (i = 0; i < n; i++) {
        checksum += c[i];
    }
    
    return checksum;
}

int main() {
    const int n = 128;
    int result1, result2;
    
    printf("Testing SIMT transformation with OpenMP offloading\n");
    
    #if HAS_OFFLOAD
    /* First call: force GPU offloading path (use_gpu=1) */
    printf("Calling with use_gpu=1 (should trigger SIMT transformation)\n");
    result1 = simt_test(n, 1);
    
    /* Second call: use CPU path (use_gpu=0) */
    printf("Calling with use_gpu=0 (should use regular SIMD)\n");
    result2 = simt_test(n, 0);
    
    printf("Results: with_gpu=%d, without_gpu=%d\n", result1, result2);
    
    /* Verify both paths produce the same result */
    if (result1 == result2) {
        printf("SUCCESS: Both paths produced identical results\n");
    } else {
        printf("WARNING: Results differ between GPU and CPU paths\n");
    }
    #else
    printf("OpenMP target offloading not supported in this configuration\n");
    printf("Compile with -fopenmp and appropriate -foffload flag\n");
    #endif
    
    return 0;
}
