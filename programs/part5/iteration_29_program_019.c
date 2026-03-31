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
        a[i] = i * 2 + (i % 3);
        b[i] = i * 3 - (i % 5);
    }
    
    // Runtime condition for offloading (based on argc)
    int use_offload = (argc > 1) ? atoi(argv[1]) : 1;
    
    if (use_offload) {
        // Offloaded computation with complex OpenMP nesting
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:N], b[0:N]) map(from: result) reduction(+:result)
        for (int i = 0; i < N; i++) {
            // Non-trivial loop body with device function call
            int temp = a[i] * b[i];
            temp = device_compute(temp);
            result += temp - (i % 7);
        }
        
        printf("Offloaded computation result: %d\n", result);
    } else {
        // Host fallback with similar computation
        #pragma omp parallel for simd reduction(+:result)
        for (int i = 0; i < N; i++) {
            int temp = a[i] * b[i];
            temp = temp * 2 + 1;  // Manual device_compute equivalent
            result += temp - (i % 7);
        }
        
        printf("Host computation result: %d\n", result);
    }
    
    // Additional nested construct to increase complexity
    int partial_results[4] = {0};
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:N]) map(from: partial_results[0:4]) \
        num_teams(2) num_threads(2)
    for (int i = 0; i < N; i++) {
        int team_id = omp_get_team_num();
        if (team_id < 4) {
            #pragma omp atomic
            partial_results[team_id] += a[i] * (i % 11);
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
