#ifdef _OPENMP
#include <omp.h>
#endif
#include <stdio.h>
#include <stdlib.h>

#ifdef _OPENMP_TARGET

/* Test function marked noinline to prevent optimization */
__attribute__((noinline))
int simt_test(int n, int use_gpu) {
    int a[128], b[128], c[128];
    int checksum = 0;
    
    /* Initialize arrays with simple patterns */
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = 2 * i;
        c[i] = 0;
    }
    
    /* Target SIMD region with conditional offloading */
    #pragma omp target simd if(use_gpu) \
        map(to: a[0:n], b[0:n]) map(from: c[0:n])
    for (int i = 0; i < n; i++) {
        /* Diagnostic output inside target region */
        if (i == 0) {
            #pragma omp critical
            printf("Offloaded loop executed with use_gpu=%d\n", use_gpu);
        }
        c[i] = a[i] + b[i];
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < n; i++) {
        checksum += c[i];
    }
    
    return checksum;
}

#endif /* _OPENMP_TARGET */

int main() {
    int result1 = 0, result2 = 0;
    
#ifdef _OPENMP_TARGET
    const int n = 128;
    
    /* First call: force GPU offloading path */
    result1 = simt_test(n, 1);
    
    /* Second call: force host execution path */
    result2 = simt_test(n, 0);
    
    printf("Checksum with GPU: %d\n", result1);
    printf("Checksum without GPU: %d\n", result2);
    
    /* Verify correctness */
    int expected = 0;
    for (int i = 0; i < n; i++) {
        expected += i + (2 * i);  /* a[i] + b[i] */
    }
    
    if (result1 == expected && result2 == expected) {
        printf("Results are correct!\n");
    } else {
        printf("Results mismatch! Expected: %d\n", expected);
    }
#else
    printf("OpenMP target offloading not supported in this configuration.\n");
#endif
    
    return 0;
}
