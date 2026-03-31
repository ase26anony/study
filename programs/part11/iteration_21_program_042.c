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
        // Diagnostic output to ensure execution
        if (use_gpu) {
            #pragma omp critical
            printf("Offloaded loop executed with use_gpu=%d at i=%d\n", use_gpu, i);
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
    printf("Testing SIMT transformation with GPU offloading...\n");
    
    // Test both paths: with and without GPU offloading
    simt_test(N, 1);  // Should trigger SIMT path
    simt_test(N, 0);  // Should use host fallback
    
    printf("Test completed.\n");
#else
    printf("OpenMP target offloading not supported in this configuration.\n");
    printf("Compile with -fopenmp and appropriate offload target (e.g., -foffload=nvptx-none)\n");
#endif
    
    return 0;
}
