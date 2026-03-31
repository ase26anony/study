/* test_partition_codes.c - Cover GCC omp-oacc-neuter-broadcast.cc partition codes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define CHUNK 128

/* Test function for partition code 0: gang redundant */
void test_gang_redundant(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n], dest[0:n]) copyout(sum[0:1]) \
        present_or_copy(src[0:n]) /* gang redundant by default */
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 2.0f;
            local_sum += src[i];
        }
        
        #pragma acc single
        *sum = local_sum;
    }
}

/* Test function for partition code 1: gang partitioned */
void test_gang_partitioned(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(sum[0:1])
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 3.0f;
            local_sum += src[i];
        }
        
        #pragma acc single
        *sum = local_sum;
    }
}

/* Test function for partition code 2: worker partitioned */
void test_worker_partitioned(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(sum[0:1])
    {
        float local_sum = 0.0f;
        
        #pragma acc loop worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 4.0f;
            local_sum += src[i];
        }
        
        *sum = local_sum;
    }
}

/* Test function for partition code 3: gang+worker partitioned */
void test_gang_worker_partitioned(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(sum[0:1])
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 5.0f;
            local_sum += src[i];
        }
        
        *sum = local_sum;
    }
}

/* Test function for partition code 4: vector partitioned */
void test_vector_partitioned(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(sum[0:1])
    {
        float local_sum = 0.0f;
        
        #pragma acc loop vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 6.0f;
            local_sum += src[i];
        }
        
        *sum = local_sum;
    }
}

/* Test function for partition code 5: gang+vector partitioned */
void test_gang_vector_partitioned(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(sum[0:1])
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 7.0f;
            local_sum += src[i];
        }
        
        *sum = local_sum;
    }
}

/* Test function for partition code 6: worker+vector partitioned */
void test_worker_vector_partitioned(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(sum[0:1])
    {
        float local_sum = 0.0f;
        
        #pragma acc loop worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 8.0f;
            local_sum += src[i];
        }
        
        *sum = local_sum;
    }
}

/* Test function for partition code 7: fully partitioned */
void test_fully_partitioned(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(sum[0:1])
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 9.0f;
            local_sum += src[i];
        }
        
        *sum = local_sum;
    }
}

/* Additional tests with explicit data clauses and partition modifiers */
void test_explicit_partition_modifiers(float *src, float *dest1, float *dest2, 
                                       float *dest3, int n, volatile int mode) {
    /* Use volatile to prevent compile-time elimination */
    float *target_dest;
    
    switch (mode % 8) {
        case 0:
            #pragma acc parallel copy(src[0:n]) copy(dest1[0:n])
            {
                #pragma acc loop gang
                for (int i = 0; i < n; i++) {
                    dest1[i] = src[i] + 1.0f;
                }
            }
            break;
            
        case 1:
            #pragma acc parallel copy(src[0:n]) copy(dest1[0:n])
            {
                #pragma acc loop gang
                for (int i = 0; i < n; i++) {
                    dest1[i] = src[i] + 2.0f;
                }
            }
            break;
            
        case 2:
            #pragma acc parallel copy(src[0:n]) copy(dest2[0:n])
            {
                #pragma acc loop worker
                for (int i = 0; i < n; i++) {
                    dest2[i] = src[i] + 3.0f;
                }
            }
            break;
            
        case 3:
            #pragma acc parallel copy(src[0:n]) copy(dest2[0:n])
            {
                #pragma acc loop gang worker
                for (int i = 0; i < n; i++) {
                    dest2[i] = src[i] + 4.0f;
                }
            }
            break;
            
        case 4:
            #pragma acc parallel copy(src[0:n]) copy(dest3[0:n])
            {
                #pragma acc loop vector
                for (int i = 0; i < n; i++) {
                    dest3[i] = src[i] + 5.0f;
                }
            }
            break;
            
        case 5:
            #pragma acc parallel copy(src[0:n]) copy(dest3[0:n])
            {
                #pragma acc loop gang vector
                for (int i = 0; i < n; i++) {
                    dest3[i] = src[i] + 6.0f;
                }
            }
            break;
            
        case 6:
            #pragma acc parallel copy(src[0:n]) copy(dest1[0:n])
            {
                #pragma acc loop worker vector
                for (int i = 0; i < n; i++) {
                    dest1[i] = src[i] + 7.0f;
                }
            }
            break;
            
        case 7:
            #pragma acc parallel copy(src[0:n]) copy(dest2[0:n])
            {
                #pragma acc loop gang worker vector
                for (int i = 0; i < n; i++) {
                    dest2[i] = src[i] + 8.0f;
                }
            }
            break;
    }
}

/* Test with nested loops to trigger complex partitioning */
void test_nested_partitioning(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n], dest[0:n]) copyout(sum[0:1])
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang
        for (int i = 0; i < n; i += CHUNK) {
            #pragma acc loop worker
            for (int j = 0; j < CHUNK && (i + j) < n; j++) {
                #pragma acc loop vector
                for (int k = 0; k < 4; k++) {
                    int idx = i + j;
                    dest[idx] = src[idx] * (k + 1);
                    local_sum += src[idx];
                }
            }
        }
        
        #pragma acc atomic update
        *sum += local_sum;
    }
}

/* Test with data regions and explicit data clauses */
void test_data_regions(float *src, float *dest, int n) {
    #pragma acc data copyin(src[0:n]) copyout(dest[0:n])
    {
        #pragma acc parallel
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] * 10.0f;
            }
        }
        
        #pragma acc parallel
        {
            #pragma acc loop worker
            for (int i = 0; i < n; i++) {
                dest[i] += 1.0f;
            }
        }
        
        #pragma acc parallel
        {
            #pragma acc loop vector
            for (int i = 0; i < n; i++) {
                dest[i] += 2.0f;
            }
        }
    }
}

int main() {
    const int total_size = N;
    float *src = (float*)malloc(total_size * sizeof(float));
    float *dest1 = (float*)malloc(total_size * sizeof(float));
    float *dest2 = (float*)malloc(total_size * sizeof(float));
    float *dest3 = (float*)malloc(total_size * sizeof(float));
    float sums[8] = {0};
    
    /* Initialize source array with patterned data */
    for (int i = 0; i < total_size; i++) {
        src[i] = (float)(i % 100) * 0.1f;
    }
    
    printf("Testing OpenACC partition codes...\n");
    
    /* Test all 8 partition codes systematically */
    test_gang_redundant(src, dest1, total_size, &sums[0]);
    test_gang_partitioned(src, dest2, total_size, &sums[1]);
    test_worker_partitioned(src, dest3, total_size, &sums[2]);
    test_gang_worker_partitioned(src, dest1, total_size, &sums[3]);
    test_vector_partitioned(src, dest2, total_size, &sums[4]);
    test_gang_vector_partitioned(src, dest3, total_size, &sums[5]);
    test_worker_vector_partitioned(src, dest1, total_size, &sums[6]);
    test_fully_partitioned(src, dest2, total_size, &sums[7]);
    
    /* Test with volatile control to prevent optimization */
    volatile int mode = 0;
    for (int i = 0; i < 8; i++) {
        test_explicit_partition_modifiers(src, dest1, dest2, dest3, total_size, mode + i);
    }
    
    /* Test nested partitioning */
    float nested_sum = 0.0f;
    test_nested_partitioning(src, dest3, total_size, &nested_sum);
    
    /* Test data regions */
    test_data_regions(src, dest1, total_size);
    
    /* Compute final checksum to ensure all computations are used */
    float final_checksum = 0.0f;
    for (int i = 0; i < 8; i++) {
        final_checksum += sums[i];
    }
    
    for (int i = 0; i < total_size; i++) {
        final_checksum += dest1[i] + dest2[i] + dest3[i];
    }
    final_checksum += nested_sum;
    
    printf("Final checksum: %f\n", final_checksum);
    printf("Partition code tests completed.\n");
    
    /* Cleanup */
    free(src);
    free(dest1);
    free(dest2);
    free(dest3);
    
    return 0;
}
