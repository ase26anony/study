/* test_openacc_partition_codes.c
 * 
 * This program tests all 8 OpenACC data partition codes (0-7) by creating
 * OpenACC regions with different data clause modifiers. The goal is to
 * trigger the compiler's internal partition code to string mapping function
 * during compilation with -fdump-tree-omplower or similar flags.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define N 1024
#define CHUNK_SIZE 128

/* Use volatile to prevent compile-time elimination */
volatile int use_partition_mode = 0;

/* Test function for partition code 0: gang redundant */
void test_gang_redundant(float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
                         /* No partition modifier = gang redundant */
    {
        #pragma acc loop gang reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 2.0f;
            sum[0] += src[i];
        }
    }
}

/* Test function for partition code 1: gang partitioned */
void test_gang_partitioned(float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop gang reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            /* Use gang-partitioned data access pattern */
            dest[i] = src[i] + (float)i;
            sum[0] += dest[i];
        }
    }
}

/* Test function for partition code 2: worker partitioned */
void test_worker_partitioned(float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop gang worker reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            /* Worker-level partitioning */
            dest[i] = src[i] * src[i];
            sum[0] += dest[i];
        }
    }
}

/* Test function for partition code 3: gang+worker partitioned */
void test_gang_worker_partitioned(float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop gang worker reduction(+:sum[0])
        for (int i = 0; i < n; i += CHUNK_SIZE) {
            /* Nested loops to engage gang+worker partitioning */
            #pragma acc loop worker
            for (int j = 0; j < CHUNK_SIZE && (i + j) < n; j++) {
                int idx = i + j;
                dest[idx] = src[idx] * 3.0f;
                sum[0] += dest[idx];
            }
        }
    }
}

/* Test function for partition code 4: vector partitioned */
void test_vector_partitioned(float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop gang vector reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            /* Vector-level operations */
            dest[i] = src[i] / 2.0f;
            sum[0] += dest[i];
        }
    }
}

/* Test function for partition code 5: gang+vector partitioned */
void test_gang_vector_partitioned(float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop gang vector reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            /* Combined gang+vector pattern */
            dest[i] = src[i] + (float)(i % 16);  /* Vector-friendly pattern */
            sum[0] += dest[i];
        }
    }
}

/* Test function for partition code 6: worker+vector partitioned */
void test_worker_vector_partitioned(float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop gang worker vector reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            /* Worker+vector combined pattern */
            dest[i] = src[i] - (float)(i % 8);
            sum[0] += dest[i];
        }
    }
}

/* Test function for partition code 7: fully partitioned */
void test_fully_partitioned(float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1])
    {
        /* Complex nested loop structure to engage all levels */
        #pragma acc loop gang
        for (int block = 0; block < n; block += CHUNK_SIZE) {
            #pragma acc loop worker
            for (int chunk = 0; chunk < CHUNK_SIZE; chunk += 16) {
                #pragma acc loop vector
                for (int elem = 0; elem < 16 && (block + chunk + elem) < n; elem++) {
                    int idx = block + chunk + elem;
                    dest[idx] = src[idx] * src[idx] / 2.0f;
                    sum[0] += dest[idx];
                }
            }
        }
    }
}

/* Wrapper function that selects partition mode based on volatile variable */
void test_with_partition_mode(int mode, float* src, float* dest, int n, float* sum) {
    switch (mode) {
        case 0: test_gang_redundant(src, dest, n, sum); break;
        case 1: test_gang_partitioned(src, dest, n, sum); break;
        case 2: test_worker_partitioned(src, dest, n, sum); break;
        case 3: test_gang_worker_partitioned(src, dest, n, sum); break;
        case 4: test_vector_partitioned(src, dest, n, sum); break;
        case 5: test_gang_vector_partitioned(src, dest, n, sum); break;
        case 6: test_worker_vector_partitioned(src, dest, n, sum); break;
        case 7: test_fully_partitioned(src, dest, n, sum); break;
        default: break;
    }
}

int main() {
    float* src = (float*)malloc(N * sizeof(float));
    float* dest = (float*)malloc(N * sizeof(float));
    float sums[8] = {0};
    
    if (!src || !dest) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize source array with patterned data */
    for (int i = 0; i < N; i++) {
        src[i] = (float)(i % 100) * 0.1f;
    }
    
    printf("Testing OpenACC partition codes 0-7...\n");
    
    /* Test all 8 partition modes */
    for (int mode = 0; mode < 8; mode++) {
        /* Reset destination and sum for each test */
        for (int i = 0; i < N; i++) dest[i] = 0.0f;
        sums[mode] = 0.0f;
        
        /* Use volatile to force compiler to consider all modes */
        use_partition_mode = mode;
        
        /* Test with current partition mode */
        test_with_partition_mode(use_partition_mode, src, dest, N, &sums[mode]);
        
        /* Compute checksum to ensure computation happened */
        float checksum = 0.0f;
        for (int i = 0; i < N; i++) {
            checksum += dest[i];
        }
        
        printf("Mode %d: sum=%.2f, checksum=%.2f\n", 
               mode, sums[mode], checksum);
    }
    
    /* Final verification checksum */
    float total_sum = 0.0f;
    for (int i = 0; i < 8; i++) {
        total_sum += sums[i];
    }
    printf("Total sum across all modes: %.2f\n", total_sum);
    
    free(src);
    free(dest);
    
    return 0;
}
