#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __GNUC__
#define VOLATILE volatile
#else
#define VOLATILE
#endif

/* Struct with array members for testing nested partition analysis */
struct DataContainer {
    int matrix[50][50];
    int vector[100];
    double values[30][30][30];
};

/* Function to prevent dead code elimination */
void use_result(int result) {
    VOLATILE int sink = result;
    if (sink == 0) printf("Result: %d\n", sink);
}

int main(int argc, char *argv[]) {
    int i, j, k;
    int checksum = 0;
    
    /* Multi-dimensional arrays for partition testing */
    int arr2d[100][100];
    int arr3d[50][50][50];
    double dyn_arr[200][200];
    
    /* Initialize arrays */
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            arr2d[i][j] = i + j;
        }
    }
    
    for (i = 0; i < 50; i++) {
        for (j = 0; j < 50; j++) {
            for (k = 0; k < 50; k++) {
                arr3d[i][j][k] = i * j * k;
            }
        }
    }
    
    for (i = 0; i < 200; i++) {
        for (j = 0; j < 200; j++) {
            dyn_arr[i][j] = (double)(i * j) / 100.0;
        }
    }
    
    /* Struct instance */
    struct DataContainer container;
    memset(&container, 0, sizeof(container));
    
    /* Dynamic memory for pointer-based mappings */
    int *dyn_mem = (int*)malloc(1000 * sizeof(int));
    for (i = 0; i < 1000; i++) {
        dyn_mem[i] = i * 2;
    }
    
    /* Volatile variable to control partition selection and prevent optimization */
    VOLATILE int partition_selector = 0;
    if (argc > 1) partition_selector = atoi(argv[1]);
    
    /* CASE 0: Gang redundant (default mapping) */
    #pragma acc data copy(arr2d)
    {
        #pragma acc parallel loop gang
        for (i = 0; i < 100; i++) {
            for (j = 0; j < 100; j++) {
                arr2d[i][j] += 1;
            }
        }
    }
    
    /* CASE 1: Gang partitioned */
    #pragma acc data copy(arr2d[0:50][gang])
    {
        #pragma acc parallel loop gang
        for (i = 0; i < 50; i++) {
            for (j = 0; j < 100; j++) {
                arr2d[i][j] += 2;
            }
        }
    }
    
    /* CASE 2: Worker partitioned */
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
    
    /* CASE 3: Gang+Worker partitioned */
    #pragma acc data copy(arr3d[0:25][gang][worker])
    {
        #pragma acc parallel loop gang worker
        for (i = 0; i < 25; i++) {
            for (j = 0; j < 50; j++) {
                for (k = 0; k < 50; k++) {
                    arr3d[i][j][k] += 4;
                }
            }
        }
    }
    
    /* CASE 4: Vector partitioned */
    #pragma acc data copy(dyn_arr[0:100][vector])
    {
        #pragma acc parallel loop gang vector
        for (i = 0; i < 100; i++) {
            #pragma acc loop vector
            for (j = 0; j < 200; j++) {
                dyn_arr[i][j] += 5.0;
            }
        }
    }
    
    /* CASE 5: Gang+Vector partitioned */
    #pragma acc data copy(arr2d[gang][0:50][vector])
    {
        #pragma acc parallel loop gang vector
        for (i = 0; i < 100; i++) {
            #pragma acc loop vector
            for (j = 0; j < 50; j++) {
                arr2d[i][j] += 6;
            }
        }
    }
    
    /* CASE 6: Worker+Vector partitioned */
    #pragma acc data copy(container.matrix[worker][vector])
    {
        #pragma acc parallel loop gang worker vector
        for (i = 0; i < 50; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < 50; j++) {
                container.matrix[i][j] += 7;
            }
        }
    }
    
    /* CASE 7: Fully partitioned (gang+worker+vector) */
    #pragma acc data copy(container.values[0:10][gang][worker][vector])
    {
        #pragma acc parallel loop gang worker vector
        for (i = 0; i < 10; i++) {
            for (j = 0; j < 30; j++) {
                for (k = 0; k < 30; k++) {
                    container.values[i][j][k] += 8.0;
                }
            }
        }
    }
    
    /* Dynamic memory with partition clauses */
    #pragma acc data copy(dyn_mem[0:500][gang])
    {
        #pragma acc parallel loop gang
        for (i = 0; i < 500; i++) {
            dyn_mem[i] += 9;
        }
    }
    
    /* Conditional partition selection to ensure all code paths are considered */
    for (int ptype = 0; ptype < 8; ++ptype) {
        if (partition_selector == ptype) {
            switch (ptype) {
                case 0:
                    #pragma acc data copy(arr2d)
                    {
                        #pragma acc parallel loop gang
                        for (i = 0; i < 10; i++) {
                            for (j = 0; j < 10; j++) {
                                arr2d[i][j] += 10;
                            }
                        }
                    }
                    break;
                case 1:
                    #pragma acc data copy(arr2d[0:20][gang])
                    {
                        #pragma acc parallel loop gang
                        for (i = 0; i < 20; i++) {
                            for (j = 0; j < 20; j++) {
                                arr2d[i][j] += 11;
                            }
                        }
                    }
                    break;
                case 2:
                    #pragma acc data copy(arr2d[0:30][worker])
                    {
                        #pragma acc parallel loop gang worker
                        for (i = 0; i < 30; i++) {
                            #pragma acc loop worker
                            for (j = 0; j < 30; j++) {
                                arr2d[i][j] += 12;
                            }
                        }
                    }
                    break;
                case 3:
                    #pragma acc data copy(arr3d[0:10][gang][worker])
                    {
                        #pragma acc parallel loop gang worker
                        for (i = 0; i < 10; i++) {
                            for (j = 0; j < 10; j++) {
                                for (k = 0; k < 10; k++) {
                                    arr3d[i][j][k] += 13;
                                }
                            }
                        }
                    }
                    break;
                case 4:
                    #pragma acc data copy(dyn_arr[0:50][vector])
                    {
                        #pragma acc parallel loop gang vector
                        for (i = 0; i < 50; i++) {
                            #pragma acc loop vector
                            for (j = 0; j < 50; j++) {
                                dyn_arr[i][j] += 14.0;
                            }
                        }
                    }
                    break;
                case 5:
                    #pragma acc data copy(arr2d[gang][0:40][vector])
                    {
                        #pragma acc parallel loop gang vector
                        for (i = 0; i < 40; i++) {
                            #pragma acc loop vector
                            for (j = 0; j < 40; j++) {
                                arr2d[i][j] += 15;
                            }
                        }
                    }
                    break;
                case 6:
                    #pragma acc data copy(container.vector[worker+vector])
                    {
                        #pragma acc parallel loop gang worker vector
                        for (i = 0; i < 100; i++) {
                            container.vector[i] += 16;
                        }
                    }
                    break;
                case 7:
                    #pragma acc data copy(container.values[0:5][gang][worker][vector])
                    {
                        #pragma acc parallel loop gang worker vector
                        for (i = 0; i < 5; i++) {
                            for (j = 0; j < 5; j++) {
                                for (k = 0; k < 5; k++) {
                                    container.values[i][j][k] += 17.0;
                                }
                            }
                        }
                    }
                    break;
            }
        }
    }
    
    /* Nested constructs with structured data movement */
    #pragma acc enter data copyin(container.matrix[gang][worker])
    
    #pragma acc parallel loop gang worker present(container.matrix[gang][worker])
    for (i = 0; i < 50; i++) {
        #pragma acc loop worker
        for (j = 0; j < 50; j++) {
            container.matrix[i][j] += 18;
        }
    }
    
    #pragma acc exit data copyout(container.matrix[gang][worker])
    
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
    
    for (i = 0; i < 1000; i++) {
        checksum += dyn_mem[i];
    }
    
    for (i = 0; i < 50; i++) {
        for (j = 0; j < 50; j++) {
            checksum += container.matrix[i][j];
        }
    }
    
    for (i = 0; i < 100; i++) {
        checksum += container.vector[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    use_result(checksum);
    
    free(dyn_mem);
    return 0;
}
