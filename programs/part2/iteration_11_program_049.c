/* test_partition_codes.c - Cover partition code string mapping in omp-oacc-neuter-broadcast.cc */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 128

/* Use volatile to prevent compile-time elimination */
static volatile int use_partition_mode = 0;

/* Test function for gang redundant (code 0) */
void test_gang_redundant(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
                         gang(num:32) vector_length(32)
    {
        float local_sum = 0.0f;
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 2.0f;
            local_sum += src[i];
        }
        if (acc_on_device(acc_device_not_host)) {
            #pragma acc atomic update
            sum[0] += local_sum;
        }
    }
}

/* Test function for gang partitioned (code 1) */
void test_gang_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
                         gang(num:16) vector_length(32)
    {
        float local_sum = 0.0f;
        #pragma acc loop gang gang_partitioned(src, dest) reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 3.0f;
            local_sum += src[i];
        }
        if (acc_on_device(acc_device_not_host)) {
            #pragma acc atomic update
            sum[0] += local_sum;
        }
    }
}

/* Test function for worker partitioned (code 2) */
void test_worker_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
                         num_gangs(8) num_workers(4) vector_length(32)
    {
        float local_sum = 0.0f;
        #pragma acc loop gang worker worker_partitioned(src, dest) reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 4.0f;
            local_sum += src[i];
        }
        if (acc_on_device(acc_device_not_host)) {
            #pragma acc atomic update
            sum[0] += local_sum;
        }
    }
}

/* Test function for gang+worker partitioned (code 3) */
void test_gang_worker_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
                         num_gangs(8) num_workers(4) vector_length(32)
    {
        float local_sum = 0.0f;
        #pragma acc loop gang worker gang_partitioned(src) worker_partitioned(dest) reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 5.0f;
            local_sum += src[i];
        }
        if (acc_on_device(acc_device_not_host)) {
            #pragma acc atomic update
            sum[0] += local_sum;
        }
    }
}

/* Test function for vector partitioned (code 4) */
void test_vector_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
                         num_gangs(8) vector_length(64)
    {
        float local_sum = 0.0f;
        #pragma acc loop gang vector vector_partitioned(src, dest) reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 6.0f;
            local_sum += src[i];
        }
        if (acc_on_device(acc_device_not_host)) {
            #pragma acc atomic update
            sum[0] += local_sum;
        }
    }
}

/* Test function for gang+vector partitioned (code 5) */
void test_gang_vector_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
                         num_gangs(8) vector_length(64)
    {
        float local_sum = 0.0f;
        #pragma acc loop gang vector gang_partitioned(src) vector_partitioned(dest) reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 7.0f;
            local_sum += src[i];
        }
        if (acc_on_device(acc_device_not_host)) {
            #pragma acc atomic update
            sum[0] += local_sum;
        }
    }
}

/* Test function for worker+vector partitioned (code 6) */
void test_worker_vector_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
                         num_gangs(4) num_workers(2) vector_length(64)
    {
        float local_sum = 0.0f;
        #pragma acc loop worker vector worker_partitioned(src) vector_partitioned(dest) reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 8.0f;
            local_sum += src[i];
        }
        if (acc_on_device(acc_device_not_host)) {
            #pragma acc atomic update
            sum[0] += local_sum;
        }
    }
}

/* Test function for fully partitioned (code 7) */
void test_fully_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
                         num_gangs(4) num_workers(2) vector_length(64)
    {
        float local_sum = 0.0f;
        #pragma acc loop gang worker vector \
                         gang_partitioned(src) worker_partitioned(dest) vector_partitioned(dest) \
                         reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 9.0f;
            local_sum += src[i];
        }
        if (acc_on_device(acc_device_not_host)) {
            #pragma acc atomic update
            sum[0] += local_sum;
        }
    }
}

/* Alternative approach using data regions with explicit partition clauses */
void test_data_partition_modes(float *src, float *dest, int n, float *sum) {
    /* Test different data partition modes using volatile to prevent elimination */
    switch (use_partition_mode) {
        case 0:
            #pragma acc data copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1])
            #pragma acc parallel loop gang reduction(+:sum[0])
            for (int i = 0; i < n; i++) {
                dest[i] = src[i];
                sum[0] += src[i];
            }
            break;
            
        case 1:
            #pragma acc data copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1])
            #pragma acc parallel loop gang gang_partitioned(src, dest) reduction(+:sum[0])
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] * 2;
                sum[0] += src[i];
            }
            break;
            
        case 2:
            #pragma acc data copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1])
            #pragma acc parallel loop worker worker_partitioned(src, dest) reduction(+:sum[0])
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] * 3;
                sum[0] += src[i];
            }
            break;
            
        case 3:
            #pragma acc data copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1])
            #pragma acc parallel loop gang worker \
                         gang_partitioned(src) worker_partitioned(dest) reduction(+:sum[0])
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] * 4;
                sum[0] += src[i];
            }
            break;
            
        case 4:
            #pragma acc data copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1])
            #pragma acc parallel loop vector vector_partitioned(src, dest) reduction(+:sum[0])
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] * 5;
                sum[0] += src[i];
            }
            break;
            
        case 5:
            #pragma acc data copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1])
            #pragma acc parallel loop gang vector \
                         gang_partitioned(src) vector_partitioned(dest) reduction(+:sum[0])
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] * 6;
                sum[0] += src[i];
            }
            break;
            
        case 6:
            #pragma acc data copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1])
            #pragma acc parallel loop worker vector \
                         worker_partitioned(src) vector_partitioned(dest) reduction(+:sum[0])
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] * 7;
                sum[0] += src[i];
            }
            break;
            
        case 7:
            #pragma acc data copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1])
            #pragma acc parallel loop gang worker vector \
                         gang_partitioned(src) worker_partitioned(dest) vector_partitioned(dest) \
                         reduction(+:sum[0])
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] * 8;
                sum[0] += src[i];
            }
            break;
    }
}

/* Multi-dimensional array test to trigger complex partitioning */
void test_multi_dim_partition(float src[M][M], float dest[M][M], float *sum) {
    /* This should generate various partition codes for 2D arrays */
    #pragma acc data copy(src[0:M][0:M]) copy(dest[0:M][0:M]) copy(sum[0:1])
    {
        #pragma acc parallel loop gang collapse(2) reduction(+:sum[0])
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                dest[i][j] = src[i][j] * 2.0f;
                sum[0] += src[i][j];
            }
        }
        
        #pragma acc parallel loop gang worker collapse(2) reduction(+:sum[0])
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                dest[i][j] += src[i][j];
                sum[0] += src[i][j];
            }
        }
        
        #pragma acc parallel loop vector collapse(2) reduction(+:sum[0])
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                dest[i][j] *= 1.5f;
                sum[0] += src[i][j];
            }
        }
    }
}

int main() {
    float *src = (float*)malloc(N * sizeof(float));
    float *dest = (float*)malloc(N * sizeof(float));
    float sum = 0.0f;
    float total_checksum = 0.0f;
    
    /* Initialize source array */
    for (int i = 0; i < N; i++) {
        src[i] = (float)(i % 100) * 0.1f;
    }
    
    printf("Testing OpenACC partition modes...\n");
    
    /* Test all partition modes using individual functions */
    test_gang_redundant(src, dest, N, &sum);
    total_checksum += sum;
    printf("  Gang redundant complete, sum = %f\n", sum);
    
    sum = 0.0f;
    test_gang_partitioned(src, dest, N, &sum);
    total_checksum += sum;
    printf("  Gang partitioned complete, sum = %f\n", sum);
    
    sum = 0.0f;
    test_worker_partitioned(src, dest, N, &sum);
    total_checksum += sum;
    printf("  Worker partitioned complete, sum = %f\n", sum);
    
    sum = 0.0f;
    test_gang_worker_partitioned(src, dest, N, &sum);
    total_checksum += sum;
    printf("  Gang+Worker partitioned complete, sum = %f\n", sum);
    
    sum = 0.0f;
    test_vector_partitioned(src, dest, N, &sum);
    total_checksum += sum;
    printf("  Vector partitioned complete, sum = %f\n", sum);
    
    sum = 0.0f;
    test_gang_vector_partitioned(src, dest, N, &sum);
    total_checksum += sum;
    printf("  Gang+Vector partitioned complete, sum = %f\n", sum);
    
    sum = 0.0f;
    test_worker_vector_partitioned(src, dest, N, &sum);
    total_checksum += sum;
    printf("  Worker+Vector partitioned complete, sum = %f\n", sum);
    
    sum = 0.0f;
    test_fully_partitioned(src, dest, N, &sum);
    total_checksum += sum;
    printf("  Fully partitioned complete, sum = %f\n", sum);
    
    /* Test all modes using switch-based approach */
    for (int mode = 0; mode < 8; mode++) {
        use_partition_mode = mode;
        sum = 0.0f;
        memset(dest, 0, N * sizeof(float));
        test_data_partition_modes(src, dest, N, &sum);
        total_checksum += sum;
        printf("  Mode %d complete, sum = %f\n", mode, sum);
    }
    
    /* Test multi-dimensional arrays */
    float src2d[M][M], dest2d[M][M];
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            src2d[i][j] = (float)((i * M + j) % 100) * 0.01f;
        }
    }
    
    sum = 0.0f;
    test_multi_dim_partition(src2d, dest2d, &sum);
    total_checksum += sum;
    printf("  Multi-dimensional partition complete, sum = %f\n", sum);
    
    /* Final checksum to ensure all computations happened */
    printf("\nTotal checksum: %f\n", total_checksum);
    
    free(src);
    free(dest);
    
    return 0;
}
