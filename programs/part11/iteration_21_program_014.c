#ifdef _OPENMP
#include <omp.h>
#endif
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure the function isn't optimized away */
__attribute__((noinline))
#ifdef _OPENMP_TARGET
void simt_test(int n, int use_gpu, int *a, int *b, int *c)
{
    int i;
    
    /* This should trigger the SIMT transformation when compiled for GPU offloading */
    #pragma omp target simd if(use_gpu) map(tofrom: a[0:n], b[0:n], c[0:n])
    for (i = 0; i < n; i++) {
        /* Diagnostic output to verify execution path */
        if (use_gpu) {
            /* Use builtin to avoid stdio dependencies in offloaded code */
            __builtin_printf("Offloaded loop executed with use_gpu=%d, i=%d\n", 
                           use_gpu, i);
        }
        c[i] = a[i] + b[i];
    }
}
#else
/* Dummy implementation when offloading not supported */
void simt_test(int n, int use_gpu, int *a, int *b, int *c)
{
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}
#endif

int main(void)
{
    const int N = 128;
    int a[N], b[N], c[N];
    int checksum1 = 0, checksum2 = 0;
    
    /* Initialize arrays with simple patterns */
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = 2 * i;
        c[i] = 0;
    }
    
#ifdef _OPENMP_TARGET
    printf("Testing with OpenMP target offloading support\n");
    
    /* First call: use_gpu=1 - should trigger SIMT path */
    simt_test(N, 1, a, b, c);
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < N; i++) {
        checksum1 += c[i];
    }
    
    /* Re-initialize for second test */
    for (int i = 0; i < N; i++) {
        c[i] = 0;
    }
    
    /* Second call: use_gpu=0 - should take host fallback path */
    simt_test(N, 0, a, b, c);
    
    for (int i = 0; i < N; i++) {
        checksum2 += c[i];
    }
    
    printf("Checksum with use_gpu=1: %d\n", checksum1);
    printf("Checksum with use_gpu=0: %d\n", checksum2);
    printf("Expected checksum: %d\n", 3 * (N-1) * N / 2);
#else
    printf("OpenMP target offloading not supported in this configuration\n");
    /* Fallback execution without offloading */
    simt_test(N, 0, a, b, c);
    for (int i = 0; i < N; i++) {
        checksum1 += c[i];
    }
    printf("Checksum (host only): %d\n", checksum1);
#endif
    
    return 0;
}
