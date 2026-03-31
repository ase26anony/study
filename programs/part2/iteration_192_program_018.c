#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Volatile variables to prevent dead code elimination
volatile int use_gang = 1;
volatile int use_worker = 1;
volatile int use_vector = 1;
volatile int use_combined = 1;

// Struct with array members for testing nested components
struct DataContainer {
    int matrix[50][50];
    int vector[1000];
    int scalar;
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
    // Multi-dimensional arrays for partition testing
    int arr3d[10][20][30];
    int arr2d[100][100];
    int arr1d[1000];
    
    // Initialize arrays
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            for (int k = 0; k < 30; k++) {
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
    
    // Struct instance
    struct DataContainer container;
    memset(&container, 0, sizeof(container));
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 50; j++) {
            container.matrix[i][j] = i * j;
        }
    }
    for (int i = 0; i < 1000; i++) {
        container.vector[i] = i * 2;
    }
    container.scalar = 42;
    
    // Dynamic allocated memory
    int *dyn_arr = (int *)malloc(500 * sizeof(int));
    for (int i = 0; i < 500; i++) {
        dyn_arr[i] = i * 3;
    }
    
    int total_checksum = 0;
    
    // Case 0: gang redundant (default mapping)
    if (use_gang) {
        #pragma acc data copy(arr1d[0:1000])
        {
            #pragma acc parallel loop
            for (int i = 0; i < 1000; i++) {
                arr1d[i] += 1;
            }
        }
        total_checksum += compute_checksum(arr1d, 1000);
    }
    
    // Case 1: gang partitioned
    if (use_gang) {
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
    }
    
    // Case 2: worker partitioned
    if (use_worker) {
        #pragma acc data copy(arr1d[0:1000][worker])
        {
            #pragma acc parallel loop worker
            for (int i = 0; i < 1000; i++) {
                arr1d[i] += 3;
            }
        }
        total_checksum += compute_checksum(arr1d, 1000);
    }
    
    // Case 3: gang+worker partitioned
    if (use_gang && use_worker) {
        #pragma acc data copy(arr3d[0:10][gang][worker])
        {
            #pragma acc parallel loop gang worker
            for (int i = 0; i < 10; i++) {
                for (int j = 0; j < 20; j++) {
                    for (int k = 0; k < 30; k++) {
                        arr3d[i][j][k] += 4;
                    }
                }
            }
        }
        total_checksum += compute_checksum(&arr3d[0][0][0], 10 * 20 * 30);
    }
    
    // Case 4: vector partitioned
    if (use_vector) {
        #pragma acc data copy(dyn_arr[0:500][vector])
        {
            #pragma acc parallel loop vector
            for (int i = 0; i < 500; i++) {
                dyn_arr[i] += 5;
            }
        }
        total_checksum += compute_checksum(dyn_arr, 500);
    }
    
    // Case 5: gang+vector partitioned
    if (use_gang && use_vector) {
        #pragma acc data copy(arr2d[0:100][gang][vector])
        {
            #pragma acc parallel loop gang vector
            for (int i = 0; i < 100; i++) {
                for (int j = 0; j < 100; j++) {
                    arr2d[i][j] += 6;
                }
            }
        }
        total_checksum += compute_checksum(&arr2d[0][0], 100 * 100);
    }
    
    // Case 6: worker+vector partitioned
    if (use_worker && use_vector) {
        #pragma acc data copy(container.vector[0:1000][worker+vector])
        {
            #pragma acc parallel loop worker vector
            for (int i = 0; i < 1000; i++) {
                container.vector[i] += 7;
            }
        }
        total_checksum += compute_checksum(container.vector, 1000);
    }
    
    // Case 7: fully partitioned (gang+worker+vector)
    if (use_gang && use_worker && use_vector) {
        #pragma acc data copy(arr3d[0:10][gang][worker][vector])
        {
            #pragma acc parallel loop gang worker vector
            for (int i = 0; i < 10; i++) {
                for (int j = 0; j < 20; j++) {
                    for (int k = 0; k < 30; k++) {
                        arr3d[i][j][k] += 8;
                    }
                }
            }
        }
        total_checksum += compute_checksum(&arr3d[0][0][0], 10 * 20 * 30);
    }
    
    // Test nested struct with array members
    if (use_combined) {
        #pragma acc data copy(container.matrix[0:50][gang], container.vector[0:1000][vector])
        {
            #pragma acc parallel loop gang
            for (int i = 0; i < 50; i++) {
                #pragma acc loop vector
                for (int j = 0; j < 50; j++) {
                    container.matrix[i][j] += 9;
                }
            }
            
            #pragma acc parallel loop vector
            for (int i = 0; i < 1000; i++) {
                container.vector[i] += 10;
            }
        }
        total_checksum += compute_checksum(&container.matrix[0][0], 50 * 50);
        total_checksum += compute_checksum(container.vector, 1000);
    }
    
    // Test enter/exit data with partitions
    if (use_gang) {
        #pragma acc enter data copyin(arr1d[0:1000][gang])
        #pragma acc parallel loop gang present(arr1d[0:1000][gang])
        for (int i = 0; i < 1000; i++) {
            arr1d[i] += 11;
        }
        #pragma acc exit data copyout(arr1d[0:1000][gang])
        total_checksum += compute_checksum(arr1d, 1000);
    }
    
    // Conditional partition selection using command-line argument
    int partition_type = 0;
    if (argc > 1) {
        partition_type = atoi(argv[1]) % 8;
    }
    
    // Array mapping partition codes to keywords
    const char* partition_keywords[] = {
        "",
        "[gang]",
        "[worker]",
        "[gang][worker]",
        "[vector]",
        "[gang][vector]",
        "[worker+vector]",
        "[gang][worker][vector]"
    };
    
    // Use the selected partition type
    #pragma acc data copy(arr1d[0:1000])  // This will use the partition_keywords[partition_type]
    {
        // Note: We can't directly use string substitution in pragmas, so we use conditionals
        if (partition_type == 0) {
            #pragma acc parallel loop
            for (int i = 0; i < 1000; i++) arr1d[i] += 12;
        } else if (partition_type == 1) {
            #pragma acc parallel loop gang
            for (int i = 0; i < 1000; i++) arr1d[i] += 12;
        } else if (partition_type == 2) {
            #pragma acc parallel loop worker
            for (int i = 0; i < 1000; i++) arr1d[i] += 12;
        } else if (partition_type == 3) {
            #pragma acc parallel loop gang worker
            for (int i = 0; i < 1000; i++) arr1d[i] += 12;
        } else if (partition_type == 4) {
            #pragma acc parallel loop vector
            for (int i = 0; i < 1000; i++) arr1d[i] += 12;
        } else if (partition_type == 5) {
            #pragma acc parallel loop gang vector
            for (int i = 0; i < 1000; i++) arr1d[i] += 12;
        } else if (partition_type == 6) {
            #pragma acc parallel loop worker vector
            for (int i = 0; i < 1000; i++) arr1d[i] += 12;
        } else if (partition_type == 7) {
            #pragma acc parallel loop gang worker vector
            for (int i = 0; i < 1000; i++) arr1d[i] += 12;
        }
    }
    total_checksum += compute_checksum(arr1d, 1000);
    
    // Print final checksum to prevent optimization
    printf("Total checksum: %d\n", total_checksum);
    
    // Clean up
    free(dyn_arr);
    
    return 0;
}
