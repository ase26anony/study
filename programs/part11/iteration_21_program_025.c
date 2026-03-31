#ifdef _OPENMP
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

/* Test function with noinline to prevent optimization */
__attribute__((noinline))
int simt_test(int n, int use_gpu) {
    int a[128], b[128], c[128];
    int i, sum = 0;
    
    /* Initialize arrays */
    for (i = 0; i < n; i++) {
        a[i] = i;
        b[i] = 2 * i;
        c[i] = 0;
    }
    
    /* Target SIMD region with conditional offloading */
    #pragma omp target simd if(use_gpu) map(tofrom: a[0:n], b[0:n], c[0:n])
    for (i = 0; i < n; i++) {
        /* Diagnostic output inside target region */
        if (i == 0) {
            #pragma omp critical
            printf("Offloaded loop executed with use_gpu=%d\n", use_gpu);
        }
        c[i] = a[i] + b[i];
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (i = 0; i < n; i++) {
        sum += c[i];
    }
    
    return sum;
}

int main() {
    int n = 128;
    int result1, result2;
    
    printf("Testing SIMT transformation with GPU offloading...\n");
    
    /* First call: attempt GPU offloading */
    result1 = simt_test(n, 1);
    printf("Result with use_gpu=1: %d\n", result1);
    
    /* Second call: force host execution */
    result2 = simt_test(n, 0);
    printf("Result with use_gpu=0: %d\n", result2);
    
    /* Verify results match */
    if (result1 == result2) {
        printf("Results match - test passed\n");
    } else {
        printf("Results differ - test failed\n");
    }
    
    return 0;
}

#else
/* Fallback for non-OpenMP compilation */
int main() {
    printf("OpenMP not supported - test skipped\n");
    return 0;
}
#endif
