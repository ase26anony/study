/* test_partition_codes.c - Cover GCC's OpenACC partition code string mapping */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define CHUNK 128

/* Helper to prevent optimization */
static volatile int use_partition_mode = 0;

/* Test functions for each partition code */

/* Code 0: gang redundant */
void test_gang_redundant(const float* src, float* dest, int n, float* sum) {
    use_partition_mode = 0;
    float local_sum = 0.0f;
    
    #pragma acc parallel copyin(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 2.0f;
            local_sum += src[i];
        }
    }
    *sum = local_sum;
}

/* Code 1: gang partitioned */
void test_gang_partitioned(const float* src, float* dest, int n, float* sum) {
    use_partition_mode = 1;
    float local_sum = 0.0f;
    
    #pragma acc parallel copyin(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 3.0f;
            local_sum += src[i] * 2.0f;
        }
    }
    *sum = local_sum;
}

/* Code 2: worker partitioned */
void test_worker_partitioned(const float* src, float* dest, int n, float* sum) {
    use_partition_mode = 2;
    float local_sum = 0.0f;
    
    #pragma acc parallel copyin(src[0:n]) create(dest[0:n]) copyout(local_sum) \
                         num_gangs(2) num_workers(4) vector_length(16)
    {
        #pragma acc loop worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 4.0f;
            local_sum += src[i] * 3.0f;
        }
    }
    *sum = local_sum;
}

/* Code 3: gang+worker partitioned */
void test_gang_worker_partitioned(const float* src, float* dest, int n, float* sum) {
    use_partition_mode = 3;
    float local_sum = 0.0f;
    
    #pragma acc parallel copyin(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(4) vector_length(16)
    {
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 5.0f;
            local_sum += src[i] * 4.0f;
        }
    }
    *sum = local_sum;
}

/* Code 4: vector partitioned */
void test_vector_partitioned(const float* src, float* dest, int n, float* sum) {
    use_partition_mode = 4;
    float local_sum = 0.0f;
    
    #pragma acc parallel copyin(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(1) num_workers(1) vector_length(64)
    {
        #pragma acc loop vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 6.0f;
            local_sum += src[i] * 5.0f;
        }
    }
    *sum = local_sum;
}

/* Code 5: gang+vector partitioned */
void test_gang_vector_partitioned(const float* src, float* dest, int n, float* sum) {
    use_partition_mode = 5;
    float local_sum = 0.0f;
    
    #pragma acc parallel copyin(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(1) vector_length(32)
    {
        #pragma acc loop gang vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 7.0f;
            local_sum += src[i] * 6.0f;
        }
    }
    *sum = local_sum;
}

/* Code 6: worker+vector partitioned */
void test_worker_vector_partitioned(const float* src, float* dest, int n, float* sum) {
    use_partition_mode = 6;
    float local_sum = 0.0f;
    
    #pragma acc parallel copyin(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(1) num_workers(4) vector_length(32)
    {
        #pragma acc loop worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 8.0f;
            local_sum += src[i] * 7.0f;
        }
    }
    *sum = local_sum;
}

/* Code 7: fully partitioned */
void test_fully_partitioned(const float* src, float* dest, int n, float* sum) {
    use_partition_mode = 7;
    float local_sum = 0.0f;
    
    #pragma acc parallel copyin(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(4) vector_length(32)
    {
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 9.0f;
            local_sum += src[i] * 8.0f;
        }
    }
    *sum = local_sum;
}

/* Alternative approach using data regions with explicit partition clauses */

void test_data_partition_gang(float* data, int n) {
    use_partition_mode = 1;
    
    #pragma acc data copy(data[0:n])
    {
        #pragma acc parallel present(data[0:n]) num_gangs(4)
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                data[i] = data[i] * 1.5f;
            }
        }
    }
}

void test_data_partition_worker(float* data, int n) {
    use_partition_mode = 2;
    
    #pragma acc data copy(data[0:n])
    {
        #pragma acc parallel present(data[0:n]) num_workers(4)
        {
            #pragma acc loop worker
            for (int i = 0; i < n; i++) {
                data[i] = data[i] * 2.5f;
            }
        }
    }
}

void test_data_partition_vector(float* data, int n) {
    use_partition_mode = 4;
    
    #pragma acc data copy(data[0:n])
    {
        #pragma acc parallel present(data[0:n]) vector_length(32)
        {
            #pragma acc loop vector
            for (int i = 0; i < n; i++) {
                data[i] = data[i] * 3.5f;
            }
        }
    }
}

/* Test with explicit data clause modifiers */
void test_explicit_partition_modifiers(float* src, float* dest, int n) {
    float sum = 0.0f;
    
    /* Test different combinations of partition modifiers */
    #pragma acc data copyin(src[0:n]) copy(dest[0:n]) copyout(sum)
    {
        /* gang partitioned */
        #pragma acc parallel present(src, dest) reduction(+:sum) \
                         num_gangs(4)
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                dest[i] = src[i];
                sum += src[i];
            }
        }
        
        /* worker partitioned */
        #pragma acc parallel present(src, dest) reduction(+:sum) \
                         num_workers(4)
        {
            #pragma acc loop worker
            for (int i = 0; i < n; i++) {
                dest[i] += src[i];
                sum += dest[i];
            }
        }
        
        /* vector partitioned */
        #pragma acc parallel present(src, dest) reduction(+:sum) \
                         vector_length(32)
        {
            #pragma acc loop vector
            for (int i = 0; i < n; i++) {
                dest[i] *= 2.0f;
                sum += dest[i];
            }
        }
    }
    
    /* Force use of volatile to prevent dead code elimination */
    if (use_partition_mode > 0) {
        printf("Intermediate sum: %f\n", sum);
    }
}

/* Main test driver */
int main() {
    float* src = (float*)malloc(N * sizeof(float));
    float* dest = (float*)malloc(N * sizeof(float));
    float sums[8];
    float total_sum = 0.0f;
    
    /* Initialize source data */
    for (int i = 0; i < N; i++) {
        src[i] = (float)(i % 100) * 0.1f;
        dest[i] = 0.0f;
    }
    
    printf("Testing OpenACC partition code coverage...\n");
    
    /* Test each partition mode */
    test_gang_redundant(src, dest, N, &sums[0]);
    test_gang_partitioned(src, dest, N, &sums[1]);
    test_worker_partitioned(src, dest, N, &sums[2]);
    test_gang_worker_partitioned(src, dest, N, &sums[3]);
    test_vector_partitioned(src, dest, N, &sums[4]);
    test_gang_vector_partitioned(src, dest, N, &sums[5]);
    test_worker_vector_partitioned(src, dest, N, &sums[6]);
    test_fully_partitioned(src, dest, N, &sums[7]);
    
    /* Test data regions with partition */
    test_data_partition_gang(dest, N);
    test_data_partition_worker(dest, N);
    test_data_partition_vector(dest, N);
    
    /* Test explicit modifiers */
    test_explicit_partition_modifiers(src, dest, N);
    
    /* Compute final checksum */
    for (int i = 0; i < 8; i++) {
        total_sum += sums[i];
    }
    
    /* Add array checksum */
    float array_sum = 0.0f;
    for (int i = 0; i < N; i++) {
        array_sum += dest[i];
    }
    total_sum += array_sum;
    
    printf("Final checksum: %f\n", total_sum);
    printf("Partition mode used (volatile): %d\n", use_partition_mode);
    
    free(src);
    free(dest);
    
    return 0;
}
