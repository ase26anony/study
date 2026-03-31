#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 100
#define M 50
#define P 25

/* Struct with array members for requirement #6 */
struct DataContainer {
    int grid[N][M];
    float values[N];
    double matrix[M][P];
};

/* Volatile variable to prevent optimization (requirement #5) */
volatile int select_partition = 0;

/* Array mapping integers to partition keywords */
const char* partition_map[] = {
    "",            /* 0: gang redundant (default) */
    "[gang]",      /* 1: gang partitioned */
    "[worker]",    /* 2: worker partitioned */
    "[gang+worker]", /* 3: gang+worker partitioned */
    "[vector]",    /* 4: vector partitioned */
    "[gang+vector]", /* 5: gang+vector partitioned */
    "[worker+vector]", /* 6: worker+vector partitioned */
    "[gang+worker+vector]" /* 7: fully partitioned */
};

int main() {
    int i, j, k;
    int checksum = 0;
    
    /* Multi-dimensional arrays (requirement #2) */
    int arr3d[N][M][P];
    double matrix2d[N][M];
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            matrix2d[i][j] = i * 100 + j;
            for (k = 0; k < P; k++) {
                arr3d[i][j][k] = i * 10000 + j * 100 + k;
            }
        }
    }
    
    /* Struct with array members (requirement #6) */
    struct DataContainer container;
    memset(&container, 0, sizeof(container));
    
    /* Dynamic data (requirement #3) */
    int *dyn_arr = (int*)malloc(N * M * sizeof(int));
    for (i = 0; i < N * M; i++) {
        dyn_arr[i] = i * 2;
    }
    
    printf("Starting OpenACC partition coverage test...\n");
    
    /* Case 0: Gang redundant (default) */
    #pragma acc data copy(arr3d)
    {
        #pragma acc parallel loop gang
        for (i = 0; i < N; i++) {
            #pragma acc loop worker
            for (j = 0; j < M; j++) {
                #pragma acc loop vector
                for (k = 0; k < P; k++) {
                    arr3d[i][j][k] += 1;
                }
            }
        }
    }
    
    /* Case 1: Gang partitioned */
    #pragma acc data copy(arr3d[0:N][gang])
    {
        #pragma acc parallel loop gang
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                for (k = 0; k < P; k++) {
                    arr3d[i][j][k] += 2;
                }
            }
        }
    }
    
    /* Case 2: Worker partitioned */
    #pragma acc data copy(matrix2d[0:N][worker])
    {
        #pragma acc parallel loop gang worker
        for (i = 0; i < N; i++) {
            #pragma acc loop worker
            for (j = 0; j < M; j++) {
                matrix2d[i][j] += 3.0;
            }
        }
    }
    
    /* Case 3: Gang+worker partitioned */
    #pragma acc data copy(arr3d[0:N][gang][worker])
    {
        #pragma acc parallel loop gang worker
        for (i = 0; i < N; i++) {
            #pragma acc loop worker
            for (j = 0; j < M; j++) {
                for (k = 0; k < P; k++) {
                    arr3d[i][j][k] += 4;
                }
            }
        }
    }
    
    /* Case 4: Vector partitioned */
    #pragma acc data copy(dyn_arr[0:N*M][vector])
    {
        #pragma acc parallel loop vector
        for (i = 0; i < N * M; i++) {
            dyn_arr[i] += 5;
        }
    }
    
    /* Case 5: Gang+vector partitioned */
    #pragma acc data copy(container.grid[gang][vector])
    {
        #pragma acc parallel loop gang vector
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                container.grid[i][j] = i * j;
            }
        }
    }
    
    /* Case 6: Worker+vector partitioned */
    #pragma acc data copy(container.values[worker+vector])
    {
        #pragma acc parallel loop worker vector
        for (i = 0; i < N; i++) {
            container.values[i] = i * 1.5f;
        }
    }
    
    /* Case 7: Fully partitioned (gang+worker+vector) */
    #pragma acc data copy(arr3d[gang][worker][vector])
    {
        #pragma acc parallel loop gang worker vector collapse(3)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                for (k = 0; k < P; k++) {
                    arr3d[i][j][k] += 7;
                }
            }
        }
    }
    
    /* Nested and combined constructs (requirement #4) */
    #pragma acc enter data copyin(container.matrix[gang][worker])
    {
        #pragma acc parallel loop gang worker present(container.matrix[gang][worker])
        for (i = 0; i < M; i++) {
            for (j = 0; j < P; j++) {
                container.matrix[i][j] = i * 10.0 + j;
            }
        }
    }
    #pragma acc exit data copyout(container.matrix[gang][worker])
    
    /* Conditional partition selection (requirement #5) */
    for (int ptype = 0; ptype < 8; ++ptype) {
        if (select_partition == ptype) {
            /* This creates code where compiler must handle multiple partition types */
            #pragma acc data copy(matrix2d[0:N][0:M])
            {
                #pragma acc parallel loop gang
                for (i = 0; i < N; i++) {
                    for (j = 0; j < M; j++) {
                        matrix2d[i][j] += ptype;
                    }
                }
            }
        }
    }
    
    /* Compute checksum */
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            checksum += (int)matrix2d[i][j];
            for (k = 0; k < P; k++) {
                checksum += arr3d[i][j][k];
            }
        }
        checksum += (int)container.values[i];
    }
    
    for (i = 0; i < N * M; i++) {
        checksum += dyn_arr[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(dyn_arr);
    
    return 0;
}
