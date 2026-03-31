/* test_neuter_broadcast.c
 * Comprehensive test to cover all partitioning states in GCC's
 * omp-oacc-neuter-broadcast.cc switch statement (cases 0-7)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 128
#define M 64
#define P 32

/* Pattern A: Different variable types and storage classes */
static int global_static = 42;  /* May become gang redundant */

/* Pattern B: Multi-dimensional arrays */
int multi_array[2][N][M];

/* Pattern C: Dynamic memory */
int *dynamic_array;

/* Pattern D: Struct with multiple members */
struct DataPoint {
    int id;
    float value;
    double precision;
};

/* Function containing complex OpenACC region */
void test_openacc_partitioning(void) {
    int i, j, k;
    
    /* Local scalars with different usages */
    int gang_redundant_var = 0;      /* Case 0: gang redundant */
    int gang_partitioned_var = 1;    /* Case 1: gang partitioned */
    int worker_partitioned_var = 2;  /* Case 2: worker partitioned */
    int vector_partitioned_var = 4;  /* Case 4: vector partitioned */
    
    /* Arrays with different dimensions and usages */
    int arr1d[N];                    /* Various partitioning states */
    float arr2d[N][M];               /* 2D array for complex partitioning */
    double arr3d[P][N][M];           /* 3D array to trigger full analysis */
    
    /* Struct array */
    struct DataPoint data_points[N];
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        arr1d[i] = i;
        data_points[i].id = i;
        data_points[i].value = i * 0.5f;
        data_points[i].precision = i * 0.25;
        
        for (j = 0; j < M; j++) {
            arr2d[i][j] = (i + j) * 0.1f;
        }
    }
    
    for (i = 0; i < P; i++) {
        for (j = 0; j < N; j++) {
            for (k = 0; k < M; k++) {
                arr3d[i][j][k] = (i + j + k) * 0.01;
            }
        }
    }
    
    /* Complex OpenACC parallel region with multiple data clauses */
    #pragma acc parallel loop copyin(global_static) \
        copy(arr1d[0:N], arr2d[0:N][0:M], arr3d[0:P][0:N][0:M]) \
        copy(data_points[0:N]) \
        copyout(dynamic_array[0:N*M]) \
        create(gang_redundant_var, gang_partitioned_var, \
               worker_partitioned_var, vector_partitioned_var) \
        reduction(+:gang_redundant_var) \
        private(i, j, k)
    for (int idx = 0; idx < N * M; idx++) {
        int gi = idx / M;  /* Gang dimension */
        int wi = idx % M;  /* Worker dimension */
        
        /* Access with different patterns to trigger various partitioning */
        
        /* Gang redundant access (case 0) */
        gang_redundant_var += global_static;
        
        /* Gang partitioned access (case 1) */
        gang_partitioned_var = gi * 2;
        
        /* Worker partitioned access (case 2) */
        worker_partitioned_var = wi * 3;
        
        /* Vector partitioned access (case 4) */
        #pragma acc loop vector
        for (int vi = 0; vi < 32; vi++) {
            vector_partitioned_var = vi * 4;
            
            /* Complex conditional to prevent optimization */
            if (vi % 2 == 0) {
                /* Access 1D array - may trigger gang+worker partitioned (case 3) */
                arr1d[gi] += vi;
                
                /* Access 2D array - may trigger gang+vector partitioned (case 5) */
                arr2d[gi][vi] *= 1.1f;
                
                /* Access 3D array - may trigger worker+vector partitioned (case 6) */
                if (gi < P && wi < N && vi < M) {
                    arr3d[gi][wi][vi] += 0.5;
                }
            } else {
                /* Different access pattern for odd indices */
                arr1d[wi] -= vi;
                
                /* Struct member access - may trigger fully partitioned (case 7) */
                if (gi < N) {
                    data_points[gi].value += vi * 0.01f;
                    data_points[gi].precision -= vi * 0.001;
                }
            }
        }
        
        /* Nested loops with different iteration spaces */
        #pragma acc loop worker
        for (int wj = 0; wj < 16; wj++) {
            int combined = gi + wj;
            
            /* Mixed array accesses to create complex data flow */
            if (combined % 3 == 0) {
                /* Access that might be gang+worker partitioned (case 3) */
                arr2d[gi][wj] = combined * 0.2f;
            }
            
            #pragma acc loop vector
            for (int vj = 0; vj < 8; vj++) {
                /* Access that might be gang+worker+vector partitioned (case 7) */
                if (gi < P && wj < N && vj < M) {
                    arr3d[gi][wj][vj] = (gi * 1000) + (wj * 100) + vj;
                }
                
                /* Dynamic array access */
                dynamic_array[idx] = gi + wj + vj;
            }
        }
    }
    
    /* Verify some results to ensure code isn't dead */
    int sum = 0;
    for (i = 0; i < N; i++) {
        sum += arr1d[i];
    }
    printf("Array sum: %d\n", sum);
}

/* OpenMP version for additional coverage */
void test_openmp_partitioning(void) {
    int i, j;
    int local_var = 0;
    int shared_var = 100;
    int private_var = 200;
    int firstprivate_var = 300;
    int reduction_var = 0;
    
    int omp_arr[N][M];
    float omp_float_arr[P][N];
    
    /* Initialize */
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            omp_arr[i][j] = i * M + j;
        }
    }
    
    for (i = 0; i < P; i++) {
        for (j = 0; j < N; j++) {
            omp_float_arr[i][j] = (i + j) * 0.5f;
        }
    }
    
    /* Complex OpenMP target region with teams and distribute */
    #pragma omp target teams distribute parallel for \
        map(tofrom: omp_arr[0:N][0:M], omp_float_arr[0:P][0:N]) \
        map(to: shared_var) \
        private(private_var) \
        firstprivate(firstprivate_var) \
        reduction(+:reduction_var) \
        collapse(2)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            /* Different partitioning patterns */
            int team_id = omp_get_team_num();
            int thread_id = omp_get_thread_num();
            
            /* Various accesses to trigger different partitioning states */
            omp_arr[i][j] += team_id + thread_id;
            
            if (i < P && j < N) {
                omp_float_arr[i][j] *= (team_id % 2 == 0) ? 1.1f : 0.9f;
            }
            
            /* Conditional with complex data flow */
            if ((i + j) % 3 == 0) {
                private_var = i * j;
                reduction_var += private_var;
            } else if ((i + j) % 5 == 0) {
                firstprivate_var -= j;
            }
            
            /* Vector-like operation */
            #pragma omp simd
            for (int k = 0; k < 4; k++) {
                omp_arr[i][j] += k * shared_var;
            }
        }
    }
    
    /* Verify */
    int total = 0;
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            total += omp_arr[i][j];
        }
    }
    printf("OMP total: %d\n", total);
}

int main(void) {
    /* Allocate dynamic memory */
    dynamic_array = (int*)malloc(N * M * sizeof(int));
    if (!dynamic_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize global data */
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < N; j++) {
            for (int k = 0; k < M; k++) {
                multi_array[i][j][k] = i * 10000 + j * 100 + k;
            }
        }
    }
    
    printf("Testing OpenACC partitioning...\n");
    test_openacc_partitioning();
    
    printf("Testing OpenMP partitioning...\n");
    test_openmp_partitioning();
    
    /* Cleanup */
    free(dynamic_array);
    
    printf("Test completed successfully\n");
    return 0;
}
