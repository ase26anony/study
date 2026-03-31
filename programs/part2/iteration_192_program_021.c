#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Volatile variables to prevent optimization
volatile int use_gang = 1;
volatile int use_worker = 1;
volatile int use_vector = 1;
volatile int use_combined = 1;

// Struct with array members for testing nested components
struct DataContainer {
    int grid[64][64];
    int matrix[32][32][32];
    int linear[1024];
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
    int checksum = 0;
    
    // Multi-dimensional arrays
    int arr3d[8][8][8];
    int arr2d[64][64];
    int *dyn_arr;
    
    // Struct instance
    struct DataContainer container;
    
    // Initialize arrays
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 8; k++) {
                arr3d[i][j][k] = i + j + k;
            }
        }
    }
    
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            arr2d[i][j] = i * j;
        }
    }
    
    // Initialize struct
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            container.grid[i][j] = i - j;
        }
    }
    
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            for (int k = 0; k < 32; k++) {
                container.matrix[i][j][k] = i * j * k;
            }
        }
    }
    
    for (int i = 0; i < 1024; i++) {
        container.linear[i] = i % 256;
    }
    
    // Dynamic allocation
    dyn_arr = (int *)malloc(4096 * sizeof(int));
    for (int i = 0; i < 4096; i++) {
        dyn_arr[i] = i;
    }
    
    printf("Starting OpenACC partition coverage test...\n");
    
    // Case 0: Gang redundant (default mapping)
    if (use_gang) {
        #pragma acc data copy(arr2d)
        {
            #pragma acc parallel loop gang
            for (int i = 0; i < 64; i++) {
                for (int j = 0; j < 64; j++) {
                    arr2d[i][j] += 1;
                }
            }
        }
        checksum += compute_checksum(&arr2d[0][0], 64*64);
    }
    
    // Case 1: Gang partitioned
    if (use_gang) {
        #pragma acc data copy(arr2d[0:64][0:64][gang])
        {
            #pragma acc parallel loop gang
            for (int i = 0; i < 64; i++) {
                #pragma acc loop worker vector
                for (int j = 0; j < 64; j++) {
                    arr2d[i][j] += 2;
                }
            }
        }
        checksum += compute_checksum(&arr2d[0][0], 64*64);
    }
    
    // Case 2: Worker partitioned
    if (use_worker) {
        #pragma acc data copy(arr2d[0:64][0:64][worker])
        {
            #pragma acc parallel loop gang worker
            for (int i = 0; i < 64; i++) {
                #pragma acc loop vector
                for (int j = 0; j < 64; j++) {
                    arr2d[i][j] += 3;
                }
            }
        }
        checksum += compute_checksum(&arr2d[0][0], 64*64);
    }
    
    // Case 3: Gang+worker partitioned
    if (use_combined) {
        #pragma acc data copy(arr3d[0:8][gang][worker])
        {
            #pragma acc parallel loop gang worker
            for (int i = 0; i < 8; i++) {
                #pragma acc loop vector
                for (int j = 0; j < 8; j++) {
                    for (int k = 0; k < 8; k++) {
                        arr3d[i][j][k] += 4;
                    }
                }
            }
        }
        checksum += compute_checksum(&arr3d[0][0][0], 8*8*8);
    }
    
    // Case 4: Vector partitioned
    if (use_vector) {
        #pragma acc data copy(dyn_arr[0:4096][vector])
        {
            #pragma acc parallel loop vector
            for (int i = 0; i < 4096; i++) {
                dyn_arr[i] += 5;
            }
        }
        checksum += compute_checksum(dyn_arr, 4096);
    }
    
    // Case 5: Gang+vector partitioned
    if (use_combined) {
        #pragma acc data copy(container.grid[0:64][gang][vector])
        {
            #pragma acc parallel loop gang vector
            for (int i = 0; i < 64; i++) {
                for (int j = 0; j < 64; j++) {
                    container.grid[i][j] += 6;
                }
            }
        }
        checksum += compute_checksum(&container.grid[0][0], 64*64);
    }
    
    // Case 6: Worker+vector partitioned
    if (use_combined) {
        #pragma acc data copy(container.matrix[0:32][worker][vector])
        {
            #pragma acc parallel loop worker vector
            for (int i = 0; i < 32; i++) {
                for (int j = 0; j < 32; j++) {
                    for (int k = 0; k < 32; k++) {
                        container.matrix[i][j][k] += 7;
                    }
                }
            }
        }
        checksum += compute_checksum(&container.matrix[0][0][0], 32*32*32);
    }
    
    // Case 7: Fully partitioned (gang+worker+vector)
    if (use_combined) {
        #pragma acc data copy(container.matrix[gang][worker][vector])
        {
            #pragma acc parallel loop gang worker vector
            for (int i = 0; i < 32; i++) {
                for (int j = 0; j < 32; j++) {
                    for (int k = 0; k < 32; k++) {
                        container.matrix[i][j][k] += 8;
                    }
                }
            }
        }
        checksum += compute_checksum(&container.matrix[0][0][0], 32*32*32);
    }
    
    // Test with struct member arrays
    #pragma acc data copy(container.linear[0:1024][gang], container.grid[0:64][0:64][vector])
    {
        #pragma acc parallel loop gang
        for (int i = 0; i < 1024; i++) {
            container.linear[i] += 9;
        }
        
        #pragma acc parallel loop vector
        for (int i = 0; i < 64; i++) {
            for (int j = 0; j < 64; j++) {
                container.grid[i][j] += 10;
            }
        }
    }
    checksum += compute_checksum(container.linear, 1024);
    checksum += compute_checksum(&container.grid[0][0], 64*64);
    
    // Test enter/exit data with partitions
    #pragma acc enter data copyin(container.linear[0:512][worker])
    #pragma acc parallel loop worker present(container.linear[0:512][worker])
    for (int i = 0; i < 512; i++) {
        container.linear[i] += 11;
    }
    #pragma acc exit data copyout(container.linear[0:512][worker])
    checksum += compute_checksum(container.linear, 512);
    
    // Conditional partition selection using volatile variables
    for (int iter = 0; iter < 3; iter++) {
        if (use_gang && (iter % 2 == 0)) {
            #pragma acc data copy(arr2d[0:32][0:32][gang])
            {
                #pragma acc parallel loop gang
                for (int i = 0; i < 32; i++) {
                    for (int j = 0; j < 32; j++) {
                        arr2d[i][j] += 12;
                    }
                }
            }
        }
        
        if (use_vector && (iter % 3 == 0)) {
            #pragma acc data copy(dyn_arr[1024:2048][vector])
            {
                #pragma acc parallel loop vector
                for (int i = 1024; i < 3072; i++) {
                    dyn_arr[i] += 13;
                }
            }
        }
    }
    checksum += compute_checksum(&arr2d[0][0], 32*32);
    checksum += compute_checksum(dyn_arr + 1024, 2048);
    
    printf("Final checksum: %d\n", checksum);
    
    // Cleanup
    free(dyn_arr);
    
    return 0;
}
