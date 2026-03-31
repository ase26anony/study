/* Test program to cover all partitioning states in GCC's omp-oacc-neuter-broadcast pass */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 32
#define P 8

/* Pattern A: Mix of scalars and arrays with different data clauses */
void test_openacc_partitioning(int *result) {
    int i, j, k;
    
    /* Different partitioning states will be assigned to these variables */
    int scalar_gang_redundant;          /* Likely case 0 */
    int scalar_gang_partitioned;        /* Likely case 1 */
    int scalar_worker_partitioned;      /* Likely case 2 */
    int scalar_gang_worker_partitioned; /* Likely case 3 */
    int scalar_vector_partitioned;      /* Likely case 4 */
    
    /* Multi-dimensional arrays for complex partitioning */
    int arr3d[M][P][N];                 /* Will get various partitioning states */
    int arr2d[M][N];                    /* 2D array */
    int arr1d[N];                       /* 1D array */
    
    /* Initialize data */
    for (i = 0; i < N; i++) {
        arr1d[i] = i;
    }
    
    for (i = 0; i < M; i++) {
        for (j = 0; j < N; j++) {
            arr2d[i][j] = i * N + j;
        }
    }
    
    for (i = 0; i < M; i++) {
        for (j = 0; j < P; j++) {
            for (k = 0; k < N; k++) {
                arr3d[i][j][k] = i * P * N + j * N + k;
            }
        }
    }
    
    /* OpenACC parallel region with complex data clauses and usage patterns */
    #pragma acc parallel loop copyin(arr1d, arr2d, arr3d) \
                              copyout(result[0:N]) \
                              private(scalar_gang_redundant) \
                              firstprivate(scalar_gang_partitioned) \
                              reduction(+:scalar_worker_partitioned) \
                              gang vector
    for (i = 0; i < N; i++) {
        int local_gang_var = 0;      /* Additional local variable */
        int local_worker_var = 0;    /* Worker-level variable */
        int local_vector_var = 0;    /* Vector-level variable */
        
        /* Access scalar with gang-level pattern - may become gang redundant */
        scalar_gang_redundant = arr1d[i] % 256;
        
        /* Access scalar with worker-level pattern */
        #pragma acc loop worker
        for (j = 0; j < P; j++) {
            scalar_worker_partitioned += arr3d[0][j][i];
            local_worker_var = j;
            
            /* Nested vector loop for vector partitioning */
            #pragma acc loop vector
            for (k = 0; k < 4; k++) {
                scalar_vector_partitioned = arr3d[0][j][i] + k;
                local_vector_var = k;
                
                /* Complex conditional accessing different array dimensions */
                if (arr1d[i] > 512) {
                    scalar_gang_partitioned = arr2d[j % M][i];
                } else {
                    scalar_gang_worker_partitioned = arr3d[j % M][k % P][i];
                }
            }
        }
        
        /* Combine results from different partitioning levels */
        result[i] = scalar_gang_redundant + scalar_worker_partitioned + 
                   scalar_vector_partitioned + local_gang_var;
    }
}

/* Pattern B: Multi-dimensional array with explicit gang/worker/vector clauses */
void test_openacc_multi_dim() {
    int i, j, k;
    int multi_arr[M][P][N];
    int result_arr[N];
    
    /* Initialize */
    for (i = 0; i < M; i++) {
        for (j = 0; j < P; j++) {
            for (k = 0; k < N; k++) {
                multi_arr[i][j][k] = i * j * k;
            }
        }
    }
    
    /* Complex OpenACC region with explicit gang, worker, vector distribution */
    #pragma acc parallel loop gang copy(multi_arr) copyout(result_arr) \
                              num_gangs(4) num_workers(4) vector_length(32)
    for (i = 0; i < M; i++) {
        int gang_var = 0;
        
        #pragma acc loop worker
        for (j = 0; j < P; j++) {
            int worker_var = 0;
            
            #pragma acc loop vector
            for (k = 0; k < N; k++) {
                int vector_var = 0;
                
                /* Access patterns that trigger different partitioning states */
                gang_var += multi_arr[i][j][k] % 2;      /* Gang partitioned */
                worker_var += multi_arr[i][j][k] % 3;    /* Worker partitioned */
                vector_var = multi_arr[i][j][k] % 5;     /* Vector partitioned */
                
                /* Combined partitioning accesses */
                if ((i + j + k) % 7 == 0) {
                    result_arr[k] = gang_var + worker_var + vector_var;
                }
            }
        }
    }
}

/* Pattern C: Dynamic memory and pointers */
void test_openacc_dynamic() {
    int i, j;
    int *dynamic_arr;
    int **ptr_arr;
    
    /* Allocate dynamic memory */
    dynamic_arr = (int *)malloc(N * sizeof(int));
    ptr_arr = (int **)malloc(M * sizeof(int *));
    
    for (i = 0; i < M; i++) {
        ptr_arr[i] = (int *)malloc(P * sizeof(int));
        for (j = 0; j < P; j++) {
            ptr_arr[i][j] = i * P + j;
        }
    }
    
    for (i = 0; i < N; i++) {
        dynamic_arr[i] = i * 2;
    }
    
    /* OpenACC with dynamic data */
    #pragma acc enter data copyin(dynamic_arr[0:N])
    #pragma acc enter data copyin(ptr_arr[0:M])
    
    for (i = 0; i < M; i++) {
        #pragma acc enter data copyin(ptr_arr[i][0:P])
    }
    
    #pragma acc parallel loop gang worker vector present(dynamic_arr, ptr_arr)
    for (i = 0; i < M; i++) {
        int local_sum = 0;
        
        #pragma acc loop vector
        for (j = 0; j < P; j++) {
            /* Pointer access with complex indexing */
            local_sum += ptr_arr[i][j] + dynamic_arr[(i * P + j) % N];
        }
        
        /* Store result back */
        dynamic_arr[i % N] = local_sum;
    }
    
    #pragma acc exit data copyout(dynamic_arr[0:N])
    
    /* Cleanup */
    for (i = 0; i < M; i++) {
        free(ptr_arr[i]);
    }
    free(ptr_arr);
    free(dynamic_arr);
}

/* Pattern D: OpenMP target regions for additional coverage */
#ifdef _OPENMP
void test_openmp_partitioning() {
    int i, j;
    int omp_arr[N][M];
    int omp_result[N];
    
    /* Initialize */
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            omp_arr[i][j] = i * M + j;
        }
        omp_result[i] = 0;
    }
    
    /* OpenMP target with teams and distribute */
    #pragma omp target teams distribute parallel for \
                map(to: omp_arr) map(from: omp_result) \
                num_teams(4) num_threads(8)
    for (i = 0; i < N; i++) {
        int team_var = 0;      /* Team/gang level */
        int thread_var = 0;    /* Thread/worker level */
        
        #pragma omp simd
        for (j = 0; j < M; j++) {
            int simd_var = 0;  /* SIMD/vector level */
            
            /* Different access patterns for different partitioning */
            team_var += omp_arr[i][j] % 2;
            thread_var += omp_arr[i][j] % 3;
            simd_var = omp_arr[i][j] % 5;
            
            /* Combined result that uses all levels */
            omp_result[i] = team_var + thread_var + simd_var;
        }
    }
    
    /* Verify results */
    int sum = 0;
    for (i = 0; i < N; i++) {
        sum += omp_result[i];
    }
    printf("OpenMP result sum: %d\n", sum);
}
#endif

/* Main function that exercises all patterns */
int main() {
    int i;
    int result[N];
    int checksum = 0;
    
    printf("Testing OpenACC/OpenMP partitioning coverage...\n");
    
    /* Initialize result array */
    for (i = 0; i < N; i++) {
        result[i] = 0;
    }
    
    /* Test 1: OpenACC with mixed variables */
    test_openacc_partitioning(result);
    
    /* Calculate checksum */
    for (i = 0; i < N; i++) {
        checksum += result[i];
    }
    printf("Test 1 checksum: %d\n", checksum);
    
    /* Test 2: Multi-dimensional arrays */
    test_openacc_multi_dim();
    
    /* Test 3: Dynamic memory */
    test_openacc_dynamic();
    
    /* Test 4: OpenMP (if available) */
    #ifdef _OPENMP
    test_openmp_partitioning();
    #endif
    
    /* Final verification */
    if (checksum != 0) {
        printf("All tests completed successfully.\n");
        return 0;
    } else {
        printf("Warning: Zero checksum detected.\n");
        return 1;
    }
}
