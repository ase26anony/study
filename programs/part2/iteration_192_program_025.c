#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 100
#define M 100
#define P 100

/* Volatile variables to prevent optimization */
volatile int use_gang = 1;
volatile int use_worker = 1;
volatile int use_vector = 1;
volatile int use_combined = 1;

/* Struct with array members */
struct DataContainer {
    int array1[N][M];
    int array2[N][M];
    int *dynamic_arr;
};

/* Function to initialize arrays */
void init_arrays(struct DataContainer *dc) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            dc->array1[i][j] = i * M + j;
            dc->array2[i][j] = (i * M + j) * 2;
        }
    }
    dc->dynamic_arr = (int*)malloc(N * M * sizeof(int));
    for (int i = 0; i < N * M; i++) {
        dc->dynamic_arr[i] = i * 3;
    }
}

/* Function to compute checksum */
int compute_checksum(struct DataContainer *dc) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            sum += dc->array1[i][j] + dc->array2[i][j];
        }
    }
    for (int i = 0; i < N * M; i++) {
        sum += dc->dynamic_arr[i];
    }
    return sum;
}

int main() {
    struct DataContainer dc;
    int multi_dim[P][N][M];
    int *dyn_3d = (int*)malloc(P * N * M * sizeof(int));
    
    /* Initialize all data */
    init_arrays(&dc);
    
    /* Initialize multi-dimensional arrays */
    for (int i = 0; i < P; i++) {
        for (int j = 0; j < N; j++) {
            for (int k = 0; k < M; k++) {
                multi_dim[i][j][k] = i * N * M + j * M + k;
            }
        }
    }
    
    for (int i = 0; i < P * N * M; i++) {
        dyn_3d[i] = i * 5;
    }
    
    int checksum = 0;
    
    /* ========== CASE 0: gang redundant ========== */
    if (use_gang) {
        #pragma acc data copy(dc.array1[0:N][0:M])
        {
            #pragma acc parallel loop gang
            for (int i = 0; i < N; i++) {
                #pragma acc loop worker vector
                for (int j = 0; j < M; j++) {
                    dc.array1[i][j] += 1;
                }
            }
        }
    }
    
    /* ========== CASE 1: gang partitioned ========== */
    if (use_gang) {
        #pragma acc data copy(dc.array2[0:N][gang])
        {
            #pragma acc parallel loop gang
            for (int i = 0; i < N; i++) {
                #pragma acc loop worker vector
                for (int j = 0; j < M; j++) {
                    dc.array2[i][j] += 2;
                }
            }
        }
    }
    
    /* ========== CASE 2: worker partitioned ========== */
    if (use_worker) {
        #pragma acc data copy(dc.array1[0:N][worker])
        {
            #pragma acc parallel loop gang
            for (int i = 0; i < N; i++) {
                #pragma acc loop worker
                for (int j = 0; j < M; j++) {
                    dc.array1[i][j] += 3;
                }
            }
        }
    }
    
    /* ========== CASE 3: gang+worker partitioned ========== */
    if (use_combined) {
        #pragma acc data copy(multi_dim[0:P][gang][worker])
        {
            #pragma acc parallel loop gang
            for (int i = 0; i < P; i++) {
                #pragma acc loop worker
                for (int j = 0; j < N; j++) {
                    #pragma acc loop vector
                    for (int k = 0; k < M; k++) {
                        multi_dim[i][j][k] += 4;
                    }
                }
            }
        }
    }
    
    /* ========== CASE 4: vector partitioned ========== */
    if (use_vector) {
        #pragma acc data copy(dc.dynamic_arr[0:N*M][vector])
        {
            #pragma acc parallel loop vector
            for (int i = 0; i < N * M; i++) {
                dc.dynamic_arr[i] += 5;
            }
        }
    }
    
    /* ========== CASE 5: gang+vector partitioned ========== */
    if (use_gang && use_vector) {
        #pragma acc data copy(multi_dim[0:P][gang][vector])
        {
            #pragma acc parallel loop gang vector
            for (int i = 0; i < P; i++) {
                for (int j = 0; j < N * M; j++) {
                    int idx = i * N * M + j;
                    if (idx < P * N * M) {
                        dyn_3d[idx] += 6;
                    }
                }
            }
        }
    }
    
    /* ========== CASE 6: worker+vector partitioned ========== */
    if (use_worker && use_vector) {
        #pragma acc data copy(dc.array2[0:N][worker+vector])
        {
            #pragma acc parallel loop gang
            for (int i = 0; i < N; i++) {
                #pragma acc loop worker vector
                for (int j = 0; j < M; j++) {
                    dc.array2[i][j] += 7;
                }
            }
        }
    }
    
    /* ========== CASE 7: fully partitioned ========== */
    if (use_gang && use_worker && use_vector) {
        #pragma acc data copy(multi_dim[0:P][gang][worker][vector])
        {
            #pragma acc parallel loop gang worker vector
            for (int i = 0; i < P * N * M; i++) {
                int idx = i;
                if (idx < P * N * M) {
                    dyn_3d[idx] += 8;
                }
            }
        }
    }
    
    /* ========== Test struct with array members ========== */
    if (use_combined) {
        #pragma acc data copy(dc.array1[gang], dc.array2[vector])
        {
            #pragma acc parallel loop gang
            for (int i = 0; i < N; i++) {
                #pragma acc loop vector
                for (int j = 0; j < M; j++) {
                    dc.array1[i][j] += 9;
                    dc.array2[i][j] += 10;
                }
            }
        }
    }
    
    /* ========== Nested constructs ========== */
    if (use_gang) {
        #pragma acc enter data copyin(dc.array1[0:N][gang])
        {
            #pragma acc parallel loop gang present(dc.array1[gang])
            for (int i = 0; i < N; i++) {
                #pragma acc loop worker vector
                for (int j = 0; j < M; j++) {
                    dc.array1[i][j] += 11;
                }
            }
        }
        #pragma acc exit data copyout(dc.array1[0:N][gang])
    }
    
    /* ========== Conditional partition selection ========== */
    for (int iter = 0; iter < 3; iter++) {
        if (use_gang && (iter % 3 == 0)) {
            #pragma acc data copy(dc.array1[0:N][gang])
            {
                #pragma acc parallel loop gang
                for (int i = 0; i < N; i++) {
                    for (int j = 0; j < M; j++) {
                        dc.array1[i][j] += 12;
                    }
                }
            }
        } else if (use_worker && (iter % 3 == 1)) {
            #pragma acc data copy(dc.array2[0:N][worker])
            {
                #pragma acc parallel loop gang
                for (int i = 0; i < N; i++) {
                    #pragma acc loop worker
                    for (int j = 0; j < M; j++) {
                        dc.array2[i][j] += 13;
                    }
                }
            }
        } else if (use_vector && (iter % 3 == 2)) {
            #pragma acc data copy(dc.dynamic_arr[0:N*M][vector])
            {
                #pragma acc parallel loop vector
                for (int i = 0; i < N * M; i++) {
                    dc.dynamic_arr[i] += 14;
                }
            }
        }
    }
    
    /* Compute final checksum */
    checksum = compute_checksum(&dc);
    
    /* Add multi-dimensional array contributions */
    for (int i = 0; i < P; i++) {
        for (int j = 0; j < N; j++) {
            for (int k = 0; k < M; k++) {
                checksum += multi_dim[i][j][k];
            }
        }
    }
    
    for (int i = 0; i < P * N * M; i++) {
        checksum += dyn_3d[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(dc.dynamic_arr);
    free(dyn_3d);
    
    return 0;
}
