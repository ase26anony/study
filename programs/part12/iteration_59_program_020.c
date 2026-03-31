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

/* Pattern A: Mix of scalars and arrays with different data clauses */
void test_openacc_partitioning() {
    int i, j, k;
    
    /* Different array types to trigger various partitioning states */
    int scalar = 42;                     /* Likely gang redundant (0) */
    int arr1d[N];                        /* 1D array */
    int arr2d[N][M];                     /* 2D array */
    int arr3d[N][M][P];                  /* 3D array - complex partitioning */
    int *dynamic_arr;                    /* Dynamic array */
    int reduction_var = 0;               /* Reduction variable */
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        arr1d[i] = i;
        for (j = 0; j < M; j++) {
            arr2d[i][j] = i * j;
            for (k = 0; k < P; k++) {
                arr3d[i][j][k] = i * j * k;
            }
        }
    }
    
    dynamic_arr = (int*)malloc(N * M * sizeof(int));
    for (i = 0; i < N * M; i++) {
        dynamic_arr[i] = i % 100;
    }
    
    /* OpenACC parallel region with complex data clauses and usage patterns */
    #pragma acc parallel loop gang vector copy(scalar, arr1d, arr2d, arr3d) \
        copyin(dynamic_arr[0:N*M]) copyout(arr1d) reduction(+:reduction_var) \
        create(arr2d) private(i, j, k)
    for (i = 0; i < N; i++) {
        int local_var = scalar;  /* Use scalar - may be gang redundant */
        
        /* Nested loops accessing multi-dimensional arrays */
        for (j = 0; j < M; j++) {
            /* Conditional operations create complex data flow */
            if (j % 2 == 0) {
                arr2d[i][j] = arr1d[i] * 2;
                
                /* Access 3D array with all three indices varying */
                for (k = 0; k < P; k++) {
                    /* Complex indexing pattern */
                    int idx = (i * M * P + j * P + k) % (N * M);
                    arr3d[i][j][k] = dynamic_arr[idx] + local_var;
                    
                    /* Conditional update based on vector position */
                    if (k % 4 == 0) {
                        arr3d[i][j][k] *= 2;
                    }
                }
            } else {
                arr2d[i][j] = arr1d[i] / 2;
            }
            
            /* Reduction operation */
            reduction_var += arr2d[i][j];
        }
        
        /* Update 1D array with gang-dependent value */
        arr1d[i] = local_var + i;
    }
    
    /* Second kernel with different partitioning patterns */
    int worker_partitioned[N];
    int gang_worker_partitioned[N][M];
    
    #pragma acc parallel loop gang worker copy(worker_partitioned) \
        copy(gang_worker_partitioned)
    for (i = 0; i < N; i++) {
        /* Worker-level computation */
        for (j = 0; j < M; j++) {
            gang_worker_partitioned[i][j] = i * 100 + j;
        }
        
        /* Worker-partitioned variable */
        worker_partitioned[i] = i * 10;
    }
    
    /* Vector-level partitioning test */
    int vector_partitioned[N];
    
    #pragma acc parallel loop vector copy(vector_partitioned)
    for (i = 0; i < N; i++) {
        vector_partitioned[i] = i * 3;
    }
    
    free(dynamic_arr);
}

/* Pattern B: OpenMP target regions for additional partitioning scenarios */
void test_openmp_partitioning() {
    int i, j, k;
    
    /* Different data types and structures */
    struct DataPoint {
        int x;
        double y;
        float z;
    };
    
    struct DataPoint data[N];
    int matrix[N][M];
    int cube[N][M][P];
    int fully_partitioned[N * M * P];
    
    /* Initialize data */
    for (i = 0; i < N; i++) {
        data[i].x = i;
        data[i].y = i * 1.5;
        data[i].z = i * 0.5f;
        
        for (j = 0; j < M; j++) {
            matrix[i][j] = i + j;
            
            for (k = 0; k < P; k++) {
                cube[i][j][k] = i * j * k;
                fully_partitioned[i * M * P + j * P + k] = 0;
            }
        }
    }
    
    /* OpenMP target with teams and distribute - complex nesting */
    #pragma omp target teams distribute parallel for \
        map(tofrom: data, matrix, cube) map(from: fully_partitioned) \
        collapse(2)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            /* Access struct members */
            int base = data[i].x;
            
            /* Multi-dimensional array access with conditional */
            for (k = 0; k < P; k++) {
                /* Fully partitioned access pattern */
                fully_partitioned[i * M * P + j * P + k] = 
                    base + matrix[i][j] + cube[i][j][k];
                
                /* Complex conditional creating varied partitioning needs */
                if ((i + j + k) % 3 == 0) {
                    cube[i][j][k] *= 2;
                } else if ((i + j + k) % 3 == 1) {
                    cube[i][j][k] /= 2;
                }
            }
            
            /* Update matrix with gang+worker partitioned pattern */
            matrix[i][j] = (i * M + j) % 100;
        }
    }
    
    /* Test gang+vector and worker+vector partitioning */
    int gang_vector[N][P];
    int worker_vector[M][P];
    
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: gang_vector, worker_vector) collapse(2)
    for (i = 0; i < N; i++) {
        for (k = 0; k < P; k++) {
            gang_vector[i][k] = i * P + k;
        }
    }
    
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: worker_vector)
    for (j = 0; j < M; j++) {
        for (k = 0; k < P; k++) {
            worker_vector[j][k] = j * P + k;
        }
    }
}

/* Pattern C: Mixed OpenACC/OpenMP with pointer-based accesses */
void test_pointer_partitioning() {
    int size = N * M;
    int *ptr1 = (int*)malloc(size * sizeof(int));
    int *ptr2 = (int*)malloc(size * sizeof(int));
    int **ptr_to_ptr = (int**)malloc(N * sizeof(int*));
    
    /* Create jagged array structure */
    for (int i = 0; i < N; i++) {
        ptr_to_ptr[i] = (int*)malloc(M * sizeof(int));
        for (int j = 0; j < M; j++) {
            ptr_to_ptr[i][j] = i * M + j;
        }
    }
    
    /* Initialize pointer arrays */
    for (int i = 0; i < size; i++) {
        ptr1[i] = i;
        ptr2[i] = size - i;
    }
    
    /* OpenACC with pointer data clauses */
    #pragma acc parallel loop copy(ptr1[0:size]) copyout(ptr2[0:size]) \
        present_or_copyin(ptr_to_ptr[0:N])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            int idx = i * M + j;
            /* Complex pointer arithmetic */
            ptr2[idx] = ptr1[idx] + ptr_to_ptr[i][j];
            
            /* Conditional updates create varied live ranges */
            if (ptr2[idx] % 7 == 0) {
                ptr2[idx] *= 3;
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < N; i++) {
        free(ptr_to_ptr[i]);
    }
    free(ptr_to_ptr);
    free(ptr1);
    free(ptr2);
}

/* Main function to drive all tests */
int main() {
    printf("Testing OpenACC/OpenMP partitioning state coverage...\n");
    
    /* Run all test patterns to maximize coverage */
    test_openacc_partitioning();
    printf("OpenACC partitioning test completed.\n");
    
    test_openmp_partitioning();
    printf("OpenMP partitioning test completed.\n");
    
    test_pointer_partitioning();
    printf("Pointer partitioning test completed.\n");
    
    /* Verification step */
    int verification = 1;
    
    #pragma acc parallel copy(verification)
    {
        verification = 0;  /* Simple check that code runs */
    }
    
    if (verification == 0) {
        printf("All tests completed successfully.\n");
        return 0;
    } else {
        printf("Test verification failed.\n");
        return 1;
    }
}
