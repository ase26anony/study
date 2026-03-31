#ifdef _OPENMP
#include <omp.h>
#endif
#include <stdio.h>
#include <stdlib.h>

#define N 128

/* Prevent inlining to ensure the SIMT transformation isn't optimized away */
__attribute__((noinline,noipa))
void simt_test(int n, int use_gpu, int *checksum)
{
    int a[N], b[N], c[N];
    
    /* Initialize arrays with simple patterns */
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = 2 * i;
        c[i] = 0;
    }
    
    /* This is the key construct: target simd with conditional offloading */
    #pragma omp target simd if(use_gpu) map(tofrom: a[0:n], b[0:n], c[0:n])
    for (int i = 0; i < n; i++) {
        /* Diagnostic output to verify execution */
        if (i == 0) {
            #ifdef _OPENMP_TARGET
            printf("Offloaded loop executed with use_gpu=%d\n", use_gpu);
            #endif
        }
        c[i] = a[i] + b[i];
    }
    
    /* Compute checksum to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += c[i];
    }
    *checksum = sum;
}

int main(void)
{
#ifdef _OPENMP_TARGET
    int checksum1 = 0, checksum2 = 0;
    
    /* First call: force GPU offloading path (use_gpu=1) */
    printf("Testing with use_gpu=1 (should trigger SIMT transformation):\n");
    simt_test(N, 1, &checksum1);
    printf("Checksum 1: %d\n", checksum1);
    
    /* Second call: force host execution path (use_gpu=0) */
    printf("\nTesting with use_gpu=0 (should use host execution):\n");
    simt_test(N, 0, &checksum2);
    printf("Checksum 2: %d\n", checksum2);
    
    /* Verify both paths produced same result */
    if (checksum1 == checksum2) {
        printf("\nSUCCESS: Both execution paths produced identical results\n");
    } else {
        printf("\nWARNING: Results differ between execution paths\n");
    }
    
    return 0;
#else
    printf("OpenMP target offloading not supported in this configuration\n");
    return 0;
#endif
}
