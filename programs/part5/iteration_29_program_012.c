#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1000

#pragma omp declare target
int device_compute(int a, int b) {
    return (a * b) / (a + b + 1);
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
    
    int final_sum = 0;
    
    // Use argc as runtime condition to choose execution path
    // This encourages generation of conditional labels
    int use_offload = (argc > 1) ? atoi(argv[1]) : 1;
    
    if (use_offload) {
        // Offloaded computation with complex OpenMP nesting
        #pragma omp target teams distribute parallel for simd \
            map(to: array_a[0:N], array_b[0:N]) \
            map(from: result[0:N]) \
            reduction(+:final_sum) \
            num_teams(4) thread_limit(128)
        for (int i = 0; i < N; i++) {
            // Non-trivial loop body with device function call
            int temp = device_compute(array_a[i], array_b[i]);
            result[i] = temp + i;
            final_sum += result[i];
            
            // Additional computation to create substantial GIMPLE sequence
            for (int j = 0; j < 3; j++) {
                result[i] += (array_a[i] + array_b[i]) % (j + 2);
            }
        }
    } else {
        // Host fallback version - still with OpenMP but no offloading
        #pragma omp parallel for simd reduction(+:final_sum)
        for (int i = 0; i < N; i++) {
            int temp = (array_a[i] * array_b[i]) / (array_a[i] + array_b[i] + 1);
            result[i] = temp + i;
            final_sum += result[i];
            
            for (int j = 0; j < 3; j++) {
                result[i] += (array_a[i] + array_b[i]) % (j + 2);
            }
        }
    }
    
    // Verify computation with checksum
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum = (checksum + result[i]) % 1000000;
    }
    
    printf("Final sum: %d\n", final_sum);
    printf("Checksum: %d\n", checksum);
    printf("Execution mode: %s\n", use_offload ? "offload" : "host");
    
    // Cleanup
    free(array_a);
    free(array_b);
    free(result);
    
    return 0;
}
