#ifdef _OPENMP
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _OPENMP_TARGET

/* Test function marked noinline to prevent optimization */
__attribute__((noinline, noipa))
int simt_test(int n, int use_gpu) {
    int a[128], b[128], c[128];
    int i;
    
    /* Initialize arrays with simple patterns */
    for (i = 0; i < n; i++) {
        a[i] = i;
        b[i] = 2 * i;
        c[i] = 0;
    }
    
    /* This is the key construct: target simd with if clause */
    #pragma omp target simd if(use_gpu) map(tofrom: a[0:n], b[0:n], c[0:n])
    for (i = 0; i < n; i++) {
        /* Diagnostic output inside target region */
        if (use_gpu) {
            /* Use __builtin_printf to avoid external function calls in offloaded code */
            __builtin_printf("Offloaded loop executed with use_gpu=%d, i=%d\n", use_gpu, i);
        }
        c[i] = a[i] + b[i];
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (i = 0; i < n; i++) {
        checksum += c[i];
    }
    
    return checksum;
}

#endif /* _OPENMP_TARGET */

int main() {
    int result1 = 0, result2 = 0;
    
#ifdef _OPENMP_TARGET
    const int n = 128;
    
    /* First call: use_gpu = 1 - should trigger SIMT path */
    result1 = simt_test(n, 1);
    
    /* Second call: use_gpu = 0 - should take fallback path */
    result2 = simt_test(n, 0);
    
    printf("Result with GPU (use_gpu=1): %d\n", result1);
    printf("Result without GPU (use_gpu=0): %d\n", result2);
    
    /* Expected result: sum of i + 2*i for i=0..127 */
    int expected = 0;
    for (int i = 0; i < n; i++) {
        expected += i + 2*i;
    }
    
    if (result1 == expected && result2 == expected) {
        printf("Test PASSED: Both paths produced correct results\n");
    } else {
        printf("Test FAILED: Expected %d, got %d and %d\n", expected, result1, result2);
        return 1;
    }
#else
    printf("OpenMP target offloading not supported - test skipped\n");
#endif
    
    return 0;
}

#else
int main() {
    printf("OpenMP not supported - test skipped\n");
    return 0;
}
#endif /* _OPENMP */
