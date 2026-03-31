#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1000

#pragma omp declare target
int device_compute(int x) {
    return x * 2 + 1;
}
#pragma omp end declare target

int main(int argc, char *argv[]) {
    int *a = (int *)malloc(N * sizeof(int));
    int *b = (int *)malloc(N * sizeof(int));
    int result = 0;
    
    // Initialize arrays with non-trivial patterns
    for (int i = 0; i < N; i++) {
        a[i] = i % 100;
        b[i] = (i * 3) % 100;
    }
    
    // Runtime condition for offloading (based on argc)
    int use_offload = (argc > 1) ? atoi(argv[1]) : 1;
    
    if (use_offload) {
        // Offloaded computation with complex OpenMP nesting
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:N], b[0:N]) map(tofrom: result) \
            reduction(+:result) num_teams(4) thread_limit(128)
        for (int i = 0; i < N; i++) {
            // Non-constant trip count via data-dependent access
            int idx = (i + a[i % 100]) % N;
            int temp = device_compute(a[idx] + b[idx]);
            result += temp;
            
            // Additional computation to create substantial GIMPLE
            if (idx % 2 == 0) {
                result -= a[idx] / 3;
            } else {
                result += b[idx] / 2;
            }
        }
        
        printf("Offloaded computation result: %d\n", result);
    } else {
        // Host fallback version
        #pragma omp parallel for reduction(+:result)
        for (int i = 0; i < N; i++) {
            int idx = (i + a[i % 100]) % N;
            result += a[idx] + b[idx];
        }
        
        printf("Host computation result: %d\n", result);
    }
    
    // Additional nested OpenMP construct to stress SIMT transformation
    int partial_results[4] = {0};
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:N]) map(from: partial_results[0:4]) \
        num_teams(2) num_threads(64)
    for (int i = 0; i < N; i++) {
        int team = omp_get_team_num();
        if (team < 4) {
            #pragma omp atomic
            partial_results[team] += a[i];
        }
    }
    
    int checksum = result;
    for (int i = 0; i < 4; i++) {
        checksum += partial_results[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    
    free(a);
    free(b);
    
    return 0;
}
