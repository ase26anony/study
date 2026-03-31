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
    int *array_a = (int *)malloc(N * sizeof(int));
    int *array_b = (int *)malloc(N * sizeof(int));
    int *result = (int *)malloc(N * sizeof(int));
    
    // Initialize arrays with non-trivial patterns
    for (int i = 0; i < N; i++) {
        array_a[i] = (i * 3) % 97;
        array_b[i] = (i * 7) % 113;
        result[i] = 0;
    }
    
    int use_offloading = 0;
    
    // Runtime condition for offloading decision
    if (argc > 1 && strcmp(argv[1], "gpu") == 0) {
        use_offloading = 1;
    }
    
    int final_sum = 0;
    
    if (use_offloading) {
        // Complex nested OpenMP construct for SIMT transformation
        #pragma omp target teams distribute parallel for simd \
            map(to: array_a[0:N], array_b[0:N]) \
            map(from: result[0:N]) \
            reduction(+:final_sum) \
            num_teams(4) thread_limit(128)
        for (int i = 0; i < N; i++) {
            // Non-constant, data-dependent computation
            int temp = device_compute(array_a[i], array_b[i]);
            
            // Complex loop body with branching
            if (temp > 50) {
                result[i] = temp * 2 - array_a[i];
            } else {
                result[i] = temp + array_b[i] / 3;
            }
            
            // Reduction operation
            final_sum += result[i];
        }
    } else {
        // Host fallback version
        #pragma omp parallel for simd reduction(+:final_sum)
        for (int i = 0; i < N; i++) {
            int temp = (array_a[i] * array_b[i]) / (array_a[i] + array_b[i] + 1);
            
            if (temp > 50) {
                result[i] = temp * 2 - array_a[i];
            } else {
                result[i] = temp + array_b[i] / 3;
            }
            
            final_sum += result[i];
        }
    }
    
    // Compute checksum to prevent optimization removal
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum ^= result[i];
    }
    
    printf("Final sum: %d\n", final_sum);
    printf("Checksum: %d\n", checksum);
    printf("Offloading used: %s\n", use_offloading ? "YES" : "NO");
    
    free(array_a);
    free(array_b);
    free(result);
    
    return 0;
}
