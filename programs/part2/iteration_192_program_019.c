#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Force compiler to consider all branches */
volatile int select_partition = 0;

/* Struct with array members for complex mapping */
struct DataContainer {
    int matrix[50][50];
    float vector[1000];
    double cube[10][10][10];
};

/* Partition type mapping */
const char* partition_types[] = {
    "gang",
    "worker", 
    "vector",
    "gang,worker",
    "gang,vector",
    "worker,vector",
    "gang,worker,vector"
};

int main(int argc, char *argv[]) {
    int i, j, k;
    int checksum = 0;
    
    /* Multi-dimensional arrays for different partition scenarios */
    int arr_2d[100][100];
    float arr_3d[20][20][20];
    double *dyn_arr;
    struct DataContainer container;
    
    /* Initialize arrays */
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            arr_2d[i][j] = i * 100 + j;
        }
    }
    
    for (i = 0; i < 20; i++) {
        for (j = 0; j < 20; j++) {
            for (k = 0; k < 20; k++) {
                arr_3d[i][j][k] = i * 400 + j * 20 + k;
            }
        }
    }
    
    /* Dynamic allocation */
    dyn_arr = (double*)malloc(1000 * sizeof(double));
    for (i = 0; i < 1000; i++) {
        dyn_arr[i] = i * 1.5;
    }
    
    /* Initialize struct */
    memset(&container, 0, sizeof(container));
    for (i = 0; i < 50; i++) {
        for (j = 0; j < 50; j++) {
            container.matrix[i][j] = i * 50 + j;
        }
    }
    
    /* CASE 0: Gang redundant (default mapping) */
    #pragma acc data copy(arr_2d)
    {
        #pragma acc parallel loop gang
        for (i = 0; i < 100; i++) {
            for (j = 0; j < 100; j++) {
                arr_2d[i][j] += 1;
            }
        }
    }
    
    /* CASE 1: Gang partitioned */
    #pragma acc data copy(arr_2d[0:50][gang])
    {
        #pragma acc parallel loop gang
        for (i = 0; i < 50; i++) {
            for (j = 0; j < 100; j++) {
                arr_2d[i][j] += 2;
            }
        }
    }
    
    /* CASE 2: Worker partitioned */
    #pragma acc data copy(arr_2d[0:100][worker])
    {
        #pragma acc parallel loop gang worker
        for (i = 0; i < 100; i++) {
            #pragma acc loop worker
            for (j = 0; j < 100; j++) {
                arr_2d[i][j] += 3;
            }
        }
    }
    
    /* CASE 3: Gang+Worker partitioned */
    #pragma acc data copy(arr_3d[0:10][gang][worker])
    {
        #pragma acc parallel loop gang worker
        for (i = 0; i < 10; i++) {
            #pragma acc loop worker
            for (j = 0; j < 20; j++) {
                for (k = 0; k < 20; k++) {
                    arr_3d[i][j][k] += 4.0f;
                }
            }
        }
    }
    
    /* CASE 4: Vector partitioned */
    #pragma acc data copy(dyn_arr[0:500][vector])
    {
        #pragma acc parallel loop vector
        for (i = 0; i < 500; i++) {
            dyn_arr[i] += 5.0;
        }
    }
    
    /* CASE 5: Gang+Vector partitioned */
    #pragma acc data copy(arr_2d[gang][0:50][vector])
    {
        #pragma acc parallel loop gang vector
        for (i = 0; i < 100; i++) {
            #pragma acc loop vector
            for (j = 0; j < 50; j++) {
                arr_2d[i][j] += 6;
            }
        }
    }
    
    /* CASE 6: Worker+Vector partitioned */
    #pragma acc data copy(arr_3d[worker][0:10][vector])
    {
        #pragma acc parallel loop gang worker vector
        for (i = 0; i < 20; i++) {
            #pragma acc loop worker
            for (j = 0; j < 10; j++) {
                #pragma acc loop vector
                for (k = 0; k < 20; k++) {
                    arr_3d[i][j][k] += 7.0f;
                }
            }
        }
    }
    
    /* CASE 7: Fully partitioned (gang+worker+vector) */
    #pragma acc data copy(arr_3d[gang][worker][vector])
    {
        #pragma acc parallel loop gang worker vector
        for (i = 0; i < 20; i++) {
            #pragma acc loop worker
            for (j = 0; j < 20; j++) {
                #pragma acc loop vector
                for (k = 0; k < 20; k++) {
                    arr_3d[i][j][k] += 8.0f;
                }
            }
        }
    }
    
    /* Struct with partitioned array members */
    #pragma acc data copy(container.matrix[gang], container.vector[vector])
    {
        #pragma acc parallel loop gang
        for (i = 0; i < 50; i++) {
            #pragma acc loop vector
            for (j = 0; j < 50; j++) {
                container.matrix[i][j] += 9;
            }
        }
        
        #pragma acc parallel loop vector
        for (i = 0; i < 1000; i++) {
            container.vector[i] += 10.0f;
        }
    }
    
    /* Nested constructs with enter/exit data */
    #pragma acc enter data copyin(container.cube[gang][worker])
    #pragma acc parallel loop gang worker present(container.cube[gang][worker])
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            for (k = 0; k < 10; k++) {
                container.cube[i][j][k] += 11.0;
            }
        }
    }
    #pragma acc exit data copyout(container.cube[gang][worker])
    
    /* Conditional partition selection using volatile */
    for (int ptype = 0; ptype < 7; ++ptype) {
        if (select_partition == ptype) {
            #pragma acc data copy(arr_2d[0:50][partition_types[ptype]])
            {
                #pragma acc parallel loop gang
                for (i = 0; i < 50; i++) {
                    for (j = 0; j < 100; j++) {
                        arr_2d[i][j] += 12;
                    }
                }
            }
        }
    }
    
    /* Complex multi-dimensional partition */
    #pragma acc data copy(arr_3d[0:5][gang][0:10][worker][0:15][vector])
    {
        #pragma acc parallel loop gang worker vector
        for (i = 0; i < 5; i++) {
            #pragma acc loop worker
            for (j = 0; j < 10; j++) {
                #pragma acc loop vector
                for (k = 0; k < 15; k++) {
                    arr_3d[i][j][k] += 13.0f;
                }
            }
        }
    }
    
    /* Compute checksum */
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            checksum += arr_2d[i][j];
        }
    }
    
    for (i = 0; i < 20; i++) {
        for (j = 0; j < 20; j++) {
            for (k = 0; k < 20; k++) {
                checksum += (int)arr_3d[i][j][k];
            }
        }
    }
    
    for (i = 0; i < 1000; i++) {
        checksum += (int)dyn_arr[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    
    free(dyn_arr);
    return 0;
}

#ifdef __cplusplus
}
#endif
