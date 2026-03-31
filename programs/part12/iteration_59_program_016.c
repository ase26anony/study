/* test_neuter_broadcast.c - Comprehensive test for GCC omp-oacc-neuter-broadcast pass
 * 
 * This program uses OpenACC constructs to trigger the neuter-broadcast pass
 * and exercise all partitioning states (0-7) in the switch statement.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 64
#define P 32

/* Pattern A: Mix of scalars and arrays with different data clauses */
void test_openacc_partitioning() {
    int i, j, k;
    
    /* Static arrays with different dimensions */
    int arr1d[N];                    /* 1D array */
    int arr2d[M][N];                 /* 2D array */
    int arr3d[P][M][N];              /* 3D array - Pattern B */
    
    /* Scalars with different storage classes */
    int scalar_private;
    int scalar_firstprivate = 42;
    int scalar_reduction = 0;
    int scalar_gang_redundant;
    
    /* Dynamic memory - Pattern C */
    int *dynamic_arr = (int*)malloc(N * sizeof(int));
    
    /* Initialize data */
    for (i = 0; i < N; i++) {
        arr1d[i] = i % 100;
        if (dynamic_arr) dynamic_arr[i] = (i * 2) % 100;
    }
    
    for (i = 0; i < M; i++) {
        for (j = 0; j < N; j++) {
            arr2d[i][j] = (i + j) % 100;
        }
    }
    
    for (i = 0; i < P; i++) {
        for (j = 0; j < M; j++) {
            for (k = 0; k < N; k++) {
                arr3d[i][j][k] = (i + j + k) % 100;
            }
        }
    }
    
    /* Pattern D: Using a struct */
    struct DataPoint {
        int x;
        int y;
        float value;
    };
    struct DataPoint data_array[N];
    for (i = 0; i < N; i++) {
        data_array[i].x = i;
        data_array[i].y = i * 2;
        data_array[i].value = i * 0.5f;
    }
    
    /* Complex OpenACC parallel region with nested loops and conditionals */
    #pragma acc parallel loop gang vector copy(arr1d, arr2d, arr3d) \
        copyin(scalar_firstprivate) copyout(scalar_gang_redundant) \
        create(scalar_private) reduction(+:scalar_reduction) \
        copy(dynamic_arr[0:N]) copy(data_array)
    for (i = 0; i < N; i++) {
        /* Scalar with gang-redundant partitioning (likely case 0) */
        int local_gang_var = scalar_firstprivate;
        
        /* Worker-level computation */
        #pragma acc loop worker
        for (j = 0; j < M; j++) {
            /* Worker-partitioned access (case 2) */
            int worker_local = arr2d[j][i] * 2;
            
            /* Vector-level computation with conditionals */
            #pragma acc loop vector
            for (k = 0; k < P; k++) {
                /* Vector-partitioned access (case 4) */
                int vector_local = arr3d[k][j][i];
                
                /* Complex conditional creating different data flows */
                if (vector_local > 50) {
                    /* Gang+vector partitioned (case 5) */
                    arr1d[i] += vector_local;
                    
                    /* Worker+vector partitioned (case 6) */
                    worker_local += k;
                } else {
                    /* Fully partitioned access (case 7) */
                    arr3d[k][j][i] = vector_local * 3;
                }
                
                /* Reduction variable - special partitioning */
                scalar_reduction += vector_local;
            }
            
            /* Gang+worker partitioned (case 3) */
            arr2d[j][i] = worker_local;
        }
        
        /* Gang-partitioned write (case 1) */
        scalar_gang_redundant = local_gang_var + i;
        
        /* Access struct members with different patterns */
        data_array[i].value = data_array[i].x * data_array[i].y * 0.01f;
        
        /* Dynamic memory access */
        if (dynamic_arr) {
            dynamic_arr[i] = arr1d[i] * 3;
        }
    }
    
    /* Additional parallel region with different clause combinations */
    int shared_var = 100;
    int private_var;
    
    #pragma acc parallel loop gang worker vector \
        copy(shared_var) private(private_var) \
        copy(arr1d) copy(arr2d) copy(arr3d)
    for (i = 0; i < N/2; i++) {
        private_var = i;
        
        /* Nested loops accessing multi-dimensional arrays */
        #pragma acc loop seq
        for (j = 0; j < M/2; j++) {
            #pragma acc loop seq
            for (k = 0; k < P/2; k++) {
                /* Mixed partitioning patterns */
                arr3d[k][j][i] += arr2d[j][i] + arr1d[i] + shared_var + private_var;
            }
        }
    }
    
    /* Verify results to ensure code isn't dead */
    int checksum = 0;
    for (i = 0; i < N; i++) {
        checksum += arr1d[i] % 256;
    }
    printf("Checksum: %d\n", checksum);
    
    if (dynamic_arr) free(dynamic_arr);
}

/* OpenMP version for additional coverage */
#ifdef _OPENMP
void test_openmp_partitioning() {
    int i, j;
    int omp_arr[N][M];
    int omp_scalar = 10;
    int omp_reduction = 0;
    
    /* Initialize */
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            omp_arr[i][j] = (i * j) % 100;
        }
    }
    
    /* OpenMP target region with teams and distribute */
    #pragma omp target teams distribute parallel for \
        map(tofrom: omp_arr) map(to: omp_scalar) reduction(+:omp_reduction) \
        private(j)
    for (i = 0; i < N; i++) {
        int local_var = omp_scalar + i;
        
        #pragma omp simd
        for (j = 0; j < M; j++) {
            /* Various partitioning patterns */
            if (omp_arr[i][j] > 50) {
                omp_arr[i][j] = local_var;
                omp_reduction += 1;
            } else {
                omp_arr[i][j] = omp_arr[i][j] * 2;
                omp_reduction += 2;
            }
        }
    }
    
    printf("OMP Reduction: %d\n", omp_reduction);
}
#endif

/* Main function that calls both test variants */
int main() {
    printf("Testing OpenACC partitioning patterns...\n");
    test_openacc_partitioning();
    
    #ifdef _OPENMP
    printf("\nTesting OpenMP partitioning patterns...\n");
    test_openmp_partitioning();
    #endif
    
    printf("\nTest completed successfully.\n");
    return 0;
}
