#ifdef _OPENMP
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _OPENMP_TARGET

/* Test function marked noinline to prevent optimization */
__attribute__((noinline))
static int simt_test(int n, int use_gpu) {
    int a[128], b[128], c[128];
    int i, sum = 0;
    
    /* Initialize arrays with simple patterns */
    for (i = 0; i < n; i++) {
        a[i] = i;
        b[i] = 2 * i;
        c[i] = 0;
    }
    
    /* 
     * This pragma should trigger the SIMT transformation:
     * - target: enables offloading
     * - simd: enables SIMD/SIMT transformation
     * - if(use_gpu): creates conditional wrapper
     * - map clauses: ensure data movement for offloading
     */
    #pragma omp target simd if(use_gpu) \
        map(to: a[0:n], b[0:n]) map(from: c[0:n])
    for (i = 0; i < n; i++) {
        /* Diagnostic output to ensure code execution */
        if (use_gpu) {
            /* Use __builtin_printf to avoid stdio overhead in offload region */
            __builtin_printf("Offloaded loop executed with use_gpu=%d, i=%d\n", 
                           use_gpu, i);
        }
        c[i] = a[i] + b[i];
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (i = 0; i < n; i++) {
        sum += c[i];
    }
    
    return sum;
}

int main(void) {
    int n = 128;
    int result1, result2;
    
    printf("Testing SIMT transformation with GPU offloading...\n");
    
    /* First call: use_gpu=1 - should trigger SIMT path */
    result1 = simt_test(n, 1);
    printf("Result with use_gpu=1: %d\n", result1);
    
    /* Second call: use_gpu=0 - should use host fallback */
    result2 = simt_test(n, 0);
    printf("Result with use_gpu=0: %d\n", result2);
    
    /* Verify results match (should both be sum of i + 2*i = 3*i) */
    int expected = 0;
    for (int i = 0; i < n; i++) {
        expected += 3 * i;
    }
    
    if (result1 == expected && result2 == expected) {
        printf("Test passed! Both paths produced correct results.\n");
        return 0;
    } else {
        printf("Test failed! Expected %d, got %d and %d\n", 
               expected, result1, result2);
        return 1;
    }
}

#else /* _OPENMP_TARGET not defined */

int main(void) {
    printf("OpenMP target offloading not supported in this configuration.\n");
    printf("Compile with -foffload flag to enable GPU offloading.\n");
    return 0;
}

#endif /* _OPENMP_TARGET */

#else /* _OPENMP not defined */

int main(void) {
    printf("OpenMP not supported in this configuration.\n");
    return 0;
}

#endif /* _OPENMP */
