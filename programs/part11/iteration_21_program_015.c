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
    
    // Verify computation
    int expected = 0;
    for (int i = 0; i < n; i++) {
        expected += 3 * i;
    }
    
    if (checksum != expected) {
        printf("Error: checksum mismatch! Got %d, expected %d\n", checksum, expected);
    }
}
#endif

int main()
{
#ifdef _OPENMP_TARGET
    printf("Testing SIMT transformation with GPU offloading...\n");
    
    // Test with GPU offloading enabled
    printf("Calling with use_gpu=1:\n");
    simt_test(N, 1);
    
    // Test with GPU offloading disabled (host fallback)
    printf("Calling with use_gpu=0:\n");
    simt_test(N, 0);
    
    printf("Test completed.\n");
#else
    printf("OpenMP target offloading not supported in this configuration.\n");
    printf("Compile with -fopenmp and appropriate -foffload flag.\n");
#endif
    
    return 0;
}
