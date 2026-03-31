/* test_openacc_partitions.c
 * 
 * This program systematically tests OpenACC data partition modes
 * to trigger the partition code string lookup function in GCC's
 * omp-oacc-neuter-broadcast.cc (lines 335-343).
 *
 * Compile with: gcc -O2 -fopenacc -fdump-tree-omplower -fdump-tree-optimized test_openacc_partitions.c -o test_acc_partitions
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define N 1024
#define SEED 42

/* Use volatile to prevent compile-time elimination */
static volatile int use_partition_mode = 0;

/* Initialize array with pattern */
void init_array(float *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = (float)((i + 1) % 100) * 0.1f;
    }
}

/* Test 0: gang redundant (no partition modifier) */
void test_gang_redundant(float *src, float *dest, int n, float *sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n], dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 2.0f;
            local_sum += src[i];
        }
    }
    
    *sum = local_sum;
}

/* Test 1: gang partitioned */
void test_gang_partitioned(float *src, float *dest, int n, float *sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 3.0f;
            local_sum += src[i] * 2.0f;
        }
    }
    
    *sum = local_sum;
}

/* Test 2: worker partitioned */
void test_worker_partitioned(float *src, float *dest, int n, float *sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 4.0f;
            local_sum += src[i] * 3.0f;
        }
    }
    
    *sum = local_sum;
}

/* Test 3: gang+worker partitioned */
void test_gang_worker_partitioned(float *src, float *dest, int n, float *sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 5.0f;
            local_sum += src[i] * 4.0f;
        }
    }
    
    *sum = local_sum;
}

/* Test 4: vector partitioned */
void test_vector_partitioned(float *src, float *dest, int n, float *sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 6.0f;
            local_sum += src[i] * 5.0f;
        }
    }
    
    *sum = local_sum;
}

/* Test 5: gang+vector partitioned */
void test_gang_vector_partitioned(float *src, float *dest, int n, float *sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 7.0f;
            local_sum += src[i] * 6.0f;
        }
    }
    
    *sum = local_sum;
}

/* Test 6: worker+vector partitioned */
void test_worker_vector_partitioned(float *src, float *dest, int n, float *sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 8.0f;
            local_sum += src[i] * 7.0f;
        }
    }
    
    *sum = local_sum;
}

/* Test 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(float *src, float *dest, int n, float *sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 9.0f;
            local_sum += src[i] * 8.0f;
        }
    }
    
    *sum = local_sum;
}

/* Wrapper that selects partition mode based on volatile variable */
void test_partition_wrapper(int mode, float *src, float *dest, int n, float *sum) {
    switch (mode) {
        case 0: test_gang_redundant(src, dest, n, sum); break;
        case 1: test_gang_partitioned(src, dest, n, sum); break;
        case 2: test_worker_partitioned(src, dest, n, sum); break;
        case 3: test_gang_worker_partitioned(src, dest, n, sum); break;
        case 4: test_vector_partitioned(src, dest, n, sum); break;
        case 5: test_gang_vector_partitioned(src, dest, n, sum); break;
        case 6: test_worker_vector_partitioned(src, dest, n, sum); break;
        case 7: test_fully_partitioned(src, dest, n, sum); break;
        default: *sum = -1.0f; break;
    }
}

int main() {
    float *src = (float*)malloc(N * sizeof(float));
    float *dest = (float*)malloc(N * sizeof(float));
    float sums[8] = {0};
    float total_checksum = 0.0f;
    
    if (!src || !dest) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_array(src, N);
    
    /* Test all 8 partition modes */
    for (int mode = 0; mode < 8; mode++) {
        /* Use volatile to force compiler to consider all modes */
        use_partition_mode = mode;
        
        /* Clear destination */
        for (int i = 0; i < N; i++) dest[i] = 0.0f;
        
        /* Test this partition mode */
        test_partition_wrapper(use_partition_mode, src, dest, N, &sums[mode]);
        
        /* Compute checksum from destination array */
        float dest_sum = 0.0f;
        for (int i = 0; i < N; i++) {
            dest_sum += dest[i];
        }
        
        total_checksum += sums[mode] + dest_sum;
        
        #ifdef _OPENACC
        printf("Mode %d: sum = %f, dest_sum = %f\n", 
               mode, sums[mode], dest_sum);
        #endif
    }
    
    /* Final checksum to prevent dead code elimination */
    printf("Total checksum: %f\n", total_checksum);
    
    /* Additional tests with explicit data clauses and partition modifiers */
    /* These directly test the specific partition codes */
    
    /* Test with gang modifier on data clause */
    {
        float test_arr[N];
        init_array(test_arr, N);
        float test_sum = 0.0f;
        
        #pragma acc parallel copy(test_arr[0:N]) copyout(test_sum)
        {
            #pragma acc loop gang reduction(+:test_sum)
            for (int i = 0; i < N; i++) {
                test_sum += test_arr[i];
            }
        }
        total_checksum += test_sum;
    }
    
    /* Test with worker modifier on data clause */
    {
        float test_arr[N];
        init_array(test_arr, N);
        float test_sum = 0.0f;
        
        #pragma acc parallel copy(test_arr[0:N]) copyout(test_sum)
        {
            #pragma acc loop worker reduction(+:test_sum)
            for (int i = 0; i < N; i++) {
                test_sum += test_arr[i] * 2.0f;
            }
        }
        total_checksum += test_sum;
    }
    
    /* Test with vector modifier on data clause */
    {
        float test_arr[N];
        init_array(test_arr, N);
        float test_sum = 0.0f;
        
        #pragma acc parallel copy(test_arr[0:N]) copyout(test_sum)
        {
            #pragma acc loop vector reduction(+:test_sum)
            for (int i = 0; i < N; i++) {
                test_sum += test_arr[i] * 3.0f;
            }
        }
        total_checksum += test_sum;
    }
    
    /* Test with gang worker vector (fully partitioned) */
    {
        float test_arr[N];
        init_array(test_arr, N);
        float test_sum = 0.0f;
        
        #pragma acc parallel copy(test_arr[0:N]) copyout(test_sum)
        {
            #pragma acc loop gang worker vector reduction(+:test_sum)
            for (int i = 0; i < N; i++) {
                test_sum += test_arr[i] * 4.0f;
            }
        }
        total_checksum += test_sum;
    }
    
    printf("Final total checksum: %f\n", total_checksum);
    
    free(src);
    free(dest);
    
    return 0;
}
