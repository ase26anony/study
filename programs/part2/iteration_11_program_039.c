/* test_openacc_partitions.c - Coverage for omp-oacc-neuter-broadcast.cc partition codes */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define N 1024
#define M 32

/* Helper to prevent optimization */
static volatile int use_partition = 0;

/* Test functions for each partition code */

/* Code 0: gang redundant */
void test_gang_redundant(float *src, float *dest, int n, float *sum) {
    use_partition = 0; /* Prevent dead code elimination */
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) /* gang redundant */
    {
        #pragma acc loop gang reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 2.0f;
            sum[0] += src[i];
        }
    }
}

/* Code 1: gang partitioned */
void test_gang_partitioned(float *src, float *dest, int n, float *sum) {
    use_partition = 1;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop gang /* Explicit gang loop */
        for (int i = 0; i < n; i += M) {
            #pragma acc loop worker vector
            for (int j = i; j < i + M && j < n; j++) {
                dest[j] = src[j] * 3.0f;
                if (j % 16 == 0) sum[0] += src[j];
            }
        }
    }
}

/* Code 2: worker partitioned */
void test_worker_partitioned(float *src, float *dest, int n, float *sum) {
    use_partition = 2;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i += M) {
            #pragma acc loop worker /* worker partitioned */
            for (int j = i; j < i + M && j < n; j++) {
                #pragma acc loop vector
                for (int k = 0; k < 4; k++) {
                    dest[j] += src[j] * (k + 1);
                }
                sum[0] += dest[j];
            }
        }
    }
}

/* Code 3: gang+worker partitioned */
void test_gang_worker_partitioned(float *src, float *dest, int n, float *sum) {
    use_partition = 3;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop gang worker /* gang+worker partitioned */
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 4.0f;
            #pragma acc loop vector
            for (int k = 0; k < 8; k++) {
                sum[0] += src[i] * k;
            }
        }
    }
}

/* Code 4: vector partitioned */
void test_vector_partitioned(float *src, float *dest, int n, float *sum) {
    use_partition = 4;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector /* vector partitioned */
            for (int k = 0; k < 16; k++) {
                dest[i] += src[i] * k;
            }
            sum[0] += dest[i];
        }
    }
}

/* Code 5: gang+vector partitioned */
void test_gang_vector_partitioned(float *src, float *dest, int n, float *sum) {
    use_partition = 5;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop gang vector /* gang+vector partitioned */
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 5.0f;
            sum[0] += src[i];
        }
    }
}

/* Code 6: worker+vector partitioned */
void test_worker_vector_partitioned(float *src, float *dest, int n, float *sum) {
    use_partition = 6;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i += M) {
            #pragma acc loop worker vector /* worker+vector partitioned */
            for (int j = i; j < i + M && j < n; j++) {
                dest[j] = src[j] * 6.0f;
                sum[0] += src[j];
            }
        }
    }
}

/* Code 7: fully partitioned */
void test_fully_partitioned(float *src, float *dest, int n, float *sum) {
    use_partition = 7;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop gang worker vector /* fully partitioned */
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 7.0f;
            sum[0] += src[i];
        }
    }
}

/* Additional tests with explicit data clauses and partition modifiers */

void test_explicit_data_clauses(float *src, float *dest, int n, float *sum) {
    /* Test various data clause combinations that map to partition codes */
    
    /* gang partitioned data */
    #pragma acc data copyin(src[0:n]) copyout(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc parallel present(src, dest, sum)
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                dest[i] = src[i];
                sum[0] += src[i];
            }
        }
    }
    
    /* worker partitioned data */
    float *worker_data = (float *)malloc(n * sizeof(float));
    #pragma acc data create(worker_data[0:n])
    {
        #pragma acc parallel present(worker_data)
        {
            #pragma acc loop worker
            for (int i = 0; i < n; i++) {
                worker_data[i] = i * 1.5f;
            }
        }
    }
    free(worker_data);
    
    /* vector partitioned data */
    float *vector_data = (float *)malloc(n * sizeof(float));
    #pragma acc data copyout(vector_data[0:n])
    {
        #pragma acc parallel present(vector_data)
        {
            #pragma acc loop vector
            for (int i = 0; i < n; i++) {
                vector_data[i] = i * 2.5f;
            }
        }
    }
    free(vector_data);
}

/* Complex nested partitioning test */
void test_nested_partitioning(float *src, float *dest, int n, float *sum) {
    use_partition = 8; /* Will use default case */
    
    /* Multi-dimensional array for complex partitioning */
    float matrix[M][M];
    
    #pragma acc data copyin(src[0:n]) copyout(dest[0:n]) create(matrix) copy(sum[0:1])
    {
        #pragma acc parallel present(src, dest, matrix, sum)
        {
            /* Initialize matrix with gang partitioning */
            #pragma acc loop gang
            for (int i = 0; i < M; i++) {
                #pragma acc loop worker
                for (int j = 0; j < M; j++) {
                    matrix[i][j] = i * M + j;
                }
            }
            
            /* Process with worker+vector partitioning */
            #pragma acc loop gang
            for (int i = 0; i < n; i += M) {
                #pragma acc loop worker vector
                for (int j = i; j < i + M && j < n; j++) {
                    int idx = j % (M * M);
                    dest[j] = src[j] + matrix[idx / M][idx % M];
                    sum[0] += dest[j];
                }
            }
        }
    }
}

int main() {
    const int n = N;
    float *src = (float *)malloc(n * sizeof(float));
    float *dest = (float *)malloc(n * sizeof(float));
    float sums[8] = {0}; /* One sum per test case */
    
    /* Initialize source array */
    for (int i = 0; i < n; i++) {
        src[i] = (float)(i + 1) * 0.1f;
    }
    
    printf("Testing OpenACC partition codes 0-7...\n");
    
    /* Test all partition codes */
    test_gang_redundant(src, dest, n, &sums[0]);
    printf("  Gang redundant complete, sum = %f\n", sums[0]);
    
    test_gang_partitioned(src, dest, n, &sums[1]);
    printf("  Gang partitioned complete, sum = %f\n", sums[1]);
    
    test_worker_partitioned(src, dest, n, &sums[2]);
    printf("  Worker partitioned complete, sum = %f\n", sums[2]);
    
    test_gang_worker_partitioned(src, dest, n, &sums[3]);
    printf("  Gang+worker partitioned complete, sum = %f\n", sums[3]);
    
    test_vector_partitioned(src, dest, n, &sums[4]);
    printf("  Vector partitioned complete, sum = %f\n", sums[4]);
    
    test_gang_vector_partitioned(src, dest, n, &sums[5]);
    printf("  Gang+vector partitioned complete, sum = %f\n", sums[5]);
    
    test_worker_vector_partitioned(src, dest, n, &sums[6]);
    printf("  Worker+vector partitioned complete, sum = %f\n", sums[6]);
    
    test_fully_partitioned(src, dest, n, &sums[7]);
    printf("  Fully partitioned complete, sum = %f\n", sums[7]);
    
    /* Additional tests */
    float extra_sum = 0;
    test_explicit_data_clauses(src, dest, n, &extra_sum);
    test_nested_partitioning(src, dest, n, &extra_sum);
    
    /* Compute final checksum */
    float final_checksum = 0;
    for (int i = 0; i < 8; i++) {
        final_checksum += sums[i];
    }
    for (int i = 0; i < n; i++) {
        final_checksum += dest[i];
    }
    
    printf("\nFinal checksum: %f\n", final_checksum);
    printf("Partition code testing complete.\n");
    
    free(src);
    free(dest);
    
    return 0;
}
