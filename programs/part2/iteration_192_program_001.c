#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Use volatile to prevent dead code elimination */
volatile int use_gang = 1;
volatile int use_worker = 1;
volatile int use_vector = 1;
volatile int use_combined = 1;

/* Struct with array members for complex mapping */
struct DataContainer {
    int matrix[50][50];
    int vector[1000];
    double values[200];
};

/* Function to compute checksum */
int compute_checksum(int *arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    int i, j, k;
    int checksum = 0;
    
    /* Multi-dimensional arrays for different partition types */
    int arr_3d[20][30][40];
    int arr_2d[100][100];
    int arr_1d[1000];
    
    /* Initialize arrays */
    for (i = 0; i < 20; i++) {
        for (j = 0; j < 30; j++) {
            for (k = 0; k < 40; k++) {
                arr_3d[i][j][k] = i + j + k;
            }
        }
    }
    
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            arr_2d[i][j] = i * j;
        }
    }
    
    for (i = 0; i < 1000; i++) {
        arr_1d[i] = i;
    }
    
    /* Dynamic allocated memory */
    int *dyn_arr = (int *)malloc(500 * sizeof(int));
    for (i = 0; i < 500; i++) {
        dyn_arr[i] = i * 2;
    }
    
    /* Struct instance */
    struct DataContainer container;
    memset(&container, 0, sizeof(container));
    for (i = 0; i < 1000; i++) {
        container.vector[i] = i * 3;
    }
    
    printf("Starting OpenACC partition coverage test...\n");
    
    /* Case 0: gang redundant (default mapping) */
    if (use_gang) {
        #pragma acc data copy(arr_1d[0:1000])
        {
            #pragma acc parallel loop gang
            for (i = 0; i < 1000; i++) {
                arr_1d[i] += 1;
            }
        }
        checksum += compute_checksum(arr_1d, 1000);
    }
    
    /* Case 1: gang partitioned */
    if (use_gang) {
        #pragma acc data copy(arr_2d[0:100][gang])
        {
            #pragma acc parallel loop gang
            for (i = 0; i < 100; i++) {
                for (j = 0; j < 100; j++) {
                    arr_2d[i][j] += 2;
                }
            }
        }
        checksum += arr_2d[0][0];
    }
    
    /* Case 2: worker partitioned */
    if (use_worker) {
        #pragma acc data copy(arr_1d[0:1000][worker])
        {
            #pragma acc parallel loop worker
            for (i = 0; i < 1000; i++) {
                arr_1d[i] += 3;
            }
        }
        checksum += compute_checksum(arr_1d, 100);
    }
    
    /* Case 3: gang+worker partitioned */
    if (use_gang && use_worker) {
        #pragma acc data copy(arr_2d[0:100][gang][worker])
        {
            #pragma acc parallel loop gang worker
            for (i = 0; i < 100; i++) {
                for (j = 0; j < 100; j++) {
                    arr_2d[i][j] += 4;
                }
            }
        }
        checksum += arr_2d[50][50];
    }
    
    /* Case 4: vector partitioned */
    if (use_vector) {
        #pragma acc data copy(dyn_arr[0:500][vector])
        {
            #pragma acc parallel loop vector
            for (i = 0; i < 500; i++) {
                dyn_arr[i] += 5;
            }
        }
        checksum += compute_checksum(dyn_arr, 100);
    }
    
    /* Case 5: gang+vector partitioned */
    if (use_gang && use_vector) {
        #pragma acc data copy(arr_2d[0:100][gang][vector])
        {
            #pragma acc parallel loop gang vector
            for (i = 0; i < 100; i++) {
                for (j = 0; j < 100; j++) {
                    arr_2d[i][j] += 6;
                }
            }
        }
        checksum += arr_2d[25][25];
    }
    
    /* Case 6: worker+vector partitioned */
    if (use_worker && use_vector) {
        #pragma acc data copy(arr_1d[0:1000][worker][vector])
        {
            #pragma acc parallel loop worker vector
            for (i = 0; i < 1000; i++) {
                arr_1d[i] += 7;
            }
        }
        checksum += compute_checksum(arr_1d, 200);
    }
    
    /* Case 7: fully partitioned (gang+worker+vector) */
    if (use_combined) {
        #pragma acc data copy(arr_3d[0:20][gang][worker][vector])
        {
            #pragma acc parallel loop gang worker vector collapse(3)
            for (i = 0; i < 20; i++) {
                for (j = 0; j < 30; j++) {
                    for (k = 0; k < 40; k++) {
                        arr_3d[i][j][k] += 8;
                    }
                }
            }
        }
        checksum += arr_3d[10][15][20];
    }
    
    /* Test with struct members */
    #pragma acc data copy(container.vector[0:1000][gang], container.values[0:200][vector])
    {
        #pragma acc parallel loop gang
        for (i = 0; i < 1000; i++) {
            container.vector[i] += 9;
        }
        
        #pragma acc parallel loop vector
        for (i = 0; i < 200; i++) {
            container.values[i] = i * 1.5;
        }
    }
    checksum += container.vector[500];
    
    /* Test nested constructs with enter/exit data */
    int *host_arr = (int *)malloc(200 * sizeof(int));
    for (i = 0; i < 200; i++) {
        host_arr[i] = i * 10;
    }
    
    #pragma acc enter data copyin(host_arr[0:200][worker])
    {
        #pragma acc parallel loop worker present(host_arr[0:200][worker])
        for (i = 0; i < 200; i++) {
            host_arr[i] += 10;
        }
    }
    #pragma acc exit data copyout(host_arr[0:200][worker])
    
    checksum += compute_checksum(host_arr, 50);
    
    /* Conditional partition selection using volatile variables */
    for (int iter = 0; iter < 3; iter++) {
        volatile int selector = iter;
        
        if (selector == 0) {
            #pragma acc data copy(arr_1d[0:100][gang])
            {
                #pragma acc parallel loop gang
                for (i = 0; i < 100; i++) {
                    arr_1d[i] += 11;
                }
            }
        } else if (selector == 1) {
            #pragma acc data copy(arr_1d[0:100][vector])
            {
                #pragma acc parallel loop vector
                for (i = 0; i < 100; i++) {
                    arr_1d[i] += 12;
                }
            }
        } else {
            #pragma acc data copy(arr_1d[0:100][worker][vector])
            {
                #pragma acc parallel loop worker vector
                for (i = 0; i < 100; i++) {
                    arr_1d[i] += 13;
                }
            }
        }
    }
    
    /* Complex multi-dimensional partition */
    #pragma acc data copy(arr_3d[0:10][gang][0:20][worker][0:30][vector])
    {
        #pragma acc parallel loop gang worker vector collapse(3)
        for (i = 0; i < 10; i++) {
            for (j = 0; j < 20; j++) {
                for (k = 0; k < 30; k++) {
                    arr_3d[i][j][k] += 14;
                }
            }
        }
    }
    
    /* Final checksum computation to ensure all computations are used */
    int final_checksum = 0;
    final_checksum += compute_checksum(arr_1d, 1000);
    final_checksum += compute_checksum(&arr_2d[0][0], 10000);
    
    for (i = 0; i < 20; i++) {
        for (j = 0; j < 30; j++) {
            for (k = 0; k < 40; k++) {
                final_checksum += arr_3d[i][j][k];
            }
        }
    }
    
    final_checksum += compute_checksum(dyn_arr, 500);
    final_checksum += compute_checksum(container.vector, 1000);
    final_checksum += compute_checksum(host_arr, 200);
    
    printf("Final checksum: %d\n", final_checksum);
    printf("Test completed.\n");
    
    /* Cleanup */
    free(dyn_arr);
    free(host_arr);
    
    return 0;
}
