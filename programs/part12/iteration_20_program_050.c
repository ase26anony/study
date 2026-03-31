/* Test program to cover partition string mapping in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_partition.c
 * Or with diagnostics: gcc -O1 -fopenacc -fopenacc-diag=par -fopt-info-omp-all -o test_partition test_partition.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define NUM_CONFIGS 8
#define MAX_GANGS 32
#define MAX_WORKERS 16
#define VECTOR_LENGTH 128

/* Structure to store runtime query results */
typedef struct {
    int gang_cnt;
    int worker_cnt;
    int vector_len;
    int config_id;
} partition_config_t;

/* Function to perform computation with specific partition configuration */
void test_partition_config(int config_id, int gang_cnt, int worker_cnt, int vector_len, 
                          partition_config_t *results, int *checksum) {
    int i, j, k;
    const int N = 1024;
    int *data = (int*)malloc(N * sizeof(int));
    int *dev_data = NULL;
    
    /* Initialize data */
    for (i = 0; i < N; i++) {
        data[i] = i;
    }
    
    /* Copy data to device */
    #pragma acc enter data copyin(data[0:N])
    
    /* Execute with specific partition configuration */
    #pragma acc parallel num_gangs(gang_cnt) num_workers(worker_cnt) vector_length(vector_len) \
                present(data[0:N])
    {
        int g, w, v;
        #ifdef _OPENACC
        g = acc_get_num_gangs(acc_async_sync);
        w = acc_get_num_workers(acc_async_sync);
        v = acc_get_vector_length();
        #else
        g = 1; w = 1; v = 1;
        #endif
        
        /* Store configuration in first element (redundant across all threads) */
        if (acc_on_device(acc_device_not_host)) {
            data[0] = g;
            data[1] = w;
            data[2] = v;
            data[3] = config_id;
        }
        
        /* Perform some computation to ensure region isn't optimized away */
        #pragma acc loop gang
        for (i = 0; i < gang_cnt; i++) {
            #pragma acc loop worker
            for (j = 0; j < worker_cnt; j++) {
                #pragma acc loop vector
                for (k = 0; k < vector_len && (i*worker_cnt*vector_len + j*vector_len + k) < N; k++) {
                    int idx = i*worker_cnt*vector_len + j*vector_len + k;
                    if (idx < N) {
                        data[idx] = data[idx] * 2 + 1;
                    }
                }
            }
        }
    }
    
    /* Copy results back */
    #pragma acc update host(data[0:4])
    
    /* Store query results */
    results[config_id].gang_cnt = data[0];
    results[config_id].worker_cnt = data[1];
    results[config_id].vector_len = data[2];
    results[config_id].config_id = data[3];
    
    /* Update checksum */
    for (i = 0; i < N; i++) {
        *checksum += data[i];
    }
    
    /* Cleanup */
    #pragma acc exit data delete(data[0:N])
    free(data);
}

/* Test nested parallelism with collapse clause */
void test_nested_partitions(int *checksum) {
    int i, j;
    const int N = 512;
    const int M = 256;
    int *matrix = (int*)malloc(N * M * sizeof(int));
    
    /* Initialize matrix */
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            matrix[i*M + j] = i + j;
        }
    }
    
    /* Test 1: Gang redundant (collapse all) */
    #pragma acc data copy(matrix[0:N*M])
    {
        #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1)
        {
            #pragma acc loop gang worker vector collapse(2)
            for (i = 0; i < N; i++) {
                for (j = 0; j < M; j++) {
                    matrix[i*M + j] *= 2;
                }
            }
        }
        
        /* Test 2: Gang partitioned */
        #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1)
        {
            #pragma acc loop gang
            for (i = 0; i < N; i++) {
                #pragma acc loop worker vector
                for (j = 0; j < M; j++) {
                    matrix[i*M + j] += i;
                }
            }
        }
        
        /* Test 3: Worker partitioned */
        #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1)
        {
            #pragma acc loop gang worker
            for (i = 0; i < N; i++) {
                #pragma acc loop vector
                for (j = 0; j < M; j++) {
                    matrix[i*M + j] += j;
                }
            }
        }
        
        /* Test 4: Vector partitioned */
        #pragma acc parallel num_gangs(1) num_workers(1) vector_length(128)
        {
            #pragma acc loop gang worker vector
            for (i = 0; i < N; i++) {
                for (j = 0; j < M; j++) {
                    matrix[i*M + j] -= (i + j);
                }
            }
        }
    }
    
    /* Update checksum */
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            *checksum += matrix[i*M + j];
        }
    }
    
    free(matrix);
}

/* Test with dynamic values and runtime conditions */
void test_dynamic_partitions(int *checksum) {
    int i;
    const int N = 1000;
    int *data = (int*)malloc(N * sizeof(int));
    int gang_cnt, worker_cnt, vector_len;
    
    /* Initialize data */
    for (i = 0; i < N; i++) {
        data[i] = i % 100;
    }
    
    /* Loop with different dynamic configurations */
    for (gang_cnt = 1; gang_cnt <= 4; gang_cnt *= 2) {
        for (worker_cnt = 1; worker_cnt <= 4; worker_cnt *= 2) {
            for (vector_len = 1; vector_len <= 64; vector_len *= 8) {
                
                /* Use acc_malloc to potentially trigger diagnostic paths */
                void *dev_ptr = acc_malloc(N * sizeof(int));
                if (dev_ptr == NULL) {
                    /* This might trigger diagnostic output including partition info */
                    fprintf(stderr, "acc_malloc failed for config: gangs=%d, workers=%d, vector=%d\n",
                            gang_cnt, worker_cnt, vector_len);
                } else {
                    acc_free(dev_ptr);
                }
                
                /* Execute with dynamic configuration */
                #pragma acc parallel num_gangs(gang_cnt) num_workers(worker_cnt) \
                            vector_length(vector_len) copy(data[0:N])
                {
                    int g, w, v;
                    #ifdef _OPENACC
                    g = __builtin_acc_get_num_gangs();
                    w = __builtin_acc_get_num_workers();
                    v = __builtin_acc_get_vector_length();
                    #else
                    g = gang_cnt;
                    w = worker_cnt;
                    v = vector_len;
                    #endif
                    
                    #pragma acc loop gang
                    for (i = 0; i < N; i += (N/gang_cnt)) {
                        int start = i;
                        int end = (i + N/gang_cnt) < N ? (i + N/gang_cnt) : N;
                        #pragma acc loop worker vector
                        for (int j = start; j < end; j++) {
                            data[j] = data[j] * g + w * v;
                        }
                    }
                }
                
                /* Update checksum */
                for (i = 0; i < N; i++) {
                    *checksum += data[i];
                }
            }
        }
    }
    
    free(data);
}

/* Test kernels directive with different partitionings */
void test_kernels_partitions(int *checksum) {
    int i, j;
    const int N = 256;
    int *array = (int*)malloc(N * N * sizeof(int));
    
    /* Initialize */
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            array[i*N + j] = i * j;
        }
    }
    
    /* kernels directive allows runtime to choose partitioning */
    #pragma acc kernels copy(array[0:N*N])
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            #pragma acc loop worker
            for (j = 0; j < N; j++) {
                array[i*N + j] = array[i*N + j] / (i + j + 1);
            }
        }
        
        /* Another loop with vector partitioning */
        #pragma acc loop gang vector
        for (i = 0; i < N; i++) {
            for (j = 0; j < N; j++) {
                array[i*N + j] += (i - j);
            }
        }
    }
    
    /* Update checksum */
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            *checksum += array[i*N + j];
        }
    }
    
    free(array);
}

int main() {
    int i;
    int total_checksum = 0;
    partition_config_t results[NUM_CONFIGS];
    
    printf("Testing OpenACC partition string coverage...\n");
    
    /* Test all 8 partition configurations (mapping to cases 0-7) */
    
    /* 0: gang redundant */
    test_partition_config(0, 1, 1, 1, results, &total_checksum);
    
    /* 1: gang partitioned */
    test_partition_config(1, 8, 1, 1, results, &total_checksum);
    
    /* 2: worker partitioned */
    test_partition_config(2, 1, 4, 1, results, &total_checksum);
    
    /* 3: gang+worker partitioned */
    test_partition_config(3, 4, 2, 1, results, &total_checksum);
    
    /* 4: vector partitioned */
    test_partition_config(4, 1, 1, 128, results, &total_checksum);
    
    /* 5: gang+vector partitioned */
    test_partition_config(5, 4, 1, 64, results, &total_checksum);
    
    /* 6: worker+vector partitioned */
    test_partition_config(6, 1, 2, 64, results, &total_checksum);
    
    /* 7: fully partitioned */
    test_partition_config(7, 4, 2, 64, results, &total_checksum);
    
    /* Print configuration results */
    printf("\nPartition configuration results:\n");
    for (i = 0; i < NUM_CONFIGS; i++) {
        printf("Config %d: gangs=%d, workers=%d, vector=%d\n",
               i, results[i].gang_cnt, results[i].worker_cnt, results[i].vector_len);
    }
    
    /* Test nested parallelism */
    test_nested_partitions(&total_checksum);
    
    /* Test dynamic partitions */
    test_dynamic_partitions(&total_checksum);
    
    /* Test kernels directive */
    test_kernels_partitions(&total_checksum);
    
    /* Final checksum to ensure no computation was optimized away */
    printf("\nTotal checksum: %d\n", total_checksum);
    printf("(Non-zero checksum indicates all regions executed)\n");
    
    /* Force a diagnostic by using invalid configuration */
    printf("\nTesting edge case with large vector length...\n");
    {
        int small_data[10] = {0};
        #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1024) copy(small_data[0:10])
        {
            #pragma acc loop vector
            for (int i = 0; i < 10; i++) {
                small_data[i] = i;
            }
        }
        
        /* Add to checksum */
        for (i = 0; i < 10; i++) {
            total_checksum += small_data[i];
        }
    }
    
    printf("Final checksum: %d\n", total_checksum);
    
    return 0;
}
