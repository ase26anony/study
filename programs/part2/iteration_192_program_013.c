#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 100
#define M 50
#define P 25

/* Struct with array members for requirement #6 */
struct DataContainer {
    int matrix[N][M];
    float vector[N];
    double cube[N][M][P];
};

/* Volatile variable to prevent dead code elimination */
volatile int select_partition = 0;

/* Array mapping integers to partition keywords */
const char* partition_map[] = {
    "",          /* 0: gang redundant (default) */
    "[gang]",    /* 1: gang partitioned */
    "[worker]",  /* 2: worker partitioned */
    "[gang+worker]",  /* 3: gang+worker partitioned */
    "[vector]",  /* 4: vector partitioned */
    "[gang+vector]",  /* 5: gang+vector partitioned */
    "[worker+vector]",  /* 6: worker+vector partitioned */
    "[gang+worker+vector]"  /* 7: fully partitioned */
};

int main() {
    int i, j, k;
    long long checksum = 0;
    
    /* Multi-dimensional arrays for requirement #2 */
    int arr2d[N][M];
    float arr3d[N][M][P];
    
    /* Dynamic data for requirement #3 */
    int *dyn_arr = (int*)malloc(N * M * sizeof(int));
    double *dyn_matrix = (double*)malloc(N * M * P * sizeof(double));
    
    /* Struct instance for requirement #6 */
    struct DataContainer container;
    
    /* Initialize all arrays */
    #pragma acc parallel loop collapse(2) gang worker vector
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            arr2d[i][j] = i * M + j;
        }
    }
    
    #pragma acc parallel loop collapse(3) gang worker vector
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            for (k = 0; k < P; k++) {
                arr3d[i][j][k] = (float)(i * M * P + j * P + k);
            }
        }
    }
    
    /* CASE 0: Gang redundant (default mapping) */
    #pragma acc data copy(arr2d)
    {
        #pragma acc parallel loop gang
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                arr2d[i][j] += 1;
            }
        }
    }
    
    /* CASE 1: Gang partitioned */
    #pragma acc data copy(arr2d[0:N][gang])
    {
        #pragma acc parallel loop gang
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                arr2d[i][j] += 2;
            }
        }
    }
    
    /* CASE 2: Worker partitioned */
    #pragma acc data copy(arr2d[0:N][worker])
    {
        #pragma acc parallel loop gang worker
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                arr2d[i][j] += 3;
            }
        }
    }
    
    /* CASE 3: Gang+Worker partitioned */
    #pragma acc data copy(arr2d[0:N][gang+worker])
    {
        #pragma acc parallel loop gang worker
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                arr2d[i][j] += 4;
            }
        }
    }
    
    /* CASE 4: Vector partitioned */
    #pragma acc data copy(arr2d[0:N][vector])
    {
        #pragma acc parallel loop vector
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                arr2d[i][j] += 5;
            }
        }
    }
    
    /* CASE 5: Gang+Vector partitioned */
    #pragma acc data copy(arr2d[0:N][gang+vector])
    {
        #pragma acc parallel loop gang vector
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                arr2d[i][j] += 6;
            }
        }
    }
    
    /* CASE 6: Worker+Vector partitioned */
    #pragma acc data copy(arr2d[0:N][worker+vector])
    {
        #pragma acc parallel loop worker vector
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                arr2d[i][j] += 7;
            }
        }
    }
    
    /* CASE 7: Fully partitioned (gang+worker+vector) */
    #pragma acc data copy(arr3d[0:N][0:M][0:P][gang+worker+vector])
    {
        #pragma acc parallel loop collapse(3) gang worker vector
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                for (k = 0; k < P; k++) {
                    arr3d[i][j][k] += 8.0f;
                }
            }
        }
    }
    
    /* Dynamic data with partition clauses - requirement #3 */
    #pragma acc data copy(dyn_arr[0:N*M][gang])
    {
        #pragma acc parallel loop gang
        for (i = 0; i < N * M; i++) {
            dyn_arr[i] = i * 2;
        }
    }
    
    /* Multi-dimensional partition on different dimensions - requirement #2 */
    #pragma acc data copy(arr3d[0:50][gang][worker][vector])
    {
        #pragma acc parallel loop collapse(3) gang worker vector
        for (i = 0; i < 50; i++) {
            for (j = 0; j < M; j++) {
                for (k = 0; k < P; k++) {
                    arr3d[i][j][k] *= 2.0f;
                }
            }
        }
    }
    
    /* Struct with array members - requirement #6 */
    #pragma acc data copy(container.matrix[gang], container.vector[vector])
    {
        #pragma acc parallel loop gang
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                container.matrix[i][j] = i + j;
            }
        }
        
        #pragma acc parallel loop vector
        for (i = 0; i < N; i++) {
            container.vector[i] = (float)i;
        }
    }
    
    /* Nested and combined constructs - requirement #4 */
    #pragma acc enter data copyin(dyn_matrix[0:N*M*P][gang+worker])
    
    #pragma acc parallel loop collapse(3) gang worker present(dyn_matrix[gang+worker])
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            for (k = 0; k < P; k++) {
                int idx = i * M * P + j * P + k;
                dyn_matrix[idx] = (double)idx;
            }
        }
    }
    
    #pragma acc exit data copyout(dyn_matrix[0:N*M*P][gang+worker])
    
    /* Conditional partition selection - requirement #5 */
    for (int ptype = 0; ptype < 8; ++ptype) {
        if (select_partition == ptype) {
            switch (ptype) {
                case 0:
                    #pragma acc data copy(arr2d)
                    {
                        #pragma acc parallel loop
                        for (i = 0; i < N; i++) {
                            for (j = 0; j < M; j++) {
                                arr2d[i][j] += 10;
                            }
                        }
                    }
                    break;
                case 1:
                    #pragma acc data copy(arr2d[0:N][gang])
                    {
                        #pragma acc parallel loop gang
                        for (i = 0; i < N; i++) {
                            for (j = 0; j < M; j++) {
                                arr2d[i][j] += 20;
                            }
                        }
                    }
                    break;
                case 2:
                    #pragma acc data copy(arr2d[0:N][worker])
                    {
                        #pragma acc parallel loop worker
                        for (i = 0; i < N; i++) {
                            for (j = 0; j < M; j++) {
                                arr2d[i][j] += 30;
                            }
                        }
                    }
                    break;
                case 3:
                    #pragma acc data copy(arr2d[0:N][gang+worker])
                    {
                        #pragma acc parallel loop gang worker
                        for (i = 0; i < N; i++) {
                            for (j = 0; j < M; j++) {
                                arr2d[i][j] += 40;
                            }
                        }
                    }
                    break;
                case 4:
                    #pragma acc data copy(arr2d[0:N][vector])
                    {
                        #pragma acc parallel loop vector
                        for (i = 0; i < N; i++) {
                            for (j = 0; j < M; j++) {
                                arr2d[i][j] += 50;
                            }
                        }
                    }
                    break;
                case 5:
                    #pragma acc data copy(arr2d[0:N][gang+vector])
                    {
                        #pragma acc parallel loop gang vector
                        for (i = 0; i < N; i++) {
                            for (j = 0; j < M; j++) {
                                arr2d[i][j] += 60;
                            }
                        }
                    }
                    break;
                case 6:
                    #pragma acc data copy(arr2d[0:N][worker+vector])
                    {
                        #pragma acc parallel loop worker vector
                        for (i = 0; i < N; i++) {
                            for (j = 0; j < M; j++) {
                                arr2d[i][j] += 70;
                            }
                        }
                    }
                    break;
                case 7:
                    #pragma acc data copy(arr3d[0:N][0:M][0:P][gang+worker+vector])
                    {
                        #pragma acc parallel loop collapse(3) gang worker vector
                        for (i = 0; i < N; i++) {
                            for (j = 0; j < M; j++) {
                                for (k = 0; k < P; k++) {
                                    arr3d[i][j][k] += 80.0f;
                                }
                            }
                        }
                    }
                    break;
            }
        }
    }
    
    /* Compute checksum */
    #pragma acc parallel loop reduction(+:checksum)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            checksum += arr2d[i][j];
        }
    }
    
    printf("Final checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(dyn_arr);
    free(dyn_matrix);
    
    return 0;
}
