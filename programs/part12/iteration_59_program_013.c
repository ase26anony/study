/* Test program to cover partitioning state mapping in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -foffload=disable -fdump-tree-all -fprofile-arcs -ftest-coverage test.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 64
#define P 32

/* Function containing complex OpenACC regions with various data partitioning patterns */
void test_partitioning_patterns(int n, int m, int p) {
    /* Pattern A: Different scalar and array types with explicit clauses */
    int scalar_private = 42;                    /* Likely gang redundant (0) */
    int scalar_firstprivate = 100;              /* Likely gang redundant (0) */
    int reduction_sum = 0;                      /* Reduction variable */
    
    /* Pattern B: Multi-dimensional arrays with different access patterns */
    int arr_3d[M][P][N];                        /* 3D array for complex partitioning */
    int arr_2d[M][N];                           /* 2D array */
    int arr_1d[N];                              /* 1D array */
    
    /* Pattern C: Pointer-based dynamic memory */
    int *dynamic_arr = (int*)malloc(N * sizeof(int));
    int **jagged_arr = (int**)malloc(M * sizeof(int*));
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr_1d[i] = i % 100;
        if (dynamic_arr) dynamic_arr[i] = i % 50;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr_2d[i][j] = (i * j) % 100;
        }
        if (jagged_arr) {
            jagged_arr[i] = (int*)malloc(P * sizeof(int));
            for (int k = 0; k < P; k++) {
                jagged_arr[i][k] = (i + k) % 100;
            }
        }
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            for (int k = 0; k < N; k++) {
                arr_3d[i][j][k] = (i + j + k) % 100;
            }
        }
    }
    
    /* OpenACC region 1: Complex parallel loop with multiple data clauses */
    #pragma acc parallel loop gang vector_length(32) \
        copy(arr_1d[0:N]) \
        copyin(arr_2d[0:M][0:N]) \
        copyout(arr_3d[0:M][0:P][0:N]) \
        private(scalar_private) \
        firstprivate(scalar_firstprivate) \
        reduction(+:reduction_sum) \
        create(dynamic_arr[0:N])
    for (int i = 0; i < n; i++) {
        /* Nested loops to create complex data flow */
        int local_var = scalar_private + i;     /* Private variable */
        
        /* Access 1D array - may be vector partitioned (4) */
        arr_1d[i] = arr_1d[i] * 2;
        
        /* Conditional access to create varying partitioning needs */
        if (i % 2 == 0) {
            /* Access 2D array - may be gang partitioned (1) */
            for (int j = 0; j < m; j++) {
                arr_2d[j % M][i] += local_var;
            }
        } else {
            /* Access 3D array - may be fully partitioned (7) */
            for (int j = 0; j < p; j++) {
                for (int k = 0; k < 10; k++) {
                    arr_3d[i % M][j % P][k] = local_var + k;
                }
            }
        }
        
        /* Access dynamic memory - different partitioning */
        if (dynamic_arr) {
            dynamic_arr[i] = i * 3;
        }
        
        reduction_sum += local_var;
    }
    
    /* OpenACC region 2: Kernels construct with worker-level parallelism */
    #pragma acc kernels \
        copy(arr_1d[0:N]) \
        copy(arr_2d[0:M][0:N]) \
        present(arr_3d[0:M][0:P][0:N])
    {
        /* Worker-level loops */
        #pragma acc loop worker
        for (int i = 0; i < m; i++) {
            int worker_local = i * 2;           /* Worker partitioned (2) */
            
            #pragma acc loop vector
            for (int j = 0; j < n; j++) {
                /* Worker+vector partitioned (6) */
                arr_2d[i][j] += worker_local + j;
                
                /* Nested conditional for gang+worker partitioning (3) */
                if (j % 3 == 0) {
                    #pragma acc loop gang
                    for (int k = 0; k < p; k++) {
                        /* Gang+worker partitioned (3) */
                        arr_3d[k % M][i][j] = arr_2d[i][j] * k;
                    }
                }
            }
        }
        
        /* Additional loop for gang+vector partitioning (5) */
        #pragma acc loop gang vector
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                /* Gang+vector partitioned (5) */
                arr_1d[i] += arr_2d[j % M][i];
            }
        }
    }
    
    /* OpenACC region 3: Combined gang/worker/vector with collapse */
    #pragma acc parallel loop collapse(2) gang worker vector \
        copy(arr_1d[0:N]) \
        copy(arr_2d[0:M][0:N])
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            /* Complex expression mixing different array accesses */
            int idx = (i * n + j) % N;
            arr_1d[idx] = arr_2d[i][j] + arr_1d[idx];
            
            /* Conditional that creates different live ranges */
            if (arr_1d[idx] > 1000) {
                arr_1d[idx] = 1000;
            }
        }
    }
    
    /* Clean up dynamic memory */
    if (dynamic_arr) free(dynamic_arr);
    if (jagged_arr) {
        for (int i = 0; i < M; i++) {
            if (jagged_arr[i]) free(jagged_arr[i]);
        }
        free(jagged_arr);
    }
    
    printf("Reduction sum: %d\n", reduction_sum);
}

/* OpenMP version for additional coverage */
#ifdef _OPENMP
void test_omp_partitioning(int n) {
    int arr1[N], arr2[N][M];
    int private_var = 10;
    int firstprivate_var = 20;
    int reduction_var = 0;
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        arr1[i] = i;
        for (int j = 0; j < M; j++) {
            arr2[i][j] = i * j;
        }
    }
    
    /* OpenMP target region with teams and distribute */
    #pragma omp target teams distribute parallel for \
        map(tofrom: arr1[0:N]) \
        map(to: arr2[0:N][0:M]) \
        private(private_var) \
        firstprivate(firstprivate_var) \
        reduction(+:reduction_var)
    for (int i = 0; i < n; i++) {
        private_var = i;
        
        /* Nested loops to trigger different partitioning */
        for (int j = 0; j < M; j++) {
            arr1[i] += arr2[i][j] + private_var + firstprivate_var;
        }
        
        reduction_var += arr1[i];
        
        /* Conditional with different array accesses */
        if (i % 4 == 0) {
            for (int k = 0; k < 10; k++) {
                arr1[(i + k) % N] = k;
            }
        }
    }
    
    printf("OMP reduction: %d\n", reduction_var);
}
#endif

/* Main function with validation */
int main() {
    printf("Testing OpenACC partitioning patterns...\n");
    
    /* Call with different sizes to trigger various optimizations */
    test_partitioning_patterns(N, M, P);
    
    #ifdef _OPENMP
    printf("\nTesting OpenMP partitioning patterns...\n");
    test_omp_partitioning(N);
    #endif
    
    /* Simple validation */
    int validation_sum = 0;
    for (int i = 0; i < 100; i++) {
        validation_sum += i;
    }
    
    printf("Validation sum: %d\n", validation_sum);
    
    if (validation_sum == 4950) {
        printf("Test PASSED\n");
        return 0;
    } else {
        printf("Test FAILED\n");
        return 1;
    }
}
