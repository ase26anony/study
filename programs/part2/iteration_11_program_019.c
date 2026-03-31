/* test_omp_acc_partition_codes.c
 * 
 * This program systematically tests OpenACC data partition modes
 * to trigger coverage of the partition code string mapping function
 * in GCC's omp-oacc-neuter-broadcast.cc (lines 335-343).
 *
 * Compile with: gcc -O2 -fopenacc -fdump-tree-omplower -fdump-tree-optimized -o test_partition test_omp_acc_partition_codes.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define GANG_SIZE 32
#define WORKER_SIZE 4
#define VECTOR_SIZE 32

/* Use volatile to prevent compile-time elimination of partition logic */
static volatile int use_partition_mode = 0;

/* Test function for partition code 0: gang redundant */
void test_gang_redundant(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
                         num_gangs(GANG_SIZE) num_workers(1) vector_length(VECTOR_SIZE)
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 2.0f;
            local_sum += src[i];
        }
        
        #pragma acc single
        {
            sum[0] = local_sum;
        }
    }
}

/* Test function for partition code 1: gang partitioned */
void test_gang_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
                         num_gangs(GANG_SIZE) num_workers(1) vector_length(VECTOR_SIZE)
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang gang partitioned reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 3.0f;
            local_sum += src[i];
        }
        
        #pragma acc single
        {
            sum[0] = local_sum;
        }
    }
}

/* Test function for partition code 2: worker partitioned */
void test_worker_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
                         num_gangs(GANG_SIZE) num_workers(WORKER_SIZE) vector_length(VECTOR_SIZE)
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang worker worker partitioned reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 4.0f;
            local_sum += src[i];
        }
        
        #pragma acc single
        {
            sum[0] = local_sum;
        }
    }
}

/* Test function for partition code 3: gang+worker partitioned */
void test_gang_worker_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
                         num_gangs(GANG_SIZE) num_workers(WORKER_SIZE) vector_length(VECTOR_SIZE)
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang worker gang worker partitioned reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 5.0f;
            local_sum += src[i];
        }
        
        #pragma acc single
        {
            sum[0] = local_sum;
        }
    }
}

/* Test function for partition code 4: vector partitioned */
void test_vector_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
                         num_gangs(GANG_SIZE) num_workers(1) vector_length(VECTOR_SIZE)
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang vector vector partitioned reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 6.0f;
            local_sum += src[i];
        }
        
        #pragma acc single
        {
            sum[0] = local_sum;
        }
    }
}

/* Test function for partition code 5: gang+vector partitioned */
void test_gang_vector_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
                         num_gangs(GANG_SIZE) num_workers(1) vector_length(VECTOR_SIZE)
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang vector gang vector partitioned reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 7.0f;
            local_sum += src[i];
        }
        
        #pragma acc single
        {
            sum[0] = local_sum;
        }
    }
}

/* Test function for partition code 6: worker+vector partitioned */
void test_worker_vector_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
                         num_gangs(GANG_SIZE) num_workers(WORKER_SIZE) vector_length(VECTOR_SIZE)
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang worker vector worker vector partitioned reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 8.0f;
            local_sum += src[i];
        }
        
        #pragma acc single
        {
            sum[0] = local_sum;
        }
    }
}

/* Test function for partition code 7: fully partitioned */
void test_fully_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
                         num_gangs(GANG_SIZE) num_workers(WORKER_SIZE) vector_length(VECTOR_SIZE)
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang worker vector gang worker vector partitioned reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 9.0f;
            local_sum += src[i];
        }
        
        #pragma acc single
        {
            sum[0] = local_sum;
        }
    }
}

/* Alternative approach using data regions with explicit partition clauses */
void test_data_partition_modes(float *src, float *dest1, float *dest2, float *dest3, 
                               float *dest4, float *dest5, float *dest6, float *dest7,
                               int n, float *sums) {
    /* This function uses different data clauses to trigger various partition codes */
    
    /* Code 0: gang redundant (implicit) */
    #pragma acc data copy(src[0:n], dest1[0:n], sums[0:1])
    {
        #pragma acc parallel
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                dest1[i] = src[i] * 1.5f;
            }
        }
    }
    
    /* Code 1: gang partitioned */
    #pragma acc data copy(src[0:n]) create(gang partitioned: dest2[0:n]) copy(sums[1:1])
    {
        #pragma acc parallel
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                dest2[i] = src[i] * 2.5f;
            }
        }
    }
    
    /* Code 2: worker partitioned */
    #pragma acc data copy(src[0:n]) create(worker partitioned: dest3[0:n]) copy(sums[2:1])
    {
        #pragma acc parallel num_workers(WORKER_SIZE)
        {
            #pragma acc loop gang worker
            for (int i = 0; i < n; i++) {
                dest3[i] = src[i] * 3.5f;
            }
        }
    }
    
    /* Code 3: gang+worker partitioned */
    #pragma acc data copy(src[0:n]) create(gang worker partitioned: dest4[0:n]) copy(sums[3:1])
    {
        #pragma acc parallel num_workers(WORKER_SIZE)
        {
            #pragma acc loop gang worker
            for (int i = 0; i < n; i++) {
                dest4[i] = src[i] * 4.5f;
            }
        }
    }
    
    /* Code 4: vector partitioned */
    #pragma acc data copy(src[0:n]) create(vector partitioned: dest5[0:n]) copy(sums[4:1])
    {
        #pragma acc parallel vector_length(VECTOR_SIZE)
        {
            #pragma acc loop gang vector
            for (int i = 0; i < n; i++) {
                dest5[i] = src[i] * 5.5f;
            }
        }
    }
    
    /* Code 5: gang+vector partitioned */
    #pragma acc data copy(src[0:n]) create(gang vector partitioned: dest6[0:n]) copy(sums[5:1])
    {
        #pragma acc parallel vector_length(VECTOR_SIZE)
        {
            #pragma acc loop gang vector
            for (int i = 0; i < n; i++) {
                dest6[i] = src[i] * 6.5f;
            }
        }
    }
    
    /* Code 6: worker+vector partitioned */
    #pragma acc data copy(src[0:n]) create(worker vector partitioned: dest7[0:n]) copy(sums[6:1])
    {
        #pragma acc parallel num_workers(WORKER_SIZE) vector_length(VECTOR_SIZE)
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < n; i++) {
                dest7[i] = src[i] * 7.5f;
            }
        }
    }
}

int main() {
    float *src = (float*)malloc(N * sizeof(float));
    float *dest = (float*)malloc(N * sizeof(float));
    float sums[8] = {0};
    
    /* Initialize source array with patterned data */
    for (int i = 0; i < N; i++) {
        src[i] = (float)(i % 100) * 0.1f;
    }
    
    /* Clear destination arrays */
    memset(dest, 0, N * sizeof(float));
    
    printf("Testing OpenACC partition modes to trigger coverage of partition code mapping...\n");
    
    /* Use volatile variable to prevent compile-time elimination */
    for (int mode = 0; mode < 8; mode++) {
        use_partition_mode = mode;
        
        /* Call different test functions based on mode */
        switch (mode) {
            case 0:
                test_gang_redundant(src, dest, N, &sums[0]);
                break;
            case 1:
                test_gang_partitioned(src, dest, N, &sums[1]);
                break;
            case 2:
                test_worker_partitioned(src, dest, N, &sums[2]);
                break;
            case 3:
                test_gang_worker_partitioned(src, dest, N, &sums[3]);
                break;
            case 4:
                test_vector_partitioned(src, dest, N, &sums[4]);
                break;
            case 5:
                test_gang_vector_partitioned(src, dest, N, &sums[5]);
                break;
            case 6:
                test_worker_vector_partitioned(src, dest, N, &sums[6]);
                break;
            case 7:
                test_fully_partitioned(src, dest, N, &sums[7]);
                break;
        }
        
        /* Simple computation to ensure data is used */
        float check = 0.0f;
        for (int i = 0; i < N; i++) {
            check += dest[i];
        }
        printf("Mode %d: dest sum = %f, reduction sum = %f\n", mode, check, sums[mode]);
    }
    
    /* Alternative test with explicit data partition clauses */
    printf("\nTesting with explicit data partition clauses...\n");
    float *dest1 = (float*)malloc(N * sizeof(float));
    float *dest2 = (float*)malloc(N * sizeof(float));
    float *dest3 = (float*)malloc(N * sizeof(float));
    float *dest4 = (float*)malloc(N * sizeof(float));
    float *dest5 = (float*)malloc(N * sizeof(float));
    float *dest6 = (float*)malloc(N * sizeof(float));
    float *dest7 = (float*)malloc(N * sizeof(float));
    
    float alt_sums[7] = {0};
    
    test_data_partition_modes(src, dest1, dest2, dest3, dest4, dest5, dest6, dest7, N, alt_sums);
    
    /* Compute final checksum to ensure all computations are observable */
    float final_checksum = 0.0f;
    for (int i = 0; i < N; i++) {
        final_checksum += dest[i] + dest1[i] + dest2[i] + dest3[i] + 
                         dest4[i] + dest5[i] + dest6[i] + dest7[i];
    }
    
    for (int i = 0; i < 8; i++) {
        final_checksum += sums[i];
    }
    
    for (int i = 0; i < 7; i++) {
        final_checksum += alt_sums[i];
    }
    
    printf("\nFinal checksum: %f\n", final_checksum);
    printf("Test completed. Compile with -fdump-tree-omplower to see partition code generation.\n");
    
    /* Cleanup */
    free(src);
    free(dest);
    free(dest1);
    free(dest2);
    free(dest3);
    free(dest4);
    free(dest5);
    free(dest6);
    free(dest7);
    
    return 0;
}
