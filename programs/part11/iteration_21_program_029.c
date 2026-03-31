#ifdef _OPENMP
#include <omp.h>
#endif
#include <stdio.h>
#include <stdlib.h>

#ifdef _OPENMP_TARGET

/* Prevent inlining to ensure the SIMT transformation isn't optimized away */
__attribute__((noinline, noipa))
void simt_test(int n, int use_gpu, int *checksum)
{
    int a[128], b[128], c[128];
    
    /* Initialize arrays with simple patterns */
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = 2 * i;
        c[i] = 0;
    }
    
    /* Target SIMD construct with conditional offloading - this should trigger SIMT transformation */
    #pragma omp target simd if(use_gpu) map(tofrom: a[0:n], b[0:n], c[0:n])
    for (int i = 0; i < n; i++) {
        /* Diagnostic output to ensure code executes */
        if (use_gpu) {
            /* Use builtin to avoid requiring stdio on device */
            __builtin_printf("Offloaded loop iteration %d with use_gpu=%d\n", i, use_gpu);
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

#endif /* _OPENMP_TARGET */

int main()
{
#ifdef _OPENMP_TARGET
    int checksum1 = 0, checksum2 = 0;
    const int n = 128;
    
    printf("Testing SIMT transformation with GPU offloading...\n");
    
    /* First call with GPU offloading enabled */
    printf("Call 1: use_gpu = 1\n");
    simt_test(n, 1, &checksum1);
    
    /* Second call with GPU offloading disabled */
    printf("Call 2: use_gpu = 0\n");
    simt_test(n, 0, &checksum2);
    
    printf("Checksum with GPU: %d\n", checksum1);
    printf("Checksum without GPU: %d\n", checksum2);
    
    /* Verify computation correctness */
    int expected_sum = 0;
    for (int i = 0; i < n; i++) {
        expected_sum += i + 2 * i;  /* a[i] + b[i] */
    }
    
    if (checksum1 == expected_sum && checksum2 == expected_sum) {
        printf("Test PASSED - Both paths computed correctly\n");
    } else {
        printf("Test FAILED - Computation error\n");
        return 1;
    }
#else
    printf("OpenMP target offloading not supported - skipping SIMT test\n");
#endif
    
    return 0;
}
