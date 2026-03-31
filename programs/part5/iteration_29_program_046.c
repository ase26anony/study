#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1000

#pragma omp declare target
int device_compute(int x, int y) {
    return x * y + (x >> 2) - (y << 1);
}
#pragma omp end declare target

int main(int argc, char *argv[]) {
    int *array_a = (int *)malloc(N * sizeof(int));
    int *array_b = (int *)malloc(N * sizeof(int));
    int result = 0;
    
    // Initialize arrays with non-trivial patterns
    for (int i = 0; i < N; i++) {
        array_a[i] = i * 2 + (i % 7);
        array_b[i] = i * 3 - (i % 5);
    }
    
    // Use argc to create runtime condition for offloading
    // This encourages conditional label generation
    int use_offload = (argc > 1) ? atoi(argv[1]) : 1;
    
    if (use_offload) {
        // Offloaded computation with complex OpenMP nesting
        #pragma omp target teams distribute parallel for simd \
            map(to: array_a[0:N], array_b[0:N]) \
            map(tofrom: result) \
            reduction(+:result) \
            num_teams(4) thread_limit(128)
        for (int i = 0; i < N; i++) {
            // Non-trivial loop body with device function call
            // and data-dependent computation
            int temp = device_compute(array_a[i], array_b[i]);
            result += temp + (i % 3) * (array_a[i] > array_b[i] ? 1 : -1);
            
            // Additional computation to create substantial GIMPLE
            array_a[i] = (array_a[i] + temp) % 256;
            array_b[i] = (array_b[i] - temp) & 0xFF;
        }
        
        // Additional target region with different SIMD configuration
        int partial_results[4] = {0};
        #pragma omp target teams distribute simd \
            map(to: array_a[0:N]) \
            map(from: partial_results[0:4]) \
            num_teams(4)
        for (int i = 0; i < N; i++) {
            int team_id = omp_get_team_num();
            if (team_id < 4) {
                partial_results[team_id] += array_a[i] * (i % 8);
            }
        }
        
        // Combine partial results
        for (int i = 0; i < 4; i++) {
            result += partial_results[i];
        }
    } else {
        // Host fallback version - still with OpenMP but no offloading
        #pragma omp parallel for simd reduction(+:result)
        for (int i = 0; i < N; i++) {
            int temp = array_a[i] * array_b[i] + (array_a[i] >> 2) - (array_b[i] << 1);
            result += temp + (i % 3) * (array_a[i] > array_b[i] ? 1 : -1);
            array_a[i] = (array_a[i] + temp) % 256;
            array_b[i] = (array_b[i] - temp) & 0xFF;
        }
    }
    
    // Verify computation with simple checksum
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum = (checksum + array_a[i] + array_b[i]) & 0xFFFF;
    }
    
    printf("Result: %d, Checksum: %d\n", result, checksum);
    printf("Offload used: %s\n", use_offload ? "YES" : "NO");
    
    free(array_a);
    free(array_b);
    
    return 0;
}
