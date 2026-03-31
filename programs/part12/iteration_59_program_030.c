/* test_neuter_broadcast.c
 * Comprehensive test to trigger all partitioning states in GCC's
 * omp-oacc-neuter-broadcast pass switch statement (cases 0-7)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 128
#define DIM1 16
#define DIM2 8
#define DIM3 4

/* Pattern A: Mix of scalars and arrays with different data clauses */
void test_openacc_partitioning() {
    int i, j, k;
    
    /* Different types of variables that may get different partitioning states */
    int scalar_private = 42;                    /* Likely gang redundant (0) */
    int scalar_firstprivate = 100;              /* Likely gang redundant (0) */
    int reduction_sum = 0;                      /* Special reduction handling */
    
    /* 1D arrays - different partitioning based on usage */
    int arr1d[SIZE];                            /* Base array */
    int arr1d_copy[SIZE];                       /* For copyout */
    int arr1d_create[SIZE];                     /* For create */
    
    /* 2D arrays - more complex partitioning */
    int arr2d[DIM1][DIM2];
    
    /* 3D arrays - most complex partitioning scenarios */
    int arr3d[DIM1][DIM2][DIM3];
    
    /* Initialize data */
    for (i = 0; i < SIZE; i++) {
        arr1d[i] = i;
        arr1d_copy[i] = 0;
        arr1d_create[i] = i * 2;
    }
    
    for (i = 0; i < DIM1; i++) {
        for (j = 0; j < DIM2; j++) {
            arr2d[i][j] = i * DIM2 + j;
            for (k = 0; k < DIM3; k++) {
                arr3d[i][j][k] = i * DIM2 * DIM3 + j * DIM3 + k;
            }
        }
    }
    
    /* OpenACC parallel region with complex data clauses and nested loops
     * This should trigger various partitioning analyses */
    #pragma acc parallel loop gang vector \
        copy(arr1d[0:SIZE]) \
        copyout(arr1d_copy[0:SIZE]) \
        create(arr1d_create[0:SIZE]) \
        copy(arr2d[0:DIM1][0:DIM2]) \
        copy(arr3d[0:DIM1][0:DIM2][0:DIM3]) \
        private(scalar_private) \
        firstprivate(scalar_firstprivate) \
        reduction(+:reduction_sum)
    for (i = 0; i < SIZE; i++) {
        int worker_local;
        int vector_local;
        
        /* Access with gang dimension (i) */
        arr1d_copy[i] = arr1d[i] + scalar_firstprivate;
        
        /* Nested loops to create complex access patterns */
        for (j = 0; j < DIM2; j++) {
            worker_local = j;
            
            /* Conditional to create control flow divergence */
            if (i % 2 == 0) {
                /* Access 2D array with gang(i) and worker(j) dimensions */
                arr2d[i % DIM1][j] += worker_local;
                
                for (k = 0; k < DIM3; k++) {
                    vector_local = k;
                    
                    /* Access 3D array with all three dimensions
                     * This may trigger different partitioning states */
                    if (j % 2 == 0) {
                        arr3d[i % DIM1][j][k] += vector_local + worker_local;
                    } else {
                        arr3d[i % DIM1][j][k] -= vector_local;
                    }
                }
            }
        }
        
        /* Reduction operation */
        reduction_sum += arr1d[i];
        
        /* Vector-level operation */
        #pragma acc loop vector
        for (k = 0; k < DIM3; k++) {
            int vector_private = k * 2;
            arr1d_create[i % SIZE] += vector_private;
        }
    }
    
    /* Verify results */
    int check_sum = 0;
    for (i = 0; i < SIZE; i++) {
        check_sum += arr1d[i];
    }
    
    printf("OpenACC test: reduction_sum = %d, expected = %d\n", 
           reduction_sum, check_sum);
}

/* Pattern B: Multi-dimensional array intensive computation */
void test_multidim_partitioning() {
    int i, j, k;
    const int N = 32, M = 16, L = 8;
    
    /* Multi-dimensional arrays that may get different partitioning */
    int cube[N][M][L];
    int matrix[N][M];
    int vector[N];
    
    /* Initialize */
    for (i = 0; i < N; i++) {
        vector[i] = i;
        for (j = 0; j < M; j++) {
            matrix[i][j] = i * M + j;
            for (k = 0; k < L; k++) {
                cube[i][j][k] = i * M * L + j * L + k;
            }
        }
    }
    
    /* Complex OpenACC region with explicit gang/worker/vector clauses */
    #pragma acc parallel loop gang worker vector \
        copy(cube[0:N][0:M][0:L]) \
        copy(matrix[0:N][0:M]) \
        copy(vector[0:N])
    for (i = 0; i < N; i++) {
        int gang_var = i;  /* Varies with gang */
        
        #pragma acc loop worker
        for (j = 0; j < M; j++) {
            int worker_var = j;  /* Varies with worker */
            
            #pragma acc loop vector
            for (k = 0; k < L; k++) {
                int vector_var = k;  /* Varies with vector */
                
                /* Complex access pattern mixing all dimensions */
                cube[i][j][k] = cube[i][j][k] * 2 
                              + gang_var * 100 
                              + worker_var * 10 
                              + vector_var;
                
                /* Conditional access to matrix */
                if (k % 2 == 0) {
                    matrix[i][j] += vector_var;
                }
                
                /* Cross-dimensional access */
                if (i > 0 && j > 0 && k > 0) {
                    cube[i][j][k] += cube[i-1][j-1][k-1] / 2;
                }
            }
            
            /* Worker-level operation on matrix */
            matrix[i][j] += worker_var * 1000;
        }
        
        /* Gang-level operation on vector */
        vector[i] += gang_var * 10000;
    }
    
    printf("Multi-dim test completed\n");
}

/* Pattern C: Pointer-based and dynamic memory */
void test_pointer_partitioning() {
    int i, j;
    const int N = 64;
    
    /* Static arrays */
    int static_arr[N];
    
    /* Dynamic memory */
    int *dynamic_arr = (int*)malloc(N * sizeof(int));
    int **ptr_arr = (int**)malloc(N * sizeof(int*));
    
    if (!dynamic_arr || !ptr_arr) {
        printf("Memory allocation failed\n");
        return;
    }
    
    /* Initialize */
    for (i = 0; i < N; i++) {
        static_arr[i] = i * 2;
        dynamic_arr[i] = i * 3;
        ptr_arr[i] = &dynamic_arr[i];
    }
    
    /* OpenACC with pointer-based accesses */
    #pragma acc parallel loop gang vector \
        copy(static_arr[0:N]) \
        copy(dynamic_arr[0:N]) \
        copy(ptr_arr[0:N]) \
        copy(dynamic_arr[0:N])  /* Explicit copy of pointed-to data */
    for (i = 0; i < N; i++) {
        int idx = i;
        
        /* Access through pointer - complex partitioning analysis */
        *ptr_arr[idx] = static_arr[idx] + dynamic_arr[idx];
        
        /* Nested loop with conditional */
        #pragma acc loop vector
        for (j = 0; j < 4; j++) {
            if (idx % 3 == 0) {
                dynamic_arr[idx] += j;
            } else if (idx % 3 == 1) {
                static_arr[idx] -= j;
            } else {
                *ptr_arr[idx] *= (j + 1);
            }
        }
    }
    
    /* Cleanup */
    free(ptr_arr);
    free(dynamic_arr);
    
    printf("Pointer test completed\n");
}

/* Pattern D: Struct-based data (C++ style in C) */
typedef struct {
    int x;
    float y;
    double z;
    int arr[4];
} DataStruct;

void test_struct_partitioning() {
    int i, j;
    const int N = 48;
    
    DataStruct struct_array[N];
    
    /* Initialize struct array */
    for (i = 0; i < N; i++) {
        struct_array[i].x = i;
        struct_array[i].y = i * 1.5f;
        struct_array[i].z = i * 2.5;
        for (j = 0; j < 4; j++) {
            struct_array[i].arr[j] = i * 10 + j;
        }
    }
    
    /* OpenACC with struct array - members may get different partitioning */
    #pragma acc parallel loop gang worker \
        copy(struct_array[0:N])
    for (i = 0; i < N; i++) {
        /* Access different struct members with different patterns */
        struct_array[i].x *= 2;
        
        #pragma acc loop worker
        for (j = 0; j < 4; j++) {
            /* Worker-level access to struct array member */
            struct_array[i].arr[j] += j * 100;
            
            /* Conditional floating point operations */
            if (struct_array[i].arr[j] % 2 == 0) {
                struct_array[i].y += 1.0f;
            } else {
                struct_array[i].z -= 0.5;
            }
        }
        
        /* Vector-level operation simulated */
        int vector_sum = 0;
        for (j = 0; j < 4; j++) {
            vector_sum += struct_array[i].arr[j];
        }
        struct_array[i].x += vector_sum / 4;
    }
    
    printf("Struct test completed\n");
}

/* Pattern E: Mixed OpenMP for comparison */
#ifdef _OPENMP
void test_openmp_partitioning() {
    int i, j;
    const int N = 56;
    
    int omp_arr[N][N];
    int omp_vec[N];
    
    /* Initialize */
    for (i = 0; i < N; i++) {
        omp_vec[i] = i;
        for (j = 0; j < N; j++) {
            omp_arr[i][j] = i * N + j;
        }
    }
    
    /* OpenMP target with teams and distribute */
    #pragma omp target teams distribute parallel for \
        map(tofrom: omp_arr[0:N][0:N]) \
        map(tofrom: omp_vec[0:N])
    for (i = 0; i < N; i++) {
        int team_var = i;
        
        #pragma omp parallel for
        for (j = 0; j < N; j++) {
            int thread_var = j;
            
            /* Complex access pattern */
            omp_arr[i][j] = omp_arr[i][j] * 3 
                          + team_var * 1000 
                          + thread_var * 10;
            
            /* Reduction-like pattern */
            omp_vec[i] += omp_arr[i][j] % 7;
        }
    }
    
    printf("OpenMP test completed\n");
}
#endif

/* Main function that runs all tests */
int main() {
    printf("Starting comprehensive partitioning state test...\n");
    
    /* Run all test patterns */
    test_openacc_partitioning();
    test_multidim_partitioning();
    test_pointer_partitioning();
    test_struct_partitioning();
    
    #ifdef _OPENMP
    test_openmp_partitioning();
    #endif
    
    printf("All tests completed successfully!\n");
    
    return 0;
}
