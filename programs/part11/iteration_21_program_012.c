#ifdef _OPENMP
#include <omp.h>
#endif
#include <stdio.h>
#include <stdlib.h>

#define N 128

/* Prevent inlining to ensure the SIMT transformation isn't optimized away */
__attribute__((noinline))
#ifdef _OPENMP_TARGET
void simt_test(int n, int use_gpu)
{
    int a[N], b[N], c[N];
    
    /* Initialize arrays with simple patterns */
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = 2 * i;
        c[i] = 0;
    }
    
    /* 
     * Target SIMD construct with conditional offloading
     * This should trigger the SIMT transformation when compiled for GPU offloading
     */
    #pragma omp target simd if(use_gpu) map(tofrom: a[0:n], b[0:n], c[0:n])
    for (int i = 0; i < n; i++) {
        /* Diagnostic output to ensure code execution */
        if (use_gpu) {
            /* Use __builtin_printf to avoid stdio overhead in offloaded code */
            __builtin_printf("Offloaded loop executed with use_gpu=%d, i=%d\n", 
                           use_gpu, i);
        }
        c[i] = a[i] + b[i];
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += c[i];
    }
    
    /* Verify computation */
    printf("Checksum for use_gpu=%d: %d (expected: %d)\n", 
           use_gpu, checksum, 3 * n * (n - 1) / 2);
}
#else
/* Fallback implementation when offloading is not supported */
void simt_test(int n, int use_gpu)
{
    printf("OpenMP target offloading not supported in this configuration\n");
}
#endif

int main(void)
{
    int n = N;
    
#ifdef _OPENMP_TARGET
    /* Test both paths: with and without GPU offloading */
    printf("Testing SIMT transformation with GPU offloading...\n");
    simt_test(n, 1);  /* Should trigger SIMT path */
    
    printf("\nTesting SIMT transformation without GPU offloading...\n");
    simt_test(n, 0);  /* Should take fallback path */
#else
    printf("OpenMP target offloading not enabled at compile time\n");
    simt_test(n, 0);
#endif
    
    return 0;
}
