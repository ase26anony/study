#ifdef _OPENMP
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _OPENMP_TARGET
__attribute__((noinline))
void simt_test(int n, int use_gpu) {
    int a[128], b[128], c[128];
    
    // Initialize arrays
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = 2 * i;
        c[i] = 0;
    }
    
    // Target SIMD construct with conditional offloading
    #pragma omp target simd if(use_gpu) map(tofrom: a[0:n], b[0:n], c[0:n])
    for (int i = 0; i < n; i++) {
        // Diagnostic output inside target region
        if (i == 0) {
            #pragma omp critical
            __builtin_printf("Offloaded loop executed with use_gpu=%d\n", use_gpu);
        }
        c[i] = a[i] + b[i];
    }
    
    // Compute checksum to prevent optimization
    int checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += c[i];
    }
    
    // Verify computation
    int expected = 0;
    for (int i = 0; i < n; i++) {
        expected += 3 * i;
    }
    
    if (checksum != expected) {
        __builtin_printf("Checksum mismatch: %d != %d\n", checksum, expected);
    }
}
#endif // _OPENMP_TARGET

int main() {
#ifdef _OPENMP_TARGET
    const int n = 128;
    
    // Test with GPU offloading enabled
    printf("Testing with GPU offloading (use_gpu=1):\n");
    simt_test(n, 1);
    
    // Test with GPU offloading disabled
    printf("\nTesting without GPU offloading (use_gpu=0):\n");
    simt_test(n, 0);
    
    printf("Test completed.\n");
#else
    printf("OpenMP target offloading not supported in this configuration.\n");
#endif
    
    return 0;
}
#else
int main() {
    printf("OpenMP not supported.\n");
    return 0;
}
#endif
