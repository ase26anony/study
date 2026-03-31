#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Use volatile to prevent dead code elimination */
volatile int use_gang = 1;
volatile int use_worker = 1;
volatile int use_vector = 1;
volatile int use_combined = 1;

/* Struct with array members for testing nested components */
struct DataContainer {
    int matrix[50][50];
    int vector[1000];
    double values[200];
};

/* Function to compute checksum */
int compute_checksum(int *data, size_t n) {
    int sum = 0;
    for (size_t i = 0; i < n; i++) {
        sum += data[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    /* Multi-dimensional arrays for partition testing */
    int arr3d[10][20][30];
    int arr2d[100][100];
    int arr1d[1000];
    
    /* Dynamic arrays for pointer-based mappings */
    int *dyn_arr = (int*)malloc(500 * sizeof(int));
    double *dyn_matrix = (double*)malloc(100 * 100 * sizeof(double));
    
    /* Struct instance */
    struct DataContainer container;
    
    /* Initialize all arrays */
    memset(arr3d, 0, sizeof(arr3d));
    memset(arr2d, 0, sizeof(arr2d));
    memset(arr1d, 0, sizeof(arr1d));
    memset(dyn_arr, 0, 500 * sizeof(int));
    memset(dyn_matrix, 0, 100 * 100 * sizeof(double));
    memset(&container, 0, sizeof(container));
    
    int total_checksum = 0;
    
    /* Case 0: Gang redundant (default mapping) */
    if (use_gang) {
        #pragma acc data copy(arr1d[0:1000])  /* gang redundant */
        {
            #pragma acc parallel loop
            for (int i = 0; i < 1000; i++) {
                arr1d[i] += i % 7;
            }
        }
        total_checksum += compute_checksum(arr1d, 1000);
    }
    
    /* Case 1: Gang partitioned */
    if (use_gang) {
        #pragma acc data copy(arr2d[0:100][0:100][gang])
        {
            #pragma acc parallel loop gang
            for (int i = 0; i < 100; i++) {
                for (int j = 0; j < 100; j++) {
                    arr2d[i][j] += (i + j) % 11;
                }
            }
        }
        total_checksum += compute_checksum(&arr2d[0][0], 100 * 100);
    }
    
    /* Case 2: Worker partitioned */
    if (use_worker) {
        #pragma acc data copy(dyn_arr[0:500][worker])
        {
            #pragma acc parallel loop worker
            for (int i = 0; i < 500; i++) {
                dyn_arr[i] += i % 13;
            }
        }
        total_checksum += compute_checksum(dyn_arr, 500);
    }
    
    /* Case 3: Gang+worker partitioned */
    if (use_combined) {
        #pragma acc data copy(arr3d[0:10][0:20][0:30][gang+worker])
        {
            #pragma acc parallel loop gang worker
            for (int i = 0; i < 10; i++) {
                for (int j = 0; j < 20; j++) {
                    for (int k = 0; k < 30; k++) {
                        arr3d[i][j][k] += (i * j + k) % 17;
                    }
                }
            }
        }
        total_checksum += compute_checksum(&arr3d[0][0][0], 10 * 20 * 30);
    }
    
    /* Case 4: Vector partitioned */
    if (use_vector) {
        #pragma acc data copy(container.vector[0:1000][vector])
        {
            #pragma acc parallel loop vector
            for (int i = 0; i < 1000; i++) {
                container.vector[i] += i % 19;
            }
        }
        total_checksum += compute_checksum(container.vector, 1000);
    }
    
    /* Case 5: Gang+vector partitioned */
    if (use_combined) {
        /* Using enter/exit data for structured data movement */
        #pragma acc enter data copyin(container.matrix[0:50][0:50][gang+vector])
        
        #pragma acc parallel loop gang vector present(container.matrix[gang+vector])
        for (int i = 0; i < 50; i++) {
            for (int j = 0; j < 50; j++) {
                container.matrix[i][j] += (i * 31 + j) % 23;
            }
        }
        
        #pragma acc exit data copyout(container.matrix[0:50][0:50][gang+vector])
        total_checksum += compute_checksum(&container.matrix[0][0], 50 * 50);
    }
    
    /* Case 6: Worker+vector partitioned */
    if (use_combined) {
        #pragma acc data copy(dyn_matrix[0:10000][worker+vector])
        {
            #pragma acc parallel loop worker vector
            for (int i = 0; i < 10000; i++) {
                dyn_matrix[i] += (i % 29) * 0.5;
            }
        }
        /* Convert double checksum to int for consistency */
        double dsum = 0;
        for (int i = 0; i < 10000; i++) {
            dsum += dyn_matrix[i];
        }
        total_checksum += (int)dsum;
    }
    
    /* Case 7: Fully partitioned (gang+worker+vector) */
    if (use_gang && use_worker && use_vector) {
        /* Complex mapping with all three partition types */
        #pragma acc data copy(arr2d[0:100][0:100][gang+worker+vector])
        {
            #pragma acc parallel loop gang worker vector
            for (int i = 0; i < 100; i++) {
                #pragma acc loop worker vector
                for (int j = 0; j < 100; j++) {
                    arr2d[i][j] += (i * j) % 31;
                }
            }
        }
        total_checksum += compute_checksum(&arr2d[0][0], 100 * 100);
    }
    
    /* Test conditional partition selection */
    for (int iter = 0; iter < 3; iter++) {
        volatile int selector = iter;
        
        if (selector == 0) {
            #pragma acc data copy(arr1d[0:100][gang])
            {
                #pragma acc parallel loop gang
                for (int i = 0; i < 100; i++) {
                    arr1d[i] += 1;
                }
            }
        } else if (selector == 1) {
            #pragma acc data copy(arr1d[0:100][worker])
            {
                #pragma acc parallel loop worker
                for (int i = 0; i < 100; i++) {
                    arr1d[i] += 2;
                }
            }
        } else if (selector == 2) {
            #pragma acc data copy(arr1d[0:100][vector])
            {
                #pragma acc parallel loop vector
                for (int i = 0; i < 100; i++) {
                    arr1d[i] += 3;
                }
            }
        }
    }
    
    /* Nested constructs with different partition types */
    #pragma acc data copy(arr2d[0:100][0:100][gang])
    {
        #pragma acc parallel loop gang
        for (int i = 0; i < 100; i++) {
            #pragma acc loop worker
            for (int j = 0; j < 100; j++) {
                arr2d[i][j] += 5;
            }
        }
        
        /* Inner region with different partition */
        #pragma acc parallel loop vector copy(arr2d[0:100][0:100][vector])
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < 100; j++) {
                arr2d[i][j] += 7;
            }
        }
    }
    
    /* Final checksum computation */
    total_checksum += compute_checksum(arr1d, 1000);
    total_checksum += compute_checksum(&arr2d[0][0], 100 * 100);
    
    printf("Total checksum: %d\n", total_checksum);
    
    /* Cleanup */
    free(dyn_arr);
    free(dyn_matrix);
    
    return 0;
}
