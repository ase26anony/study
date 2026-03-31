/* test_openacc_partitions.c
 * Designed to exercise GCC's OpenACC partition code mapping logic
 * Compile with: gcc -O2 -fopenacc -fdump-tree-omplower -fdump-tree-optimized test.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define N 1024
#define CHUNK_SIZE 128

/* Use volatile to prevent compile-time elimination */
static volatile int force_partition_mode = 0;

/* Test function for partition code 0: gang redundant */
void test_gang_redundant(float *src, float *dest, int n, float *sum) {
    force_partition_mode = 0;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
        present_or_copy(src[0:n]) /* gang redundant by default */
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 2.0f;
            local_sum += src[i];
        }
        
        if (__builtin_acc_on_device()) {
            #pragma acc atomic update
            sum[0] += local_sum;
        }
    }
}

/* Test function for partition code 1: gang partitioned */
void test_gang_partitioned(float *src, float *dest, int n, float *sum) {
    force_partition_mode = 1;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
        copy gang(src[0:n], dest[0:n]) /* gang partitioned */
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 3.0f;
            local_sum += src[i];
        }
        
        if (__builtin_acc_on_device()) {
            #pragma acc atomic update
            sum[0] += local_sum;
        }
    }
}

/* Test function for partition code 2: worker partitioned */
void test_worker_partitioned(float *src, float *dest, int n, float *sum) {
    force_partition_mode = 2;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
        copy worker(src[0:n], dest[0:n]) /* worker partitioned */
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 4.0f;
            local_sum += src[i];
        }
        
        if (__builtin_acc_on_device()) {
            #pragma acc atomic update
            sum[0] += local_sum;
        }
    }
}

/* Test function for partition code 3: gang+worker partitioned */
void test_gang_worker_partitioned(float *src, float *dest, int n, float *sum) {
    force_partition_mode = 3;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
        copy gang worker(src[0:n], dest[0:n]) /* gang+worker partitioned */
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 5.0f;
            local_sum += src[i];
        }
        
        if (__builtin_acc_on_device()) {
            #pragma acc atomic update
            sum[0] += local_sum;
        }
    }
}

/* Test function for partition code 4: vector partitioned */
void test_vector_partitioned(float *src, float *dest, int n, float *sum) {
    force_partition_mode = 4;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
        copy vector(src[0:n], dest[0:n]) /* vector partitioned */
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 6.0f;
            local_sum += src[i];
        }
        
        if (__builtin_acc_on_device()) {
            #pragma acc atomic update
            sum[0] += local_sum;
        }
    }
}

/* Test function for partition code 5: gang+vector partitioned */
void test_gang_vector_partitioned(float *src, float *dest, int n, float *sum) {
    force_partition_mode = 5;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
        copy gang vector(src[0:n], dest[0:n]) /* gang+vector partitioned */
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 7.0f;
            local_sum += src[i];
        }
        
        if (__builtin_acc_on_device()) {
            #pragma acc atomic update
            sum[0] += local_sum;
        }
    }
}

/* Test function for partition code 6: worker+vector partitioned */
void test_worker_vector_partitioned(float *src, float *dest, int n, float *sum) {
    force_partition_mode = 6;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
        copy worker vector(src[0:n], dest[0:n]) /* worker+vector partitioned */
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 8.0f;
            local_sum += src[i];
        }
        
        if (__builtin_acc_on_device()) {
            #pragma acc atomic update
            sum[0] += local_sum;
        }
    }
}

/* Test function for partition code 7: fully partitioned */
void test_fully_partitioned(float *src, float *dest, int n, float *sum) {
    force_partition_mode = 7;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
        copy gang worker vector(src[0:n], dest[0:n]) /* fully partitioned */
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 9.0f;
            local_sum += src[i];
        }
        
        if (__builtin_acc_on_device()) {
            #pragma acc atomic update
            sum[0] += local_sum;
        }
    }
}

/* Additional test using kernels construct with data regions */
void test_kernels_partitions(float *src, float *dest1, float *dest2, int n, float *sum) {
    /* Mix different partition modes in a single function */
    
    /* gang partitioned */
    #pragma acc data copy(src[0:n]) copy(dest1[0:n]) copy(sum[0:1]) \
        create gang(dest1[0:n])
    {
        #pragma acc kernels
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                dest1[i] = src[i] * 1.5f;
            }
        }
    }
    
    /* worker+vector partitioned */
    #pragma acc data copy(src[0:n]) copy(dest2[0:n]) copy(sum[0:1]) \
        copy worker vector(dest2[0:n])
    {
        #pragma acc kernels
        {
            float local_sum = 0.0f;
            #pragma acc loop gang worker vector reduction(+:local_sum)
            for (int i = 0; i < n; i++) {
                dest2[i] = src[i] * 2.5f;
                local_sum += src[i];
            }
            
            if (__builtin_acc_on_device()) {
                #pragma acc atomic update
                sum[0] += local_sum;
            }
        }
    }
}

/* Test with nested loops to engage complex partitioning */
void test_nested_partitions(float *src, float *dest, int n, float *sum) {
    int chunks = n / CHUNK_SIZE;
    
    /* Use different partition modes based on volatile variable */
    switch (force_partition_mode % 8) {
        case 0:
            #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1])
            break;
        case 1:
            #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
                copy gang(src[0:n], dest[0:n])
            break;
        case 2:
            #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
                copy worker(src[0:n], dest[0:n])
            break;
        case 3:
            #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
                copy gang worker(src[0:n], dest[0:n])
            break;
        case 4:
            #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
                copy vector(src[0:n], dest[0:n])
            break;
        case 5:
            #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
                copy gang vector(src[0:n], dest[0:n])
            break;
        case 6:
            #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
                copy worker vector(src[0:n], dest[0:n])
            break;
        case 7:
            #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
                copy gang worker vector(src[0:n], dest[0:n])
            break;
    }
    
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang collapse(2) reduction(+:local_sum)
        for (int chunk = 0; chunk < chunks; chunk++) {
            #pragma acc loop worker vector
            for (int i = 0; i < CHUNK_SIZE; i++) {
                int idx = chunk * CHUNK_SIZE + i;
                dest[idx] = src[idx] * (chunk + 1.0f);
                local_sum += src[idx];
            }
        }
        
        if (__builtin_acc_on_device()) {
            #pragma acc atomic update
            sum[0] += local_sum;
        }
    }
}

int main() {
    float *src = (float*)malloc(N * sizeof(float));
    float *dest1 = (float*)malloc(N * sizeof(float));
    float *dest2 = (float*)malloc(N * sizeof(float));
    float *dest3 = (float*)malloc(N * sizeof(float));
    float sum[8] = {0};  /* One sum per partition mode */
    
    if (!src || !dest1 || !dest2 || !dest3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize source array with patterned data */
    for (int i = 0; i < N; i++) {
        src[i] = (i % 100) * 0.1f;
    }
    
    printf("Testing OpenACC partition modes...\n");
    
    /* Test all 8 partition codes systematically */
    test_gang_redundant(src, dest1, N, &sum[0]);
    test_gang_partitioned(src, dest2, N, &sum[1]);
    test_worker_partitioned(src, dest3, N, &sum[2]);
    test_gang_worker_partitioned(src, dest1, N, &sum[3]);
    test_vector_partitioned(src, dest2, N, &sum[4]);
    test_gang_vector_partitioned(src, dest3, N, &sum[5]);
    test_worker_vector_partitioned(src, dest1, N, &sum[6]);
    test_fully_partitioned(src, dest2, N, &sum[7]);
    
    /* Test kernels construct with data regions */
    test_kernels_partitions(src, dest3, dest1, N, &sum[0]);
    
    /* Test with nested loops and dynamic partition selection */
    for (int mode = 0; mode < 8; mode++) {
        force_partition_mode = mode;
        test_nested_partitions(src, dest2, N, &sum[mode]);
    }
    
    /* Compute final checksum to ensure all computations happened */
    float final_checksum = 0.0f;
    for (int i = 0; i < N; i++) {
        final_checksum += dest1[i] + dest2[i] + dest3[i];
    }
    for (int i = 0; i < 8; i++) {
        final_checksum += sum[i];
    }
    
    printf("Final checksum: %f\n", final_checksum);
    printf("Partition sums: ");
    for (int i = 0; i < 8; i++) {
        printf("%f ", sum[i]);
    }
    printf("\n");
    
    /* Verify some results */
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (dest1[i] != src[i] * 8.0f && dest1[i] != src[i] * 2.5f) {
            errors++;
            if (errors < 5) {
                printf("Mismatch at dest1[%d]: %f != expected\n", i, dest1[i]);
            }
        }
    }
    
    if (errors > 0) {
        printf("Found %d errors in dest1\n", errors);
    } else {
        printf("All tests completed successfully\n");
    }
    
    free(src);
    free(dest1);
    free(dest2);
    free(dest3);
    
    return 0;
}
