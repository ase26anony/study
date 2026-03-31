/* test_openacc_partitions.c
 * 
 * This program systematically tests all 8 OpenACC data partition modes
 * to trigger the partition code string lookup in GCC's omp-oacc-neuter-broadcast.cc
 * 
 * Compile with: gcc -O2 -fopenacc -fdump-tree-omplower -fdump-tree-optimized test_openacc_partitions.c -o test_partitions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define CHUNK_SIZE 128

/* Helper function to initialize arrays */
void init_array(float *arr, int size, float base_value) {
    for (int i = 0; i < size; i++) {
        arr[i] = base_value + (i % 100) * 0.1f;
    }
}

/* Test 0: gang redundant (no partition modifier) */
void test_gang_redundant(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
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

/* Test 1: gang partitioned */
void test_gang_partitioned(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 3.0f;
            local_sum += src[i] * 2.0f;
        }
    }
    
    *reduction = local_sum;
}

/* Test 2: worker partitioned */
void test_worker_partitioned(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 4.0f;
            local_sum += src[i] * 3.0f;
        }
    }
    
    *reduction = local_sum;
}

/* Test 3: gang+worker partitioned */
void test_gang_worker_partitioned(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 5.0f;
            local_sum += src[i] * 4.0f;
        }
    }
    
    *reduction = local_sum;
}

/* Test 4: vector partitioned */
void test_vector_partitioned(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 6.0f;
            local_sum += src[i] * 5.0f;
        }
    }
    
    *reduction = local_sum;
}

/* Test 5: gang+vector partitioned */
void test_gang_vector_partitioned(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 7.0f;
            local_sum += src[i] * 6.0f;
        }
    }
    
    *reduction = local_sum;
}

/* Test 6: worker+vector partitioned */
void test_worker_vector_partitioned(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 8.0f;
            local_sum += src[i] * 7.0f;
        }
    }
    
    *reduction = local_sum;
}

/* Test 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 9.0f;
            local_sum += src[i] * 8.0f;
        }
    }
    
    *reduction = local_sum;
}

/* Additional tests with explicit data clauses and partition modifiers */
void test_explicit_partition_clauses(float *src, float *dest1, float *dest2, int n) {
    /* Use volatile to prevent compile-time elimination */
    volatile int mode = 0;
    
    /* Test different partition modes based on runtime value */
    for (int m = 0; m < 8; m++) {
        mode = m;
        
        #pragma acc data copy(src[0:n])
        {
            if (mode == 0) {
                /* gang redundant */
                #pragma acc parallel copy(dest1[0:n])
                {
                    #pragma acc loop gang
                    for (int i = 0; i < n; i++) {
                        dest1[i] = src[i] * 1.5f;
                    }
                }
            }
            else if (mode == 1) {
                /* gang partitioned - using data region with explicit clause */
                #pragma acc data copy(dest1[0:n])
                {
                    #pragma acc parallel 
                    {
                        #pragma acc loop gang
                        for (int i = 0; i < n; i++) {
                            dest1[i] = src[i] * 2.5f;
                        }
                    }
                }
            }
            else if (mode == 2) {
                /* worker partitioned */
                #pragma acc parallel copy(dest1[0:n])
                {
                    #pragma acc loop gang worker
                    for (int i = 0; i < n; i++) {
                        dest1[i] = src[i] * 3.5f;
                    }
                }
            }
            else if (mode == 3) {
                /* gang+worker partitioned */
                #pragma acc parallel copy(dest1[0:n])
                {
                    #pragma acc loop gang worker
                    for (int i = 0; i < n; i += CHUNK_SIZE) {
                        for (int j = 0; j < CHUNK_SIZE && (i + j) < n; j++) {
                            dest1[i + j] = src[i + j] * 4.5f;
                        }
                    }
                }
            }
            else if (mode == 4) {
                /* vector partitioned */
                #pragma acc parallel copy(dest1[0:n])
                {
                    #pragma acc loop gang vector
                    for (int i = 0; i < n; i++) {
                        dest1[i] = src[i] * 5.5f;
                    }
                }
            }
            else if (mode == 5) {
                /* gang+vector partitioned */
                #pragma acc parallel copy(dest1[0:n])
                {
                    #pragma acc loop gang vector
                    for (int i = 0; i < n; i++) {
                        dest1[i] = src[i] * 6.5f;
                    }
                }
            }
            else if (mode == 6) {
                /* worker+vector partitioned */
                #pragma acc parallel copy(dest1[0:n])
                {
                    #pragma acc loop gang worker vector
                    for (int i = 0; i < n; i++) {
                        dest1[i] = src[i] * 7.5f;
                    }
                }
            }
            else if (mode == 7) {
                /* fully partitioned */
                #pragma acc parallel copy(dest1[0:n])
                {
                    #pragma acc loop gang worker vector
                    for (int i = 0; i < n; i++) {
                        dest1[i] = src[i] * 8.5f;
                    }
                }
            }
        }
    }
}

/* Test with kernels construct for additional coverage */
void test_kernels_partitions(float *src, float *dest, int n) {
    #pragma acc kernels copy(src[0:n], dest[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            float temp = src[i];
            #pragma acc loop worker
            for (int j = 0; j < 4; j++) {
                temp += 0.1f * j;
            }
            #pragma acc loop vector
            for (int k = 0; k < 2; k++) {
                temp -= 0.05f * k;
            }
            dest[i] = temp;
        }
    }
}

int main() {
    const int total_size = N;
    float *src = (float*)malloc(total_size * sizeof(float));
    float *dest1 = (float*)malloc(total_size * sizeof(float));
    float *dest2 = (float*)malloc(total_size * sizeof(float));
    float reductions[8];
    float final_checksum = 0.0f;
    
    if (!src || !dest1 || !dest2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize source arrays with patterned data */
    init_array(src, total_size, 1.0f);
    memset(dest1, 0, total_size * sizeof(float));
    memset(dest2, 0, total_size * sizeof(float));
    
    printf("Testing OpenACC data partition modes...\n");
    
    /* Execute all 8 partition mode tests */
    test_gang_redundant(src, dest1, total_size, &reductions[0]);
    test_gang_partitioned(src, dest1, total_size, &reductions[1]);
    test_worker_partitioned(src, dest1, total_size, &reductions[2]);
    test_gang_worker_partitioned(src, dest1, total_size, &reductions[3]);
    test_vector_partitioned(src, dest1, total_size, &reductions[4]);
    test_gang_vector_partitioned(src, dest1, total_size, &reductions[5]);
    test_worker_vector_partitioned(src, dest1, total_size, &reductions[6]);
    test_fully_partitioned(src, dest1, total_size, &reductions[7]);
    
    /* Test with explicit clauses and runtime mode selection */
    test_explicit_partition_clauses(src, dest1, dest2, total_size);
    
    /* Test with kernels construct */
    test_kernels_partitions(src, dest2, total_size);
    
    /* Compute final checksum to ensure all computations are used */
    for (int i = 0; i < total_size; i++) {
        final_checksum += dest1[i] + dest2[i];
    }
    for (int i = 0; i < 8; i++) {
        final_checksum += reductions[i];
    }
    
    printf("Final checksum: %f\n", final_checksum);
    
    /* Cleanup */
    free(src);
    free(dest1);
    free(dest2);
    
    return 0;
}
