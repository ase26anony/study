#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Volatile variable to prevent dead code elimination
volatile int select_partition = 0;

// Struct with array members for testing
struct DataContainer {
    int matrix[50][50];
    int vector[1000];
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

int main() {
    int total_checksum = 0;
    
    // 1. Multi-dimensional arrays for partition testing
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
            arr2d[i][j] = i * j;
        }
    }
    
    for (int i = 0; i < 1000; i++) {
        arr1d[i] = i;
    }
    
    // 2. Dynamic allocated memory
    int *dyn_arr = (int*)malloc(500 * sizeof(int));
    for (int i = 0; i < 500; i++) {
        dyn_arr[i] = i * 2;
    }
    
    // 3. Struct with arrays
    struct DataContainer container;
    memset(&container, 0, sizeof(container));
    for (int i = 0; i < 1000; i++) {
        container.vector[i] = i * 3;
    }
    
    // Use volatile to force all code paths to be considered
    // This helps ensure all partition types are processed by the compiler
    
    // CASE 0: Gang redundant (default mapping)
    #pragma acc data copy(arr1d[0:1000])
    {
        #pragma acc parallel loop
        for (int i = 0; i < 1000; i++) {
            arr1d[i] += 1;
        }
    }
    total_checksum += compute_checksum(arr1d, 1000);
    
    // CASE 1: Gang partitioned
    #pragma acc data copy(arr2d[0:100][gang])
    {
        #pragma acc parallel loop gang
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < 100; j++) {
                arr2d[i][j] += 2;
            }
        }
    }
    total_checksum += compute_checksum(&arr2d[0][0], 100*100);
    
    // CASE 2: Worker partitioned
    #pragma acc data copy(arr2d[0:100][worker])
    {
        #pragma acc parallel loop worker
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < 100; j++) {
                arr2d[i][j] += 3;
            }
        }
    }
    total_checksum += compute_checksum(&arr2d[0][0], 100*100);
    
    // CASE 3: Gang+worker partitioned
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
    total_checksum += compute_checksum(&arr3d[0][0][0], 20*30*40);
    
    // CASE 4: Vector partitioned
    #pragma acc data copy(dyn_arr[0:500][vector])
    {
        #pragma acc parallel loop vector
        for (int i = 0; i < 500; i++) {
            dyn_arr[i] += 5;
        }
    }
    total_checksum += compute_checksum(dyn_arr, 500);
    
    // CASE 5: Gang+vector partitioned
    #pragma acc data copy(arr2d[0:100][gang][vector])
    {
        #pragma acc parallel loop gang vector
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < 100; j++) {
                arr2d[i][j] += 6;
            }
        }
    }
    total_checksum += compute_checksum(&arr2d[0][0], 100*100);
    
    // CASE 6: Worker+vector partitioned
    #pragma acc data copy(container.vector[0:1000][worker+vector])
    {
        #pragma acc parallel loop worker vector
        for (int i = 0; i < 1000; i++) {
            container.vector[i] += 7;
        }
    }
    total_checksum += compute_checksum(container.vector, 1000);
    
    // CASE 7: Fully partitioned (gang+worker+vector)
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
    total_checksum += compute_checksum(&arr3d[0][0][0], 20*30*40);
    
    // Test with enter/exit data for structured data movement
    int structured_arr[200];
    for (int i = 0; i < 200; i++) {
        structured_arr[i] = i;
    }
    
    // Additional test with enter/exit data
    #pragma acc enter data copyin(structured_arr[0:200][gang])
    #pragma acc parallel loop gang present(structured_arr[0:200][gang])
    for (int i = 0; i < 200; i++) {
        structured_arr[i] += 9;
    }
    #pragma acc exit data copyout(structured_arr[0:200][gang])
    
    total_checksum += compute_checksum(structured_arr, 200);
    
    // Conditional partition selection using volatile
    // This forces the compiler to consider multiple partition types
    for (int iter = 0; iter < 8; iter++) {
        select_partition = iter;
        
        if (select_partition == 0) {
            #pragma acc data copy(arr1d[0:100])
            {
                #pragma acc parallel loop
                for (int i = 0; i < 100; i++) {
                    arr1d[i] += 1;
                }
            }
        } else if (select_partition == 1) {
            #pragma acc data copy(arr2d[0:50][gang])
            {
                #pragma acc parallel loop gang
                for (int i = 0; i < 50; i++) {
                    for (int j = 0; j < 100; j++) {
                        arr2d[i][j] += 1;
                    }
                }
            }
        } else if (select_partition == 2) {
            #pragma acc data copy(arr2d[0:50][worker])
            {
                #pragma acc parallel loop worker
                for (int i = 0; i < 50; i++) {
                    for (int j = 0; j < 100; j++) {
                        arr2d[i][j] += 1;
                    }
                }
            }
        } else if (select_partition == 3) {
            #pragma acc data copy(arr3d[0:10][gang][worker])
            {
                #pragma acc parallel loop gang worker
                for (int i = 0; i < 10; i++) {
                    for (int j = 0; j < 30; j++) {
                        for (int k = 0; k < 40; k++) {
                            arr3d[i][j][k] += 1;
                        }
                    }
                }
            }
        } else if (select_partition == 4) {
            #pragma acc data copy(dyn_arr[0:250][vector])
            {
                #pragma acc parallel loop vector
                for (int i = 0; i < 250; i++) {
                    dyn_arr[i] += 1;
                }
            }
        } else if (select_partition == 5) {
            #pragma acc data copy(arr2d[0:50][gang][vector])
            {
                #pragma acc parallel loop gang vector
                for (int i = 0; i < 50; i++) {
                    for (int j = 0; j < 100; j++) {
                        arr2d[i][j] += 1;
                    }
                }
            }
        } else if (select_partition == 6) {
            #pragma acc data copy(container.vector[0:500][worker+vector])
            {
                #pragma acc parallel loop worker vector
                for (int i = 0; i < 500; i++) {
                    container.vector[i] += 1;
                }
            }
        } else if (select_partition == 7) {
            #pragma acc data copy(arr3d[0:10][gang][worker][vector])
            {
                #pragma acc parallel loop gang worker vector
                for (int i = 0; i < 10; i++) {
                    for (int j = 0; j < 30; j++) {
                        for (int k = 0; k < 40; k++) {
                            arr3d[i][j][k] += 1;
                        }
                    }
                }
            }
        }
    }
    
    // Final checksum computation
    total_checksum += compute_checksum(arr1d, 1000);
    total_checksum += compute_checksum(&arr2d[0][0], 100*100);
    total_checksum += compute_checksum(&arr3d[0][0][0], 20*30*40);
    total_checksum += compute_checksum(dyn_arr, 500);
    total_checksum += compute_checksum(container.vector, 1000);
    total_checksum += compute_checksum(structured_arr, 200);
    
    printf("Total checksum: %d\n", total_checksum);
    
    free(dyn_arr);
    return 0;
}
