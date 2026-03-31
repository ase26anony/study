/* test_neuter_broadcast.c - Comprehensive test for omp-oacc-neuter-broadcast.cc coverage */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 64
#define P 32

/* Pattern A: Various scalar and array types with different data clauses */
void test_openacc_partitioning(int *results) {
    int i, j, k;
    
    /* Different partitioning states will be assigned to these variables */
    int scalar_gang_redundant;          /* Case 0: gang redundant */
    int scalar_gang_partitioned;        /* Case 1: gang partitioned */
    int scalar_worker_partitioned;      /* Case 2: worker partitioned */
    int scalar_gang_worker_partitioned; /* Case 3: gang+worker partitioned */
    int scalar_vector_partitioned;      /* Case 4: vector partitioned */
    
    /* Multi-dimensional arrays for complex partitioning */
    int arr_3d[N][M][P];                /* Case 5-7: various combined partitions */
    int arr_2d[N][M];
    float arr_1d[N];
    
    /* Initialize data */
    for (i = 0; i < N; i++) {
        arr_1d[i] = i * 1.0f;
        for (j = 0; j < M; j++) {
            arr_2d[i][j] = i * j;
            for (k = 0; k < P; k++) {
                arr_3d[i][j][k] = i * j * k;
            }
        }
    }
    
    /* Pattern B: Multi-dimensional array access with complex indexing */
    #pragma acc parallel loop gang worker vector \
        copyin(arr_3d, arr_2d, arr_1d) \
        copyout(results[0:N*M*P]) \
        private(scalar_gang_redundant) \
        firstprivate(scalar_gang_partitioned) \
        create(scalar_worker_partitioned, scalar_gang_worker_partitioned, scalar_vector_partitioned)
    for (i = 0; i < N; i++) {
        /* Gang-level computation */
        scalar_gang_redundant = i;
        
        /* Worker-level computation with conditionals */
        #pragma acc loop worker
        for (j = 0; j < M; j++) {
            scalar_worker_partitioned = j;
            
            /* Vector-level computation with nested loops */
            #pragma acc loop vector
            for (k = 0; k < P; k++) {
                scalar_vector_partitioned = k;
                scalar_gang_worker_partitioned = i + j;
                
                /* Complex conditional to create varied data flow */
                if (arr_3d[i][j][k] % 2 == 0) {
                    results[i * M * P + j * P + k] = 
                        arr_3d[i][j][k] + 
                        arr_2d[i][j] + 
                        (int)arr_1d[i] +
                        scalar_gang_redundant +
                        scalar_gang_partitioned +
                        scalar_worker_partitioned +
                        scalar_gang_worker_partitioned +
                        scalar_vector_partitioned;
                } else {
                    results[i * M * P + j * P + k] = 
                        arr_3d[i][j][k] * 2 -
                        arr_2d[i][j] -
                        (int)arr_1d[i];
                }
            }
        }
    }
}

/* Pattern C: Variable-length data and pointers */
void test_openacc_pointers() {
    int size = 1000;
    int *dynamic_arr = (int *)malloc(size * sizeof(int));
    int *host_ptr = (int *)malloc(size * sizeof(int));
    
    /* Initialize host data */
    for (int i = 0; i < size; i++) {
        host_ptr[i] = i;
    }
    
    /* Copy to device */
    #pragma acc enter data copyin(host_ptr[0:size])
    #pragma acc enter data create(dynamic_arr[0:size])
    
    /* Complex kernel with pointer arithmetic */
    #pragma acc parallel loop gang vector \
        present(host_ptr[0:size], dynamic_arr[0:size])
    for (int i = 0; i < size; i++) {
        int *ptr = &dynamic_arr[i];
        int offset = i % 10;
        
        /* Pointer-based access with varying patterns */
        for (int j = 0; j < offset; j++) {
            ptr[j] = host_ptr[i] * j;
        }
        
        /* Conditional pointer update */
        if (i > size/2) {
            ptr = &host_ptr[i];
            *ptr = *ptr * 2;
        }
    }
    
    #pragma acc exit data copyout(dynamic_arr[0:size])
    #pragma acc exit data delete(host_ptr[0:size])
    
    free(dynamic_arr);
    free(host_ptr);
}

/* Pattern D: Struct-based data for C++-like partitioning */
struct ComplexData {
    int gang_data;
    float worker_data;
    double vector_data;
    int mixed_data[4];
};

void test_openacc_structs() {
    struct ComplexData data_arr[N];
    
    /* Initialize struct array */
    for (int i = 0; i < N; i++) {
        data_arr[i].gang_data = i;
        data_arr[i].worker_data = i * 1.5f;
        data_arr[i].vector_data = i * 2.5;
        for (int j = 0; j < 4; j++) {
            data_arr[i].mixed_data[j] = i * j;
        }
    }
    
    /* Kernel accessing different struct members in different ways */
    #pragma acc parallel loop gang worker vector \
        copy(data_arr[0:N])
    for (int i = 0; i < N; i++) {
        /* Different partitioning for different struct members */
        data_arr[i].gang_data += i;           /* Likely gang partitioned */
        
        #pragma acc loop worker
        for (int j = 0; j < 4; j++) {
            data_arr[i].mixed_data[j] += j;   /* Worker partitioned */
        }
        
        #pragma acc loop vector
        for (int j = 0; j < 4; j++) {
            float temp = data_arr[i].worker_data * j;
            data_arr[i].vector_data += temp;  /* Vector partitioned */
        }
    }
}

/* OpenMP version for broader coverage */
void test_openmp_partitioning(int *results) {
    int i, j, k;
    int arr_3d[N][M][P];
    int arr_2d[N][M];
    float arr_1d[N];
    
    /* Initialize */
    for (i = 0; i < N; i++) {
        arr_1d[i] = i * 2.0f;
        for (j = 0; j < M; j++) {
            arr_2d[i][j] = i + j;
            for (k = 0; k < P; k++) {
                arr_3d[i][j][k] = i - j + k;
            }
        }
    }
    
    /* OpenMP target with teams and distribute */
    #pragma omp target teams distribute parallel for \
        map(to: arr_3d, arr_2d, arr_1d) \
        map(from: results[0:N*M*P]) \
        private(i, j, k)
    for (int idx = 0; idx < N * M * P; idx++) {
        i = idx / (M * P);
        j = (idx % (M * P)) / P;
        k = idx % P;
        
        /* Complex computation with conditionals */
        int temp = arr_3d[i][j][k];
        float ftemp = arr_1d[i];
        
        if (temp > 0) {
            results[idx] = temp + (int)ftemp + arr_2d[i][j];
        } else {
            results[idx] = temp * 2 - (int)ftemp - arr_2d[i][j];
        }
        
        /* Nested computation to force partitioning analysis */
        for (int t = 0; t < 3; t++) {
            results[idx] += t * (i % (t + 1));
        }
    }
}

/* Combined test driver */
int main() {
    int *acc_results = (int *)malloc(N * M * P * sizeof(int));
    int *omp_results = (int *)malloc(N * M * P * sizeof(int));
    
    printf("Testing OpenACC partitioning...\n");
    test_openacc_partitioning(acc_results);
    
    printf("Testing OpenACC pointers...\n");
    test_openacc_pointers();
    
    printf("Testing OpenACC structs...\n");
    test_openacc_structs();
    
    printf("Testing OpenMP partitioning...\n");
    test_openmp_partitioning(omp_results);
    
    /* Simple validation - check that results are non-zero */
    int acc_sum = 0, omp_sum = 0;
    for (int i = 0; i < N * M * P; i++) {
        acc_sum += acc_results[i];
        omp_sum += omp_results[i];
    }
    
    printf("OpenACC result sum: %d\n", acc_sum);
    printf("OpenMP result sum: %d\n", omp_sum);
    
    /* Basic correctness check */
    if (acc_sum != 0 && omp_sum != 0) {
        printf("Test PASSED: Both OpenACC and OpenMP produced non-zero results\n");
    } else {
        printf("Test FAILED: One or both results are zero\n");
        return 1;
    }
    
    free(acc_results);
    free(omp_results);
    
    return 0;
}
