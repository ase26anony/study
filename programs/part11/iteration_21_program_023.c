#ifdef _OPENMP
#include <omp.h>
#endif
#include <stdio.h>
#include <stdlib.h>

#define N 128

/* Prevent inlining to ensure the function isn't optimized away */
__attribute__((noinline))
int simt_test(int n, int use_gpu) {
    int a[N], b[N], c[N];
    int checksum = 0;
    
    /* Initialize arrays with simple patterns */
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = 2 * i;
        c[i] = 0;
    }
    
    /* The key construct: target simd with conditional offloading */
#ifdef _OPENMP_TARGET
    #pragma omp target simd if(use_gpu) \
        map(tofrom: a[0:n], b[0:n], c[0:n])
    for (int i = 0; i < n; i++) {
        /* Diagnostic output inside target region */
        if (use_gpu) {
            /* Use builtin to avoid stdio mapping issues */
            __builtin_printf("Offloaded loop executed with use_gpu=%d, i=%d\n", 
                           use_gpu, i);
        }
        c[i] = a[i] + b[i];
    }
#else
    /* Fallback for non-offloading compilation */
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
#endif
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < n; i++) {
        checksum += c[i];
    }
    
    return checksum;
}

int main() {
    int result1, result2;
    
    printf("Testing SIMT transformation with GPU offloading...\n");
    
    /* First call: with GPU offloading (use_gpu=1) */
    printf("\nCall 1: use_gpu=1\n");
    result1 = simt_test(N, 1);
    printf("Checksum 1: %d\n", result1);
    
    /* Second call: without GPU offloading (use_gpu=0) */
    printf("\nCall 2: use_gpu=0\n");
    result2 = simt_test(N, 0);
    printf("Checksum 2: %d\n", result2);
    
    /* Verify results match (both should compute same checksum) */
    if (result1 == result2) {
        printf("\nSUCCESS: Both paths produced identical results\n");
    } else {
        printf("\nWARNING: Results differ - check offloading behavior\n");
    }
    
    return 0;
}
