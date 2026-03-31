/* test_omp_acc_partition_codes.c
 * 
 * This program systematically tests OpenACC data partition codes 0-7
 * to trigger the partition string lookup function in GCC's omp-oacc-neuter-broadcast.cc.
 * 
 * Compile with: gcc -O2 -fopenacc -fdump-tree-omplower -fdump-tree-optimized -o test_partitions test_omp_acc_partition_codes.c
 * 
 * The -fdump-tree-omplower flag is critical to trigger the compiler's internal
 * OpenACC/OpenMP offload processing where partition codes are generated and mapped.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _OPENACC
#include <openacc.h>
#endif

#define N 1024
#define CHUNK_SIZE 128

/* Helper function to initialize arrays with patterned data */
void init_array(float *arr, int size, float base_value) {
    for (int i = 0; i < size; i++) {
        arr[i] = base_value + (i % 100) * 0.1f;
    }
}

/* Test case 0: gang redundant partition */
void test_gang_redundant(float *src, float *dest, int n, float *reduction_sum) {
    float local_sum = 0.0f;
    
    /* Use volatile to prevent compile-time elimination */
    volatile int use_gang_redundant = 1;
    
    if (use_gang_redundant) {
        /* Code 0: gang redundant - no partition modifiers */
        #pragma acc parallel copy(src[0:n], dest[0:n]) copyout(local_sum)
        {
            #pragma acc loop gang reduction(+:local_sum)
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] * 2.0f;
                local_sum += src[i];
            }
        }
    }
    
    *reduction_sum += local_sum;
}

/* Test case 1: gang partitioned */
void test_gang_partitioned(float *src, float *dest, int n, float *reduction_sum) {
    float local_sum = 0.0f;
    
    /* Code 1: gang partitioned */
    #pragma acc parallel copy(src[0:n]) create(dest[0:n]) copyout(local_sum)
    {
        /* Explicit gang partitioning on data */
        #pragma acc loop gang independent gang(dest) reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 3.0f;
            local_sum += src[i] * 0.5f;
        }
    }
    
    *reduction_sum += local_sum;
}

/* Test case 2: worker partitioned */
void test_worker_partitioned(float *src, float *dest, int n, float *reduction_sum) {
    float local_sum = 0.0f;
    
    /* Code 2: worker partitioned */
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop worker independent worker(dest) reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] + 1.0f;
            local_sum += dest[i];
        }
    }
    
    *reduction_sum += local_sum;
}

/* Test case 3: gang+worker partitioned */
void test_gang_worker_partitioned(float *src, float *dest, int n, float *reduction_sum) {
    float local_sum = 0.0f;
    
    /* Code 3: gang+worker partitioned */
    #pragma acc parallel copy(src[0:n]) create(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang worker independent gang worker(dest) reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * src[i];
            local_sum += dest[i] * 0.1f;
        }
    }
    
    *reduction_sum += local_sum;
}

/* Test case 4: vector partitioned */
void test_vector_partitioned(float *src, float *dest, int n, float *reduction_sum) {
    float local_sum = 0.0f;
    
    /* Code 4: vector partitioned */
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop vector independent vector(dest) reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] / 2.0f;
            local_sum += dest[i];
        }
    }
    
    *reduction_sum += local_sum;
}

/* Test case 5: gang+vector partitioned */
void test_gang_vector_partitioned(float *src, float *dest, int n, float *reduction_sum) {
    float local_sum = 0.0f;
    
    /* Code 5: gang+vector partitioned */
    #pragma acc parallel copy(src[0:n]) create(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang vector independent gang vector(dest) reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 1.5f;
            local_sum += src[i] - dest[i];
        }
    }
    
    *reduction_sum += local_sum;
}

/* Test case 6: worker+vector partitioned */
void test_worker_vector_partitioned(float *src, float *dest, int n, float *reduction_sum) {
    float local_sum = 0.0f;
    
    /* Code 6: worker+vector partitioned */
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop worker vector independent worker vector(dest) reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] + i * 0.01f;
            local_sum += dest[i];
        }
    }
    
    *reduction_sum += local_sum;
}

/* Test case 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(float *src, float *dest, int n, float *reduction_sum) {
    float local_sum = 0.0f;
    
    /* Code 7: fully partitioned */
    #pragma acc parallel copy(src[0:n]) create(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang worker vector independent gang worker vector(dest) reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * src[i] * 0.5f;
            local_sum += dest[i];
        }
    }
    
    *reduction_sum += local_sum;
}

/* Combined test that uses all partition modes based on runtime input */
void test_all_partitions_combined(float *src, float *dests[8], int n, float *reduction_sums) {
    volatile int partition_mode = 0; /* Volatile to prevent compile-time elimination */
    
    for (int mode = 0; mode < 8; mode++) {
        partition_mode = mode; /* Force runtime dependency */
        
        switch (partition_mode) {
            case 0:
                test_gang_redundant(src, dests[0], n, &reduction_sums[0]);
                break;
            case 1:
                test_gang_partitioned(src, dests[1], n, &reduction_sums[1]);
                break;
            case 2:
                test_worker_partitioned(src, dests[2], n, &reduction_sums[2]);
                break;
            case 3:
                test_gang_worker_partitioned(src, dests[3], n, &reduction_sums[3]);
                break;
            case 4:
                test_vector_partitioned(src, dests[4], n, &reduction_sums[4]);
                break;
            case 5:
                test_gang_vector_partitioned(src, dests[5], n, &reduction_sums[5]);
                break;
            case 6:
                test_worker_vector_partitioned(src, dests[6], n, &reduction_sums[6]);
                break;
            case 7:
                test_fully_partitioned(src, dests[7], n, &reduction_sums[7]);
                break;
        }
    }
}

int main() {
    float *src = (float*)malloc(N * sizeof(float));
    float *dests[8];
    float reduction_sums[8] = {0};
    float final_checksum = 0.0f;
    
    /* Initialize source array */
    init_array(src, N, 1.0f);
    
    /* Allocate destination arrays */
    for (int i = 0; i < 8; i++) {
        dests[i] = (float*)malloc(N * sizeof(float));
        memset(dests[i], 0, N * sizeof(float));
    }
    
    printf("Testing OpenACC partition codes 0-7...\n");
    
    /* Test all partition modes */
    test_all_partitions_combined(src, dests, N, reduction_sums);
    
    /* Compute final checksum to ensure all computations are retained */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < N; j++) {
            final_checksum += dests[i][j];
        }
        final_checksum += reduction_sums[i];
    }
    
    printf("Final checksum: %f\n", final_checksum);
    
    /* Cleanup */
    free(src);
    for (int i = 0; i < 8; i++) {
        free(dests[i]);
    }
    
    return 0;
}
