/* test_omp_acc_partition_codes.c
 * 
 * This program systematically tests OpenACC data partition codes 0-7
 * to trigger the string mapping logic in GCC's omp-oacc-neuter-broadcast.cc
 * 
 * Compile with: gcc -O2 -fopenacc -fdump-tree-omplower -fdump-tree-optimized -o test_partition test_omp_acc_partition_codes.c
 * For NVIDIA offload: gcc -O2 -fopenacc -fdump-tree-omplower -foffload=nvptx-none -o test_partition_offload test_omp_acc_partition_codes.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _OPENACC
#include <openacc.h>
#endif

#define N 1024
#define CHUNK_SIZE 128

/* Volatile control variable to prevent compile-time elimination */
static volatile int use_partition_mode = 0;

/* Test function for partition code 0: gang redundant */
void test_gang_redundant(const float* src, float* dest, int n, float* reduction) {
    float local_sum = 0.0f;
    
    /* OpenACC parallel region with gang redundant data partitioning */
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(2) vector_length(32)
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
    
    /* Explicit gang partitioning on data */
    #pragma acc parallel copyin(src[0:n]) copyout(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        /* Data is partitioned across gangs */
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
    
    /* Explicit worker partitioning on data */
    #pragma acc parallel copyin(src[0:n]) copyout(dest[0:n]) copyout(local_sum) \
                         num_gangs(2) num_workers(4) vector_length(32)
    {
        /* Data is partitioned across workers */
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
    
    /* Explicit gang and worker partitioning on data */
    #pragma acc parallel create(dest[0:n]) copyin(src[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(4) vector_length(32)
    {
        /* Data is partitioned across both gangs and workers */
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
    
    /* Explicit vector partitioning on data */
    #pragma acc parallel copyin(src[0:n]) copyout(dest[0:n]) copyout(local_sum) \
                         num_gangs(2) num_workers(2) vector_length(64)
    {
        /* Data is partitioned across vector lanes */
        #pragma acc loop gang worker vector reduction(+:local_sum)
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
    
    /* Explicit gang and vector partitioning on data */
    #pragma acc parallel copyin(src[0:n]) copyout(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(2) vector_length(64)
    {
        /* Data is partitioned across gangs and vector lanes */
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
    
    /* Explicit worker and vector partitioning on data */
    #pragma acc parallel copyin(src[0:n]) copyout(dest[0:n]) copyout(local_sum) \
                         num_gangs(2) num_workers(4) vector_length(64)
    {
        /* Data is partitioned across workers and vector lanes */
        #pragma acc loop worker vector reduction(+:local_sum)
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
    
    /* Fully partitioned data across all dimensions */
    #pragma acc parallel copyin(src[0:n]) copyout(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(4) vector_length(64)
    {
        /* Data is partitioned across gangs, workers, and vector lanes */
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 9.0f;
            local_sum += src[i] * 8.0f;
        }
    }
    
    *reduction = local_sum;
}

/* Wrapper function that selects partition mode based on volatile variable */
void test_partition_wrapper(int mode, const float* src, float* dest, int n, float* reduction) {
    switch (mode) {
        case 0: test_gang_redundant(src, dest, n, reduction); break;
        case 1: test_gang_partitioned(src, dest, n, reduction); break;
        case 2: test_worker_partitioned(src, dest, n, reduction); break;
        case 3: test_gang_worker_partitioned(src, dest, n, reduction); break;
        case 4: test_vector_partitioned(src, dest, n, reduction); break;
        case 5: test_gang_vector_partitioned(src, dest, n, reduction); break;
        case 6: test_worker_vector_partitioned(src, dest, n, reduction); break;
        case 7: test_fully_partitioned(src, dest, n, reduction); break;
        default: *reduction = -1.0f; break;
    }
}

int main() {
    float *src = (float*)malloc(N * sizeof(float));
    float *dest = (float*)malloc(N * sizeof(float));
    float reductions[8];
    float final_checksum = 0.0f;
    
    if (!src || !dest) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize source array with patterned data */
    for (int i = 0; i < N; i++) {
        src[i] = (float)(i % 100) * 0.1f;
    }
    
    printf("Testing OpenACC data partition codes 0-7...\n");
    
    /* Test all 8 partition modes */
    for (int mode = 0; mode < 8; mode++) {
        /* Clear destination array */
        memset(dest, 0, N * sizeof(float));
        
        /* Use volatile variable to control which mode is tested */
        use_partition_mode = mode;
        
        /* Test the specific partition mode */
        test_partition_wrapper(use_partition_mode, src, dest, N, &reductions[mode]);
        
        /* Compute checksum from destination array */
        float dest_sum = 0.0f;
        for (int i = 0; i < N; i++) {
            dest_sum += dest[i];
        }
        
        printf("Mode %d: reduction = %f, dest_sum = %f\n", 
               mode, reductions[mode], dest_sum);
        
        final_checksum += reductions[mode] + dest_sum;
    }
    
    /* Additional test with data clauses that explicitly specify partitioning */
    printf("\nTesting explicit data clause partitioning...\n");
    
    /* Test with gang partitioned data clause */
    {
        float test_data[N];
        for (int i = 0; i < N; i++) test_data[i] = (float)i;
        
        #pragma acc data copyin(test_data[0:N]) copyout(dest[0:N])
        {
            #pragma acc parallel present(test_data, dest) num_gangs(4)
            {
                #pragma acc loop gang
                for (int i = 0; i < N; i++) {
                    dest[i] = test_data[i] * 2.0f;
                }
            }
        }
    }
    
    /* Test with vector partitioned data clause */
    {
        float test_data[N];
        for (int i = 0; i < N; i++) test_data[i] = (float)i * 0.5f;
        
        #pragma acc data copyin(test_data[0:N]) copyout(dest[0:N])
        {
            #pragma acc parallel present(test_data, dest) vector_length(32)
            {
                #pragma acc loop vector
                for (int i = 0; i < N; i++) {
                    dest[i] = test_data[i] * 3.0f;
                }
            }
        }
    }
    
    /* Test with fully partitioned data clause */
    {
        float test_data[N];
        float result = 0.0f;
        
        for (int i = 0; i < N; i++) test_data[i] = (float)i * 0.25f;
        
        #pragma acc data copyin(test_data[0:N]) copy(result)
        {
            #pragma acc parallel present(test_data) copyout(result) \
                         num_gangs(4) num_workers(2) vector_length(64)
            {
                #pragma acc loop gang worker vector reduction(+:result)
                for (int i = 0; i < N; i++) {
                    result += test_data[i];
                }
            }
        }
        
        final_checksum += result;
    }
    
    printf("\nFinal checksum: %f\n", final_checksum);
    printf("All partition modes tested.\n");
    
    /* Verify results to ensure code wasn't optimized away */
    if (final_checksum > 0.0f) {
        printf("SUCCESS: Computations performed\n");
    } else {
        printf("WARNING: Checksum is zero - computations may have been optimized away\n");
    }
    
    free(src);
    free(dest);
    
    return 0;
}
