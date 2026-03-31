#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 100
#define M 100
#define P 100

// Volatile variables to prevent optimization
volatile int use_gang = 1;
volatile int use_worker = 1;
volatile int use_vector = 1;
volatile int use_combined = 1;

struct DataContainer {
    int grid[N][M];
    int linear[N*M];
    int multi[N][M][P];
};

// Helper to initialize arrays
void init_arrays(struct DataContainer *dc) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            dc->grid[i][j] = i * M + j;
            dc->linear[i*M + j] = i * M + j;
            for (int k = 0; k < P; k++) {
                dc->multi[i][j][k] = i * M * P + j * P + k;
            }
        }
    }
}

// Compute checksum
int compute_checksum(struct DataContainer *dc) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            sum += dc->grid[i][j];
            sum += dc->linear[i*M + j];
            for (int k = 0; k < P; k++) {
                sum += dc->multi[i][j][k];
            }
        }
    }
    return sum;
}

int main() {
    struct DataContainer dc;
    int *dynamic_arr = (int*)malloc(N * M * sizeof(int));
    int checksum = 0;
    
    // Initialize data
    init_arrays(&dc);
    for (int i = 0; i < N*M; i++) {
        dynamic_arr[i] = i;
    }
    
    printf("Starting OpenACC partition coverage test...\n");
    
    // Case 0: gang redundant (default mapping)
    if (use_gang) {
        #pragma acc data copy(dc.grid)
        {
            #pragma acc parallel loop gang
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    dc.grid[i][j] += 1;
                }
            }
        }
    }
    
    // Case 1: gang partitioned
    if (use_gang) {
        #pragma acc data copy(dc.grid[0:N][gang])
        {
            #pragma acc parallel loop gang
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    dc.grid[i][j] += 2;
                }
            }
        }
    }
    
    // Case 2: worker partitioned
    if (use_worker) {
        #pragma acc data copy(dc.linear[0:N*M][worker])
        {
            #pragma acc parallel loop worker
            for (int i = 0; i < N*M; i++) {
                dc.linear[i] += 3;
            }
        }
    }
    
    // Case 3: gang+worker partitioned
    if (use_combined) {
        #pragma acc data copy(dc.multi[0:N][gang][worker])
        {
            #pragma acc parallel loop gang worker
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        dc.multi[i][j][k] += 4;
                    }
                }
            }
        }
    }
    
    // Case 4: vector partitioned
    if (use_vector) {
        #pragma acc data copy(dynamic_arr[0:N*M][vector])
        {
            #pragma acc parallel loop vector
            for (int i = 0; i < N*M; i++) {
                dynamic_arr[i] += 5;
            }
        }
    }
    
    // Case 5: gang+vector partitioned
    if (use_combined) {
        #pragma acc data copy(dc.grid[0:N][gang][vector])
        {
            #pragma acc parallel loop gang vector
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    dc.grid[i][j] += 6;
                }
            }
        }
    }
    
    // Case 6: worker+vector partitioned
    if (use_combined) {
        #pragma acc data copy(dc.linear[0:N*M][worker][vector])
        {
            #pragma acc parallel loop worker vector
            for (int i = 0; i < N*M; i++) {
                dc.linear[i] += 7;
            }
        }
    }
    
    // Case 7: fully partitioned (gang+worker+vector)
    if (use_combined) {
        #pragma acc data copy(dc.multi[0:N][gang][worker][vector])
        {
            #pragma acc parallel loop gang worker vector
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        dc.multi[i][j][k] += 8;
                    }
                }
            }
        }
    }
    
    // Test with enter/exit data for structured data movement
    int *host_arr = (int*)malloc(N * sizeof(int));
    for (int i = 0; i < N; i++) host_arr[i] = i;
    
    #pragma acc enter data copyin(host_arr[0:N][gang])
    #pragma acc parallel loop gang present(host_arr[0:N][gang])
    for (int i = 0; i < N; i++) {
        host_arr[i] *= 2;
    }
    #pragma acc exit data copyout(host_arr[0:N][gang])
    
    // Test struct with array members
    struct {
        int x[N][M];
        int y[N][M];
    } s;
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            s.x[i][j] = i + j;
            s.y[i][j] = i - j;
        }
    }
    
    #pragma acc data copy(s.x[gang], s.y[vector])
    {
        #pragma acc parallel loop gang
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                s.x[i][j] += 1;
            }
        }
        
        #pragma acc parallel loop vector
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                s.y[i][j] += 1;
            }
        }
    }
    
    // Conditional partition selection using volatile
    volatile int ptype = 0;
    int test_arr[N][M];
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            test_arr[i][j] = i * j;
        }
    }
    
    // This loop with volatile condition forces compiler to consider multiple paths
    for (ptype = 0; ptype < 4; ptype++) {
        if (ptype == 0) {
            #pragma acc data copy(test_arr[gang])
            {
                #pragma acc parallel loop gang
                for (int i = 0; i < N; i++) {
                    for (int j = 0; j < M; j++) {
                        test_arr[i][j] += ptype;
                    }
                }
            }
        } else if (ptype == 1) {
            #pragma acc data copy(test_arr[worker])
            {
                #pragma acc parallel loop worker
                for (int i = 0; i < N; i++) {
                    for (int j = 0; j < M; j++) {
                        test_arr[i][j] += ptype;
                    }
                }
            }
        } else if (ptype == 2) {
            #pragma acc data copy(test_arr[vector])
            {
                #pragma acc parallel loop vector
                for (int i = 0; i < N; i++) {
                    for (int j = 0; j < M; j++) {
                        test_arr[i][j] += ptype;
                    }
                }
            }
        } else if (ptype == 3) {
            #pragma acc data copy(test_arr[gang][worker])
            {
                #pragma acc parallel loop gang worker
                for (int i = 0; i < N; i++) {
                    for (int j = 0; j < M; j++) {
                        test_arr[i][j] += ptype;
                    }
                }
            }
        }
    }
    
    // Compute final checksum
    checksum += compute_checksum(&dc);
    
    for (int i = 0; i < N*M; i++) {
        checksum += dynamic_arr[i];
        checksum += host_arr[i % N];
    }
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            checksum += s.x[i][j] + s.y[i][j];
            checksum += test_arr[i][j];
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    
    // Cleanup
    free(dynamic_arr);
    free(host_arr);
    
    return 0;
}
