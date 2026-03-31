/* test_openacc_partitions.c
 * 
 * This program systematically tests OpenACC data partition modes
 * to trigger the partition code string lookup logic in GCC's
 * omp-oacc-neuter-broadcast.cc (lines 335-343).
 *
 * Compile with: gcc -O2 -fopenacc -fdump-tree-omplower -fdump-tree-optimized test_openacc_partitions.c -o test_program
 * For offload debugging: gcc -O1 -fopenacc -fdump-tree-all test_openacc_partitions.c -o test_program_debug
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _OPENACC
#include <openacc.h>
#endif

#define N 1024
#define NUM_CASES 8

/* Use volatile to prevent compile-time elimination */
static volatile int force_different_partitions = 0;

/* Test function for partition code 0: gang redundant */
void test_gang_redundant(const float* src, float* dest, int n, float* sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
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

/* Test function for partition code 1: gang partitioned */
void test_gang_partitioned(const float* src, float* dest, int n, float* sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            /* Use gang-partitioned data access */
            dest[i] = src[i] * 3.0f;
            local_sum += dest[i];
        }
    }
    
    *sum = local_sum;
}

/* Test function for partition code 2: worker partitioned */
void test_worker_partitioned(const float* src, float* dest, int n, float* sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) create(dest[0:n]) copyout(local_sum) \
                         num_gangs(2) num_workers(4) vector_length(16)
    {
        #pragma acc loop worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 4.0f;
            local_sum += dest[i];
        }
    }
    
    *sum = local_sum;
}

/* Test function for partition code 3: gang+worker partitioned */
void test_gang_worker_partitioned(const float* src, float* dest, int n, float* sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(4) vector_length(16)
    {
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 5.0f;
            local_sum += dest[i];
        }
    }
    
    *sum = local_sum;
}

/* Test function for partition code 4: vector partitioned */
void test_vector_partitioned(const float* src, float* dest, int n, float* sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(1) num_workers(1) vector_length(64)
    {
        #pragma acc loop vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 6.0f;
            local_sum += dest[i];
        }
    }
    
    *sum = local_sum;
}

/* Test function for partition code 5: gang+vector partitioned */
void test_gang_vector_partitioned(const float* src, float* dest, int n, float* sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(1) vector_length(32)
    {
        #pragma acc loop gang vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 7.0f;
            local_sum += dest[i];
        }
    }
    
    *sum = local_sum;
}

/* Test function for partition code 6: worker+vector partitioned */
void test_worker_vector_partitioned(const float* src, float* dest, int n, float* sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(1) num_workers(4) vector_length(32)
    {
        #pragma acc loop worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 8.0f;
            local_sum += dest[i];
        }
    }
    
    *sum = local_sum;
}

/* Test function for partition code 7: fully partitioned */
void test_fully_partitioned(const float* src, float* dest, int n, float* sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(4) vector_length(16)
    {
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 9.0f;
            local_sum += dest[i];
        }
    }
    
    *sum = local_sum;
}

/* Alternative approach using explicit data clauses with partition modifiers */
void test_explicit_partition_clauses(const float* src, float* dest, int n, float* sum) {
    float local_sum = 0.0f;
    
    /* Force different partition modes based on volatile variable */
    if (force_different_partitions % 8 == 0) {
        /* gang redundant - no partition modifier */
        #pragma acc data copy(src[0:n], dest[0:n])
        #pragma acc parallel copyout(local_sum)
        {
            #pragma acc loop gang reduction(+:local_sum)
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] * 1.1f;
                local_sum += dest[i];
            }
        }
    }
    else if (force_different_partitions % 8 == 1) {
        /* gang partitioned */
        #pragma acc data copy(src[0:n]) copy(dest[0:n])
        #pragma acc parallel copyout(local_sum)
        {
            #pragma acc loop gang reduction(+:local_sum)
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] * 1.2f;
                local_sum += dest[i];
            }
        }
    }
    else if (force_different_partitions % 8 == 2) {
        /* worker partitioned */
        #pragma acc data copy(src[0:n]) copy(dest[0:n])
        #pragma acc parallel copyout(local_sum) num_workers(4)
        {
            #pragma acc loop worker reduction(+:local_sum)
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] * 1.3f;
                local_sum += dest[i];
            }
        }
    }
    else if (force_different_partitions % 8 == 3) {
        /* gang+worker partitioned */
        #pragma acc data copy(src[0:n]) copy(dest[0:n])
        #pragma acc parallel copyout(local_sum) num_gangs(4) num_workers(2)
        {
            #pragma acc loop gang worker reduction(+:local_sum)
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] * 1.4f;
                local_sum += dest[i];
            }
        }
    }
    else if (force_different_partitions % 8 == 4) {
        /* vector partitioned */
        #pragma acc data copy(src[0:n]) copy(dest[0:n])
        #pragma acc parallel copyout(local_sum) vector_length(64)
        {
            #pragma acc loop vector reduction(+:local_sum)
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] * 1.5f;
                local_sum += dest[i];
            }
        }
    }
    else if (force_different_partitions % 8 == 5) {
        /* gang+vector partitioned */
        #pragma acc data copy(src[0:n]) copy(dest[0:n])
        #pragma acc parallel copyout(local_sum) num_gangs(4) vector_length(32)
        {
            #pragma acc loop gang vector reduction(+:local_sum)
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] * 1.6f;
                local_sum += dest[i];
            }
        }
    }
    else if (force_different_partitions % 8 == 6) {
        /* worker+vector partitioned */
        #pragma acc data copy(src[0:n]) copy(dest[0:n])
        #pragma acc parallel copyout(local_sum) num_workers(4) vector_length(32)
        {
            #pragma acc loop worker vector reduction(+:local_sum)
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] * 1.7f;
                local_sum += dest[i];
            }
        }
    }
    else {
        /* fully partitioned */
        #pragma acc data copy(src[0:n]) copy(dest[0:n])
        #pragma acc parallel copyout(local_sum) num_gangs(4) num_workers(2) vector_length(16)
        {
            #pragma acc loop gang worker vector reduction(+:local_sum)
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] * 1.8f;
                local_sum += dest[i];
            }
        }
    }
    
    *sum = local_sum;
}

int main() {
    float* src = (float*)malloc(N * sizeof(float));
    float* dest = (float*)malloc(N * sizeof(float));
    float sums[NUM_CASES];
    float final_checksum = 0.0f;
    
    if (!src || !dest) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize source array with patterned data */
    for (int i = 0; i < N; i++) {
        src[i] = (float)(i % 100) * 0.1f;
    }
    
    printf("Testing OpenACC data partition modes...\n");
    
    /* Test each partition mode explicitly */
    test_gang_redundant(src, dest, N, &sums[0]);
    final_checksum += sums[0];
    
    test_gang_partitioned(src, dest, N, &sums[1]);
    final_checksum += sums[1];
    
    test_worker_partitioned(src, dest, N, &sums[2]);
    final_checksum += sums[2];
    
    test_gang_worker_partitioned(src, dest, N, &sums[3]);
    final_checksum += sums[3];
    
    test_vector_partitioned(src, dest, N, &sums[4]);
    final_checksum += sums[4];
    
    test_gang_vector_partitioned(src, dest, N, &sums[5]);
    final_checksum += sums[5];
    
    test_worker_vector_partitioned(src, dest, N, &sums[6]);
    final_checksum += sums[6];
    
    test_fully_partitioned(src, dest, N, &sums[7]);
    final_checksum += sums[7];
    
    /* Also test with explicit partition clauses using volatile control */
    float extra_sum = 0.0f;
    for (int i = 0; i < 8; i++) {
        force_different_partitions = i;
        test_explicit_partition_clauses(src, dest, N, &extra_sum);
        final_checksum += extra_sum;
    }
    
    /* Compute a simple validation */
    float validation_sum = 0.0f;
    for (int i = 0; i < N; i++) {
        validation_sum += src[i];
    }
    
    printf("Final checksum: %f\n", final_checksum);
    printf("Source array sum: %f\n", validation_sum);
    
    /* Print individual results to prevent dead code elimination */
    printf("Partition mode results:\n");
    for (int i = 0; i < NUM_CASES; i++) {
        printf("  Mode %d: %f\n", i, sums[i]);
    }
    
    free(src);
    free(dest);
    
    return 0;
}
