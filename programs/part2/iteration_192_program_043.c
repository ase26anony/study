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
    int tensor[20][20][20];
};

int main() {
    int i, j, k;
    int checksum = 0;
    
    /* Multi-dimensional arrays for partition testing */
    int arr2d[100][100];
    int arr3d[50][50][50];
    
    /* Dynamic arrays for pointer-based mappings */
    int *dyn_arr = (int*)malloc(10000 * sizeof(int));
    int *dyn_matrix = (int*)malloc(100 * 100 * sizeof(int));
    
    /* Struct instance */
    struct DataContainer container;
    
    /* Initialize all arrays */
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            arr2d[i][j] = i * 100 + j;
        }
    }
    
    for (i = 0; i < 50; i++) {
        for (j = 0; j < 50; j++) {
            for (k = 0; k < 50; k++) {
                arr3d[i][j][k] = i * 2500 + j * 50 + k;
            }
        }
    }
    
    for (i = 0; i < 10000; i++) {
        dyn_arr[i] = i;
    }
    
    for (i = 0; i < 100 * 100; i++) {
        dyn_matrix[i] = i;
    }
    
    memset(&container, 0, sizeof(container));
    
    /* Case 0: Gang redundant (default mapping) */
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
    }
    
    /* Case 1: Gang partitioned */
    if (use_gang) {
        #pragma acc data copy(arr2d[0:100][gang])
        {
            #pragma acc parallel loop gang
            for (i = 0; i < 100; i++) {
                #pragma acc loop worker vector
                for (j = 0; j < 100; j++) {
                    arr2d[i][j] += 2;
                }
            }
        }
    }
    
    /* Case 2: Worker partitioned */
    if (use_worker) {
        #pragma acc data copy(arr2d[0:100][worker])
        {
            #pragma acc parallel loop gang worker
            for (i = 0; i < 100; i++) {
                #pragma acc loop vector
                for (j = 0; j < 100; j++) {
                    arr2d[i][j] += 3;
                }
            }
        }
    }
    
    /* Case 3: Gang+worker partitioned */
    if (use_combined) {
        #pragma acc data copy(arr3d[0:50][gang][worker])
        {
            #pragma acc parallel loop gang worker
            for (i = 0; i < 50; i++) {
                #pragma acc loop vector
                for (j = 0; j < 50; j++) {
                    for (k = 0; k < 50; k++) {
                        arr3d[i][j][k] += 4;
                    }
                }
            }
        }
    }
    
    /* Case 4: Vector partitioned */
    if (use_vector) {
        #pragma acc data copy(dyn_arr[0:10000][vector])
        {
            #pragma acc parallel loop vector
            for (i = 0; i < 10000; i++) {
                dyn_arr[i] += 5;
            }
        }
    }
    
    /* Case 5: Gang+vector partitioned */
    if (use_combined) {
        #pragma acc data copy(container.matrix[gang][vector])
        {
            #pragma acc parallel loop gang vector
            for (i = 0; i < 50; i++) {
                for (j = 0; j < 50; j++) {
                    container.matrix[i][j] = i * 50 + j;
                }
            }
        }
    }
    
    /* Case 6: Worker+vector partitioned */
    if (use_combined) {
        #pragma acc data copy(container.vector[worker+vector])
        {
            #pragma acc parallel loop worker vector
            for (i = 0; i < 1000; i++) {
                container.vector[i] = i * 2;
            }
        }
    }
    
    /* Case 7: Fully partitioned (gang+worker+vector) */
    if (use_combined) {
        #pragma acc data copy(container.tensor[gang][worker][vector])
        {
            #pragma acc parallel loop gang worker vector collapse(3)
            for (i = 0; i < 20; i++) {
                for (j = 0; j < 20; j++) {
                    for (k = 0; k < 20; k++) {
                        container.tensor[i][j][k] = i * 400 + j * 20 + k;
                    }
                }
            }
        }
    }
    
    /* Test with enter/exit data for structured data movement */
    int *structured_arr = (int*)malloc(5000 * sizeof(int));
    for (i = 0; i < 5000; i++) {
        structured_arr[i] = i;
    }
    
    /* Gang partitioned with enter/exit data */
    #pragma acc enter data copyin(structured_arr[0:5000][gang])
    #pragma acc parallel loop gang present(structured_arr[gang])
    for (i = 0; i < 5000; i++) {
        structured_arr[i] += 10;
    }
    #pragma acc exit data copyout(structured_arr[0:5000])
    
    /* Conditional partition selection using volatile variables */
    volatile int ptype;
    for (ptype = 0; ptype < 8; ptype++) {
        if (use_combined) {
            switch (ptype) {
                case 0:
                    #pragma acc data copy(dyn_matrix[0:10000])
                    {
                        #pragma acc parallel loop
                        for (i = 0; i < 10000; i++) {
                            dyn_matrix[i] += 1;
                        }
                    }
                    break;
                case 1:
                    #pragma acc data copy(dyn_matrix[0:10000][gang])
                    {
                        #pragma acc parallel loop gang
                        for (i = 0; i < 100; i++) {
                            #pragma acc loop worker vector
                            for (j = 0; j < 100; j++) {
                                dyn_matrix[i * 100 + j] += 2;
                            }
                        }
                    }
                    break;
                case 2:
                    #pragma acc data copy(dyn_matrix[0:10000][worker])
                    {
                        #pragma acc parallel loop worker
                        for (i = 0; i < 10000; i++) {
                            dyn_matrix[i] += 3;
                        }
                    }
                    break;
                case 3:
                    #pragma acc data copy(arr2d[0:100][gang][worker])
                    {
                        #pragma acc parallel loop gang worker
                        for (i = 0; i < 100; i++) {
                            #pragma acc loop vector
                            for (j = 0; j < 100; j++) {
                                arr2d[i][j] += 4;
                            }
                        }
                    }
                    break;
                case 4:
                    #pragma acc data copy(dyn_arr[0:10000][vector])
                    {
                        #pragma acc parallel loop vector
                        for (i = 0; i < 10000; i++) {
                            dyn_arr[i] += 5;
                        }
                    }
                    break;
                case 5:
                    #pragma acc data copy(arr2d[gang][vector])
                    {
                        #pragma acc parallel loop gang vector
                        for (i = 0; i < 100; i++) {
                            for (j = 0; j < 100; j++) {
                                arr2d[i][j] += 6;
                            }
                        }
                    }
                    break;
                case 6:
                    #pragma acc data copy(dyn_arr[0:10000][worker+vector])
                    {
                        #pragma acc parallel loop worker vector
                        for (i = 0; i < 10000; i++) {
                            dyn_arr[i] += 7;
                        }
                    }
                    break;
                case 7:
                    #pragma acc data copy(arr3d[gang][worker][vector])
                    {
                        #pragma acc parallel loop gang worker vector collapse(2)
                        for (i = 0; i < 50; i++) {
                            for (j = 0; j < 50; j++) {
                                for (k = 0; k < 50; k++) {
                                    arr3d[i][j][k] += 8;
                                }
                            }
                        }
                    }
                    break;
            }
        }
    }
    
    /* Compute checksum to prevent optimization */
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            checksum += arr2d[i][j];
        }
    }
    
    for (i = 0; i < 50; i++) {
        for (j = 0; j < 50; j++) {
            for (k = 0; k < 50; k++) {
                checksum += arr3d[i][j][k];
            }
        }
    }
    
    for (i = 0; i < 10000; i++) {
        checksum += dyn_arr[i];
        checksum += dyn_matrix[i];
    }
    
    for (i = 0; i < 5000; i++) {
        checksum += structured_arr[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(dyn_arr);
    free(dyn_matrix);
    free(structured_arr);
    
    return 0;
}
