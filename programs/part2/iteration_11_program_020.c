/* test_omp_acc_partition_codes.c
 * 
 * This program systematically exercises GCC's OpenACC data partition
 * code mapping logic (partition codes 0-7) by generating OpenACC
 * compute constructs with explicit data clause modifiers.
 * Compile with: gcc -O2 -fopenacc -fdump-tree-omplower -fdump-tree-optimized -o test_partition test_omp_acc_partition_codes.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define N 1024
#define NUM_CASES 8

/* Use volatile to prevent compile-time elimination of partition schemes */
static volatile int force_partition_mode = 0;

/* Test function for partition code 0: gang redundant */
void test_gang_redundant(const float* src, float* dest, int n, float* reduction) {
    float local_sum = 0.0f;
    
    /* No partition modifier - should map to "gang redundant" */
    #pragma acc parallel copy(src[0:n], dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 2.0f;
            local_sum += src[i];
        }
    }
    
    *reduction = local_sum;
}

/* Test function for partition code 1: gang partitioned */
void test_gang_partitioned(const float* src, float* dest, int n, float* reduction) {
    float local_sum = 0.0f;
    
    /* Explicit gang partition modifier */
    #pragma acc parallel copy(src[0:n]) copy(gang: dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 3.0f;
            local_sum += src[i] * 2.0f;
        }
    }
    
    *reduction = local_sum;
}

/* Test function for partition code 2: worker partitioned */
void test_worker_partitioned(const float* src, float* dest, int n, float* reduction) {
    float local_sum = 0.0f;
    
    /* Explicit worker partition modifier */
    #pragma acc parallel copy(src[0:n]) copy(worker: dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 4.0f;
            local_sum += src[i] * 3.0f;
        }
    }
    
    *reduction = local_sum;
}

/* Test function for partition code 3: gang+worker partitioned */
void test_gang_worker_partitioned(const float* src, float* dest, int n, float* reduction) {
    float local_sum = 0.0f;
    
    /* Combined gang and worker partition modifier */
    #pragma acc parallel copy(src[0:n]) copy(gang worker: dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 5.0f;
            local_sum += src[i] * 4.0f;
        }
    }
    
    *reduction = local_sum;
}

/* Test function for partition code 4: vector partitioned */
void test_vector_partitioned(const float* src, float* dest, int n, float* reduction) {
    float local_sum = 0.0f;
    
    /* Explicit vector partition modifier */
    #pragma acc parallel copy(src[0:n]) copy(vector: dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 6.0f;
            local_sum += src[i] * 5.0f;
        }
    }
    
    *reduction = local_sum;
}

/* Test function for partition code 5: gang+vector partitioned */
void test_gang_vector_partitioned(const float* src, float* dest, int n, float* reduction) {
    float local_sum = 0.0f;
    
    /* Combined gang and vector partition modifier */
    #pragma acc parallel copy(src[0:n]) copy(gang vector: dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 7.0f;
            local_sum += src[i] * 6.0f;
        }
    }
    
    *reduction = local_sum;
}

/* Test function for partition code 6: worker+vector partitioned */
void test_worker_vector_partitioned(const float* src, float* dest, int n, float* reduction) {
    float local_sum = 0.0f;
    
    /* Combined worker and vector partition modifier */
    #pragma acc parallel copy(src[0:n]) copy(worker vector: dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 8.0f;
            local_sum += src[i] * 7.0f;
        }
    }
    
    *reduction = local_sum;
}

/* Test function for partition code 7: fully partitioned */
void test_fully_partitioned(const float* src, float* dest, int n, float* reduction) {
    float local_sum = 0.0f;
    
    /* Full gang+worker+vector partition modifier */
    #pragma acc parallel copy(src[0:n]) copy(gang worker vector: dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 9.0f;
            local_sum += src[i] * 8.0f;
        }
    }
    
    *reduction = local_sum;
}

/* Function pointer type for test functions */
typedef void (*test_func_t)(const float*, float*, int, float*);

int main() {
    float* src = (float*)malloc(N * sizeof(float));
    float* dest[NUM_CASES];
    float reductions[NUM_CASES];
    
    /* Array of test functions for each partition code */
    test_func_t test_functions[NUM_CASES] = {
        test_gang_redundant,
        test_gang_partitioned,
        test_worker_partitioned,
        test_gang_worker_partitioned,
        test_vector_partitioned,
        test_gang_vector_partitioned,
        test_worker_vector_partitioned,
        test_fully_partitioned
    };
    
    /* Initialize source array with patterned data */
    for (int i = 0; i < N; i++) {
        src[i] = (float)(i % 100) * 0.1f;
    }
    
    /* Allocate destination arrays for each test case */
    for (int i = 0; i < NUM_CASES; i++) {
        dest[i] = (float*)malloc(N * sizeof(float));
        for (int j = 0; j < N; j++) {
            dest[i][j] = 0.0f;
        }
        reductions[i] = 0.0f;
    }
    
    /* Execute each test function with volatile control to prevent optimization */
    for (int mode = 0; mode < NUM_CASES; mode++) {
        force_partition_mode = mode;
        
        /* Use switch to ensure all cases are considered */
        switch (force_partition_mode) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
                test_functions[mode](src, dest[mode], N, &reductions[mode]);
                break;
        }
    }
    
    /* Compute final checksum to ensure code has observable effects */
    double total_checksum = 0.0;
    for (int i = 0; i < NUM_CASES; i++) {
        for (int j = 0; j < N; j++) {
            total_checksum += (double)dest[i][j];
        }
        total_checksum += (double)reductions[i];
    }
    
    printf("Total checksum: %f\n", total_checksum);
    
    /* Cleanup */
    free(src);
    for (int i = 0; i < NUM_CASES; i++) {
        free(dest[i]);
    }
    
    return 0;
}
