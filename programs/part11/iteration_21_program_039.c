#ifdef _OPENMP
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

/* Test function marked noinline to prevent optimization */
__attribute__((noinline))
int simt_test(int n, int use_gpu) {
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    int *c = (int*)malloc(n * sizeof(int));
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = 2 * i;
        c[i] = 0;
    }
    
    int checksum = 0;
    
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
    
    free(a);
    free(b);
    free(c);
    
    return checksum;
}

int main() {
    const int n = 128;
    int checksum1, checksum2;
    
    printf("Testing SIMT transformation with GPU offloading...\n");
    
    /* First call: force GPU offloading path (use_gpu=1) */
    checksum1 = simt_test(n, 1);
    printf("Checksum with use_gpu=1: %d\n", checksum1);
    
    /* Second call: force host execution path (use_gpu=0) */
    checksum2 = simt_test(n, 0);
    printf("Checksum with use_gpu=0: %d\n", checksum2);
    
    /* Verify results match */
    if (checksum1 == checksum2) {
        printf("Results consistent - test passed\n");
    } else {
        printf("Results inconsistent - test failed\n");
    }
    
    return 0;
}

#else
/* Fallback for non-OpenMP compilers */
int main() {
    printf("OpenMP not supported - test skipped\n");
    return 0;
}
#endif
