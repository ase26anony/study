#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Volatile variables to prevent optimization
volatile int use_gang = 1;
volatile int use_worker = 1;
volatile int use_vector = 1;
volatile int use_combined = 1;

// Struct with array members
struct DataContainer {
    int matrix[50][50];
    int vector[1000];
    int tensor[20][20][20];
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
    int arr2d[100][100];
    int arr3d[50][50][50];
    
    // Initialize arrays
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            arr2d[i][j] = i * 100 + j;
        }
    }
    
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 50; j++) {
            for (int k = 0; k < 50; k++) {
                arr3d[i][j][k] = i * 2500 + j * 50 + k;
            }
        }
    }
    
    // Dynamic arrays
    int *dyn_arr = (int*)malloc(10000 * sizeof(int));
    for (int i = 0; i < 10000; i++) {
        dyn_arr[i] = i;
    }
    
    // Struct with arrays
    struct DataContainer container;
    memset(&container, 0, sizeof(container));
    
    int total_checksum = 0;
    
    // Case 0: Gang redundant (default mapping)
    if (use_gang) {
        #pragma acc data copy(arr2d)
        {
            #pragma acc parallel loop gang
            for (int i = 0; i < 100; i++) {
                for (int j = 0; j < 100; j++) {
                    arr2d[i][j] += 1;
                }
            }
        }
        total_checksum += compute_checksum(&arr2d[0][0], 100*100);
    }
    
    // Case 1: Gang partitioned
    if (use_gang) {
        #pragma acc data copy(arr2d[0:50][gang])
        {
            #pragma acc parallel loop gang
            for (int i = 0; i < 50; i++) {
                for (int j = 0; j < 100; j++) {
                    arr2d[i][j] += 2;
                }
            }
        }
        total_checksum += compute_checksum(&arr2d[0][0], 100*100);
    }
    
    // Case 2: Worker partitioned
    if (use_worker) {
        #pragma acc data copy(arr2d[0:100][worker])
        {
            #pragma acc parallel loop gang worker
            for (int i = 0; i < 100; i++) {
                for (int j = 0; j < 100; j++) {
                    arr2d[i][j] += 3;
                }
            }
        }
        total_checksum += compute_checksum(&arr2d[0][0], 100*100);
    }
    
    // Case 3: Gang+worker partitioned
    if (use_combined) {
        #pragma acc data copy(arr3d[0:25][gang][worker])
        {
            #pragma acc parallel loop gang worker
            for (int i = 0; i < 25; i++) {
                for (int j = 0; j < 50; j++) {
                    for (int k = 0; k < 50; k++) {
                        arr3d[i][j][k] += 4;
                    }
                }
            }
        }
        total_checksum += compute_checksum(&arr3d[0][0][0], 50*50*50);
    }
    
    // Case 4: Vector partitioned
    if (use_vector) {
        #pragma acc data copy(dyn_arr[0:5000][vector])
        {
            #pragma acc parallel loop vector
            for (int i = 0; i < 5000; i++) {
                dyn_arr[i] += 5;
            }
        }
        total_checksum += compute_checksum(dyn_arr, 10000);
    }
    
    // Case 5: Gang+vector partitioned
    if (use_combined) {
        #pragma acc data copy(container.matrix[gang][vector])
        {
            #pragma acc parallel loop gang vector
            for (int i = 0; i < 50; i++) {
                for (int j = 0; j < 50; j++) {
                    container.matrix[i][j] = i * 50 + j;
                }
            }
        }
        total_checksum += compute_checksum(&container.matrix[0][0], 50*50);
    }
    
    // Case 6: Worker+vector partitioned
    if (use_combined) {
        #pragma acc data copy(container.vector[0:500][worker+vector])
        {
            #pragma acc parallel loop worker vector
            for (int i = 0; i < 500; i++) {
                container.vector[i] = i * 2;
            }
        }
        total_checksum += compute_checksum(container.vector, 1000);
    }
    
    // Case 7: Fully partitioned (gang+worker+vector)
    if (use_combined) {
        #pragma acc data copy(container.tensor[gang][worker][vector])
        {
            #pragma acc parallel loop gang worker vector
            for (int i = 0; i < 20; i++) {
                for (int j = 0; j < 20; j++) {
                    for (int k = 0; k < 20; k++) {
                        container.tensor[i][j][k] = i * 400 + j * 20 + k;
                    }
                }
            }
        }
        total_checksum += compute_checksum(&container.tensor[0][0][0], 20*20*20);
    }
    
    // Test with enter/exit data for structured data movement
    int *structured_arr = (int*)malloc(1000 * sizeof(int));
    for (int i = 0; i < 1000; i++) {
        structured_arr[i] = i;
    }
    
    // Enter data with gang partition
    #pragma acc enter data copyin(structured_arr[0:500][gang])
    
    // Use the data in parallel region
    #pragma acc parallel loop gang present(structured_arr[gang])
    for (int i = 0; i < 500; i++) {
        structured_arr[i] *= 2;
    }
    
    // Exit data
    #pragma acc exit data copyout(structured_arr[0:500][gang])
    
    total_checksum += compute_checksum(structured_arr, 1000);
    
    // Conditional partition selection using command-line argument
    int partition_type = 0;
    if (argc > 1) {
        partition_type = atoi(argv[1]) % 8;
    }
    
    // Switch on partition type to test different cases
    int *test_arr = (int*)malloc(100 * sizeof(int));
    for (int i = 0; i < 100; i++) {
        test_arr[i] = i;
    }
    
    switch (partition_type) {
        case 0:
            #pragma acc data copy(test_arr)
            {
                #pragma acc parallel loop
                for (int i = 0; i < 100; i++) test_arr[i] += 1;
            }
            break;
        case 1:
            #pragma acc data copy(test_arr[gang])
            {
                #pragma acc parallel loop gang
                for (int i = 0; i < 100; i++) test_arr[i] += 2;
            }
            break;
        case 2:
            #pragma acc data copy(test_arr[worker])
            {
                #pragma acc parallel loop worker
                for (int i = 0; i < 100; i++) test_arr[i] += 3;
            }
            break;
        case 3:
            #pragma acc data copy(test_arr[gang+worker])
            {
                #pragma acc parallel loop gang worker
                for (int i = 0; i < 100; i++) test_arr[i] += 4;
            }
            break;
        case 4:
            #pragma acc data copy(test_arr[vector])
            {
                #pragma acc parallel loop vector
                for (int i = 0; i < 100; i++) test_arr[i] += 5;
            }
            break;
        case 5:
            #pragma acc data copy(test_arr[gang+vector])
            {
                #pragma acc parallel loop gang vector
                for (int i = 0; i < 100; i++) test_arr[i] += 6;
            }
            break;
        case 6:
            #pragma acc data copy(test_arr[worker+vector])
            {
                #pragma acc parallel loop worker vector
                for (int i = 0; i < 100; i++) test_arr[i] += 7;
            }
            break;
        case 7:
            #pragma acc data copy(test_arr[gang+worker+vector])
            {
                #pragma acc parallel loop gang worker vector
                for (int i = 0; i < 100; i++) test_arr[i] += 8;
            }
            break;
    }
    
    total_checksum += compute_checksum(test_arr, 100);
    
    // Print final checksum to prevent dead code elimination
    printf("Total checksum: %d\n", total_checksum);
    
    // Cleanup
    free(dyn_arr);
    free(structured_arr);
    free(test_arr);
    
    return 0;
}
