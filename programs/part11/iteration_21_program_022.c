#ifdef _OPENMP
#include <omp.h>
#endif
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure the function remains separate */
__attribute__((noinline))
#ifdef _OPENMP_TARGET
void simt_test(int n, int use_gpu)
{
    int a[128], b[128], c[128];
    
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
        /* Diagnostic output to confirm execution */
        if (use_gpu) {
            /* Use __builtin_printf to avoid potential offloading issues */
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
    
    /* Verify results match expected pattern */
    for (int i = 0; i < n; i++) {
        if (c[i] != 3 * i) {
            printf("Error at index %d: expected %d, got %d\n", 
                   i, 3 * i, c[i]);
        }
    }
    
    printf("Checksum for use_gpu=%d: %d\n", use_gpu, checksum);
}
#else
/* Dummy implementation when offloading not supported */
void simt_test(int n, int use_gpu)
{
    printf("OpenMP target offloading not supported in this configuration\n");
}
#endif

int main(void)
{
    const int n = 128;
    
#ifdef _OPENMP_TARGET
    /* Test both paths: with and without GPU offloading */
    printf("Testing SIMT transformation with GPU offloading...\n");
    simt_test(n, 1);  /* Should trigger SIMT path */
    
    printf("\nTesting SIMT transformation without GPU offloading...\n");
    simt_test(n, 0);  /* Should take fallback path */
#else
    printf("Skipping SIMT test - OpenMP target offloading not enabled\n");
    printf("Compile with -fopenmp and -foffload=<target> to enable\n");
#endif
    
    return 0;
}
