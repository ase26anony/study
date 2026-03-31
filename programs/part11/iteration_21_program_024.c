#ifdef _OPENMP
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _OPENMP_TARGET

/* Test function marked noinline to prevent optimization */
__attribute__((noinline))
int simt_test(int n, int use_gpu) {
    int a[n], b[n], c[n];
    int checksum = 0;
    
    /* Initialize arrays with simple patterns */
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = 2 * i;
        c[i] = 0;
    }
    
    /* Target SIMD construct with conditional offloading */
    #pragma omp target simd if(use_gpu) \
        map(tofrom: a[0:n], b[0:n], c[0:n])
    for (int i = 0; i < n; i++) {
        /* Diagnostic output inside target region */
        if (i == 0) {
            #pragma omp critical
            __builtin_printf("Offloaded loop executed with use_gpu=%d\n", use_gpu);
        }
        c[i] = a[i] + b[i];
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < n; i++) {
        checksum += c[i];
    }
    
    return checksum;
}

int main() {
    const int n = 128;
    int result1, result2;
    
    printf("Testing SIMT transformation with GPU offloading...\n");
    
    /* First call: attempt GPU offloading */
    result1 = simt_test(n, 1);
    printf("Result with use_gpu=1: %d\n", result1);
    
    /* Second call: force host execution */
    result2 = simt_test(n, 0);
    printf("Result with use_gpu=0: %d\n", result2);
    
    /* Verify results match (should both be sum of 3*i for i=0..127) */
    int expected = 0;
    for (int i = 0; i < n; i++) {
        expected += 3 * i;
    }
    
    if (result1 == expected && result2 == expected) {
        printf("Test PASSED: Both paths produced correct results\n");
        return 0;
    } else {
        printf("Test FAILED: Results mismatch\n");
        return 1;
    }
}

#else /* _OPENMP_TARGET not defined */

int main() {
    printf("OpenMP target offloading not supported in this configuration\n");
    printf("Compile with -foffload flag to enable GPU offloading\n");
    return 0;
}

#endif /* _OPENMP_TARGET */

#else /* _OPENMP not defined */

int main() {
    printf("OpenMP not supported in this configuration\n");
    return 0;
}

#endif /* _OPENMP */
