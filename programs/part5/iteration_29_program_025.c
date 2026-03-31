#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1000

#pragma omp declare target
int device_func(int x, int y) {
    return x * y + (x - y);
}
#pragma omp end declare target

int main(int argc, char *argv[]) {
    int *a = (int *)malloc(N * sizeof(int));
    int *b = (int *)malloc(N * sizeof(int));
    int result = 0;
    
    // Initialize arrays with non-trivial patterns
    for (int i = 0; i < N; i++) {
        a[i] = i * 2 + 1;
        b[i] = i % 7 + 3;
    }
    
    // Use argc to create runtime condition for offloading
    // This encourages conditional label generation
    int use_offload = (argc > 1) ? atoi(argv[1]) : 1;
    
    if (use_offload) {
        // Offloaded computation with complex nested OpenMP constructs
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:N], b[0:N]) map(from: result) reduction(+:result)
        for (int i = 0; i < N; i++) {
            // Non-trivial loop body with device function call
            // and data-dependent computation
            int temp = device_func(a[i], b[i]);
            result += temp * (i % 5 + 1);
            
            // Additional computation to create substantial GIMPLE sequence
            if (i % 3 == 0) {
                result -= b[i] / 2;
            } else if (i % 7 == 0) {
                result += a[i] % 11;
            }
        }
    } else {
        // Host fallback version - still uses SIMD but no offloading
        #pragma omp simd reduction(+:result)
        for (int i = 0; i < N; i++) {
            int temp = a[i] * b[i] + (a[i] - b[i]);
            result += temp * (i % 5 + 1);
            
            if (i % 3 == 0) {
                result -= b[i] / 2;
            } else if (i % 7 == 0) {
                result += a[i] % 11;
            }
        }
    }
    
    // Additional computation to prevent dead code elimination
    volatile int checksum = 0;
    for (int i = 0; i < 10; i++) {
        checksum += result % (i + 2);
    }
    
    printf("Result: %d\n", result);
    printf("Checksum: %d\n", checksum);
    
    free(a);
    free(b);
    
    return 0;
}
