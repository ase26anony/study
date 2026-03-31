#ifdef _OPENMP
#include <omp.h>
#endif
#include <stdio.h>
#include <stdlib.h>

#define N 128

/* Prevent inlining to ensure the function body isn't optimized away */
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
    
#ifdef _OPENMP_TARGET
    /* 
     * This is the key construct that should trigger the SIMT transformation.
     * The combination of 'target simd' with 'if' clause and data mapping
     * should activate the GPU offloading path with SIMT lowering.
     */
    #pragma omp target simd if(use_gpu) \
        map(to: a[0:n], b[0:n]) map(from: c[0:n])
    for (int i = 0; i < n; i++) {
        /* Diagnostic output to verify execution */
        if (use_gpu) {
            /* Use __builtin_printf to avoid potential issues with stdio in device code */
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
    
    /* First call with use_gpu=1 to trigger the SIMT path */
    printf("Calling with use_gpu=1...\n");
    result1 = simt_test(N, 1);
    printf("Result with GPU: %d\n", result1);
    
    /* Second call with use_gpu=0 to exercise the false path */
    printf("Calling with use_gpu=0...\n");
    result2 = simt_test(N, 0);
    printf("Result without GPU: %d\n", result2);
    
    /* Verify results match expected value */
    int expected = 0;
    for (int i = 0; i < N; i++) {
        expected += i + 2*i;
    }
    
    if (result1 == expected && result2 == expected) {
        printf("Test passed!\n");
        return 0;
    } else {
        printf("Test failed! Expected: %d, Got: %d and %d\n", 
               expected, result1, result2);
        return 1;
    }
}
