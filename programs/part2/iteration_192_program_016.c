#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Partition type mapping for conditional testing */
static const char* partition_keywords[] = {
    "",           /* 0: gang redundant (default) */
    "gang",       /* 1: gang partitioned */
    "worker",     /* 2: worker partitioned */
    "gang worker",/* 3: gang+worker partitioned */
    "vector",     /* 4: vector partitioned */
    "gang vector",/* 5: gang+vector partitioned */
    "worker vector",/* 6: worker+vector partitioned */
    "gang worker vector" /* 7: fully partitioned */
};

/* Struct with array members for testing nested components */
struct DataContainer {
    int matrix[50][50];
    float vector[100];
    double grid[20][20][20];
};

/* Volatile variable to prevent dead code elimination */
volatile int select_partition = 0;

int main(int argc, char *argv[]) {
    int i, j, k;
    int checksum = 0;
    
    /* Multi-dimensional arrays for partition testing */
    int arr2d[100][100];
    double arr3d[30][30][30];
    
    /* Dynamic arrays */
    int *dyn_arr = (int*)malloc(1000 * sizeof(int));
    float *dyn_matrix = (float*)malloc(200 * 200 * sizeof(float));
    
    /* Struct instance */
    struct DataContainer container;
    
    /* Initialize arrays */
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            arr2d[i][j] = i + j;
        }
    }
    
    for (i = 0; i < 30; i++) {
        for (j = 0; j < 30; j++) {
            for (k = 0; k < 30; k++) {
                arr3d[i][j][k] = i * j * k * 0.1;
            }
        }
    }
    
    for (i = 0; i < 1000; i++) {
        dyn_arr[i] = i % 100;
    }
    
    for (i = 0; i < 200*200; i++) {
        dyn_matrix[i] = i * 0.5f;
    }
    
    for (i = 0; i < 50; i++) {
        for (j = 0; j < 50; j++) {
            container.matrix[i][j] = i * j;
        }
    }
    
    /* Use command-line argument to influence partition selection */
    if (argc > 1) {
        select_partition = atoi(argv[1]) % 8;
    }
    
    printf("Testing OpenACC partition neutering with various map clauses...\n");
    
    /* ==================== CASE 0: gang redundant (default) ==================== */
    #pragma acc data copy(arr2d)
    {
        #pragma acc parallel loop collapse(2)
        for (i = 0; i < 100; i++) {
            for (j = 0; j < 100; j++) {
                arr2d[i][j] += 1;
            }
        }
    }
    
    /* ==================== CASE 1: gang partitioned ==================== */
    #pragma acc data copy(arr2d[0:50][0:100][gang])
    {
        #pragma acc parallel loop gang
        for (i = 0; i < 50; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < 100; j++) {
                arr2d[i][j] += 2;
            }
        }
    }
    
    /* ==================== CASE 2: worker partitioned ==================== */
    #pragma acc data copy(arr2d[0:100][0:50][worker])
    {
        #pragma acc parallel loop gang
        for (i = 0; i < 100; i++) {
            #pragma acc loop worker
            for (j = 0; j < 50; j++) {
                arr2d[i][j] += 3;
            }
        }
    }
    
    /* ==================== CASE 3: gang+worker partitioned ==================== */
    #pragma acc data copy(arr3d[0:20][gang][worker])
    {
        #pragma acc parallel loop gang worker collapse(2)
        for (i = 0; i < 20; i++) {
            for (j = 0; j < 30; j++) {
                #pragma acc loop vector
                for (k = 0; k < 30; k++) {
                    arr3d[i][j][k] += 1.5;
                }
            }
        }
    }
    
    /* ==================== CASE 4: vector partitioned ==================== */
    #pragma acc parallel loop vector copy(dyn_arr[0:1000][vector])
    for (i = 0; i < 1000; i++) {
        dyn_arr[i] *= 2;
    }
    
    /* ==================== CASE 5: gang+vector partitioned ==================== */
    #pragma acc data copy(container.matrix[0:50][0:50][gang+vector])
    {
        #pragma acc parallel loop gang vector collapse(2)
        for (i = 0; i < 50; i++) {
            for (j = 0; j < 50; j++) {
                container.matrix[i][j] += 5;
            }
        }
    }
    
    /* ==================== CASE 6: worker+vector partitioned ==================== */
    /* Using dynamic memory with sub-array section */
    #pragma acc data copy(dyn_matrix[0:200*200][worker+vector])
    {
        #pragma acc parallel loop worker vector
        for (i = 0; i < 200*200; i++) {
            dyn_matrix[i] += 0.25f;
        }
    }
    
    /* ==================== CASE 7: fully partitioned ==================== */
    #pragma acc data copy(arr3d[0:30][gang][worker][vector])
    {
        #pragma acc parallel loop gang worker vector collapse(3)
        for (i = 0; i < 30; i++) {
            for (j = 0; j < 30; j++) {
                for (k = 0; k < 30; k++) {
                    arr3d[i][j][k] *= 0.9;
                }
            }
        }
    }
    
    /* ==================== Conditional partition selection ==================== */
    /* This creates a single code region where compiler must handle multiple possibilities */
    for (int ptype = 0; ptype < 8; ++ptype) {
        if (select_partition == ptype) {
            /* Use different partition types based on volatile condition */
            switch (ptype) {
                case 0:
                    #pragma acc data copy(arr2d)
                    {
                        #pragma acc parallel loop
                        for (i = 0; i < 10; i++) {
                            for (j = 0; j < 10; j++) {
                                arr2d[i][j] += ptype;
                            }
                        }
                    }
                    break;
                case 1:
                    #pragma acc data copy(arr2d[0:10][0:10][gang])
                    {
                        #pragma acc parallel loop gang
                        for (i = 0; i < 10; i++) {
                            #pragma acc loop worker vector
                            for (j = 0; j < 10; j++) {
                                arr2d[i][j] += ptype;
                            }
                        }
                    }
                    break;
                case 2:
                    #pragma acc data copy(arr2d[0:10][0:10][worker])
                    {
                        #pragma acc parallel loop gang
                        for (i = 0; i < 10; i++) {
                            #pragma acc loop worker
                            for (j = 0; j < 10; j++) {
                                arr2d[i][j] += ptype;
                            }
                        }
                    }
                    break;
                case 3:
                    #pragma acc data copy(arr2d[0:10][gang][worker])
                    {
                        #pragma acc parallel loop gang worker
                        for (i = 0; i < 10; i++) {
                            #pragma acc loop vector
                            for (j = 0; j < 10; j++) {
                                arr2d[i][j] += ptype;
                            }
                        }
                    }
                    break;
                case 4:
                    #pragma acc data copy(arr2d[0:10][0:10][vector])
                    {
                        #pragma acc parallel loop vector
                        for (i = 0; i < 10; i++) {
                            for (j = 0; j < 10; j++) {
                                arr2d[i][j] += ptype;
                            }
                        }
                    }
                    break;
                case 5:
                    #pragma acc data copy(arr2d[0:10][gang][vector])
                    {
                        #pragma acc parallel loop gang vector
                        for (i = 0; i < 10; i++) {
                            for (j = 0; j < 10; j++) {
                                arr2d[i][j] += ptype;
                            }
                        }
                    }
                    break;
                case 6:
                    #pragma acc data copy(arr2d[0:10][worker][vector])
                    {
                        #pragma acc parallel loop worker vector
                        for (i = 0; i < 10; i++) {
                            for (j = 0; j < 10; j++) {
                                arr2d[i][j] += ptype;
                            }
                        }
                    }
                    break;
                case 7:
                    #pragma acc data copy(arr2d[0:10][gang][worker][vector])
                    {
                        #pragma acc parallel loop gang worker vector
                        for (i = 0; i < 10; i++) {
                            for (j = 0; j < 10; j++) {
                                arr2d[i][j] += ptype;
                            }
                        }
                    }
                    break;
            }
        }
    }
    
    /* ==================== Nested constructs with enter/exit data ==================== */
    /* Enter data with gang partition */
    #pragma acc enter data copyin(container.vector[0:100][gang])
    
    /* Parallel region using the data */
    #pragma acc parallel loop present(container.vector[0:100][gang])
    for (i = 0; i < 100; i++) {
        container.vector[i] = i * 0.1f;
    }
    
    /* Exit data */
    #pragma acc exit data copyout(container.vector[0:100][gang])
    
    /* ==================== Compute checksum ==================== */
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            checksum += arr2d[i][j];
        }
    }
    
    for (i = 0; i < 1000; i++) {
        checksum += dyn_arr[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(dyn_arr);
    free(dyn_matrix);
    
    return 0;
}
