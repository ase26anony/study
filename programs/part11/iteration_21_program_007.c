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
    
    /* Target SIMD region with conditional offloading */
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
    int checksum = 0;
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
    
    /* Call with GPU offloading enabled */
    int checksum_gpu = simt_test(n, 1);
    printf("Checksum with GPU offloading: %d\n", checksum_gpu);
    
    /* Call with GPU offloading disabled (host execution) */
    int checksum_host = simt_test(n, 0);
    printf("Checksum with host execution: %d\n", checksum_host);
    
    /* Verify results match */
    if (checksum_gpu != checksum_host) {
        printf("ERROR: Checksums don't match!\n");
        return 1;
    }
    
    printf("Test passed successfully\n");
    return 0;
}

#else
int main() {
    printf("OpenMP not supported - test skipped\n");
    return 0;
}
#endif
