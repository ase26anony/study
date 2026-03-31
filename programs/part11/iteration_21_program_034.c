#ifdef _OPENMP
#include <omp.h>
#endif
#include <stdio.h>
#include <stdlib.h>

/* Graceful compilation for systems without offloading support */
#ifdef _OPENMP
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
    #pragma omp target simd if(use_gpu) map(tofrom: a[0:n], b[0:n], c[0:n])
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
#endif /* _OPENMP */

int main() {
    const int N = 128;
    int result1 = 0, result2 = 0;
    
#ifdef _OPENMP
#ifdef _OPENMP_TARGET
    /* First call: attempt GPU offloading (SIMT path) */
    result1 = simt_test(N, 1);
    
    /* Second call: force host execution (non-SIMT path) */
    result2 = simt_test(N, 0);
    
    printf("Checksum with GPU offloading: %d\n", result1);
    printf("Checksum without GPU offloading: %d\n", result2);
    
    /* Verify correctness */
    int expected = 0;
    for (int i = 0; i < N; i++) {
        expected += i + 2 * i;
    }
    
    if (result1 == expected && result2 == expected) {
        printf("Results are correct!\n");
    } else {
        printf("Results mismatch!\n");
        return 1;
    }
#else
    printf("OpenMP target offloading not supported in this configuration.\n");
#endif
#else
    printf("OpenMP not supported.\n");
#endif
    
    return 0;
}
