#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1000

#pragma omp declare target
int device_compute(int x, int y) {
    return x * y + (x ^ y);
}
#pragma omp end declare target

int main(int argc, char *argv[]) {
    int *a = (int *)malloc(N * sizeof(int));
    int *b = (int *)malloc(N * sizeof(int));
    int result = 0;
    
    // Initialize arrays with non-trivial patterns
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 7;
        b[i] = (i * 5) % 11;
    }
    
    // Runtime condition for offloading decision
    // Use argc to create a non-constant condition
    int use_offload = (argc > 1) ? atoi(argv[1]) : 1;
    
    // Volatile variable to prevent loop optimization
    volatile int loop_bound = N;
    
    if (use_offload) {
        // Complex offloading region with nested constructs
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:N], b[0:N]) map(from: result) \
            reduction(+:result) num_teams(4) thread_limit(128)
        for (int i = 0; i < loop_bound; i++) {
            // Non-trivial loop body with device function call
            int temp = device_compute(a[i], b[i]);
            result += temp + (i % 3);
            
            // Additional computation to create complex GIMPLE
            if (temp > 100) {
                result -= (temp % 7);
            }
        }
        
        printf("Offloaded computation result: %d\n", result);
    } else {
        // Host fallback version
        #pragma omp parallel for simd reduction(+:result)
        for (int i = 0; i < loop_bound; i++) {
            int temp = a[i] * b[i] + (a[i] ^ b[i]);
            result += temp + (i % 3);
            
            if (temp > 100) {
                result -= (temp % 7);
            }
        }
        
        printf("Host computation result: %d\n", result);
    }
    
    // Additional test with nested parallel regions
    int partial_results[4] = {0};
    
    #pragma omp target teams distribute map(to: a[0:N]) map(from: partial_results[0:4])
    for (int team = 0; team < 4; team++) {
        int team_result = 0;
        
        #pragma omp parallel for simd reduction(+:team_result)
        for (int i = team * (N/4); i < (team + 1) * (N/4); i++) {
            team_result += a[i] * (i % 5);
        }
        
        partial_results[team] = team_result;
    }
    
    int final_sum = 0;
    for (int i = 0; i < 4; i++) {
        final_sum += partial_results[i];
    }
    
    printf("Final checksum: %d\n", final_sum);
    
    free(a);
    free(b);
    
    return 0;
}
