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
int compute_checksum(int *data, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += data[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    /* Multi-dimensional arrays for different partition types */
    int arr_3d[20][30][40];
    int arr_2d[100][100];
    int arr_1d[1000];
    
    /* Struct instance */
    struct DataContainer container;
    
    /* Dynamic arrays */
    int *dyn_arr = (int*)malloc(500 * sizeof(int));
    double *dyn_matrix = (double*)malloc(100 * 100 * sizeof(double));
    
    if (!dyn_arr || !dyn_matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    memset(arr_3d, 0, sizeof(arr_3d));
    memset(arr_2d, 0, sizeof(arr_2d));
    memset(arr_1d, 0, sizeof(arr_1d));
    memset(&container, 0, sizeof(container));
    memset(dyn_arr, 0, 500 * sizeof(int));
    memset(dyn_matrix, 0, 100 * 100 * sizeof(double));
    
    int total_checksum = 0;
    
    /* Case 0: Gang redundant (default mapping) */
    if (use_gang) {
        #pragma acc data copy(arr_1d)
        {
            #pragma acc parallel loop
            for (int i = 0; i < 1000; i++) {
                arr_1d[i] += i % 7;
            }
        }
        total_checksum += compute_checksum(arr_1d, 1000);
    }
    
    /* Case 1: Gang partitioned */
    if (use_gang) {
        #pragma acc data copy(arr_2d[0:50][0:50][gang])
        {
            #pragma acc parallel loop gang
            for (int i = 0; i < 50; i++) {
                for (int j = 0; j < 50; j++) {
                    arr_2d[i][j] += i + j;
                }
            }
        }
        total_checksum += compute_checksum(&arr_2d[0][0], 100 * 100);
    }
    
    /* Case 2: Worker partitioned */
    if (use_worker) {
        #pragma acc data copy(dyn_arr[0:500][worker])
        {
            #pragma acc parallel loop worker
            for (int i = 0; i < 500; i++) {
                dyn_arr[i] += i * 2;
            }
        }
        total_checksum += compute_checksum(dyn_arr, 500);
    }
    
    /* Case 3: Gang+worker partitioned */
    if (use_gang && use_worker) {
        #pragma acc data copy(arr_3d[0:10][0:20][0:30][gang+worker])
        {
            #pragma acc parallel loop gang worker
            for (int i = 0; i < 10; i++) {
                for (int j = 0; j < 20; j++) {
                    for (int k = 0; k < 30; k++) {
                        arr_3d[i][j][k] += i * j * k;
                    }
                }
            }
        }
        total_checksum += compute_checksum(&arr_3d[0][0][0], 20 * 30 * 40);
    }
    
    /* Case 4: Vector partitioned */
    if (use_vector) {
        #pragma acc data copy(container.vector[vector])
        {
            #pragma acc parallel loop vector
            for (int i = 0; i < 1000; i++) {
                container.vector[i] += i % 13;
            }
        }
        total_checksum += compute_checksum(container.vector, 1000);
    }
    
    /* Case 5: Gang+vector partitioned */
    if (use_gang && use_vector) {
        #pragma acc data copy(arr_2d[10:40][10:40][gang+vector])
        {
            #pragma acc parallel loop gang vector
            for (int i = 10; i < 50; i++) {
                for (int j = 10; j < 50; j++) {
                    arr_2d[i][j] += (i - 10) * (j - 10);
                }
            }
        }
        total_checksum += compute_checksum(&arr_2d[10][10], 40 * 40);
    }
    
    /* Case 6: Worker+vector partitioned */
    if (use_worker && use_vector) {
        #pragma acc data copy(dyn_matrix[0:10000][worker+vector])
        {
            #pragma acc parallel loop worker vector
            for (int i = 0; i < 10000; i++) {
                dyn_matrix[i] += i * 0.5;
            }
        }
        /* Convert double to int for checksum */
        int temp_sum = 0;
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < 100; j++) {
                temp_sum += (int)dyn_matrix[i * 100 + j];
            }
        }
        total_checksum += temp_sum;
    }
    
    /* Case 7: Fully partitioned (gang+worker+vector) */
    if (use_combined) {
        #pragma acc data copy(arr_3d[5:10][10:15][15:20][gang+worker+vector])
        {
            #pragma acc parallel loop gang worker vector
            for (int i = 5; i < 15; i++) {
                for (int j = 10; j < 25; j++) {
                    for (int k = 15; k < 35; k++) {
                        arr_3d[i][j][k] += (i - 5) + (j - 10) + (k - 15);
                    }
                }
            }
        }
        total_checksum += compute_checksum(&arr_3d[5][10][15], 10 * 15 * 20);
    }
    
    /* Test with enter/exit data for structured data movement */
    if (use_gang) {
        #pragma acc enter data copyin(container.matrix[gang])
        #pragma acc parallel loop gang present(container.matrix[gang])
        for (int i = 0; i < 50; i++) {
            for (int j = 0; j < 50; j++) {
                container.matrix[i][j] = i * 100 + j;
            }
        }
        #pragma acc exit data copyout(container.matrix[gang])
        total_checksum += compute_checksum(&container.matrix[0][0], 50 * 50);
    }
    
    /* Conditional partition selection using command-line arguments */
    for (int ptype = 0; ptype < 8; ptype++) {
        if (argc > 1 && atoi(argv[1]) == ptype) {
            switch (ptype) {
                case 0:
                    #pragma acc data copy(arr_1d)
                    {
                        #pragma acc parallel loop
                        for (int i = 0; i < 1000; i++) {
                            arr_1d[i] += 1;
                        }
                    }
                    break;
                case 1:
                    #pragma acc data copy(arr_2d[gang])
                    {
                        #pragma acc parallel loop gang
                        for (int i = 0; i < 100; i++) {
                            for (int j = 0; j < 100; j++) {
                                arr_2d[i][j] += 2;
                            }
                        }
                    }
                    break;
                case 2:
                    #pragma acc data copy(dyn_arr[worker])
                    {
                        #pragma acc parallel loop worker
                        for (int i = 0; i < 500; i++) {
                            dyn_arr[i] += 3;
                        }
                    }
                    break;
                case 3:
                    #pragma acc data copy(arr_3d[gang+worker])
                    {
                        #pragma acc parallel loop gang worker
                        for (int i = 0; i < 20; i++) {
                            for (int j = 0; j < 30; j++) {
                                for (int k = 0; k < 40; k++) {
                                    arr_3d[i][j][k] += 4;
                                }
                            }
                        }
                    }
                    break;
                case 4:
                    #pragma acc data copy(container.values[vector])
                    {
                        #pragma acc parallel loop vector
                        for (int i = 0; i < 200; i++) {
                            container.values[i] += 5.0;
                        }
                    }
                    break;
                case 5:
                    #pragma acc data copy(arr_2d[gang+vector])
                    {
                        #pragma acc parallel loop gang vector
                        for (int i = 0; i < 100; i++) {
                            for (int j = 0; j < 100; j++) {
                                arr_2d[i][j] += 6;
                            }
                        }
                    }
                    break;
                case 6:
                    #pragma acc data copy(dyn_matrix[worker+vector])
                    {
                        #pragma acc parallel loop worker vector
                        for (int i = 0; i < 10000; i++) {
                            dyn_matrix[i] += 7.0;
                        }
                    }
                    break;
                case 7:
                    #pragma acc data copy(arr_3d[gang+worker+vector])
                    {
                        #pragma acc parallel loop gang worker vector
                        for (int i = 0; i < 20; i++) {
                            for (int j = 0; j < 30; j++) {
                                for (int k = 0; k < 40; k++) {
                                    arr_3d[i][j][k] += 8;
                                }
                            }
                        }
                    }
                    break;
            }
        }
    }
    
    /* Print final checksum to create observable side effect */
    printf("Total checksum: %d\n", total_checksum);
    
    /* Cleanup */
    free(dyn_arr);
    free(dyn_matrix);
    
    return 0;
}
