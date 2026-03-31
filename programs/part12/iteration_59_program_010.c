/* test_neuter_broadcast.c - Comprehensive test for GCC omp-oacc-neuter-broadcast pass */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 128
#define M 64
#define P 32

/* Pattern A: Mix of scalars and arrays with different data clauses */
void test_openacc_partitioning() {
    int i, j, k;
    int scalar_private = 42;           /* Likely gang redundant (0) */
    int scalar_firstprivate = 100;     /* Likely gang redundant (0) */
    int reduction_sum = 0;             /* Reduction variable */
    
    /* Static arrays with different dimensions and mappings */
    int arr1d[N];                      /* 1D array - gang partitioned (1) */
    int arr2d[N][M];                   /* 2D array - worker partitioned (2) */
    int arr3d[N][M][P];                /* 3D array - complex partitioning */
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        arr1d[i] = i;
        for (j = 0; j < M; j++) {
            arr2d[i][j] = i + j;
            for (k = 0; k < P; k++) {
                arr3d[i][j][k] = i * j * k;
            }
        }
    }
    
    /* Pattern B: Multi-dimensional array access with complex indexing */
    #pragma acc parallel loop gang worker vector \
        copy(arr1d[0:N]) \
        copyin(arr2d[0:N][0:M]) \
        copyout(arr3d[0:N][0:M][0:P]) \
        private(scalar_private) \
        firstprivate(scalar_firstprivate) \
        reduction(+:reduction_sum)
    for (i = 0; i < N; i++) {
        /* Gang-level computation */
        int gang_local = scalar_private + i;
        arr1d[i] = gang_local * 2;
        
        /* Worker-level nested loop */
        #pragma acc loop worker
        for (j = 0; j < M; j++) {
            int worker_local = arr2d[i][j] + gang_local;
            
            /* Vector-level innermost loop with conditional */
            #pragma acc loop vector
            for (k = 0; k < P; k++) {
                /* Complex conditional to prevent optimization */
                if ((i + j + k) % 3 == 0) {
                    arr3d[i][j][k] = worker_local * k + scalar_firstprivate;
                } else if ((i + j + k) % 3 == 1) {
                    arr3d[i][j][k] = worker_local * k - scalar_firstprivate;
                } else {
                    arr3d[i][j][k] = worker_local * k / (scalar_firstprivate + 1);
                }
                
                /* Reduction operation */
                reduction_sum += arr3d[i][j][k] % 100;
            }
        }
    }
    
    printf("OpenACC reduction sum: %d\n", reduction_sum);
}

/* Pattern C: Variable-length data and pointers */
void test_dynamic_memory_partitioning() {
    int i, j;
    int *dynamic_arr;
    int **jagged_arr;
    
    /* Dynamic 1D array */
    dynamic_arr = (int*)malloc(N * M * sizeof(int));
    
    /* Jagged 2D array for complex access patterns */
    jagged_arr = (int**)malloc(N * sizeof(int*));
    for (i = 0; i < N; i++) {
        jagged_arr[i] = (int*)malloc((i % 10 + 1) * sizeof(int));
        for (j = 0; j < (i % 10 + 1); j++) {
            jagged_arr[i][j] = i * 100 + j;
        }
    }
    
    /* Initialize dynamic array */
    for (i = 0; i < N * M; i++) {
        dynamic_arr[i] = i * 2;
    }
    
    /* OpenACC with dynamic data - may trigger different partitioning */
    #pragma acc parallel loop gang vector \
        copy(dynamic_arr[0:N*M]) \
        present_or_copyin(jagged_arr[0:N])
    for (i = 0; i < N; i++) {
        int gang_vec_local = dynamic_arr[i];
        
        /* Access jagged array - complex pattern */
        #pragma acc loop vector
        for (j = 0; j < (i % 10 + 1); j++) {
            dynamic_arr[i * M + j] += jagged_arr[i][j] + gang_vec_local;
        }
    }
    
    /* Cleanup */
    free(dynamic_arr);
    for (i = 0; i < N; i++) {
        free(jagged_arr[i]);
    }
    free(jagged_arr);
}

/* Pattern D: Struct-based data for aggregate partitioning */
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int x;
    float y;
    double z;
    int arr[4];
} ComplexData;

void test_struct_partitioning() {
    int i, j;
    ComplexData struct_arr[N];
    
    /* Initialize struct array */
    for (i = 0; i < N; i++) {
        struct_arr[i].x = i;
        struct_arr[i].y = i * 0.5f;
        struct_arr[i].z = i * 0.25;
        for (j = 0; j < 4; j++) {
            struct_arr[i].arr[j] = i * 10 + j;
        }
    }
    
    /* OpenACC with struct array - members may have different partitioning */
    #pragma acc parallel loop gang worker \
        copy(struct_arr[0:N])
    for (i = 0; i < N; i++) {
        /* Access different struct members in different loops */
        struct_arr[i].x = struct_arr[i].x * 2 + i;
        
        #pragma acc loop worker
        for (j = 0; j < 4; j++) {
            struct_arr[i].arr[j] = struct_arr[i].arr[j] + struct_arr[i].x;
            struct_arr[i].y = struct_arr[i].y + struct_arr[i].arr[j] * 0.1f;
        }
        
        struct_arr[i].z = struct_arr[i].z * 1.5;
    }
}

#ifdef __cplusplus
}
#endif

/* OpenMP version to trigger different code paths */
void test_openmp_partitioning() {
    int i, j, k;
    int omp_arr1[N];
    int omp_arr2[N][M];
    double omp_arr3[N][M][P];
    
    /* Initialize */
    for (i = 0; i < N; i++) {
        omp_arr1[i] = i * 3;
        for (j = 0; j < M; j++) {
            omp_arr2[i][j] = i * j;
            for (k = 0; k < P; k++) {
                omp_arr3[i][j][k] = (double)(i + j + k) / 100.0;
            }
        }
    }
    
    /* OpenMP target with teams and distribute - similar to OpenACC gangs/workers */
    #pragma omp target teams distribute parallel for \
        map(tofrom: omp_arr1) \
        map(to: omp_arr2) \
        map(from: omp_arr3) \
        private(i, j, k)
    for (i = 0; i < N; i++) {
        int team_id = omp_get_team_num();
        int thread_id = omp_get_thread_num();
        
        /* Multi-level partitioning hints */
        omp_arr1[i] = omp_arr1[i] + team_id * 1000 + thread_id;
        
        #pragma omp parallel for private(j, k)
        for (j = 0; j < M; j++) {
            for (k = 0; k < P; k++) {
                /* Complex computation with conditionals */
                if (omp_arr2[i][j] > 100) {
                    omp_arr3[i][j][k] = omp_arr3[i][j][k] * 2.0 + (double)thread_id;
                } else {
                    omp_arr3[i][j][k] = omp_arr3[i][j][k] / 2.0 - (double)team_id;
                }
                
                /* Additional branching for control flow complexity */
                switch ((i + j + k) % 4) {
                    case 0: omp_arr3[i][j][k] += 1.0; break;
                    case 1: omp_arr3[i][j][k] -= 1.0; break;
                    case 2: omp_arr3[i][j][k] *= 1.1; break;
                    case 3: omp_arr3[i][j][k] /= 1.1; break;
                }
            }
        }
    }
    
    /* Verify some results */
    double check_sum = 0.0;
    for (i = 0; i < N; i += 8) {
        for (j = 0; j < M; j += 8) {
            for (k = 0; k < P; k += 8) {
                check_sum += omp_arr3[i][j][k];
            }
        }
    }
    printf("OpenMP check sum: %f\n", check_sum);
}

/* Main driver that calls all test patterns */
int main() {
    printf("Testing GCC neuter-broadcast pass coverage...\n");
    
    /* Test OpenACC patterns */
    test_openacc_partitioning();
    test_dynamic_memory_partitioning();
    test_struct_partitioning();
    
    /* Test OpenMP pattern */
    test_openmp_partitioning();
    
    printf("All tests completed.\n");
    return 0;
}
