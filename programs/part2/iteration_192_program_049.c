#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Volatile variables to prevent optimization
volatile int use_gang = 1;
volatile int use_worker = 1;
volatile int use_vector = 1;
volatile int use_combined = 1;

// Struct with array members for testing
struct DataContainer {
    int matrix[50][50];
    int vector[1000];
    int *dynamic_arr;
};

// Function to initialize data
void init_data(struct DataContainer *dc, int size) {
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 50; j++) {
            dc->matrix[i][j] = i * 50 + j;
        }
    }
    
    for (int i = 0; i < 1000; i++) {
        dc->vector[i] = i;
    }
    
    dc->dynamic_arr = (int*)malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        dc->dynamic_arr[i] = i * 2;
    }
}

// Function to compute checksum
int compute_checksum(struct DataContainer *dc, int dyn_size) {
    int sum = 0;
    
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 50; j++) {
            sum += dc->matrix[i][j];
        }
    }
    
    for (int i = 0; i < 1000; i++) {
        sum += dc->vector[i];
    }
    
    for (int i = 0; i < dyn_size; i++) {
        sum += dc->dynamic_arr[i];
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    const int N = 1000;
    const int DYN_SIZE = 500;
    
    // Multi-dimensional arrays
    int arr3d[20][30][40];
    int arr2d[100][100];
    
    // Struct instance
    struct DataContainer data;
    
    // Initialize arrays
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 30; j++) {
            for (int k = 0; k < 40; k++) {
                arr3d[i][j][k] = i * 1200 + j * 40 + k;
            }
        }
    }
    
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            arr2d[i][j] = i * 100 + j;
        }
    }
    
    init_data(&data, DYN_SIZE);
    
    int checksum = 0;
    
    // Case 0: Gang redundant (default mapping)
    if (use_gang) {
        #pragma acc data copy(arr2d[0:50][0:50])
        {
            #pragma acc parallel loop gang
            for (int i = 0; i < 50; i++) {
                #pragma acc loop worker vector
                for (int j = 0; j < 50; j++) {
                    arr2d[i][j] += 1;
                }
            }
        }
    }
    
    // Case 1: Gang partitioned
    if (use_gang) {
        #pragma acc data copy(arr2d[0:50][gang])
        {
            #pragma acc parallel loop gang
            for (int i = 0; i < 50; i++) {
                #pragma acc loop worker vector
                for (int j = 0; j < 50; j++) {
                    arr2d[i][j] += 2;
                }
            }
        }
    }
    
    // Case 2: Worker partitioned
    if (use_worker) {
        #pragma acc data copy(arr2d[0:50][worker])
        {
            #pragma acc parallel loop gang worker
            for (int i = 0; i < 50; i++) {
                #pragma acc loop vector
                for (int j = 0; j < 50; j++) {
                    arr2d[i][j] += 3;
                }
            }
        }
    }
    
    // Case 3: Gang+worker partitioned
    if (use_combined) {
        #pragma acc data copy(arr3d[0:10][gang][worker])
        {
            #pragma acc parallel loop gang worker
            for (int i = 0; i < 10; i++) {
                #pragma acc loop vector
                for (int j = 0; j < 30; j++) {
                    for (int k = 0; k < 40; k++) {
                        arr3d[i][j][k] += 4;
                    }
                }
            }
        }
    }
    
    // Case 4: Vector partitioned
    if (use_vector) {
        #pragma acc data copy(data.vector[0:500][vector])
        {
            #pragma acc parallel loop vector
            for (int i = 0; i < 500; i++) {
                data.vector[i] += 5;
            }
        }
    }
    
    // Case 5: Gang+vector partitioned
    if (use_combined) {
        #pragma acc data copy(arr2d[gang][0:50][vector])
        {
            #pragma acc parallel loop gang vector
            for (int i = 0; i < 100; i++) {
                #pragma acc loop worker
                for (int j = 0; j < 50; j++) {
                    arr2d[i][j] += 6;
                }
            }
        }
    }
    
    // Case 6: Worker+vector partitioned
    if (use_combined) {
        #pragma acc data copy(arr3d[0:10][worker][vector])
        {
            #pragma acc parallel loop gang worker vector
            for (int i = 0; i < 10; i++) {
                for (int j = 0; j < 30; j++) {
                    for (int k = 0; k < 40; k++) {
                        arr3d[i][j][k] += 7;
                    }
                }
            }
        }
    }
    
    // Case 7: Fully partitioned (gang+worker+vector)
    if (use_combined) {
        #pragma acc data copy(arr3d[gang][worker][vector])
        {
            #pragma acc parallel loop gang worker vector
            for (int i = 0; i < 20; i++) {
                for (int j = 0; j < 30; j++) {
                    for (int k = 0; k < 40; k++) {
                        arr3d[i][j][k] += 8;
                    }
                }
            }
        }
    }
    
    // Dynamic data with pointer-based mappings
    int *dyn_arr = (int*)malloc(N * sizeof(int));
    for (int i = 0; i < N; i++) {
        dyn_arr[i] = i;
    }
    
    // Test various partition types on dynamic data
    #pragma acc enter data copyin(dyn_arr[0:N][gang])
    #pragma acc parallel loop gang present(dyn_arr[0:N][gang])
    for (int i = 0; i < N; i++) {
        dyn_arr[i] += 9;
    }
    #pragma acc exit data copyout(dyn_arr[0:N][gang])
    
    // Nested constructs with struct members
    #pragma acc data copy(data.matrix[gang][worker], data.vector[vector])
    {
        #pragma acc parallel loop gang
        for (int i = 0; i < 50; i++) {
            #pragma acc loop worker
            for (int j = 0; j < 50; j++) {
                data.matrix[i][j] += 10;
            }
        }
        
        #pragma acc parallel loop vector
        for (int i = 0; i < 1000; i++) {
            data.vector[i] += 11;
        }
    }
    
    // Conditional partition selection using command-line arguments
    for (int ptype = 0; ptype < 8; ptype++) {
        if (argc > 1 && atoi(argv[1]) == ptype) {
            switch (ptype) {
                case 0:
                    #pragma acc data copy(arr2d[0:50][0:50])
                    {
                        #pragma acc parallel loop
                        for (int i = 0; i < 50; i++) {
                            for (int j = 0; j < 50; j++) {
                                arr2d[i][j] += 12;
                            }
                        }
                    }
                    break;
                case 1:
                    #pragma acc data copy(arr2d[0:50][gang])
                    {
                        #pragma acc parallel loop gang
                        for (int i = 0; i < 50; i++) {
                            for (int j = 0; j < 50; j++) {
                                arr2d[i][j] += 13;
                            }
                        }
                    }
                    break;
                case 2:
                    #pragma acc data copy(arr2d[0:50][worker])
                    {
                        #pragma acc parallel loop worker
                        for (int i = 0; i < 50; i++) {
                            for (int j = 0; j < 50; j++) {
                                arr2d[i][j] += 14;
                            }
                        }
                    }
                    break;
                case 3:
                    #pragma acc data copy(arr2d[gang][worker])
                    {
                        #pragma acc parallel loop gang worker
                        for (int i = 0; i < 100; i++) {
                            for (int j = 0; j < 100; j++) {
                                arr2d[i][j] += 15;
                            }
                        }
                    }
                    break;
                case 4:
                    #pragma acc data copy(data.vector[0:500][vector])
                    {
                        #pragma acc parallel loop vector
                        for (int i = 0; i < 500; i++) {
                            data.vector[i] += 16;
                        }
                    }
                    break;
                case 5:
                    #pragma acc data copy(arr2d[gang][vector])
                    {
                        #pragma acc parallel loop gang vector
                        for (int i = 0; i < 100; i++) {
                            for (int j = 0; j < 100; j++) {
                                arr2d[i][j] += 17;
                            }
                        }
                    }
                    break;
                case 6:
                    #pragma acc data copy(arr2d[worker][vector])
                    {
                        #pragma acc parallel loop worker vector
                        for (int i = 0; i < 100; i++) {
                            for (int j = 0; j < 100; j++) {
                                arr2d[i][j] += 18;
                            }
                        }
                    }
                    break;
                case 7:
                    #pragma acc data copy(arr3d[gang][worker][vector])
                    {
                        #pragma acc parallel loop gang worker vector
                        for (int i = 0; i < 20; i++) {
                            for (int j = 0; j < 30; j++) {
                                for (int k = 0; k < 40; k++) {
                                    arr3d[i][j][k] += 19;
                                }
                            }
                        }
                    }
                    break;
            }
        }
    }
    
    // Compute final checksum
    checksum += compute_checksum(&data, DYN_SIZE);
    
    // Add contributions from other arrays
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            checksum += arr2d[i][j];
        }
    }
    
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 30; j++) {
            for (int k = 0; k < 40; k++) {
                checksum += arr3d[i][j][k];
            }
        }
    }
    
    for (int i = 0; i < N; i++) {
        checksum += dyn_arr[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    
    // Cleanup
    free(data.dynamic_arr);
    free(dyn_arr);
    
    return 0;
}
