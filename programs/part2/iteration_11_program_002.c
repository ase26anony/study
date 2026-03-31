/* test_partition_codes.c - Cover GCC's omp-oacc-neuter-broadcast.cc partition string mapping */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 32

/* Helper to prevent optimization */
static volatile int use_partition = 0;

/* Test functions for each partition code 0-7 */

/* Code 0: gang redundant */
void test_gang_redundant(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n], dest[0:n]) copy(sum[0:1]) gang
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
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop gang reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 3.0f;
            sum[0] += src[i] * 2;
        }
    }
}

/* Code 2: worker partitioned */
void test_worker_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop worker reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 4.0f;
            sum[0] += src[i] * 3;
        }
    }
}

/* Code 3: gang+worker partitioned */
void test_gang_worker_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop gang worker reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 5.0f;
            sum[0] += src[i] * 4;
        }
    }
}

/* Code 4: vector partitioned */
void test_vector_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop vector reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 6.0f;
            sum[0] += src[i] * 5;
        }
    }
}

/* Code 5: gang+vector partitioned */
void test_gang_vector_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop gang vector reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 7.0f;
            sum[0] += src[i] * 6;
        }
    }
}

/* Code 6: worker+vector partitioned */
void test_worker_vector_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop worker vector reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 8.0f;
            sum[0] += src[i] * 7;
        }
    }
}

/* Code 7: fully partitioned */
void test_fully_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop gang worker vector reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 9.0f;
            sum[0] += src[i] * 8;
        }
    }
}

/* Additional tests with explicit data clauses and partition modifiers */

void test_explicit_gang_partition(float *src, float *dest, int n, float *sum) {
    #pragma acc data copyin(src[0:n]) copyout(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc parallel present(src, dest, sum)
        {
            #pragma acc loop gang reduction(+:sum[0])
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] * 10.0f;
                sum[0] += src[i] * 9;
            }
        }
    }
}

void test_explicit_worker_partition(float *src, float *dest, int n, float *sum) {
    #pragma acc data copyin(src[0:n]) copyout(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc parallel present(src, dest, sum)
        {
            #pragma acc loop worker reduction(+:sum[0])
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] * 11.0f;
                sum[0] += src[i] * 10;
            }
        }
    }
}

void test_explicit_vector_partition(float *src, float *dest, int n, float *sum) {
    #pragma acc data copyin(src[0:n]) copyout(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc parallel present(src, dest, sum)
        {
            #pragma acc loop vector reduction(+:sum[0])
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] * 12.0f;
                sum[0] += src[i] * 11;
            }
        }
    }
}

/* Test with nested loops to trigger complex partitioning */
void test_nested_partition(float *src, float *dest, int n, int m, float *sum) {
    #pragma acc parallel copy(src[0:n*m]) copy(dest[0:n*m]) copy(sum[0:1])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                dest[idx] = src[idx] * 13.0f;
                #pragma acc atomic update
                sum[0] += src[idx] * 12;
            }
        }
    }
}

/* Test with multiple arrays and different partition modes */
void test_multi_array_partition(float *a, float *b, float *c, int n, float *sum) {
    #pragma acc parallel copy(a[0:n], b[0:n]) copyout(c[0:n]) copy(sum[0:1])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i];
            sum[0] += a[i] - b[i];
        }
    }
}

int main() {
    float *src = (float*)malloc(N * sizeof(float));
    float *dest = (float*)malloc(N * sizeof(float));
    float *src2 = (float*)malloc(N * M * sizeof(float));
    float *dest2 = (float*)malloc(N * M * sizeof(float));
    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));
    float *c = (float*)malloc(N * sizeof(float));
    
    float sums[12] = {0};
    float total_sum = 0.0f;
    
    /* Initialize arrays with pattern data */
    for (int i = 0; i < N; i++) {
        src[i] = (float)(i % 100) * 0.1f;
        a[i] = (float)(i % 50) * 0.2f;
        b[i] = (float)(i % 30) * 0.3f;
    }
    
    for (int i = 0; i < N * M; i++) {
        src2[i] = (float)(i % 200) * 0.05f;
    }
    
    /* Use volatile to prevent compile-time elimination */
    if (use_partition > 0) {
        /* Call all test functions to trigger different partition codes */
        test_gang_redundant(src, dest, N, &sums[0]);
        test_gang_partitioned(src, dest, N, &sums[1]);
        test_worker_partitioned(src, dest, N, &sums[2]);
        test_gang_worker_partitioned(src, dest, N, &sums[3]);
        test_vector_partitioned(src, dest, N, &sums[4]);
        test_gang_vector_partitioned(src, dest, N, &sums[5]);
        test_worker_vector_partitioned(src, dest, N, &sums[6]);
        test_fully_partitioned(src, dest, N, &sums[7]);
        
        test_explicit_gang_partition(src, dest, N, &sums[8]);
        test_explicit_worker_partition(src, dest, N, &sums[9]);
        test_explicit_vector_partition(src, dest, N, &sums[10]);
        
        test_nested_partition(src2, dest2, N, M, &sums[11]);
        test_multi_array_partition(a, b, c, N, &total_sum);
    } else {
        /* Fallback execution path */
        test_gang_redundant(src, dest, N, &sums[0]);
        test_gang_partitioned(src, dest, N, &sums[1]);
        test_worker_partitioned(src, dest, N, &sums[2]);
        test_gang_worker_partitioned(src, dest, N, &sums[3]);
        test_vector_partitioned(src, dest, N, &sums[4]);
        test_gang_vector_partitioned(src, dest, N, &sums[5]);
        test_worker_vector_partitioned(src, dest, N, &sums[6]);
        test_fully_partitioned(src, dest, N, &sums[7]);
        
        test_explicit_gang_partition(src, dest, N, &sums[8]);
        test_explicit_worker_partition(src, dest, N, &sums[9]);
        test_explicit_vector_partition(src, dest, N, &sums[10]);
        
        test_nested_partition(src2, dest2, N, M, &sums[11]);
        test_multi_array_partition(a, b, c, N, &total_sum);
    }
    
    /* Compute final checksum to ensure code isn't optimized away */
    float final_checksum = total_sum;
    for (int i = 0; i < 12; i++) {
        final_checksum += sums[i];
    }
    
    /* Add some array elements to checksum */
    for (int i = 0; i < N; i += 64) {
        final_checksum += dest[i] + c[i];
    }
    
    printf("Final checksum: %f\n", final_checksum);
    
    free(src);
    free(dest);
    free(src2);
    free(dest2);
    free(a);
    free(b);
    free(c);
    
    return 0;
}
