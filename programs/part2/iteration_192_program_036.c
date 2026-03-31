/* Test program to cover partition neutering switch cases in omp-oacc-neuter-broadcast.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile int v_cond = 1;
volatile int v_type = 0;

/* Struct with array members */
struct DataContainer {
    int matrix[50][50];
    int vector[1000];
    int cube[20][20][20];
};

/* Partition mapping array */
const char* partition_map[] = {
    "",           /* 0: gang redundant (default) */
    "[gang]",     /* 1: gang partitioned */
    "[worker]",   /* 2: worker partitioned */
    "[gang][worker]", /* 3: gang+worker partitioned */
    "[vector]",   /* 4: vector partitioned */
    "[gang][vector]", /* 5: gang+vector partitioned */
    "[worker][vector]", /* 6: worker+vector partitioned */
    "[gang][worker][vector]" /* 7: fully partitioned */
};

int main() {
    int i, j, k;
    int checksum = 0;
    
    /* Multi-dimensional arrays */
    int arr2d[100][100];
    int arr3d[30][30][30];
    
    /* Dynamic arrays */
    int *dyn_arr = (int*)malloc(1000 * sizeof(int));
    int *dyn_matrix = (int*)malloc(100 * 100 * sizeof(int));
    
    /* Struct instance */
    struct DataContainer container;
    
    /* Initialize all arrays */
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            arr2d[i][j] = i + j;
        }
    }
    
    for (i = 0; i < 30; i++) {
        for (j = 0; j < 30; j++) {
            for (k = 0; k < 30; k++) {
                arr3d[i][j][k] = i + j + k;
            }
        }
    }
    
    for (i = 0; i < 1000; i++) {
        dyn_arr[i] = i % 100;
    }
    
    for (i = 0; i < 100*100; i++) {
        dyn_matrix[i] = i;
    }
    
    memset(&container, 0, sizeof(container));
    
    /* Case 0: Gang redundant (default mapping) */
    if (v_cond) {
        #pragma acc data copy(arr2d)
        {
            #pragma acc parallel loop gang
            for (i = 0; i < 100; i++) {
                #pragma acc loop worker vector
                for (j = 0; j < 100; j++) {
                    arr2d[i][j] += 1;
                }
            }
        }
    }
    
    /* Case 1: Gang partitioned */
    if (v_cond) {
        #pragma acc data copy(arr2d[0:50][gang])
        {
            #pragma acc parallel loop gang
            for (i = 0; i < 50; i++) {
                #pragma acc loop worker vector
                for (j = 0; j < 100; j++) {
                    arr2d[i][j] += 2;
                }
            }
        }
    }
    
    /* Case 2: Worker partitioned */
    if (v_cond) {
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
    
    /* Case 3: Gang+Worker partitioned */
    if (v_cond) {
        #pragma acc data copy(arr3d[0:20][gang][worker])
        {
            #pragma acc parallel loop gang worker
            for (i = 0; i < 20; i++) {
                #pragma acc loop vector
                for (j = 0; j < 30; j++) {
                    for (k = 0; k < 30; k++) {
                        arr3d[i][j][k] += 4;
                    }
                }
            }
        }
    }
    
    /* Case 4: Vector partitioned */
    if (v_cond) {
        #pragma acc data copy(dyn_arr[0:1000][vector])
        {
            #pragma acc parallel loop vector
            for (i = 0; i < 1000; i++) {
                dyn_arr[i] += 5;
            }
        }
    }
    
    /* Case 5: Gang+Vector partitioned */
    if (v_cond) {
        #pragma acc data copy(container.matrix[gang][vector])
        {
            #pragma acc parallel loop gang vector
            for (i = 0; i < 50; i++) {
                for (j = 0; j < 50; j++) {
                    container.matrix[i][j] += 6;
                }
            }
        }
    }
    
    /* Case 6: Worker+Vector partitioned */
    if (v_cond) {
        #pragma acc data copy(container.vector[worker+vector])
        {
            #pragma acc parallel loop worker vector
            for (i = 0; i < 1000; i++) {
                container.vector[i] += 7;
            }
        }
    }
    
    /* Case 7: Fully partitioned (gang+worker+vector) */
    if (v_cond) {
        #pragma acc data copy(container.cube[gang][worker][vector])
        {
            #pragma acc parallel loop gang worker vector collapse(3)
            for (i = 0; i < 20; i++) {
                for (j = 0; j < 20; j++) {
                    for (k = 0; k < 20; k++) {
                        container.cube[i][j][k] += 8;
                    }
                }
            }
        }
    }
    
    /* Combined constructs with nested regions */
    if (v_cond) {
        #pragma acc enter data copyin(dyn_matrix[0:10000][gang])
        
        #pragma acc parallel loop gang present(dyn_matrix[gang])
        for (i = 0; i < 100; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < 100; j++) {
                dyn_matrix[i*100 + j] += 9;
            }
        }
        
        #pragma acc exit data copyout(dyn_matrix[0:10000][gang])
    }
    
    /* Conditional partition selection using volatile variable */
    for (int ptype = 0; ptype < 8; ++ptype) {
        v_type = ptype;
        if (v_type < 8) {  /* Ensure we don't hit default case here */
            /* Use a switch to create different partition mappings */
            switch (v_type) {
                case 0:
                    #pragma acc data copy(arr2d)
                    {
                        #pragma acc parallel loop
                        for (i = 0; i < 10; i++) {
                            for (j = 0; j < 10; j++) {
                                arr2d[i][j] += 10;
                            }
                        }
                    }
                    break;
                case 1:
                    #pragma acc data copy(arr2d[gang])
                    {
                        #pragma acc parallel loop gang
                        for (i = 0; i < 10; i++) {
                            for (j = 0; j < 10; j++) {
                                arr2d[i][j] += 11;
                            }
                        }
                    }
                    break;
                case 2:
                    #pragma acc data copy(arr2d[worker])
                    {
                        #pragma acc parallel loop worker
                        for (i = 0; i < 10; i++) {
                            for (j = 0; j < 10; j++) {
                                arr2d[i][j] += 12;
                            }
                        }
                    }
                    break;
                case 3:
                    #pragma acc data copy(arr3d[gang][worker])
                    {
                        #pragma acc parallel loop gang worker
                        for (i = 0; i < 10; i++) {
                            for (j = 0; j < 10; j++) {
                                arr3d[i][j][0] += 13;
                            }
                        }
                    }
                    break;
                case 4:
                    #pragma acc data copy(dyn_arr[vector])
                    {
                        #pragma acc parallel loop vector
                        for (i = 0; i < 100; i++) {
                            dyn_arr[i] += 14;
                        }
                    }
                    break;
                case 5:
                    #pragma acc data copy(container.matrix[gang][vector])
                    {
                        #pragma acc parallel loop gang vector
                        for (i = 0; i < 10; i++) {
                            for (j = 0; j < 10; j++) {
                                container.matrix[i][j] += 15;
                            }
                        }
                    }
                    break;
                case 6:
                    #pragma acc data copy(container.vector[worker+vector])
                    {
                        #pragma acc parallel loop worker vector
                        for (i = 0; i < 100; i++) {
                            container.vector[i] += 16;
                        }
                    }
                    break;
                case 7:
                    #pragma acc data copy(container.cube[gang][worker][vector])
                    {
                        #pragma acc parallel loop gang worker vector
                        for (i = 0; i < 10; i++) {
                            for (j = 0; j < 10; j++) {
                                container.cube[i][j][0] += 17;
                            }
                        }
                    }
                    break;
            }
        }
    }
    
    /* Complex multi-dimensional partition with pointer */
    if (v_cond) {
        int (*ptr_matrix)[100] = (int (*)[100])dyn_matrix;
        #pragma acc data copy(ptr_matrix[0:50][gang][worker])
        {
            #pragma acc parallel loop gang worker
            for (i = 0; i < 50; i++) {
                #pragma acc loop vector
                for (j = 0; j < 100; j++) {
                    ptr_matrix[i][j] += 18;
                }
            }
        }
    }
    
    /* Mixed partition types in same region */
    if (v_cond) {
        #pragma acc data copy(arr2d[gang], arr3d[0:10][worker], dyn_arr[vector])
        {
            #pragma acc parallel loop gang
            for (i = 0; i < 10; i++) {
                #pragma acc loop worker
                for (j = 0; j < 10; j++) {
                    #pragma acc loop vector
                    for (k = 0; k < 10; k++) {
                        arr3d[i][j][k] += 19;
                    }
                }
            }
            
            #pragma acc parallel loop vector
            for (i = 0; i < 100; i++) {
                dyn_arr[i] += 20;
            }
        }
    }
    
    /* Compute checksum to prevent optimization */
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            checksum += arr2d[i][j];
        }
    }
    
    for (i = 0; i < 30; i++) {
        for (j = 0; j < 30; j++) {
            for (k = 0; k < 30; k++) {
                checksum += arr3d[i][j][k];
            }
        }
    }
    
    for (i = 0; i < 1000; i++) {
        checksum += dyn_arr[i];
    }
    
    for (i = 0; i < 100*100; i++) {
        checksum += dyn_matrix[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(dyn_arr);
    free(dyn_matrix);
    
    return 0;
}
