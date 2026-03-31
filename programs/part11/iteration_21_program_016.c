#ifdef _OPENMP
#include <omp.h>
#endif
#include <stdio.h>
#include <stdlib.h>

#ifdef _OPENMP_TARGET
__attribute__((noinline))
void simt_test(int n, int use_gpu)
{
    int a[128], b[128], c[128];
    
    // Initialize arrays
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = 2 * i;
        c[i] = 0;
    }
    
    // This is the key construct to trigger SIMT transformation
    #pragma omp target simd if(use_gpu) map(tofrom: a[0:n], b[0:n], c[0:n])
    for (int i = 0; i < n; i++) {
        // Diagnostic output to ensure code executes
        if (use_gpu) {
            #pragma omp critical
            printf("Offloaded loop executed with use_gpu=%d, i=%d\n", use_gpu, i);
        }
        c[i] = a[i] + b[i];
    }
    
    // Compute checksum to prevent optimization
    int checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += c[i];
    }
    
    printf("Checksum for use_gpu=%d: %d\n", use_gpu, checksum);
}
#endif

int main()
{
#ifdef _OPENMP_TARGET
    const int n = 128;
    
    // Test both paths: GPU offloading and host fallback
    printf("Testing SIMT transformation with GPU offloading...\n");
    simt_test(n, 1);  // Should trigger SIMT path
    
    printf("\nTesting SIMT transformation without GPU offloading...\n");
    simt_test(n, 0);  // Should take host fallback path
    
    return 0;
#else
    printf("OpenMP target offloading not supported in this configuration.\n");
    return 0;
#endif
}
