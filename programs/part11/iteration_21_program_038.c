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
    
    // Return checksum through global to avoid dead code elimination
    static int result[2];
    if (use_gpu) {
        result[0] = checksum;
    } else {
        result[1] = checksum;
    }
}
#endif

int main()
{
#ifdef _OPENMP_TARGET
    const int n = 128;
    
    // Test with GPU offloading enabled
    printf("Testing with GPU offloading (use_gpu=1)...\n");
    simt_test(n, 1);
    
    // Test with GPU offloading disabled
    printf("Testing without GPU offloading (use_gpu=0)...\n");
    simt_test(n, 0);
    
    printf("SIMT test completed.\n");
#else
    printf("OpenMP target offloading not supported in this configuration.\n");
#endif
    
    return 0;
}
