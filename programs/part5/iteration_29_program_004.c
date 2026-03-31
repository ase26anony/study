#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1000

#pragma omp declare target
int device_compute(int x, int y) {
    return (x * y) / (x + y + 1);
}
#pragma omp end declare target

int main(int argc, char *argv[]) {
    int *a = (int *)malloc(N * sizeof(int));
    int *b = (int *)malloc(N * sizeof(int));
    int *c = (int *)malloc(N * sizeof(int));
    
    // Initialize arrays with non-trivial patterns
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 97;
        b[i] = (i * 7) % 113;
        c[i] = 0;
    }
    
    int use_offloading = 0;
    
    // Runtime condition to decide execution path
    if (argc > 1 && strcmp(argv[1], "gpu") == 0) {
        use_offloading = 1;
    }
    
    int result = 0;
    volatile int loop_bound = N;  // Prevent constant propagation
    
    if (use_offloading) {
        // Complex nested OpenMP construct for SIMT transformation
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:N], b[0:N]) map(from: c[0:N]) \
            reduction(+:result) num_teams(4) thread_limit(128)
        for (int i = 0; i < loop_bound; i++) {
            // Non-trivial loop body with device function call
            int temp = device_compute(a[i], b[i]);
            c[i] = temp * (i % 16 + 1);
            result += c[i] / ((i % 8) + 1);
        }
        
        printf("GPU offloading mode\n");
    } else {
        // Host fallback with similar computation
        #pragma omp parallel for simd reduction(+:result)
        for (int i = 0; i < loop_bound; i++) {
            int temp = (a[i] * b[i]) / (a[i] + b[i] + 1);
            c[i] = temp * (i % 16 + 1);
            result += c[i] / ((i % 8) + 1);
        }
        
        printf("CPU fallback mode\n");
    }
    
    // Additional computation to prevent dead code elimination
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum ^= c[i];
    }
    
    printf("Result: %d, Checksum: %d\n", result, checksum);
    
    free(a);
    free(b);
    free(c);
    
    return 0;
}
