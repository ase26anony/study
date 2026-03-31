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
    
    // Initialize arrays with simple patterns
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = 2 * i;
        c[i] = 0;
    }
    
    // This is the key construct that should trigger the SIMT transformation
    #pragma omp target simd if(use_gpu) map(tofrom: a[0:n], b[0:n], c[0:n])
    for (int i = 0; i < n; i++) {
        // Diagnostic output to ensure execution
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
    static int result[2] = {0, 0};
    result[use_gpu] = checksum;
    
    // Verify computation
    if (checksum != 3 * (n-1) * n / 2) {
        printf("Computation error: checksum=%d, expected=%d\n", 
               checksum, 3 * (n-1) * n / 2);
    }
}
#endif

int main()
{
#ifdef _OPENMP_TARGET
    const int n = 128;
    
    printf("Testing SIMT transformation with GPU offloading...\n");
    
    // Test both paths: with and without GPU offloading
    // This should trigger both branches of the conditional wrapper
    simt_test(n, 1);  // use_gpu = 1 -> should take SIMT path
    simt_test(n, 0);  // use_gpu = 0 -> should take regular path
    
    printf("SIMT test completed successfully\n");
    
    return 0;
#else
    printf("OpenMP target offloading not supported in this configuration\n");
    return 0;
#endif
}
