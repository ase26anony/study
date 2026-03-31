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
volatile int use_combined = 1;

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
    float float_array[M][P];
    double double_array[P][N];
    
    // Dynamic arrays
    int *dyn_arr = (int*)malloc(N * M * sizeof(int));
    float *dyn_float = (float*)malloc(M * P * sizeof(float));
    
    // Struct instance
    struct DataContainer container;
    
    // Initialize arrays
    #pragma acc parallel loop collapse(3) gang worker vector
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                array_3d[i][j][k] = i + j + k;
            }
        }
    }
    
    memset(array_2d, 0, sizeof(array_2d));
    memset(float_array, 0, sizeof(float_array));
    memset(double_array, 0, sizeof(double_array));
    memset(dyn_arr, 0, N * M * sizeof(int));
    memset(dyn_float, 0, M * P * sizeof(float));
    memset(&container, 0, sizeof(container));
    
    long long total_checksum = 0;
    
    // Case 0: gang redundant (default mapping)
    if (use_gang) {
        #pragma acc data copy(array_2d)
        {
            #pragma acc parallel loop gang
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    array_2d[i][j] += 1;
                }
            }
        }
        total_checksum += compute_checksum(&array_2d[0][0], N * M);
    }
    
    // Case 1: gang partitioned
    if (use_gang) {
        #pragma acc data copy(array_2d[0:N][gang])
        {
            #pragma acc parallel loop gang
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    array_2d[i][j] += 2;
                }
            }
        }
        total_checksum += compute_checksum(&array_2d[0][0], N * M);
    }
    
    // Case 2: worker partitioned
    if (use_worker) {
        #pragma acc data copy(array_2d[0:N][worker])
        {
            #pragma acc parallel loop worker
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    array_2d[i][j] += 3;
                }
            }
        }
        total_checksum += compute_checksum(&array_2d[0][0], N * M);
    }
    
    // Case 3: gang+worker partitioned
    if (use_combined) {
        #pragma acc data copy(array_3d[0:N][0:M][gang+worker])
        {
            #pragma acc parallel loop gang worker collapse(2)
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        array_3d[i][j][k] += 4;
                    }
                }
            }
        }
        total_checksum += compute_checksum(&array_3d[0][0][0], N * M * P);
    }
    
    // Case 4: vector partitioned
    if (use_vector) {
        #pragma acc data copy(dyn_arr[0:N*M][vector])
        {
            #pragma acc parallel loop vector
            for (int i = 0; i < N * M; i++) {
                dyn_arr[i] += 5;
            }
        }
        total_checksum += compute_checksum(dyn_arr, N * M);
    }
    
    // Case 5: gang+vector partitioned
    if (use_combined) {
        #pragma acc data copy(float_array[0:M][gang+vector])
        {
            #pragma acc parallel loop gang vector
            for (int i = 0; i < M; i++) {
                for (int j = 0; j < P; j++) {
                    float_array[i][j] += 6.0f;
                }
            }
        }
        // Convert float to int for checksum
        int temp_sum = 0;
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < P; j++) {
                temp_sum += (int)float_array[i][j];
            }
        }
        total_checksum += temp_sum;
    }
    
    // Case 6: worker+vector partitioned
    if (use_combined) {
        #pragma acc data copy(container.grid[0:N][worker+vector])
        {
            #pragma acc parallel loop worker vector collapse(2)
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    container.grid[i][j] += 7;
                }
            }
        }
        total_checksum += compute_checksum(&container.grid[0][0], N * M);
    }
    
    // Case 7: fully partitioned (gang+worker+vector)
    if (use_combined) {
        #pragma acc data copy(array_3d[0:N][gang][worker][vector])
        {
            #pragma acc parallel loop gang worker vector collapse(3)
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        array_3d[i][j][k] += 8;
                    }
                }
            }
        }
        total_checksum += compute_checksum(&array_3d[0][0][0], N * M * P);
    }
    
    // Test with enter/exit data for structured data movement
    #pragma acc enter data copyin(double_array[0:P][gang])
    #pragma acc parallel loop gang present(double_array[0:P][gang])
    for (int i = 0; i < P; i++) {
        for (int j = 0; j < N; j++) {
            double_array[i][j] += 9.0;
        }
    }
    #pragma acc exit data copyout(double_array[0:P][gang])
    
    // Test with pointer-based dynamic array
    #pragma acc data copy(dyn_float[0:M*P][vector])
    {
        #pragma acc parallel loop vector
        for (int i = 0; i < M * P; i++) {
            dyn_float[i] += 10.0f;
        }
    }
    
    // Test struct member with partition
    #pragma acc data copy(container.values[0:M][gang], container.coords[0:P][vector])
    {
        #pragma acc parallel loop gang
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < P; j++) {
                container.values[i][j] += 11.0f;
            }
        }
        
        #pragma acc parallel loop vector
        for (int i = 0; i < P; i++) {
            for (int j = 0; j < N; j++) {
                container.coords[i][j] += 12.0;
            }
        }
    }
    
    // Conditional partition selection using volatile variables
    for (int iter = 0; iter < 3; iter++) {
        if (use_gang && iter == 0) {
            #pragma acc data copy(array_2d[0:N][gang])
            {
                #pragma acc parallel loop gang
                for (int i = 0; i < N; i++) {
                    for (int j = 0; j < M; j++) {
                        array_2d[i][j] += 13;
                    }
                }
            }
        } else if (use_worker && iter == 1) {
            #pragma acc data copy(array_2d[0:N][worker])
            {
                #pragma acc parallel loop worker
                for (int i = 0; i < N; i++) {
                    for (int j = 0; j < M; j++) {
                        array_2d[i][j] += 14;
                    }
                }
            }
        } else if (use_vector && iter == 2) {
            #pragma acc data copy(array_2d[0:N][vector])
            {
                #pragma acc parallel loop vector
                for (int i = 0; i < N; i++) {
                    for (int j = 0; j < M; j++) {
                        array_2d[i][j] += 15;
                    }
                }
            }
        }
    }
    
    // Final checksum computation
    total_checksum += compute_checksum(&array_2d[0][0], N * M);
    total_checksum += compute_checksum(dyn_arr, N * M);
    
    printf("Total checksum: %lld\n", total_checksum);
    
    // Cleanup
    free(dyn_arr);
    free(dyn_float);
    
    return 0;
}
