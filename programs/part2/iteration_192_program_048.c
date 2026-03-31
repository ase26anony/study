#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Use volatile to prevent dead code elimination */
volatile int use_gang = 1;
volatile int use_worker = 1;
volatile int use_vector = 1;
volatile int use_combined = 1;

/* Struct with array members for requirement #6 */
struct DataContainer {
    int matrix[50][50];
    int linear[1000];
    double values[200];
};

/* Helper to initialize arrays */
void init_2d(int arr[][50], int rows, int cols) {
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            arr[i][j] = i * cols + j;
}

void init_1d(int arr[], int n) {
    for (int i = 0; i < n; i++)
        arr[i] = i;
}

int main() {
    /* Multi-dimensional arrays for requirement #2 */
    int big_array[100][100];
    int md_array[50][50][50];
    
    /* Struct instance */
    struct DataContainer container;
    
    /* Dynamic arrays for requirement #3 */
    int *dyn_arr = (int*)malloc(500 * sizeof(int));
    double *dyn_matrix = (double*)malloc(100 * 100 * sizeof(double));
    
    /* Initialize all data */
    memset(big_array, 0, sizeof(big_array));
    memset(md_array, 0, sizeof(md_array));
    init_2d(container.matrix, 50, 50);
    init_1d(container.linear, 1000);
    init_1d(dyn_arr, 500);
    
    /* Array to force different partition codes */
    int checksum = 0;
    
    /* 
     * Requirement #5: Conditional partition selection
     * We'll use volatile conditions to create different code paths
     */
    
    /* Case 0: gang redundant (default mapping) */
    if (use_gang) {
        #pragma acc data copy(big_array)
        {
            #pragma acc parallel loop gang
            for (int i = 0; i < 100; i++) {
                for (int j = 0; j < 100; j++) {
                    big_array[i][j] += 1;
                }
            }
        }
        checksum += big_array[0][0];
    }
    
    /* Case 1: gang partitioned */
    if (use_gang) {
        #pragma acc data copy(big_array[0:100][gang])
        {
            #pragma acc parallel loop gang
            for (int i = 0; i < 100; i++) {
                for (int j = 0; j < 100; j++) {
                    big_array[i][j] += 2;
                }
            }
        }
        checksum += big_array[1][1];
    }
    
    /* Case 2: worker partitioned */
    if (use_worker) {
        #pragma acc data copy(container.linear[0:1000][worker])
        {
            #pragma acc parallel loop worker
            for (int i = 0; i < 1000; i++) {
                container.linear[i] *= 2;
            }
        }
        checksum += container.linear[10];
    }
    
    /* Case 3: gang+worker partitioned */
    if (use_combined) {
        #pragma acc data copy(container.matrix[0:50][gang][worker])
        {
            #pragma acc parallel loop gang worker
            for (int i = 0; i < 50; i++) {
                for (int j = 0; j < 50; j++) {
                    container.matrix[i][j] += i + j;
                }
            }
        }
        checksum += container.matrix[5][5];
    }
    
    /* Case 4: vector partitioned */
    if (use_vector) {
        #pragma acc data copy(dyn_arr[0:500][vector])
        {
            #pragma acc parallel loop vector
            for (int i = 0; i < 500; i++) {
                dyn_arr[i] -= 5;
            }
        }
        checksum += dyn_arr[100];
    }
    
    /* Case 5: gang+vector partitioned */
    if (use_combined) {
        /* Using multi-dimensional array with 2D partition */
        #pragma acc data copy(md_array[0:50][gang][vector])
        {
            #pragma acc parallel loop gang vector
            for (int i = 0; i < 50; i++) {
                for (int j = 0; j < 50; j++) {
                    for (int k = 0; k < 50; k++) {
                        md_array[i][j][k] = i * j * k;
                    }
                }
            }
        }
        checksum += md_array[2][2][2];
    }
    
    /* Case 6: worker+vector partitioned */
    if (use_worker && use_vector) {
        #pragma acc data copy(dyn_matrix[0:10000][worker+vector])
        {
            #pragma acc parallel loop worker vector
            for (int i = 0; i < 10000; i++) {
                dyn_matrix[i] = i * 0.5;
            }
        }
        checksum += (int)dyn_matrix[500];
    }
    
    /* Case 7: fully partitioned (gang+worker+vector) */
    if (use_gang && use_worker && use_vector) {
        /* 3D array with all three partition dimensions */
        #pragma acc data copy(md_array[0:50][gang][worker][vector])
        {
            #pragma acc parallel loop gang worker vector collapse(3)
            for (int i = 0; i < 50; i++) {
                for (int j = 0; j < 50; j++) {
                    for (int k = 0; k < 50; k++) {
                        md_array[i][j][k] += 100;
                    }
                }
            }
        }
        checksum += md_array[10][10][10];
    }
    
    /* Requirement #4: Nested and combined constructs */
    /* Enter data with gang partition */
    #pragma acc enter data copyin(container.values[0:200][gang])
    
    /* Parallel region using present data */
    #pragma acc parallel loop present(container.values[gang])
    for (int i = 0; i < 200; i++) {
        container.values[i] = i * 3.14;
    }
    
    /* Exit data */
    #pragma acc exit data copyout(container.values[gang])
    checksum += (int)container.values[50];
    
    /* Additional test: Struct member with mixed partitions */
    #pragma acc data copy(container.matrix[0:50][gang], container.linear[0:1000][vector])
    {
        #pragma acc parallel loop gang
        for (int i = 0; i < 50; i++) {
            #pragma acc loop vector
            for (int j = 0; j < 50; j++) {
                container.matrix[i][j] *= 2;
            }
        }
        
        #pragma acc parallel loop vector
        for (int i = 0; i < 1000; i++) {
            container.linear[i] += 10;
        }
    }
    checksum += container.matrix[20][20] + container.linear[200];
    
    /* Print checksum to prevent optimization */
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(dyn_arr);
    free(dyn_matrix);
    
    return 0;
}
