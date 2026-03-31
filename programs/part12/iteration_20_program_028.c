/* Test for OpenACC partition string coverage in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_partition.c
 * Or with diagnostics: gcc -O1 -fopenacc -fopenacc-diag=par -foffload=nvptx-none -o test_partition test_partition.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define NUM_CONFIGS 8
#define MAX_GANGS 32
#define MAX_WORKERS 16
#define MAX_VECTOR 128

/* Structure to store queried execution configuration */
typedef struct {
    int gangs;
    int workers;
    int vector;
    int config_id;
} partition_config_t;

/* Host array to store results from device */
partition_config_t host_results[NUM_CONFIGS];

/* Device array for storing results */
#pragma acc declare create(host_results)

/* Function to perform computation with specific partition configuration */
void test_partition_config(int config_id, int gang_cnt, int worker_cnt, int vector_len) {
    int i, j, k;
    const int N = 1000;
    int *data = (int*)malloc(N * sizeof(int));
    int sum = 0;
    
    /* Initialize data on host */
    for (i = 0; i < N; i++) {
        data[i] = i % 100;
    }
    
    /* Copy data to device */
    #pragma acc enter data copyin(data[0:N])
    
    /* Parallel region with specific partition configuration */
    #pragma acc parallel num_gangs(gang_cnt) num_workers(worker_cnt) vector_length(vector_len) \
                present(data[0:N]) copyout(host_results[config_id])
    {
        int g, w, v;
        
        /* Query execution configuration using OpenACC runtime API */
        g = acc_get_num_gangs(0);  /* 0 = current device */
        w = acc_get_num_workers(0);
        v = acc_get_vector_length(0);
        
        /* Store configuration in device array */
        host_results[config_id].gangs = g;
        host_results[config_id].workers = w;
        host_results[config_id].vector = v;
        host_results[config_id].config_id = config_id;
        
        /* Perform actual computation to ensure region isn't optimized away */
        #pragma acc loop gang reduction(+:sum)
        for (i = 0; i < N / 10; i++) {
            #pragma acc loop worker
            for (j = 0; j < 10; j++) {
                #pragma acc loop vector
                for (k = 0; k < 1; k++) {
                    int idx = i * 10 + j;
                    if (idx < N) {
                        sum += data[idx];
                    }
                }
            }
        }
        
        /* Store sum in first element of results for verification */
        if (g == 1 && w == 1 && v == 1) {
            host_results[config_id].gangs = sum % 1000;
        }
    }
    
    /* Update host with results from device */
    #pragma acc update host(host_results[config_id])
    
    /* Clean up */
    #pragma acc exit data delete(data[0:N])
    free(data);
}

/* Test nested parallelism with collapse clause */
void test_nested_partition(int gang_cnt, int worker_cnt, int vector_len) {
    int i, j;
    const int N = 512;
    const int M = 256;
    int *matrix = (int*)malloc(N * M * sizeof(int));
    
    /* Initialize matrix */
    #pragma acc enter data create(matrix[0:N*M])
    
    /* Complex nested parallel region with collapse */
    #pragma acc parallel num_gangs(gang_cnt) num_workers(worker_cnt) vector_length(vector_len) \
                present(matrix[0:N*M])
    {
        #pragma acc loop gang collapse(2)
        for (i = 0; i < N; i += 64) {
            for (j = 0; j < M; j += 64) {
                #pragma acc loop worker
                for (int ii = i; ii < i + 64 && ii < N; ii++) {
                    #pragma acc loop vector
                    for (int jj = j; jj < j + 64 && jj < M; jj++) {
                        int idx = ii * M + jj;
                        matrix[idx] = (ii + jj) % 256;
                    }
                }
            }
        }
    }
    
    /* Verify some values */
    int check = 0;
    #pragma acc parallel loop reduction(+:check) present(matrix[0:N*M])
    for (i = 0; i < N * M; i += 100) {
        check += matrix[i];
    }
    
    #pragma acc exit data delete(matrix[0:N*M])
    free(matrix);
    
    /* Force diagnostic path by checking acc_malloc */
    void *test_ptr = acc_malloc(1024);
    if (test_ptr == NULL) {
        printf("acc_malloc failed - may trigger diagnostic with partition info\n");
    } else {
        acc_free(test_ptr);
    }
}

/* Test with dynamic values controlled by host */
void test_dynamic_partitions() {
    int i;
    int dynamic_gangs[] = {1, 2, 4, 8};
    int dynamic_workers[] = {1, 2, 4};
    int dynamic_vector[] = {1, 32, 64};
    
    for (i = 0; i < 4; i++) {
        int g = dynamic_gangs[i % 4];
        int w = dynamic_workers[i % 3];
        int v = dynamic_vector[i % 3];
        
        /* This should trigger different partition mappings */
        #pragma acc parallel num_gangs(g) num_workers(w) vector_length(v)
        {
            /* Simple computation */
            int local_g = acc_get_num_gangs(0);
            int local_w = acc_get_num_workers(0);
            int local_v = acc_get_vector_length(0);
            
            /* Use the values to prevent optimization */
            #pragma acc loop gang
            for (int j = 0; j < local_g * 10; j++) {
                #pragma acc loop worker
                for (int k = 0; k < local_w * 5; k++) {
                    #pragma acc loop vector
                    for (int l = 0; l < local_v; l++) {
                        /* No-op that uses variables */
                        volatile int dummy = j + k + l;
                        (void)dummy;
                    }
                }
            }
        }
    }
}

/* Test kernels directive with different partition patterns */
void test_kernels_partitions() {
    int i, j;
    const int N = 1024;
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    int *c = (int*)malloc(N * sizeof(int));
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N - i;
    }
    
    /* Copy to device */
    #pragma acc enter data copyin(a[0:N], b[0:N]) create(c[0:N])
    
    /* Kernels directive with gang partitioning */
    #pragma acc kernels num_gangs(8) num_workers(1) vector_length(1) \
                present(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
    
    /* Kernels directive with vector partitioning */
    #pragma acc kernels num_gangs(1) num_workers(1) vector_length(128) \
                present(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc loop vector
        for (i = 0; i < N; i++) {
            c[i] = c[i] * 2;
        }
    }
    
    /* Kernels directive with mixed partitioning */
    #pragma acc kernels num_gangs(4) num_workers(2) vector_length(32) \
                present(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc loop gang worker
        for (i = 0; i < N/2; i++) {
            #pragma acc loop vector
            for (j = 0; j < 2; j++) {
                int idx = i * 2 + j;
                if (idx < N) {
                    c[idx] = c[idx] - a[idx] + b[idx];
                }
            }
        }
    }
    
    /* Copy back and verify */
    #pragma acc update host(c[0:N])
    
    int checksum = 0;
    for (i = 0; i < N; i++) {
        checksum = (checksum + c[i]) % 1000000;
    }
    printf("Kernels checksum: %d\n", checksum);
    
    #pragma acc exit data delete(a[0:N], b[0:N], c[0:N])
    free(a);
    free(b);
    free(c);
}

int main() {
    int i;
    int total_gangs = 0, total_workers = 0, total_vector = 0;
    
    /* Initialize device results array */
    #pragma acc enter data copyin(host_results[0:NUM_CONFIGS])
    
    printf("Testing OpenACC partition string coverage...\n");
    
    /* Test all 8 partition configurations that map to the uncovered strings */
    
    /* 0: gang redundant */
    test_partition_config(0, 1, 1, 1);
    
    /* 1: gang partitioned */
    test_partition_config(1, 8, 1, 1);
    
    /* 2: worker partitioned */
    test_partition_config(2, 1, 4, 1);
    
    /* 3: gang+worker partitioned */
    test_partition_config(3, 2, 2, 1);
    
    /* 4: vector partitioned */
    test_partition_config(4, 1, 1, 128);
    
    /* 5: gang+vector partitioned */
    test_partition_config(5, 4, 1, 64);
    
    /* 6: worker+vector partitioned */
    test_partition_config(6, 1, 2, 64);
    
    /* 7: fully partitioned */
    test_partition_config(7, 2, 2, 32);
    
    /* Update all results from device */
    #pragma acc update host(host_results[0:NUM_CONFIGS])
    
    /* Print results and compute checksum */
    printf("\nPartition configuration results:\n");
    printf("Config | Gangs | Workers | Vector\n");
    printf("-------|-------|---------|-------\n");
    
    for (i = 0; i < NUM_CONFIGS; i++) {
        printf("  %d    |  %3d  |   %3d   |  %3d\n",
               host_results[i].config_id,
               host_results[i].gangs,
               host_results[i].workers,
               host_results[i].vector);
        
        total_gangs += host_results[i].gangs;
        total_workers += host_results[i].workers;
        total_vector += host_results[i].vector;
    }
    
    printf("\nChecksum totals: gangs=%d, workers=%d, vector=%d\n",
           total_gangs, total_workers, total_vector);
    
    /* Test nested parallelism */
    printf("\nTesting nested parallelism...\n");
    test_nested_partition(4, 2, 32);
    
    /* Test dynamic partitions */
    printf("Testing dynamic partitions...\n");
    test_dynamic_partitions();
    
    /* Test kernels directive */
    printf("Testing kernels directive partitions...\n");
    test_kernels_partitions();
    
    /* Clean up */
    #pragma acc exit data delete(host_results[0:NUM_CONFIGS])
    
    /* Final verification that all configurations were tested */
    if (total_gangs > 0 && total_workers > 0 && total_vector > 0) {
        printf("\nAll partition configurations tested successfully!\n");
        return 0;
    } else {
        printf("\nWarning: Some configurations may not have been executed\n");
        return 1;
    }
}
