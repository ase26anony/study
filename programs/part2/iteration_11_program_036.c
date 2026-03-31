/* test_openacc_partitions.c
 * 
 * This program systematically tests all 8 OpenACC data partition modes
 * to trigger the partition code string lookup in GCC's omp-oacc-neuter-broadcast.cc
 * 
 * Compile with: gcc -O2 -fopenacc -fdump-tree-omplower -fdump-tree-optimized test_openacc_partitions.c -o test_program
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define N 1024
#define CHUNK_SIZE 128

/* Use volatile to prevent compile-time elimination */
volatile int use_partition_mode = 0;

/* Test function for gang redundant (partition code 0) */
void test_gang_redundant(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) 
    {
        float local_sum = 0.0f;
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 2.0f;
            local_sum += dest[i];
        }
        #pragma acc atomic update
        sum[0] += local_sum;
    }
}

/* Test function for gang partitioned (partition code 1) */
void test_gang_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy gang(dest[0:n]) copy(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 3.0f;
            local_sum += dest[i];
        }
        #pragma acc atomic update
        sum[0] += local_sum;
    }
}

/* Test function for worker partitioned (partition code 2) */
void test_worker_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy worker(dest[0:n]) copy(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 4.0f;
            local_sum += dest[i];
        }
        #pragma acc atomic update
        sum[0] += local_sum;
    }
}

/* Test function for gang+worker partitioned (partition code 3) */
void test_gang_worker_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy gang worker(dest[0:n]) copy(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 5.0f;
            local_sum += dest[i];
        }
        #pragma acc atomic update
        sum[0] += local_sum;
    }
}

/* Test function for vector partitioned (partition code 4) */
void test_vector_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy vector(dest[0:n]) copy(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 6.0f;
            local_sum += dest[i];
        }
        #pragma acc atomic update
        sum[0] += local_sum;
    }
}

/* Test function for gang+vector partitioned (partition code 5) */
void test_gang_vector_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy gang vector(dest[0:n]) copy(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 7.0f;
            local_sum += dest[i];
        }
        #pragma acc atomic update
        sum[0] += local_sum;
    }
}

/* Test function for worker+vector partitioned (partition code 6) */
void test_worker_vector_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy worker vector(dest[0:n]) copy(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 8.0f;
            local_sum += dest[i];
        }
        #pragma acc atomic update
        sum[0] += local_sum;
    }
}

/* Test function for fully partitioned (partition code 7) */
void test_fully_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy gang worker vector(dest[0:n]) copy(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 9.0f;
            local_sum += dest[i];
        }
        #pragma acc atomic update
        sum[0] += local_sum;
    }
}

/* Alternative approach using kernels construct with data regions */
void test_kernels_partitions(const float* src, float* dest, int n, float* sum) {
    /* Test all partition modes within a single function using volatile control */
    if (use_partition_mode == 0) {
        #pragma acc kernels copyin(src[0:n]) copyout(dest[0:n]) copy(sum[0:1])
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] + 1.0f;
            }
        }
    } else if (use_partition_mode == 1) {
        #pragma acc kernels copyin(src[0:n]) copyout gang(dest[0:n]) copy(sum[0:1])
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] + 2.0f;
            }
        }
    } else if (use_partition_mode == 2) {
        #pragma acc kernels copyin(src[0:n]) copyout worker(dest[0:n]) copy(sum[0:1])
        {
            #pragma acc loop gang worker
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] + 3.0f;
            }
        }
    } else if (use_partition_mode == 3) {
        #pragma acc kernels copyin(src[0:n]) copyout gang worker(dest[0:n]) copy(sum[0:1])
        {
            #pragma acc loop gang worker
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] + 4.0f;
            }
        }
    } else if (use_partition_mode == 4) {
        #pragma acc kernels copyin(src[0:n]) copyout vector(dest[0:n]) copy(sum[0:1])
        {
            #pragma acc loop gang vector
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] + 5.0f;
            }
        }
    } else if (use_partition_mode == 5) {
        #pragma acc kernels copyin(src[0:n]) copyout gang vector(dest[0:n]) copy(sum[0:1])
        {
            #pragma acc loop gang vector
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] + 6.0f;
            }
        }
    } else if (use_partition_mode == 6) {
        #pragma acc kernels copyin(src[0:n]) copyout worker vector(dest[0:n]) copy(sum[0:1])
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] + 7.0f;
            }
        }
    } else if (use_partition_mode == 7) {
        #pragma acc kernels copyin(src[0:n]) copyout gang worker vector(dest[0:n]) copy(sum[0:1])
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] + 8.0f;
            }
        }
    }
}

/* Test with present clause to trigger different code paths */
void test_present_clause_partitions(float* data, int n, float* results) {
    /* First ensure data is present on device */
    #pragma acc enter data copyin(data[0:n], results[0:8])
    
    /* Test different partition modes with present clause */
    #pragma acc parallel present(data[0:n]) present gang(results[0:8])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            data[i] = i * 1.5f;
        }
    }
    
    #pragma acc parallel present(data[0:n]) present worker(results[0:8])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            data[i] += i * 0.5f;
        }
    }
    
    #pragma acc parallel present(data[0:n]) present gang worker vector(results[0:8])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            results[i % 8] += data[i];
        }
    }
    
    #pragma acc exit data copyout(results[0:8]) delete(data[0:n])
}

/* Test with create clause */
void test_create_clause_partitions(int n, float** partial_sums) {
    #pragma acc parallel create gang(partial_sums[0:8][0:CHUNK_SIZE])
    {
        #pragma acc loop gang
        for (int g = 0; g < 8; g++) {
            #pragma acc loop worker
            for (int w = 0; w < CHUNK_SIZE; w++) {
                partial_sums[g][w] = g * 100.0f + w;
            }
        }
    }
}

int main() {
    float* src = (float*)malloc(N * sizeof(float));
    float* dest = (float*)malloc(N * sizeof(float));
    float* dest2 = (float*)malloc(N * sizeof(float));
    float* results = (float*)malloc(8 * sizeof(float));
    float** partial_sums = (float**)malloc(8 * sizeof(float*));
    float total_sum = 0.0f;
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        src[i] = (float)i;
        dest[i] = 0.0f;
        dest2[i] = 0.0f;
    }
    
    for (int i = 0; i < 8; i++) {
        results[i] = 0.0f;
        partial_sums[i] = (float*)malloc(CHUNK_SIZE * sizeof(float));
    }
    
    printf("Testing all OpenACC data partition modes...\n");
    
    /* Test all 8 partition modes systematically */
    test_gang_redundant(src, dest, N, &total_sum);
    printf("  Gang redundant complete, sum = %f\n", total_sum);
    
    test_gang_partitioned(src, dest, N, &total_sum);
    printf("  Gang partitioned complete, sum = %f\n", total_sum);
    
    test_worker_partitioned(src, dest, N, &total_sum);
    printf("  Worker partitioned complete, sum = %f\n", total_sum);
    
    test_gang_worker_partitioned(src, dest, N, &total_sum);
    printf("  Gang+Worker partitioned complete, sum = %f\n", total_sum);
    
    test_vector_partitioned(src, dest, N, &total_sum);
    printf("  Vector partitioned complete, sum = %f\n", total_sum);
    
    test_gang_vector_partitioned(src, dest, N, &total_sum);
    printf("  Gang+Vector partitioned complete, sum = %f\n", total_sum);
    
    test_worker_vector_partitioned(src, dest, N, &total_sum);
    printf("  Worker+Vector partitioned complete, sum = %f\n", total_sum);
    
    test_fully_partitioned(src, dest, N, &total_sum);
    printf("  Fully partitioned complete, sum = %f\n", total_sum);
    
    /* Test kernels construct with different partition modes */
    for (int mode = 0; mode < 8; mode++) {
        use_partition_mode = mode;
        test_kernels_partitions(src, dest2, N, &total_sum);
    }
    printf("  Kernels with all partition modes complete, sum = %f\n", total_sum);
    
    /* Test present clause variations */
    test_present_clause_partitions(dest, N, results);
    printf("  Present clause tests complete\n");
    
    /* Test create clause */
    test_create_clause_partitions(N, partial_sums);
    printf("  Create clause tests complete\n");
    
    /* Compute final checksum */
    float checksum = total_sum;
    for (int i = 0; i < N; i++) {
        checksum += dest[i] + dest2[i];
    }
    for (int i = 0; i < 8; i++) {
        checksum += results[i];
        for (int j = 0; j < CHUNK_SIZE; j++) {
            checksum += partial_sums[i][j];
        }
        free(partial_sums[i]);
    }
    
    printf("Final checksum: %f\n", checksum);
    
    /* Cleanup */
    free(src);
    free(dest);
    free(dest2);
    free(results);
    free(partial_sums);
    
    return 0;
}
