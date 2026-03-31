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
        b[i] = (i * 3) % 97;
    }
    
    // Use argc as runtime condition to decide offloading path
    int use_offload = (argc > 1) ? atoi(argv[1]) : 1;
    
    if (use_offload) {
        // Offloaded path - triggers SIMT transformation
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:N], b[0:N]) map(from: result) reduction(+:result)
        for (int i = 0; i < N; i++) {
            // Non-trivial loop body with device function call
            int temp = device_compute(a[i]);
            result += temp * b[i] / (i % 50 + 1);
        }
        
        printf("Offloaded computation result: %d\n", result);
    } else {
        // Host fallback path
        for (int i = 0; i < N; i++) {
            int temp = a[i] * 2 + 1;  // Manual device_compute equivalent
            result += temp * b[i] / (i % 50 + 1);
        }
        
        printf("Host computation result: %d\n", result);
    }
    
    // Additional nested OpenMP construct to increase complexity
    int partial_sums[4] = {0, 0, 0, 0};
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:N]) map(tofrom: partial_sums[0:4]) \
        num_teams(2) num_threads(4)
    for (int i = 0; i < N; i++) {
        int team_id = omp_get_team_num();
        if (team_id < 4) {
            #pragma omp atomic
            partial_sums[team_id] += a[i] * (i + 1);
        }
    }
    
    int checksum = 0;
    for (int i = 0; i < 4; i++) {
        checksum += partial_sums[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(a);
    free(b);
    
    return 0;
}
