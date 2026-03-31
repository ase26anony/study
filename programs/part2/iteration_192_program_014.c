#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 512
#define P 256

// Volatile variables to prevent optimization
volatile int use_gang = 1;
volatile int use_worker = 1;
volatile int use_vector = 1;
volatile int partition_type = 0;

// Struct with array members
struct DataContainer {
    int grid[N][M];
    float values[M][P];
    double coords[P][N];
};

// Function to compute checksum
long long compute_checksum(int *data, size_t size) {
    long long sum = 0;
    for (size_t i = 0; i < size; i++) {
        sum += data[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    // Multi-dimensional arrays
    int array_3d[N][M][P];
    int array_2d[N][M];
    int *dynamic_arr;
    
    // Initialize arrays
    memset(array_3d, 0, sizeof(array_3d));
    memset(array_2d, 0, sizeof(array_2d));
    
    // Allocate dynamic memory
    dynamic_arr = (int*)malloc(N * M * sizeof(int));
    memset(dynamic_arr, 0, N * M * sizeof(int));
    
    // Struct instance
    struct DataContainer container;
    memset(&container, 0, sizeof(container));
    
    long long total_checksum = 0;
    
    printf("Testing all OpenACC partition types...\n");
    
    // Case 0: gang redundant (default mapping)
    #pragma acc data copy(array_2d)
    {
        #pragma acc parallel loop gang
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                array_2d[i][j] += i + j;
            }
        }
    }
    total_checksum += compute_checksum(&array_2d[0][0], N * M);
    
    // Case 1: gang partitioned
    #pragma acc data copy(array_2d[0:N][gang])
    {
        #pragma acc parallel loop gang
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                array_2d[i][j] += i * j;
            }
        }
    }
    total_checksum += compute_checksum(&array_2d[0][0], N * M);
    
    // Case 2: worker partitioned
    #pragma acc data copy(array_2d[0:N][worker])
    {
        #pragma acc parallel loop gang worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                array_2d[i][j] += i - j;
            }
        }
    }
    total_checksum += compute_checksum(&array_2d[0][0], N * M);
    
    // Case 3: gang+worker partitioned
    #pragma acc data copy(array_3d[0:N][gang][worker])
    {
        #pragma acc parallel loop gang worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    array_3d[i][j][k] = i + j + k;
                }
            }
        }
    }
    total_checksum += compute_checksum(&array_3d[0][0][0], N * M * P);
    
    // Case 4: vector partitioned
    #pragma acc data copy(dynamic_arr[0:N*M][vector])
    {
        #pragma acc parallel loop vector
        for (int i = 0; i < N * M; i++) {
            dynamic_arr[i] += i * 2;
        }
    }
    total_checksum += compute_checksum(dynamic_arr, N * M);
    
    // Case 5: gang+vector partitioned
    #pragma acc data copy(array_2d[gang][vector])
    {
        #pragma acc parallel loop gang vector
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                array_2d[i][j] += (i << 2) | j;
            }
        }
    }
    total_checksum += compute_checksum(&array_2d[0][0], N * M);
    
    // Case 6: worker+vector partitioned
    #pragma acc data copy(container.grid[worker][vector])
    {
        #pragma acc parallel loop worker vector
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                container.grid[i][j] = i * j * 3;
            }
        }
    }
    total_checksum += compute_checksum(&container.grid[0][0], N * M);
    
    // Case 7: fully partitioned (gang+worker+vector)
    #pragma acc data copy(array_3d[gang][worker][vector])
    {
        #pragma acc parallel loop gang worker vector
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < M; j++) {
                #pragma acc loop vector
                for (int k = 0; k < P; k++) {
                    array_3d[i][j][k] += i * j * k;
                }
            }
        }
    }
    total_checksum += compute_checksum(&array_3d[0][0][0], N * M * P);
    
    // Conditional partition selection using volatile variables
    // This creates a single code region where compiler must handle multiple possibilities
    for (int iter = 0; iter < 8; iter++) {
        partition_type = iter;
        
        if (use_gang && use_worker && use_vector) {
            // Fully partitioned
            #pragma acc data copy(array_2d[gang][worker][vector])
            {
                #pragma acc parallel loop gang worker vector
                for (int i = 0; i < N; i++) {
                    #pragma acc loop worker vector
                    for (int j = 0; j < M; j++) {
                        array_2d[i][j] += partition_type;
                    }
                }
            }
        } else if (use_gang && use_worker) {
            // Gang+worker partitioned
            #pragma acc data copy(container.values[gang][worker])
            {
                #pragma acc parallel loop gang worker
                for (int i = 0; i < M; i++) {
                    #pragma acc loop worker
                    for (int j = 0; j < P; j++) {
                        container.values[i][j] = i * j * partition_type;
                    }
                }
            }
        } else if (use_gang && use_vector) {
            // Gang+vector partitioned
            #pragma acc data copy(dynamic_arr[0:N*M][gang][vector])
            {
                #pragma acc parallel loop gang vector
                for (int i = 0; i < N * M; i++) {
                    dynamic_arr[i] += partition_type * i;
                }
            }
        } else if (use_worker && use_vector) {
            // Worker+vector partitioned
            #pragma acc data copy(container.coords[worker][vector])
            {
                #pragma acc parallel loop worker vector
                for (int i = 0; i < P; i++) {
                    #pragma acc loop vector
                    for (int j = 0; j < N; j++) {
                        container.coords[i][j] = i + j + partition_type;
                    }
                }
            }
        } else if (use_gang) {
            // Gang partitioned
            #pragma acc data copy(array_2d[gang])
            {
                #pragma acc parallel loop gang
                for (int i = 0; i < N; i++) {
                    for (int j = 0; j < M; j++) {
                        array_2d[i][j] += partition_type * 2;
                    }
                }
            }
        } else if (use_worker) {
            // Worker partitioned
            #pragma acc data copy(array_2d[worker])
            {
                #pragma acc parallel loop worker
                for (int i = 0; i < N; i++) {
                    for (int j = 0; j < M; j++) {
                        array_2d[i][j] += partition_type * 3;
                    }
                }
            }
        } else if (use_vector) {
            // Vector partitioned
            #pragma acc data copy(dynamic_arr[0:N*M][vector])
            {
                #pragma acc parallel loop vector
                for (int i = 0; i < N * M; i++) {
                    dynamic_arr[i] += partition_type * 4;
                }
            }
        }
    }
    
    // Nested constructs with enter/exit data
    #pragma acc enter data copyin(container.grid[gang][worker])
    #pragma acc parallel loop present(container.grid[gang][worker])
    for (int i = 0; i < N; i++) {
        #pragma acc loop worker
        for (int j = 0; j < M; j++) {
            container.grid[i][j] *= 2;
        }
    }
    #pragma acc exit data copyout(container.grid[gang][worker])
    
    total_checksum += compute_checksum(&container.grid[0][0], N * M);
    
    // Test struct member mappings with different partitions
    #pragma acc data copy(container.grid[gang], container.values[vector], container.coords[worker])
    {
        #pragma acc parallel loop gang
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                container.grid[i][j] += 1;
            }
        }
        
        #pragma acc parallel loop vector
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < P; j++) {
                container.values[i][j] += 2.0f;
            }
        }
        
        #pragma acc parallel loop worker
        for (int i = 0; i < P; i++) {
            for (int j = 0; j < N; j++) {
                container.coords[i][j] += 3.0;
            }
        }
    }
    
    // Final checksum computation
    total_checksum += compute_checksum(&container.grid[0][0], N * M);
    total_checksum += compute_checksum(dynamic_arr, N * M);
    
    printf("Total checksum: %lld\n", total_checksum);
    
    // Cleanup
    free(dynamic_arr);
    
    return 0;
}
