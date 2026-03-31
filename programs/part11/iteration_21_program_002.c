#ifdef _OPENMP
#include <omp.h>
#endif
#include <stdio.h>
#include <stdlib.h>

/* Graceful compilation for systems without offloading support */
#ifdef _OPENMP_TARGET

/* Prevent inlining to ensure the test function remains intact */
__attribute__((noinline))
void simt_test(int n, int use_gpu, int *a, int *b, int *c) {
    int i;
    
    /* 
     * Target SIMD construct with conditional offloading.
     * This is the key construct that should trigger the SIMT transformation.
     * The 'if' clause creates the conditional wrapper needed for coverage.
     */
    #pragma omp target simd if(use_gpu) \
        map(tofrom: a[0:n], b[0:n], c[0:n])
    for (i = 0; i < n; i++) {
        /* Simple non-eliminable computation */
        c[i] = a[i] + b[i];
        
        /* Diagnostic output to verify execution path */
        if (use_gpu) {
            /* Use builtin to avoid stdio dependencies in offloaded code */
            __builtin_printf("Offloaded loop executed with use_gpu=%d at i=%d\n", 
                           use_gpu, i);
        }
    }
}

#endif /* _OPENMP_TARGET */

int main(void) {
#ifdef _OPENMP_TARGET
    const int N = 128;
    int a[N], b[N], c[N];
    int i, checksum1 = 0, checksum2 = 0;
    
    /* Initialize arrays with simple patterns */
    for (i = 0; i < N; i++) {
        a[i] = i;
        b[i] = 2 * i;
        c[i] = 0;
    }
    
    printf("Testing SIMT transformation with GPU offloading...\n");
    
    /* First call: use_gpu=1 - should trigger the SIMT path */
    simt_test(N, 1, a, b, c);
    
    /* Compute checksum to prevent dead code elimination */
    for (i = 0; i < N; i++) {
        checksum1 += c[i];
    }
    
    /* Re-initialize for second test */
    for (i = 0; i < N; i++) {
        c[i] = 0;
    }
    
    /* Second call: use_gpu=0 - should take the non-SIMT path */
    simt_test(N, 0, a, b, c);
    
    for (i = 0; i < N; i++) {
        checksum2 += c[i];
    }
    
    printf("Checksum with GPU (use_gpu=1): %d\n", checksum1);
    printf("Checksum without GPU (use_gpu=0): %d\n", checksum2);
    printf("Expected checksum: %d\n", 3 * N * (N - 1) / 2);
    
    return (checksum1 == checksum2 && 
            checksum1 == 3 * N * (N - 1) / 2) ? 0 : 1;
#else
    printf("OpenMP target offloading not supported in this configuration.\n");
    return 0;
#endif
}
