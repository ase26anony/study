/* test_openacc_partition_codes.c
 * 
 * This program systematically exercises OpenACC data partition codes 0-7
 * to trigger the string lookup logic in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -fdump-tree-omplower -fdump-tree-optimized test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define N 1024
#define M 32

/* Use volatile to prevent compile-time elimination */
static volatile int use_partition_mode = 0;

/* Test function for partition code 0: gang redundant */
void test_gang_redundant(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n], dest[0:n]) copyout(sum[0:1]) gang
    {
        float local_sum = 0.0f;
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 2.0f;
            local_sum += src[i];
        }
        sum[0] = local_sum;
    }
}

/* Test function for partition code 1: gang partitioned */
void test_gang_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 3.0f;
            local_sum += src[i];
        }
        sum[0] = local_sum;
    }
}

/* Test function for partition code 2: worker partitioned */
void test_worker_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 4.0f;
            local_sum += src[i];
        }
        sum[0] = local_sum;
    }
}

/* Test function for partition code 3: gang+worker partitioned */
void test_gang_worker_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 5.0f;
            local_sum += src[i];
        }
        sum[0] = local_sum;
    }
}

/* Test function for partition code 4: vector partitioned */
void test_vector_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 6.0f;
            local_sum += src[i];
        }
        sum[0] = local_sum;
    }
}

/* Test function for partition code 5: gang+vector partitioned */
void test_gang_vector_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 7.0f;
            local_sum += src[i];
        }
        sum[0] = local_sum;
    }
}

/* Test function for partition code 6: worker+vector partitioned */
void test_worker_vector_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 8.0f;
            local_sum += src[i];
        }
        sum[0] = local_sum;
    }
}

/* Test function for partition code 7: fully partitioned */
void test_fully_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 9.0f;
            local_sum += src[i];
        }
        sum[0] = local_sum;
    }
}

/* Additional tests with explicit data clauses and partition modifiers */
void test_explicit_partition_clauses() {
    float a[N], b[N], c[N];
    float sum1 = 0.0f, sum2 = 0.0f, sum3 = 0.0f;
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(i * 2);
        c[i] = 0.0f;
    }
    
    /* Test 1: gang partitioned with explicit data clause */
    #pragma acc data copyin(a[0:N]) copyout(b[0:N]) create(c[0:N])
    {
        #pragma acc parallel present(a, b, c)
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                c[i] = a[i] + b[i];
            }
        }
    }
    
    /* Test 2: worker partitioned with explicit data clause */
    #pragma acc data copyin(a[0:N]) copy(b[0:N])
    {
        #pragma acc parallel present(a, b)
        {
            #pragma acc loop worker
            for (int i = 0; i < N; i++) {
                b[i] = a[i] * 2.0f;
            }
        }
    }
    
    /* Test 3: vector partitioned with explicit data clause */
    #pragma acc data copy(a[0:N], b[0:N])
    {
        #pragma acc parallel present(a, b)
        {
            #pragma acc loop vector
            for (int i = 0; i < N; i++) {
                b[i] = a[i] / 2.0f;
            }
        }
    }
    
    /* Test nested loops with different partition combinations */
    float matrix[M][M];
    float vector[M];
    float result[M];
    
    /* Initialize matrix and vector */
    for (int i = 0; i < M; i++) {
        vector[i] = (float)i;
        result[i] = 0.0f;
        for (int j = 0; j < M; j++) {
            matrix[i][j] = (float)(i * M + j);
        }
    }
    
    /* Matrix-vector multiplication with gang-worker partitioning */
    #pragma acc data copyin(matrix, vector) copyout(result)
    {
        #pragma acc parallel present(matrix, vector, result)
        {
            #pragma acc loop gang
            for (int i = 0; i < M; i++) {
                float temp = 0.0f;
                #pragma acc loop worker reduction(+:temp)
                for (int j = 0; j < M; j++) {
                    temp += matrix[i][j] * vector[j];
                }
                result[i] = temp;
            }
        }
    }
    
    /* Another test with gang-vector partitioning */
    #pragma acc data copy(result[0:M])
    {
        #pragma acc parallel present(result)
        {
            #pragma acc loop gang vector
            for (int i = 0; i < M; i++) {
                result[i] = result[i] * 1.5f;
            }
        }
    }
}

/* Main function that exercises all partition modes */
int main() {
    float* src = (float*)malloc(N * sizeof(float));
    float* dest = (float*)malloc(N * sizeof(float));
    float sums[8] = {0.0f};
    
    if (!src || !dest) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize source array with patterned data */
    for (int i = 0; i < N; i++) {
        src[i] = (float)((i % 100) + 1) * 0.5f;
    }
    
    /* Clear destination array */
    for (int i = 0; i < N; i++) {
        dest[i] = 0.0f;
    }
    
    printf("Testing OpenACC partition codes 0-7...\n");
    
    /* Use volatile to force runtime selection and prevent dead code elimination */
    if (use_partition_mode >= 0) {
        /* Test all partition codes systematically */
        test_gang_redundant(src, dest, N, &sums[0]);
        
        for (int i = 0; i < N; i++) dest[i] = 0.0f;
        test_gang_partitioned(src, dest, N, &sums[1]);
        
        for (int i = 0; i < N; i++) dest[i] = 0.0f;
        test_worker_partitioned(src, dest, N, &sums[2]);
        
        for (int i = 0; i < N; i++) dest[i] = 0.0f;
        test_gang_worker_partitioned(src, dest, N, &sums[3]);
        
        for (int i = 0; i < N; i++) dest[i] = 0.0f;
        test_vector_partitioned(src, dest, N, &sums[4]);
        
        for (int i = 0; i < N; i++) dest[i] = 0.0f;
        test_gang_vector_partitioned(src, dest, N, &sums[5]);
        
        for (int i = 0; i < N; i++) dest[i] = 0.0f;
        test_worker_vector_partitioned(src, dest, N, &sums[6]);
        
        for (int i = 0; i < N; i++) dest[i] = 0.0f;
        test_fully_partitioned(src, dest, N, &sums[7]);
    }
    
    /* Test explicit partition clauses */
    test_explicit_partition_clauses();
    
    /* Compute final checksum to ensure code has side effects */
    float final_checksum = 0.0f;
    for (int i = 0; i < N; i++) {
        final_checksum += dest[i];
    }
    for (int i = 0; i < 8; i++) {
        final_checksum += sums[i];
    }
    
    printf("Final checksum: %f\n", final_checksum);
    
    /* Additional test with kernels construct */
    {
        float x[N], y[N];
        for (int i = 0; i < N; i++) {
            x[i] = (float)i;
            y[i] = 0.0f;
        }
        
        #pragma acc kernels copy(x[0:N]) copyout(y[0:N])
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                y[i] = x[i] * x[i];
            }
        }
        
        float kernel_sum = 0.0f;
        for (int i = 0; i < N; i++) {
            kernel_sum += y[i];
        }
        printf("Kernels construct result sum: %f\n", kernel_sum);
    }
    
    free(src);
    free(dest);
    
    return 0;
}
