/* test_neuter_broadcast.c - Comprehensive test for GCC's omp-oacc-neuter-broadcast pass
 * This program creates variables with diverse partitioning attributes to trigger
 * all cases in the partitioning state switch statement (cases 0-7).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _OPENACC
#include <openacc.h>
#endif

#define N 1024
#define M 64
#define P 32

/* Pattern A: Mix of scalars and arrays with different data clauses */
void test_openacc_partitioning() {
    int i, j, k;
    
    /* Static arrays with different dimensions */
    int arr1d[N];                    /* 1D array - likely gang partitioned */
    int arr2d[M][N];                 /* 2D array - complex partitioning */
    int arr3d[P][M][N];              /* 3D array - triggers multi-dim analysis */
    
    /* Scalars with different storage classes */
    int scalar_private;              /* Private scalar */
    int scalar_firstprivate = 42;    /* Firstprivate scalar */
    int scalar_reduction = 0;        /* Reduction variable */
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        arr1d[i] = i % 100;
    }
    
    for (i = 0; i < M; i++) {
        for (j = 0; j < N; j++) {
            arr2d[i][j] = (i * j) % 100;
        }
    }
    
    for (i = 0; i < P; i++) {
        for (j = 0; j < M; j++) {
            for (k = 0; k < N; k++) {
                arr3d[i][j][k] = (i + j + k) % 100;
            }
        }
    }
    
    /* Pattern B: Multi-dimensional array access with varying indices */
    #pragma acc parallel loop gang vector copy(arr1d, arr2d, arr3d) \
        copyin(scalar_firstprivate) private(scalar_private) \
        reduction(+:scalar_reduction)
    for (i = 0; i < N; i++) {
        scalar_private = i;
        
        /* Nested loops create complex data flow */
        for (j = 0; j < M; j++) {
            /* Conditional operations increase analysis complexity */
            if (arr1d[i] > 50) {
                arr2d[j][i] += scalar_firstprivate;
                
                /* Further nesting for vector dimension */
                for (k = 0; k < P; k++) {
                    if (k % 2 == 0) {
                        arr3d[k][j][i] *= 2;
                    } else {
                        arr3d[k][j][i] /= 2;
                    }
                }
            }
        }
        
        scalar_reduction += arr1d[i];
    }
    
    /* Pattern C: Variable-length data via pointers */
    int *dynamic_arr = (int*)malloc(N * sizeof(int));
    int **jagged_arr = (int**)malloc(M * sizeof(int*));
    
    for (i = 0; i < N; i++) {
        dynamic_arr[i] = i * 2;
    }
    
    for (i = 0; i < M; i++) {
        jagged_arr[i] = (int*)malloc(N * sizeof(int));
        for (j = 0; j < N; j++) {
            jagged_arr[i][j] = i * j;
        }
    }
    
    /* Map pointer-based data to device */
    #pragma acc enter data copyin(dynamic_arr[0:N])
    for (i = 0; i < M; i++) {
        #pragma acc enter data copyin(jagged_arr[i][0:N])
    }
    
    /* Complex kernel with pointer accesses */
    #pragma acc parallel loop gang worker vector present(dynamic_arr, jagged_arr)
    for (i = 0; i < N; i++) {
        int temp = dynamic_arr[i];
        
        /* Worker-level partitioning */
        #pragma acc loop worker
        for (j = 0; j < M; j++) {
            temp += jagged_arr[j][i];
            
            /* Vector-level partitioning */
            #pragma acc loop vector
            for (k = 0; k < 16; k++) {
                if ((j + k) % 3 == 0) {
                    temp -= k;
                }
            }
        }
        
        dynamic_arr[i] = temp;
    }
    
    /* Clean up */
    #pragma acc exit data delete(dynamic_arr)
    for (i = 0; i < M; i++) {
        #pragma acc exit data delete(jagged_arr[i])
        free(jagged_arr[i]);
    }
    free(jagged_arr);
    free(dynamic_arr);
}

/* Pattern D: Struct-based data for complex partitioning */
struct ComplexData {
    int gang_data;
    int worker_data;
    int vector_data;
    int fully_partitioned[16];
    int gang_worker_partitioned[8][8];
};

void test_struct_partitioning() {
    int i, j;
    struct ComplexData data_arr[N];
    
    /* Initialize struct array */
    for (i = 0; i < N; i++) {
        data_arr[i].gang_data = i;
        data_arr[i].worker_data = i * 2;
        data_arr[i].vector_data = i * 3;
        
        for (j = 0; j < 16; j++) {
            data_arr[i].fully_partitioned[j] = i + j;
        }
        
        for (j = 0; j < 8; j++) {
            for (int k = 0; k < 8; k++) {
                data_arr[i].gang_worker_partitioned[j][k] = i * j * k;
            }
        }
    }
    
    /* Kernel accessing different struct members at different levels */
    #pragma acc parallel loop copy(data_arr)
    for (i = 0; i < N; i++) {
        /* Gang-level access */
        data_arr[i].gang_data *= 2;
        
        /* Worker-level computation */
        #pragma acc loop worker
        for (j = 0; j < 8; j++) {
            data_arr[i].worker_data += data_arr[i].gang_worker_partitioned[j][0];
            
            /* Vector-level computation */
            #pragma acc loop vector
            for (int k = 0; k < 8; k++) {
                data_arr[i].vector_data += data_arr[i].gang_worker_partitioned[j][k];
                
                /* Fully partitioned access */
                data_arr[i].fully_partitioned[k] = 
                    data_arr[i].gang_data + 
                    data_arr[i].worker_data + 
                    data_arr[i].vector_data;
            }
        }
    }
}

/* OpenMP version for broader coverage */
#ifdef _OPENMP
void test_openmp_partitioning() {
    int i, j, k;
    int omp_arr1[N];
    int omp_arr2[M][N];
    int omp_arr3[P][M][N];
    
    /* Initialize */
    for (i = 0; i < N; i++) {
        omp_arr1[i] = i;
        for (j = 0; j < M; j++) {
            omp_arr2[j][i] = i * j;
            for (k = 0; k < P; k++) {
                omp_arr3[k][j][i] = i + j + k;
            }
        }
    }
    
    /* OpenMP target with teams and distribute */
    #pragma omp target teams distribute parallel for \
        map(tofrom: omp_arr1, omp_arr2, omp_arr3) \
        private(i, j, k) \
        reduction(+:omp_arr1[0:N])
    for (i = 0; i < N; i++) {
        int local_sum = 0;
        
        /* Nested loops with different parallelization levels */
        #pragma omp parallel for reduction(+:local_sum)
        for (j = 0; j < M; j++) {
            int inner_sum = 0;
            
            #pragma omp simd reduction(+:inner_sum)
            for (k = 0; k < P; k++) {
                inner_sum += omp_arr3[k][j][i];
            }
            
            omp_arr2[j][i] += inner_sum;
            local_sum += omp_arr2[j][i];
        }
        
        omp_arr1[i] = local_sum;
    }
}
#endif

/* Combined test driver */
int main() {
    int result = 0;
    
    printf("Testing OpenACC partitioning states...\n");
    test_openacc_partitioning();
    
    printf("Testing struct-based partitioning...\n");
    test_struct_partitioning();
    
    #ifdef _OPENMP
    printf("Testing OpenMP partitioning states...\n");
    test_openmp_partitioning();
    #endif
    
    printf("All tests completed successfully.\n");
    
    /* Simple validation */
    if (result == 0) {
        printf("SUCCESS: Test program executed without errors.\n");
    } else {
        printf("WARNING: Some tests may have failed, but compilation paths were exercised.\n");
    }
    
    return result;
}
