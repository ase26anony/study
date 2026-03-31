#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Force compiler to consider all branches */
volatile int select_partition = 0;

/* Partition type mapping */
#define GANG_REDUNDANT 0
#define GANG_PARTITIONED 1
#define WORKER_PARTITIONED 2
#define GANG_WORKER_PARTITIONED 3
#define VECTOR_PARTITIONED 4
#define GANG_VECTOR_PARTITIONED 5
#define WORKER_VECTOR_PARTITIONED 6
#define FULLY_PARTITIONED 7

/* Struct with array members */
struct DataContainer {
    int matrix[50][50];
    int vector[1000];
    double values[200];
};

int main() {
    int i, j, k;
    long checksum = 0;
    
    /* Multi-dimensional arrays */
    int arr3d[10][20][30];
    int arr2d[100][100];
    int *dyn_arr;
    
    /* Struct instance */
    struct DataContainer container;
    
    /* Initialize arrays */
    memset(arr3d, 0, sizeof(arr3d));
    memset(arr2d, 0, sizeof(arr2d));
    memset(&container, 0, sizeof(container));
    
    dyn_arr = (int*)malloc(1000 * sizeof(int));
    memset(dyn_arr, 0, 1000 * sizeof(int));
    
    /* Case 0: Gang redundant (default mapping) */
    if (select_partition == GANG_REDUNDANT) {
        #pragma acc data copy(arr2d)
        {
            #pragma acc parallel loop gang
            for (i = 0; i < 100; i++) {
                for (j = 0; j < 100; j++) {
                    arr2d[i][j] += i + j;
                }
            }
        }
    }
    
    /* Case 1: Gang partitioned */
    #pragma acc data copy(arr2d[0:50][gang])
    {
        #pragma acc parallel loop gang
        for (i = 0; i < 50; i++) {
            for (j = 0; j < 100; j++) {
                arr2d[i][j] += i * j;
            }
        }
    }
    
    /* Case 2: Worker partitioned */
    #pragma acc data copy(arr2d[0:100][worker])
    {
        #pragma acc parallel loop worker
        for (i = 0; i < 100; i++) {
            #pragma acc loop vector
            for (j = 0; j < 100; j++) {
                arr2d[i][j] += 1;
            }
        }
    }
    
    /* Case 3: Gang+Worker partitioned */
    #pragma acc data copy(arr3d[0:5][gang][worker])
    {
        #pragma acc parallel loop gang worker
        for (i = 0; i < 5; i++) {
            for (j = 0; j < 20; j++) {
                #pragma acc loop vector
                for (k = 0; k < 30; k++) {
                    arr3d[i][j][k] = i * j * k;
                }
            }
        }
    }
    
    /* Case 4: Vector partitioned */
    #pragma acc data copy(dyn_arr[0:1000][vector])
    {
        #pragma acc parallel loop vector
        for (i = 0; i < 1000; i++) {
            dyn_arr[i] = i * 2;
        }
    }
    
    /* Case 5: Gang+Vector partitioned */
    #pragma acc data copy(arr2d[gang][0:50][vector])
    {
        #pragma acc parallel loop gang vector
        for (i = 0; i < 100; i++) {
            for (j = 0; j < 50; j++) {
                arr2d[i][j] += i - j;
            }
        }
    }
    
    /* Case 6: Worker+Vector partitioned */
    #pragma acc data copy(container.vector[worker+vector])
    {
        #pragma acc parallel loop worker vector
        for (i = 0; i < 1000; i++) {
            container.vector[i] = i % 100;
        }
    }
    
    /* Case 7: Fully partitioned (gang+worker+vector) */
    #pragma acc data copy(arr3d[gang][worker][vector])
    {
        #pragma acc parallel loop gang worker vector collapse(3)
        for (i = 0; i < 10; i++) {
            for (j = 0; j < 20; j++) {
                for (k = 0; k < 30; k++) {
                    arr3d[i][j][k] += 1;
                }
            }
        }
    }
    
    /* Combined constructs with nested regions */
    #pragma acc enter data copyin(container.matrix[gang][worker])
    {
        #pragma acc parallel loop gang worker present(container.matrix[gang][worker])
        for (i = 0; i < 50; i++) {
            for (j = 0; j < 50; j++) {
                container.matrix[i][j] = i + j * 2;
            }
        }
    }
    #pragma acc exit data copyout(container.matrix[gang][worker])
    
    /* Dynamic partition selection (forces compiler to consider all possibilities) */
    for (int ptype = 0; ptype < 8; ptype++) {
        if (select_partition == ptype) {
            switch (ptype) {
                case GANG_REDUNDANT:
                    #pragma acc data copy(container.values)
                    {
                        #pragma acc parallel loop
                        for (i = 0; i < 200; i++) {
                            container.values[i] = i * 0.5;
                        }
                    }
                    break;
                case GANG_PARTITIONED:
                    #pragma acc data copy(container.values[gang])
                    {
                        #pragma acc parallel loop gang
                        for (i = 0; i < 200; i++) {
                            container.values[i] += 1.0;
                        }
                    }
                    break;
                case WORKER_PARTITIONED:
                    #pragma acc data copy(container.values[worker])
                    {
                        #pragma acc parallel loop worker
                        for (i = 0; i < 200; i++) {
                            container.values[i] += 2.0;
                        }
                    }
                    break;
                case GANG_WORKER_PARTITIONED:
                    #pragma acc data copy(container.values[gang][worker])
                    {
                        #pragma acc parallel loop gang worker
                        for (i = 0; i < 200; i++) {
                            container.values[i] += 3.0;
                        }
                    }
                    break;
                case VECTOR_PARTITIONED:
                    #pragma acc data copy(container.values[vector])
                    {
                        #pragma acc parallel loop vector
                        for (i = 0; i < 200; i++) {
                            container.values[i] += 4.0;
                        }
                    }
                    break;
                case GANG_VECTOR_PARTITIONED:
                    #pragma acc data copy(container.values[gang][vector])
                    {
                        #pragma acc parallel loop gang vector
                        for (i = 0; i < 200; i++) {
                            container.values[i] += 5.0;
                        }
                    }
                    break;
                case WORKER_VECTOR_PARTITIONED:
                    #pragma acc data copy(container.values[worker][vector])
                    {
                        #pragma acc parallel loop worker vector
                        for (i = 0; i < 200; i++) {
                            container.values[i] += 6.0;
                        }
                    }
                    break;
                case FULLY_PARTITIONED:
                    #pragma acc data copy(container.values[gang][worker][vector])
                    {
                        #pragma acc parallel loop gang worker vector
                        for (i = 0; i < 200; i++) {
                            container.values[i] += 7.0;
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
    
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 20; j++) {
            for (k = 0; k < 30; k++) {
                checksum += arr3d[i][j][k];
            }
        }
    }
    
    for (i = 0; i < 1000; i++) {
        checksum += dyn_arr[i];
        checksum += container.vector[i];
    }
    
    for (i = 0; i < 50; i++) {
        for (j = 0; j < 50; j++) {
            checksum += container.matrix[i][j];
        }
    }
    
    printf("Final checksum: %ld\n", checksum);
    
    free(dyn_arr);
    return 0;
}
