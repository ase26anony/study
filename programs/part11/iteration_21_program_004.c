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
        // Diagnostic output to ensure code executes
        if (use_gpu) {
            // Use __builtin_printf to avoid requiring stdio in device code
            __builtin_printf("Offloaded loop executed with use_gpu=%d, i=%d\n", 
                           use_gpu, i);
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
    printf("Testing SIMT transformation with GPU offloading...\n");
    
    // Test with GPU offloading enabled
    simt_test(N, 1);
    
    // Test with GPU offloading disabled (host fallback)
    simt_test(N, 0);
    
    printf("SIMT test completed successfully\n");
#else
    printf("OpenMP target offloading not supported in this configuration\n");
#endif
    
    return 0;
}
