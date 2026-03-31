#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Volatile variables to prevent optimization
volatile int select_partition = 0;
volatile int use_gang = 1;
volatile int use_worker = 1;
volatile int use_vector = 1;

// Struct with array members
struct DataContainer {
    int matrix[50][50];
    int vector_data[1000];
    double values[200];
};

// Function to compute checksum
int compute_checksum(int *data, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += data[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    // Multi-dimensional arrays
    int arr3d[20][30][40];
    int arr2d[100][100];
    int arr1d[1000];
    
    // Initialize arrays
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 30; j++) {
            for (int k = 0; k < 40; k++) {
                arr3d[i][j][k] = i + j + k;
            }
        }
    }
    
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            arr2d[i][j] = i * 100 + j;
        }
    }
    
    for (int i = 0; i < 1000; i++) {
        arr1d[i] = i;
    }
    
    // Dynamic allocated memory
    int *dyn_arr = (int*)malloc(500 * sizeof(int));
    for (int i = 0; i < 500; i++) {
        dyn_arr[i] = i * 2;
    }
    
    // Struct instance
    struct DataContainer container;
    memset(&container, 0, sizeof(container));
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 50; j++) {
            container.matrix[i][j] = i * j;
        }
    }
    for (int i = 0; i < 1000; i++) {
        container.vector_data[i] = i * 3;
    }
    
    int total_checksum = 0;
    
    // Case 0: Gang redundant (default mapping)
    #pragma acc data copy(arr1d[0:1000])
    {
        #pragma acc parallel loop
        for (int i = 0; i < 1000; i++) {
            arr1d[i] += 1;
        }
    }
    total_checksum += compute_checksum(arr1d, 1000);
    
    // Case 1: Gang partitioned
    #pragma acc data copy(arr2d[0:100][gang])
    {
        #pragma acc parallel loop gang
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < 100; j++) {
                arr2d[i][j] += 2;
            }
        }
    }
    total_checksum += compute_checksum(&arr2d[0][0], 100 * 100);
    
    // Case 2: Worker partitioned
    #pragma acc data copy(arr2d[0:100][worker])
    {
        #pragma acc parallel loop gang worker
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < 100; j++) {
                arr2d[i][j] += 3;
            }
        }
    }
    
    // Case 3: Gang+worker partitioned
    #pragma acc data copy(arr3d[0:20][gang][worker])
    {
        #pragma acc parallel loop gang worker
        for (int i = 0; i < 20; i++) {
            for (int j = 0; j < 30; j++) {
                for (int k = 0; k < 40; k++) {
                    arr3d[i][j][k] += 4;
                }
            }
        }
    }
    total_checksum += compute_checksum(&arr3d[0][0][0], 20 * 30 * 40);
    
    // Case 4: Vector partitioned
    #pragma acc data copy(dyn_arr[0:500][vector])
    {
        #pragma acc parallel loop vector
        for (int i = 0; i < 500; i++) {
            dyn_arr[i] += 5;
        }
    }
    total_checksum += compute_checksum(dyn_arr, 500);
    
    // Case 5: Gang+vector partitioned
    #pragma acc data copy(arr2d[0:100][gang][vector])
    {
        #pragma acc parallel loop gang vector
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < 100; j++) {
                arr2d[i][j] += 6;
            }
        }
    }
    
    // Case 6: Worker+vector partitioned
    #pragma acc data copy(container.vector_data[0:1000][worker+vector])
    {
        #pragma acc parallel loop worker vector
        for (int i = 0; i < 1000; i++) {
            container.vector_data[i] += 7;
        }
    }
    total_checksum += compute_checksum(container.vector_data, 1000);
    
    // Case 7: Fully partitioned (gang+worker+vector)
    #pragma acc data copy(arr3d[0:20][gang][worker][vector])
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
    
    // Test with enter/exit data for structured data movement
    #pragma acc enter data copyin(container.matrix[0:50][gang])
    #pragma acc parallel loop present(container.matrix[gang])
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 50; j++) {
            container.matrix[i][j] += 9;
        }
    }
    #pragma acc exit data copyout(container.matrix[0:50][gang])
    total_checksum += compute_checksum(&container.matrix[0][0], 50 * 50);
    
    // Conditional partition selection using volatile variables
    for (int ptype = 0; ptype < 8; ++ptype) {
        if (select_partition == ptype) {
            switch (ptype) {
                case 0:
                    #pragma acc data copy(arr1d[0:100])
                    {
                        #pragma acc parallel loop
                        for (int i = 0; i < 100; i++) {
                            arr1d[i] += 10;
                        }
                    }
                    break;
                case 1:
                    #pragma acc data copy(arr1d[0:100][gang])
                    {
                        #pragma acc parallel loop gang
                        for (int i = 0; i < 100; i++) {
                            arr1d[i] += 11;
                        }
                    }
                    break;
                case 2:
                    #pragma acc data copy(arr1d[0:100][worker])
                    {
                        #pragma acc parallel loop worker
                        for (int i = 0; i < 100; i++) {
                            arr1d[i] += 12;
                        }
                    }
                    break;
                case 3:
                    #pragma acc data copy(arr1d[0:100][gang][worker])
                    {
                        #pragma acc parallel loop gang worker
                        for (int i = 0; i < 100; i++) {
                            arr1d[i] += 13;
                        }
                    }
                    break;
                case 4:
                    #pragma acc data copy(arr1d[0:100][vector])
                    {
                        #pragma acc parallel loop vector
                        for (int i = 0; i < 100; i++) {
                            arr1d[i] += 14;
                        }
                    }
                    break;
                case 5:
                    #pragma acc data copy(arr1d[0:100][gang][vector])
                    {
                        #pragma acc parallel loop gang vector
                        for (int i = 0; i < 100; i++) {
                            arr1d[i] += 15;
                        }
                    }
                    break;
                case 6:
                    #pragma acc data copy(arr1d[0:100][worker][vector])
                    {
                        #pragma acc parallel loop worker vector
                        for (int i = 0; i < 100; i++) {
                            arr1d[i] += 16;
                        }
                    }
                    break;
                case 7:
                    #pragma acc data copy(arr1d[0:100][gang][worker][vector])
                    {
                        #pragma acc parallel loop gang worker vector
                        for (int i = 0; i < 100; i++) {
                            arr1d[i] += 17;
                        }
                    }
                    break;
            }
        }
    }
    
    // Combined partition types using struct members
    if (use_gang && use_worker) {
        #pragma acc data copy(container.matrix[0:50][gang], container.vector_data[0:1000][worker])
        {
            #pragma acc parallel loop gang
            for (int i = 0; i < 50; i++) {
                for (int j = 0; j < 50; j++) {
                    container.matrix[i][j] += 18;
                }
            }
            
            #pragma acc parallel loop worker
            for (int i = 0; i < 1000; i++) {
                container.vector_data[i] += 19;
            }
        }
    }
    
    // Nested constructs
    #pragma acc data copy(arr2d[0:100][gang][vector])
    {
        #pragma acc parallel loop gang
        for (int i = 0; i < 100; i++) {
            #pragma acc loop vector
            for (int j = 0; j < 100; j++) {
                arr2d[i][j] += 20;
            }
        }
    }
    
    // Final checksum computation
    total_checksum += compute_checksum(&arr2d[0][0], 100 * 100);
    total_checksum += compute_checksum(&arr3d[0][0][0], 20 * 30 * 40);
    total_checksum += compute_checksum(dyn_arr, 500);
    total_checksum += compute_checksum(container.vector_data, 1000);
    total_checksum += compute_checksum(&container.matrix[0][0], 50 * 50);
    
    printf("Total checksum: %d\n", total_checksum);
    
    free(dyn_arr);
    return 0;
}
