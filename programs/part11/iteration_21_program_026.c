#ifdef _OPENMP
#include <omp.h>
#endif
#include <stdio.h>
#include <stdlib.h>

/* Test function marked noinline to prevent optimization */
__attribute__((noinline))
#ifdef _OPENMP_TARGET
void simt_test(int n, int use_gpu, int *checksum)
#else
void simt_test(int n, int use_gpu, int *checksum)
#endif
{
    int a[128], b[128], c[128];
    
    /* Initialize arrays with simple patterns */
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = 2 * i;
        c[i] = 0;
    }
    
#ifdef _OPENMP_TARGET
    /* This is the key construct to trigger SIMT transformation */
    #pragma omp target simd if(use_gpu) map(tofrom: a[0:n], b[0:n], c[0:n])
    for (int i = 0; i < n; i++) {
        /* Diagnostic output inside target region */
        if (i == 0) {
            /* Use builtin printf to avoid external dependencies in offload region */
            __builtin_printf("Offloaded loop executed with use_gpu=%d\n", use_gpu);
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
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += c[i];
    }
    *checksum = sum;
}

int main(void)
{
    const int n = 128;
    int checksum1 = 0, checksum2 = 0;
    
#ifdef _OPENMP_TARGET
    printf("Testing with OpenMP target offloading support\n");
    
    /* First call with use_gpu=1 to potentially trigger SIMT path */
    simt_test(n, 1, &checksum1);
    printf("Checksum with use_gpu=1: %d\n", checksum1);
    
    /* Second call with use_gpu=0 to exercise the false path */
    simt_test(n, 0, &checksum2);
    printf("Checksum with use_gpu=0: %d\n", checksum2);
    
    /* Verify results match expected value */
    int expected = 0;
    for (int i = 0; i < n; i++) {
        expected += i + 2*i;  /* a[i] + b[i] */
    }
    
    if (checksum1 == expected && checksum2 == expected) {
        printf("All checksums correct!\n");
        return 0;
    } else {
        printf("Checksum mismatch!\n");
        return 1;
    }
#else
    printf("OpenMP target offloading not supported in this configuration\n");
    printf("Compile with -fopenmp and appropriate -foffload flag\n");
    return 0;
#endif
}
