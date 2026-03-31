#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile int use_gang = 1;
volatile int use_worker = 1;
volatile int use_vector = 1;
volatile int use_combined = 1;

/* Struct with array members for requirement #6 */
struct DataContainer {
    int matrix[50][50];
    int vector_data[1000];
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
    int i, j, k;
    int total_checksum = 0;
    
    /* Multi-dimensional arrays for requirement #2 */
    int arr3d[20][30][40];
    int arr2d[100][100];
    
    /* Initialize arrays */
    for (i = 0; i < 20; i++) {
        for (j = 0; j < 30; j++) {
            for (k = 0; k < 40; k++) {
                arr3d[i][j][k] = i + j + k;
            }
        }
    }
    
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            arr2d[i][j] = i * 100 + j;
        }
    }
    
    /* Struct instance */
    struct DataContainer container;
    memset(&container, 0, sizeof(container));
    
    /* Dynamic data for requirement #3 */
    int N = 1000;
    int *dyn_arr = (int *)malloc(N * sizeof(int));
    for (i = 0; i < N; i++) {
        dyn_arr[i] = i % 100;
    }
    
    printf("Starting OpenACC partition coverage test...\n");
    
    /* CASE 0: Gang redundant (default mapping) */
    if (use_gang) {
        #pragma acc data copy(arr2d)
        {
            #pragma acc parallel loop gang
            for (i = 0; i < 100; i++) {
                for (j = 0; j < 100; j++) {
                    arr2d[i][j] += 1;
                }
            }
        }
        total_checksum += compute_checksum(&arr2d[0][0], 100*100);
    }
    
    /* CASE 1: Gang partitioned */
    if (use_gang) {
        #pragma acc data copy(arr2d[0:50][gang])
        {
            #pragma acc parallel loop gang
            for (i = 0; i < 50; i++) {
                for (j = 0; j < 100; j++) {
                    arr2d[i][j] += 2;
                }
            }
        }
        total_checksum += compute_checksum(&arr2d[0][0], 50*100);
    }
    
    /* CASE 2: Worker partitioned */
    if (use_worker) {
        #pragma acc data copy(arr2d[0:100][worker])
        {
            #pragma acc parallel loop gang worker
            for (i = 0; i < 100; i++) {
                #pragma acc loop worker
                for (j = 0; j < 100; j++) {
                    arr2d[i][j] += 3;
                }
            }
        }
        total_checksum += compute_checksum(&arr2d[0][0], 100*100);
    }
    
    /* CASE 3: Gang+worker partitioned */
    if (use_combined) {
        #pragma acc data copy(arr3d[0:10][gang][worker])
        {
            #pragma acc parallel loop gang worker
            for (i = 0; i < 10; i++) {
                #pragma acc loop worker
                for (j = 0; j < 30; j++) {
                    for (k = 0; k < 40; k++) {
                        arr3d[i][j][k] += 4;
                    }
                }
            }
        }
        total_checksum += compute_checksum(&arr3d[0][0][0], 10*30*40);
    }
    
    /* CASE 4: Vector partitioned */
    if (use_vector) {
        #pragma acc data copy(dyn_arr[0:N][vector])
        {
            #pragma acc parallel loop vector
            for (i = 0; i < N; i++) {
                dyn_arr[i] += 5;
            }
        }
        total_checksum += compute_checksum(dyn_arr, N);
    }
    
    /* CASE 5: Gang+vector partitioned */
    if (use_combined) {
        #pragma acc data copy(arr2d[gang][vector])
        {
            #pragma acc parallel loop gang vector
            for (i = 0; i < 100; i++) {
                #pragma acc loop vector
                for (j = 0; j < 100; j++) {
                    arr2d[i][j] += 6;
                }
            }
        }
        total_checksum += compute_checksum(&arr2d[0][0], 100*100);
    }
    
    /* CASE 6: Worker+vector partitioned */
    if (use_combined) {
        #pragma acc data copy(arr2d[worker][vector])
        {
            #pragma acc parallel loop worker vector
            for (i = 0; i < 100; i++) {
                #pragma acc loop vector
                for (j = 0; j < 100; j++) {
                    arr2d[i][j] += 7;
                }
            }
        }
        total_checksum += compute_checksum(&arr2d[0][0], 100*100);
    }
    
    /* CASE 7: Fully partitioned (gang+worker+vector) */
    if (use_gang && use_worker && use_vector) {
        #pragma acc data copy(arr3d[gang][worker][vector])
        {
            #pragma acc parallel loop gang worker vector
            for (i = 0; i < 20; i++) {
                #pragma acc loop worker
                for (j = 0; j < 30; j++) {
                    #pragma acc loop vector
                    for (k = 0; k < 40; k++) {
                        arr3d[i][j][k] += 8;
                    }
                }
            }
        }
        total_checksum += compute_checksum(&arr3d[0][0][0], 20*30*40);
    }
    
    /* Struct with partitioned array members - requirement #6 */
    #pragma acc data copy(container.matrix[gang], container.vector_data[vector])
    {
        #pragma acc parallel loop gang
        for (i = 0; i < 50; i++) {
            #pragma acc loop vector
            for (j = 0; j < 50; j++) {
                container.matrix[i][j] = i * j;
            }
        }
        
        #pragma acc parallel loop vector
        for (i = 0; i < 1000; i++) {
            container.vector_data[i] = i;
        }
    }
    total_checksum += compute_checksum(&container.matrix[0][0], 50*50);
    total_checksum += compute_checksum(container.vector_data, 1000);
    
    /* Nested constructs for requirement #4 */
    #pragma acc enter data copyin(arr2d[0:50][gang])
    #pragma acc parallel loop present(arr2d[gang])
    for (i = 0; i < 50; i++) {
        for (j = 0; j < 100; j++) {
            arr2d[i][j] += 9;
        }
    }
    #pragma acc exit data copyout(arr2d[0:50][gang])
    total_checksum += compute_checksum(&arr2d[0][0], 50*100);
    
    /* Conditional partition selection - requirement #5 */
    for (int iter = 0; iter < 3; iter++) {
        if (use_gang) {
            #pragma acc data copy(arr2d[gang])
            {
                #pragma acc parallel loop gang
                for (i = 0; i < 100; i++) {
                    for (j = 0; j < 100; j++) {
                        arr2d[i][j] += 10;
                    }
                }
            }
        }
        
        if (use_worker) {
            #pragma acc data copy(arr2d[worker])
            {
                #pragma acc parallel loop worker
                for (i = 0; i < 100; i++) {
                    for (j = 0; j < 100; j++) {
                        arr2d[i][j] += 11;
                    }
                }
            }
        }
    }
    
    /* Final checksum computation */
    int final_checksum = 0;
    final_checksum += compute_checksum(&arr3d[0][0][0], 20*30*40);
    final_checksum += compute_checksum(&arr2d[0][0], 100*100);
    final_checksum += compute_checksum(dyn_arr, N);
    
    printf("Total checksum during execution: %d\n", total_checksum);
    printf("Final array checksum: %d\n", final_checksum);
    printf("Test completed.\n");
    
    free(dyn_arr);
    return 0;
}
