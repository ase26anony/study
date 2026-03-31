#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Force compiler to keep all code paths */
volatile int use_partition_type = 0;

/* Struct with array members for testing nested components */
struct DataContainer {
    int matrix[50][50];
    int vector[1000];
    double values[200];
};

/* Function to prevent dead code elimination */
void __attribute__((noinline)) use_result(int val) {
    volatile int sink = val;
    (void)sink;
}

int main(int argc, char *argv[]) {
    int i, j, k;
    int checksum = 0;
    
    /* Multi-dimensional arrays for partition testing */
    int arr3d[20][30][40];
    int arr2d[100][100];
    int arr1d[1000];
    
    /* Initialize arrays */
    for (i = 0; i < 20; i++)
        for (j = 0; j < 30; j++)
            for (k = 0; k < 40; k++)
                arr3d[i][j][k] = i + j + k;
    
    for (i = 0; i < 100; i++)
        for (j = 0; j < 100; j++)
            arr2d[i][j] = i * j;
    
    for (i = 0; i < 1000; i++)
        arr1d[i] = i;
    
    /* Struct with arrays */
    struct DataContainer container;
    memset(&container, 0, sizeof(container));
    
    /* Dynamic allocated memory for pointer-based mappings */
    int *dyn_arr = (int*)malloc(500 * sizeof(int));
    for (i = 0; i < 500; i++)
        dyn_arr[i] = i * 2;
    
    /* Use volatile to prevent compiler from optimizing away branches */
    volatile int cond = argc > 1 ? atoi(argv[1]) : 0;
    
    /* =========================================== */
    /* Test Case 0: Gang Redundant (default) */
    /* =========================================== */
    if (cond == 0 || cond == 8) {
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
        use_partition_type = 0;
    }
    
    /* =========================================== */
    /* Test Case 1: Gang Partitioned */
    /* =========================================== */
    if (cond == 1 || cond == 8) {
        #pragma acc data copy(arr1d[0:1000][gang])
        {
            #pragma acc parallel loop gang
            for (i = 0; i < 1000; i++) {
                arr1d[i] += 2;
            }
        }
        use_partition_type = 1;
    }
    
    /* =========================================== */
    /* Test Case 2: Worker Partitioned */
    /* =========================================== */
    if (cond == 2 || cond == 8) {
        #pragma acc data copy(arr1d[0:1000][worker])
        {
            #pragma acc parallel loop gang worker
            for (i = 0; i < 1000; i++) {
                arr1d[i] += 3;
            }
        }
        use_partition_type = 2;
    }
    
    /* =========================================== */
    /* Test Case 3: Gang+Worker Partitioned */
    /* =========================================== */
    if (cond == 3 || cond == 8) {
        #pragma acc data copy(arr2d[0:50][0:100][gang][worker])
        {
            #pragma acc parallel loop gang worker
            for (i = 0; i < 50; i++) {
                #pragma acc loop vector
                for (j = 0; j < 100; j++) {
                    arr2d[i][j] += 4;
                }
            }
        }
        use_partition_type = 3;
    }
    
    /* =========================================== */
    /* Test Case 4: Vector Partitioned */
    /* =========================================== */
    if (cond == 4 || cond == 8) {
        #pragma acc data copy(dyn_arr[0:500][vector])
        {
            #pragma acc parallel loop vector
            for (i = 0; i < 500; i++) {
                dyn_arr[i] += 5;
            }
        }
        use_partition_type = 4;
    }
    
    /* =========================================== */
    /* Test Case 5: Gang+Vector Partitioned */
    /* =========================================== */
    if (cond == 5 || cond == 8) {
        #pragma acc data copy(arr2d[0:100][0:100][gang][vector])
        {
            #pragma acc parallel loop gang vector
            for (i = 0; i < 100; i++) {
                for (j = 0; j < 100; j++) {
                    arr2d[i][j] += 6;
                }
            }
        }
        use_partition_type = 5;
    }
    
    /* =========================================== */
    /* Test Case 6: Worker+Vector Partitioned */
    /* =========================================== */
    if (cond == 6 || cond == 8) {
        #pragma acc data copy(arr1d[0:1000][worker+vector])
        {
            #pragma acc parallel loop worker vector
            for (i = 0; i < 1000; i++) {
                arr1d[i] += 7;
            }
        }
        use_partition_type = 6;
    }
    
    /* =========================================== */
    /* Test Case 7: Fully Partitioned (Gang+Worker+Vector) */
    /* =========================================== */
    if (cond == 7 || cond == 8) {
        #pragma acc data copy(arr3d[0:20][0:30][0:40][gang][worker][vector])
        {
            #pragma acc parallel loop gang worker vector collapse(3)
            for (i = 0; i < 20; i++) {
                for (j = 0; j < 30; j++) {
                    for (k = 0; k < 40; k++) {
                        arr3d[i][j][k] += 8;
                    }
                }
            }
        }
        use_partition_type = 7;
    }
    
    /* =========================================== */
    /* Test struct with array members */
    /* =========================================== */
    if (cond == 9 || cond == 8) {
        #pragma acc data copy(container.matrix[0:50][0:50][gang], \
                              container.vector[0:1000][worker], \
                              container.values[0:200][vector])
        {
            #pragma acc parallel loop gang
            for (i = 0; i < 50; i++) {
                #pragma acc loop worker vector
                for (j = 0; j < 50; j++) {
                    container.matrix[i][j] = i * 100 + j;
                }
            }
            
            #pragma acc parallel loop worker
            for (i = 0; i < 1000; i++) {
                container.vector[i] = i * 3;
            }
            
            #pragma acc parallel loop vector
            for (i = 0; i < 200; i++) {
                container.values[i] = i * 1.5;
            }
        }
    }
    
    /* =========================================== */
    /* Test nested constructs with enter/exit data */
    /* =========================================== */
    if (cond == 10 || cond == 8) {
        int *host_arr = (int*)malloc(200 * sizeof(int));
        for (i = 0; i < 200; i++) host_arr[i] = i * 10;
        
        #pragma acc enter data copyin(host_arr[0:200][gang+worker])
        
        #pragma acc parallel loop gang worker present(host_arr[0:200][gang+worker])
        for (i = 0; i < 200; i++) {
            host_arr[i] += 100;
        }
        
        #pragma acc exit data copyout(host_arr[0:200][gang+worker])
        
        free(host_arr);
    }
    
    /* =========================================== */
    /* Conditional partition selection in loop */
    /* =========================================== */
    if (cond == 11 || cond == 8) {
        /* Array mapping partition types to keywords */
        const char* partition_keywords[] = {
            "",           /* 0: gang redundant */
            "[gang]",     /* 1: gang partitioned */
            "[worker]",   /* 2: worker partitioned */
            "[gang][worker]", /* 3: gang+worker partitioned */
            "[vector]",   /* 4: vector partitioned */
            "[gang][vector]", /* 5: gang+vector partitioned */
            "[worker+vector]", /* 6: worker+vector partitioned */
            "[gang][worker][vector]" /* 7: fully partitioned */
        };
        
        int test_arr[8][100];
        
        for (int ptype = 0; ptype < 8; ptype++) {
            if (cond == 11 || (ptype % 2) == (cond % 2)) {
                /* This creates different partition mappings based on ptype */
                switch (ptype) {
                    case 0:
                        #pragma acc data copy(test_arr[ptype][0:100])
                        {
                            #pragma acc parallel loop
                            for (i = 0; i < 100; i++) {
                                test_arr[ptype][i] = ptype * 1000 + i;
                            }
                        }
                        break;
                    case 1:
                        #pragma acc data copy(test_arr[ptype][0:100][gang])
                        {
                            #pragma acc parallel loop gang
                            for (i = 0; i < 100; i++) {
                                test_arr[ptype][i] = ptype * 1000 + i;
                            }
                        }
                        break;
                    case 2:
                        #pragma acc data copy(test_arr[ptype][0:100][worker])
                        {
                            #pragma acc parallel loop worker
                            for (i = 0; i < 100; i++) {
                                test_arr[ptype][i] = ptype * 1000 + i;
                            }
                        }
                        break;
                    case 3:
                        #pragma acc data copy(test_arr[ptype][0:100][gang][worker])
                        {
                            #pragma acc parallel loop gang worker
                            for (i = 0; i < 100; i++) {
                                test_arr[ptype][i] = ptype * 1000 + i;
                            }
                        }
                        break;
                    case 4:
                        #pragma acc data copy(test_arr[ptype][0:100][vector])
                        {
                            #pragma acc parallel loop vector
                            for (i = 0; i < 100; i++) {
                                test_arr[ptype][i] = ptype * 1000 + i;
                            }
                        }
                        break;
                    case 5:
                        #pragma acc data copy(test_arr[ptype][0:100][gang][vector])
                        {
                            #pragma acc parallel loop gang vector
                            for (i = 0; i < 100; i++) {
                                test_arr[ptype][i] = ptype * 1000 + i;
                            }
                        }
                        break;
                    case 6:
                        #pragma acc data copy(test_arr[ptype][0:100][worker+vector])
                        {
                            #pragma acc parallel loop worker vector
                            for (i = 0; i < 100; i++) {
                                test_arr[ptype][i] = ptype * 1000 + i;
                            }
                        }
                        break;
                    case 7:
                        #pragma acc data copy(test_arr[ptype][0:100][gang][worker][vector])
                        {
                            #pragma acc parallel loop gang worker vector
                            for (i = 0; i < 100; i++) {
                                test_arr[ptype][i] = ptype * 1000 + i;
                            }
                        }
                        break;
                }
            }
        }
    }
    
    /* =========================================== */
    /* Compute checksum for observable side effect */
    /* =========================================== */
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            checksum += arr2d[i][j];
        }
    }
    
    for (i = 0; i < 1000; i++) {
        checksum += arr1d[i];
    }
    
    for (i = 0; i < 500; i++) {
        checksum += dyn_arr[i];
    }
    
    printf("Final checksum: %d (used partition type: %d)\n", checksum, use_partition_type);
    
    /* Cleanup */
    free(dyn_arr);
    
    return 0;
}

#ifdef __cplusplus
}
#endif
