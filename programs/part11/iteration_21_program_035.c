#ifdef _OPENMP
#include <omp.h>
#endif
#include <stdio.h>
#include <stdlib.h>

#define N 128

#ifdef _OPENMP_TARGET
__attribute__((noinline))
void simt_test(int n, int use_gpu)
{
    int a[N], b[N], c[N];
    
    // Initialize arrays
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = 2 * i;
        c[i] = 0;
    }
    
    // This pragma should trigger the SIMT transformation
    #pragma omp target simd if(use_gpu) map(tofrom: a[0:n], b[0:n], c[0:n])
    for (int i = 0; i < n; i++) {
        // Diagnostic output inside target region
        if (i == 0) {
            #pragma omp critical
            printf("Offloaded loop executed with use_gpu=%d\n", use_gpu);
        }
        c[i] = a[i] + b[i];
    }
    
    // Compute checksum to prevent optimization
    int checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += c[i];
    }
    
    // Use checksum to prevent dead code elimination
    printf("Checksum for use_gpu=%d: %d\n", use_gpu, checksum);
}
#endif

int main()
{
#ifdef _OPENMP_TARGET
    printf("Testing SIMT transformation with OpenMP offloading\n");
    
    // Test both paths: with and without GPU offloading
    simt_test(N, 1);  // Should trigger SIMT path
    simt_test(N, 0);  // Should use fallback path
    
    printf("Test completed\n");
#else
    printf("OpenMP target offloading not supported in this configuration\n");
#endif
    
    return 0;
}
