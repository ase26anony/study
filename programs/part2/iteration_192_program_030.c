#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Volatile variables to prevent optimization
volatile int use_gang = 1;
volatile int use_worker = 1;
volatile int use_vector = 1;
volatile int use_combined = 1;

// Struct with array members for complex mapping
struct DataContainer {
    int matrix[50][50];
    int vector[1000];
    int cube[20][20][20];
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
    // Initialize arrays with different dimensions
    int arr1d[1000];
    int arr2d[100][100];
    int arr3d[20][20][20];
    struct DataContainer container;
    
    // Initialize all arrays
    for (int i = 0; i < 1000; i++) {
        arr1d[i] = i % 100;
        container.vector[i] = i % 50;
    }
    
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            arr2d[i][j] = (i + j) % 100;
            if (i < 50 && j < 50) {
                container.matrix[i][j] = (i * j) % 100;
            }
        }
    }
    
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 20; j++) {
            for (int k = 0; k < 20; k++) {
                arr3d[i][j][k] = (i + j + k) % 100;
                container.cube[i][j][k] = (i * j * k) % 100;
            }
        }
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
        total_checksum += arr2d[0][0]; // Use value to prevent optimization
    }
    
    // Case 2: Worker partitioned
    if (use_worker) {
        #pragma acc data copy(arr2d[0:100][worker])
        {
            #pragma acc parallel loop worker
            for (int i = 0; i < 100; i++) {
                for (int j = 0; j < 100; j++) {
                    arr2d[i][j] += 3;
                }
            }
        }
        total_checksum += arr2d[1][1];
    }
    
    // Case 3: Gang+worker partitioned
    if (use_gang && use_worker) {
        #pragma acc data copy(arr2d[0:100][gang][worker])
        {
            #pragma acc parallel loop gang worker
            for (int i = 0; i < 100; i++) {
                for (int j = 0; j < 100; j++) {
                    arr2d[i][j] += 4;
                }
            }
        }
        total_checksum += arr2d[2][2];
    }
    
    // Case 4: Vector partitioned
    if (use_vector) {
        #pragma acc data copy(arr1d[0:1000][vector])
        {
            #pragma acc parallel loop vector
            for (int i = 0; i < 1000; i++) {
                arr1d[i] += 5;
            }
        }
        total_checksum += arr1d[10];
    }
    
    // Case 5: Gang+vector partitioned
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
        total_checksum += arr2d[3][3];
    }
    
    // Case 6: Worker+vector partitioned
    if (use_worker && use_vector) {
        #pragma acc data copy(arr2d[0:100][worker][vector])
        {
            #pragma acc parallel loop worker vector
            for (int i = 0; i < 100; i++) {
                for (int j = 0; j < 100; j++) {
                    arr2d[i][j] += 7;
                }
            }
        }
        total_checksum += arr2d[4][4];
    }
    
    // Case 7: Fully partitioned (gang+worker+vector)
    if (use_gang && use_worker && use_vector) {
        #pragma acc data copy(arr3d[0:20][gang][worker][vector])
        {
            #pragma acc parallel loop gang worker vector collapse(3)
            for (int i = 0; i < 20; i++) {
                for (int j = 0; j < 20; j++) {
                    for (int k = 0; k < 20; k++) {
                        arr3d[i][j][k] += 8;
                    }
                }
            }
        }
        total_checksum += arr3d[5][5][5];
    }
    
    // Test with dynamic memory allocation
    int *dyn_arr = (int*)malloc(500 * sizeof(int));
    for (int i = 0; i < 500; i++) {
        dyn_arr[i] = i % 50;
    }
    
    // Mixed partition types with dynamic array
    #pragma acc data copy(dyn_arr[0:500][gang])
    {
        #pragma acc parallel loop gang
        for (int i = 0; i < 500; i++) {
            dyn_arr[i] += 9;
        }
    }
    total_checksum += compute_checksum(dyn_arr, 500);
    
    // Test struct with partitioned array members
    #pragma acc data copy(container.matrix[0:50][gang], container.vector[0:1000][vector])
    {
        #pragma acc parallel loop gang
        for (int i = 0; i < 50; i++) {
            for (int j = 0; j < 50; j++) {
                container.matrix[i][j] += 10;
            }
        }
        
        #pragma acc parallel loop vector
        for (int i = 0; i < 1000; i++) {
            container.vector[i] += 11;
        }
    }
    total_checksum += container.matrix[10][10] + container.vector[100];
    
    // Test nested constructs with enter/exit data
    int *device_ptr;
    #pragma acc enter data copyin(arr1d[0:1000][worker+vector])
    {
        #pragma acc parallel loop present(arr1d[0:1000][worker+vector])
        for (int i = 0; i < 1000; i++) {
            arr1d[i] += 12;
        }
    }
    #pragma acc exit data copyout(arr1d[0:1000][worker+vector])
    
    total_checksum += compute_checksum(arr1d, 100);
    
    // Conditional partition selection using volatile
    volatile int partition_type = 0;
    for (int ptype = 0; ptype < 8; ptype++) {
        if (use_combined) {
            switch (ptype) {
                case 0:
                    #pragma acc data copy(arr2d[0:10][0:10])
                    {
                        #pragma acc parallel loop
                        for (int i = 0; i < 10; i++) {
                            for (int j = 0; j < 10; j++) {
                                arr2d[i][j] += ptype;
                            }
                        }
                    }
                    break;
                case 1:
                    #pragma acc data copy(arr2d[10:20][gang])
                    {
                        #pragma acc parallel loop gang
                        for (int i = 10; i < 20; i++) {
                            for (int j = 0; j < 10; j++) {
                                arr2d[i][j] += ptype;
                            }
                        }
                    }
                    break;
                case 2:
                    #pragma acc data copy(arr2d[20:30][worker])
                    {
                        #pragma acc parallel loop worker
                        for (int i = 20; i < 30; i++) {
                            for (int j = 0; j < 10; j++) {
                                arr2d[i][j] += ptype;
                            }
                        }
                    }
                    break;
                case 3:
                    #pragma acc data copy(arr2d[30:40][gang][worker])
                    {
                        #pragma acc parallel loop gang worker
                        for (int i = 30; i < 40; i++) {
                            for (int j = 0; j < 10; j++) {
                                arr2d[i][j] += ptype;
                            }
                        }
                    }
                    break;
                case 4:
                    #pragma acc data copy(arr1d[200:300][vector])
                    {
                        #pragma acc parallel loop vector
                        for (int i = 200; i < 300; i++) {
                            arr1d[i] += ptype;
                        }
                    }
                    break;
                case 5:
                    #pragma acc data copy(arr2d[40:50][gang][vector])
                    {
                        #pragma acc parallel loop gang vector
                        for (int i = 40; i < 50; i++) {
                            for (int j = 0; j < 10; j++) {
                                arr2d[i][j] += ptype;
                            }
                        }
                    }
                    break;
                case 6:
                    #pragma acc data copy(arr2d[50:60][worker][vector])
                    {
                        #pragma acc parallel loop worker vector
                        for (int i = 50; i < 60; i++) {
                            for (int j = 0; j < 10; j++) {
                                arr2d[i][j] += ptype;
                            }
                        }
                    }
                    break;
                case 7:
                    #pragma acc data copy(arr3d[0:10][gang][worker][vector])
                    {
                        #pragma acc parallel loop gang worker vector collapse(3)
                        for (int i = 0; i < 10; i++) {
                            for (int j = 0; j < 10; j++) {
                                for (int k = 0; k < 10; k++) {
                                    arr3d[i][j][k] += ptype;
                                }
                            }
                        }
                    }
                    break;
            }
            partition_type = ptype; // Use volatile to prevent optimization
        }
    }
    
    // Final checksum computation
    total_checksum += compute_checksum(arr1d, 1000);
    total_checksum += compute_checksum(&arr2d[0][0], 100*100);
    total_checksum += compute_checksum(&arr3d[0][0][0], 20*20*20);
    
    printf("Final checksum: %d\n", total_checksum);
    
    free(dyn_arr);
    return 0;
}
