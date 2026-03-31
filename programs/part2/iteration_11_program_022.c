/* test_openacc_partitions.c
 * Designed to trigger partition code mapping in GCC's omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -fdump-tree-omplower -fdump-tree-optimized test_openacc_partitions.c -o test_acc
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define N 1024
#define GANG_SIZE 32
#define WORKER_SIZE 4
#define VECTOR_SIZE 8

/* Use volatile to prevent compile-time elimination */
static volatile int force_partition_mode = 0;

/* Test function for gang redundant (code 0) */
void test_gang_redundant(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
                         gang(GANG_SIZE)
    {
        #pragma acc loop gang reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 2.0f;
            sum[0] += src[i];
        }
    }
}

/* Test function for gang partitioned (code 1) */
void test_gang_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy gang(src[0:n]) copy gang(dest[0:n]) \
                         copy gang(sum[0:1]) gang(GANG_SIZE)
    {
        #pragma acc loop gang reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 3.0f;
            sum[0] += src[i] * 2.0f;
        }
    }
}

/* Test function for worker partitioned (code 2) */
void test_worker_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy worker(src[0:n]) copy worker(dest[0:n]) \
                         copy worker(sum[0:1]) gang(GANG_SIZE) worker(WORKER_SIZE)
    {
        #pragma acc loop gang worker reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 4.0f;
            sum[0] += src[i] * 3.0f;
        }
    }
}

/* Test function for gang+worker partitioned (code 3) */
void test_gang_worker_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy gang worker(src[0:n]) copy gang worker(dest[0:n]) \
                         copy gang worker(sum[0:1]) gang(GANG_SIZE) worker(WORKER_SIZE)
    {
        #pragma acc loop gang worker reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 5.0f;
            sum[0] += src[i] * 4.0f;
        }
    }
}

/* Test function for vector partitioned (code 4) */
void test_vector_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy vector(src[0:n]) copy vector(dest[0:n]) \
                         copy vector(sum[0:1]) vector_length(VECTOR_SIZE)
    {
        #pragma acc loop vector reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 6.0f;
            sum[0] += src[i] * 5.0f;
        }
    }
}

/* Test function for gang+vector partitioned (code 5) */
void test_gang_vector_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy gang vector(src[0:n]) copy gang vector(dest[0:n]) \
                         copy gang vector(sum[0:1]) gang(GANG_SIZE) vector_length(VECTOR_SIZE)
    {
        #pragma acc loop gang vector reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 7.0f;
            sum[0] += src[i] * 6.0f;
        }
    }
}

/* Test function for worker+vector partitioned (code 6) */
void test_worker_vector_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy worker vector(src[0:n]) copy worker vector(dest[0:n]) \
                         copy worker vector(sum[0:1]) worker(WORKER_SIZE) vector_length(VECTOR_SIZE)
    {
        #pragma acc loop worker vector reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 8.0f;
            sum[0] += src[i] * 7.0f;
        }
    }
}

/* Test function for fully partitioned (code 7) */
void test_fully_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy gang worker vector(src[0:n]) copy gang worker vector(dest[0:n]) \
                         copy gang worker vector(sum[0:1]) gang(GANG_SIZE) worker(WORKER_SIZE) vector_length(VECTOR_SIZE)
    {
        #pragma acc loop gang worker vector reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 9.0f;
            sum[0] += src[i] * 8.0f;
        }
    }
}

/* Combined test that uses volatile to select partition mode at runtime */
void test_all_partitions_combined(const float* src, float* dest, int n, float* sum) {
    /* Force compiler to generate all partition codes by using volatile */
    int mode = force_partition_mode;
    
    if (mode == 0) {
        #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) gang(GANG_SIZE)
        {
            #pragma acc loop gang reduction(+:sum[0])
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] * 2.0f;
                sum[0] += src[i];
            }
        }
    } else if (mode == 1) {
        #pragma acc parallel copy gang(src[0:n]) copy gang(dest[0:n]) copy gang(sum[0:1]) gang(GANG_SIZE)
        {
            #pragma acc loop gang reduction(+:sum[0])
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] * 3.0f;
                sum[0] += src[i] * 2.0f;
            }
        }
    } else if (mode == 2) {
        #pragma acc parallel copy worker(src[0:n]) copy worker(dest[0:n]) copy worker(sum[0:1]) gang(GANG_SIZE) worker(WORKER_SIZE)
        {
            #pragma acc loop gang worker reduction(+:sum[0])
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] * 4.0f;
                sum[0] += src[i] * 3.0f;
            }
        }
    } else if (mode == 3) {
        #pragma acc parallel copy gang worker(src[0:n]) copy gang worker(dest[0:n]) copy gang worker(sum[0:1]) gang(GANG_SIZE) worker(WORKER_SIZE)
        {
            #pragma acc loop gang worker reduction(+:sum[0])
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] * 5.0f;
                sum[0] += src[i] * 4.0f;
            }
        }
    } else if (mode == 4) {
        #pragma acc parallel copy vector(src[0:n]) copy vector(dest[0:n]) copy vector(sum[0:1]) vector_length(VECTOR_SIZE)
        {
            #pragma acc loop vector reduction(+:sum[0])
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] * 6.0f;
                sum[0] += src[i] * 5.0f;
            }
        }
    } else if (mode == 5) {
        #pragma acc parallel copy gang vector(src[0:n]) copy gang vector(dest[0:n]) copy gang vector(sum[0:1]) gang(GANG_SIZE) vector_length(VECTOR_SIZE)
        {
            #pragma acc loop gang vector reduction(+:sum[0])
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] * 7.0f;
                sum[0] += src[i] * 6.0f;
            }
        }
    } else if (mode == 6) {
        #pragma acc parallel copy worker vector(src[0:n]) copy worker vector(dest[0:n]) copy worker vector(sum[0:1]) worker(WORKER_SIZE) vector_length(VECTOR_SIZE)
        {
            #pragma acc loop worker vector reduction(+:sum[0])
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] * 8.0f;
                sum[0] += src[i] * 7.0f;
            }
        }
    } else if (mode == 7) {
        #pragma acc parallel copy gang worker vector(src[0:n]) copy gang worker vector(dest[0:n]) copy gang worker vector(sum[0:1]) gang(GANG_SIZE) worker(WORKER_SIZE) vector_length(VECTOR_SIZE)
        {
            #pragma acc loop gang worker vector reduction(+:sum[0])
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] * 9.0f;
                sum[0] += src[i] * 8.0f;
            }
        }
    }
}

int main() {
    float *src = (float*)malloc(N * sizeof(float));
    float *dest = (float*)malloc(N * sizeof(float));
    float sums[8] = {0};
    
    /* Initialize source array with pattern */
    for (int i = 0; i < N; i++) {
        src[i] = (float)(i % 100) * 0.1f;
    }
    
    printf("Testing OpenACC partition modes (0-7)...\n");
    
    /* Test each partition mode individually */
    test_gang_redundant(src, dest, N, &sums[0]);
    
    for (int i = 0; i < N; i++) dest[i] = 0;
    test_gang_partitioned(src, dest, N, &sums[1]);
    
    for (int i = 0; i < N; i++) dest[i] = 0;
    test_worker_partitioned(src, dest, N, &sums[2]);
    
    for (int i = 0; i < N; i++) dest[i] = 0;
    test_gang_worker_partitioned(src, dest, N, &sums[3]);
    
    for (int i = 0; i < N; i++) dest[i] = 0;
    test_vector_partitioned(src, dest, N, &sums[4]);
    
    for (int i = 0; i < N; i++) dest[i] = 0;
    test_gang_vector_partitioned(src, dest, N, &sums[5]);
    
    for (int i = 0; i < N; i++) dest[i] = 0;
    test_worker_vector_partitioned(src, dest, N, &sums[6]);
    
    for (int i = 0; i < N; i++) dest[i] = 0;
    test_fully_partitioned(src, dest, N, &sums[7]);
    
    /* Also test the combined function with volatile control */
    for (int mode = 0; mode < 8; mode++) {
        force_partition_mode = mode;
        for (int i = 0; i < N; i++) dest[i] = 0;
        float temp_sum = 0;
        test_all_partitions_combined(src, dest, N, &temp_sum);
    }
    
    /* Compute final checksum */
    float final_checksum = 0;
    for (int i = 0; i < 8; i++) {
        final_checksum += sums[i];
    }
    
    /* Add some values from dest to ensure it's used */
    for (int i = 0; i < N; i += 64) {
        final_checksum += dest[i];
    }
    
    printf("Final checksum: %f\n", final_checksum);
    printf("Partition mode tests completed.\n");
    
    free(src);
    free(dest);
    
    return 0;
}
