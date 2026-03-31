#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Use volatile to prevent dead code elimination */
volatile int select_partition = 0;

/* Partition keywords mapping to switch cases:
   0: gang redundant (default map)
   1: gang partitioned
   2: worker partitioned  
   3: gang+worker partitioned
   4: vector partitioned
   5: gang+vector partitioned
   6: worker+vector partitioned
   7: fully partitioned
*/

/* Struct with array members for requirement #6 */
struct DataContainer {
    int matrix[50][50];
    float vector[100];
    double tensor[10][10][10];
};

int main() {
    int i, j, k;
    int checksum = 0;
    
    /* Multi-dimensional arrays for requirement #2 */
    int arr2d[100][100];
    double arr3d[20][20][20];
    
    /* Initialize arrays */
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            arr2d[i][j] = i + j;
        }
    }
    
    for (i = 0; i < 20; i++) {
        for (j = 0; j < 20; j++) {
            for (k = 0; k < 20; k++) {
                arr3d[i][j][k] = i * 0.1 + j * 0.2 + k * 0.3;
            }
        }
    }
    
    /* Struct instance */
    struct DataContainer container;
    memset(&container, 0, sizeof(container));
    
    /* Dynamic data for requirement #3 */
    int *dyn_arr = (int*)malloc(1000 * sizeof(int));
    for (i = 0; i < 1000; i++) {
        dyn_arr[i] = i % 100;
    }
    
    /* Case 0: Gang redundant (default mapping) */
    #pragma acc parallel loop gang copy(arr2d[0:50][0:50])
    for (i = 0; i < 50; i++) {
        for (j = 0; j < 50; j++) {
            arr2d[i][j] += 1;
        }
    }
    
    /* Case 1: Gang partitioned */
    #pragma acc parallel loop gang copy(arr2d[0:50][0:50][gang])
    for (i = 0; i < 50; i++) {
        for (j = 0; j < 50; j++) {
            arr2d[i][j] += 2;
        }
    }
    
    /* Case 2: Worker partitioned */
    #pragma acc parallel loop gang worker copy(arr2d[0:50][0:50][worker])
    for (i = 0; i < 50; i++) {
        for (j = 0; j < 50; j++) {
            arr2d[i][j] += 3;
        }
    }
    
    /* Case 3: Gang+Worker partitioned - using 2D array section */
    #pragma acc data copy(arr2d[0:30][gang][worker])
    {
        #pragma acc parallel loop gang worker
        for (i = 0; i < 30; i++) {
            for (j = 0; j < 30; j++) {
                arr2d[i][j] += 4;
            }
        }
    }
    
    /* Case 4: Vector partitioned */
    #pragma acc parallel loop vector copy(dyn_arr[0:500][vector])
    for (i = 0; i < 500; i++) {
        dyn_arr[i] *= 2;
    }
    
    /* Case 5: Gang+Vector partitioned - using struct member */
    #pragma acc data copy(container.matrix[0:20][gang][vector])
    {
        #pragma acc parallel loop gang vector
        for (i = 0; i < 20; i++) {
            for (j = 0; j < 20; j++) {
                container.matrix[i][j] = i * j;
            }
        }
    }
    
    /* Case 6: Worker+Vector partitioned - using 3D array */
    #pragma acc parallel loop gang worker vector copy(arr3d[0:10][0:10][worker+vector])
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            for (k = 0; k < 10; k++) {
                arr3d[i][j][k] += 1.0;
            }
        }
    }
    
    /* Case 7: Fully partitioned (gang+worker+vector) - using 3D array section */
    #pragma acc data copy(arr3d[0:5][gang][worker][vector])
    {
        #pragma acc parallel loop gang worker vector collapse(3)
        for (i = 0; i < 5; i++) {
            for (j = 0; j < 5; j++) {
                for (k = 0; k < 5; k++) {
                    arr3d[i][j][k] *= 2.0;
                }
            }
        }
    }
    
    /* Requirement #5: Conditional partition selection */
    for (int ptype = 0; ptype < 8; ++ptype) {
        if (select_partition == ptype) {
            switch (ptype) {
                case 0:
                    #pragma acc parallel loop copy(arr2d[10:10][10:10])
                    for (i = 10; i < 20; i++) {
                        for (j = 10; j < 20; j++) {
                            arr2d[i][j] += ptype;
                        }
                    }
                    break;
                case 1:
                    #pragma acc parallel loop gang copy(arr2d[20:10][20:10][gang])
                    for (i = 20; i < 30; i++) {
                        for (j = 20; j < 30; j++) {
                            arr2d[i][j] += ptype;
                        }
                    }
                    break;
                case 2:
                    #pragma acc parallel loop gang worker copy(arr2d[30:10][30:10][worker])
                    for (i = 30; i < 40; i++) {
                        for (j = 30; j < 40; j++) {
                            arr2d[i][j] += ptype;
                        }
                    }
                    break;
                case 3:
                    #pragma acc data copy(arr2d[40:10][gang][worker])
                    {
                        #pragma acc parallel loop gang worker
                        for (i = 40; i < 50; i++) {
                            for (j = 40; j < 50; j++) {
                                arr2d[i][j] += ptype;
                            }
                        }
                    }
                    break;
                case 4:
                    #pragma acc parallel loop vector copy(dyn_arr[500:100][vector])
                    for (i = 500; i < 600; i++) {
                        dyn_arr[i] += ptype;
                    }
                    break;
                case 5:
                    #pragma acc data copy(container.vector[0:50][gang][vector])
                    {
                        #pragma acc parallel loop gang vector
                        for (i = 0; i < 50; i++) {
                            container.vector[i] = ptype;
                        }
                    }
                    break;
                case 6:
                    #pragma acc parallel loop gang worker vector copy(container.tensor[0:5][worker+vector])
                    for (i = 0; i < 5; i++) {
                        for (j = 0; j < 5; j++) {
                            for (k = 0; k < 5; k++) {
                                container.tensor[i][j][k] = ptype;
                            }
                        }
                    }
                    break;
                case 7:
                    #pragma acc data copy(container.tensor[5:5][gang][worker][vector])
                    {
                        #pragma acc parallel loop gang worker vector collapse(3)
                        for (i = 5; i < 10; i++) {
                            for (j = 5; j < 10; j++) {
                                for (k = 5; k < 10; k++) {
                                    container.tensor[i][j][k] = ptype;
                                }
                            }
                        }
                    }
                    break;
            }
        }
    }
    
    /* Requirement #4: Nested constructs with enter/exit data */
    #pragma acc enter data copyin(container.matrix[10:10][gang])
    
    #pragma acc parallel loop gang present(container.matrix[10:10][gang])
    for (i = 10; i < 20; i++) {
        for (j = 10; j < 20; j++) {
            container.matrix[i][j] = 999;
        }
    }
    
    #pragma acc exit data copyout(container.matrix[10:10][gang])
    
    /* Compute checksum for observable side effect */
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            checksum += arr2d[i][j];
        }
    }
    
    for (i = 0; i < 1000; i++) {
        checksum += dyn_arr[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    
    free(dyn_arr);
    return 0;
}
