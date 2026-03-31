#ifdef _OPENMP
#include <omp.h>
#endif
#include <stdio.h>
#include <stdlib.h>

#ifdef _OPENMP_TARGET

/* Test function with noinline to prevent optimization */
__attribute__((noinline))
int simt_test(int n, int use_gpu) {
    int a[128], b[128], c[128];
    int i, sum = 0;
    
    /* Initialize arrays with simple patterns */
    for (i = 0; i < n; i++) {
        a[i] = i;
        b[i] = 2 * i;
        c[i] = 0;
    }
    
    /* Target SIMD construct with conditional offloading */
    #pragma omp target simd if(use_gpu) \
        map(tofrom: a[0:n], b[0:n], c[0:n])
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

#endif /* _OPENMP_TARGET */

int main() {
#ifdef _OPENMP_TARGET
    const int n = 128;
    int result1, result2;
    
    printf("Testing SIMT transformation with GPU offloading...\n");
    
    /* First call: force GPU offloading path */
    result1 = simt_test(n, 1);
    printf("Result with use_gpu=1: %d\n", result1);
    
    /* Second call: force host execution path */
    result2 = simt_test(n, 0);
    printf("Result with use_gpu=0: %d\n", result2);
    
    /* Verify results match expected value */
    int expected = 0;
    for (int i = 0; i < n; i++) {
        expected += i + 2*i;  /* a[i] + b[i] */
    }
    
    if (result1 == expected && result2 == expected) {
        printf("All tests passed!\n");
    } else {
        printf("Test failed! Expected: %d, Got: %d and %d\n", 
               expected, result1, result2);
    }
    
    return 0;
#else
    printf("OpenMP target offloading not supported in this configuration.\n");
    return 0;
#endif
}
