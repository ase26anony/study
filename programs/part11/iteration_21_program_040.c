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
    
    #ifdef _OPENMP_TARGET
    /* This construct should trigger SIMT transformation */
    #pragma omp target simd if(use_gpu) map(tofrom: a[0:n], b[0:n], c[0:n])
    for (int i = 0; i < n; i++) {
        /* Diagnostic output inside target region */
        if (use_gpu) {
            /* Use builtin to avoid external function calls in offloaded code */
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
    
    free(a);
    free(b);
    free(c);
    
    return checksum;
}
#endif

int main() {
    #ifdef _OPENMP
    const int n = 128;
    
    /* Test with GPU offloading enabled */
    printf("Testing with GPU offloading (use_gpu=1)...\n");
    int checksum1 = simt_test(n, 1);
    printf("Checksum 1: %d\n", checksum1);
    
    /* Test with GPU offloading disabled */
    printf("\nTesting without GPU offloading (use_gpu=0)...\n");
    int checksum2 = simt_test(n, 0);
    printf("Checksum 2: %d\n", checksum2);
    
    /* Verify results match */
    if (checksum1 == checksum2) {
        printf("\nResults match - test passed!\n");
    } else {
        printf("\nResults differ - test failed!\n");
        return 1;
    }
    #else
    printf("OpenMP not supported - test skipped\n");
    #endif
    
    return 0;
}
