/* Test program to cover partition code string mapping in GCC's OpenACC support.
   Compile with: gcc -O2 -fopenacc -fdump-tree-omplower -fdump-tree-optimized test.c -o test
   For full offload to NVIDIA: gcc -O2 -fopenacc -fdump-tree-omplower -foffload=nvptx-none test.c -o test_offload
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define VOLATILE_INPUT 1  /* Use volatile to prevent compile-time elimination */

/* Test functions for each partition code 0-7 */

/* Code 0: gang redundant - no partition modifier */
void test_gang_redundant(float *src, float *dest, int n, float *sum) {
    volatile int use_offload = VOLATILE_INPUT;
    
    if (use_offload) {
        #pragma acc parallel loop copy(src[0:n]) copy(dest[0:n]) reduction(+:*sum) gang
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 2.0f;
            *sum += src[i];
        }
    }
}

/* Code 1: gang partitioned */
void test_gang_partitioned(float *src, float *dest, int n, float *sum) {
    volatile int use_offload = VOLATILE_INPUT;
    
    if (use_offload) {
        #pragma acc parallel loop copy gang(src[0:n]) copy gang(dest[0:n]) reduction(+:*sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 3.0f;
            *sum += src[i] * 2;
        }
    }
}

/* Code 2: worker partitioned */
void test_worker_partitioned(float *src, float *dest, int n, float *sum) {
    volatile int use_offload = VOLATILE_INPUT;
    
    if (use_offload) {
        #pragma acc parallel loop copy worker(src[0:n]) copy worker(dest[0:n]) reduction(+:*sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 4.0f;
            *sum += src[i] * 3;
        }
    }
}

/* Code 3: gang+worker partitioned */
void test_gang_worker_partitioned(float *src, float *dest, int n, float *sum) {
    volatile int use_offload = VOLATILE_INPUT;
    
    if (use_offload) {
        #pragma acc parallel loop copy gang worker(src[0:n]) copy gang worker(dest[0:n]) reduction(+:*sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 5.0f;
            *sum += src[i] * 4;
        }
    }
}

/* Code 4: vector partitioned */
void test_vector_partitioned(float *src, float *dest, int n, float *sum) {
    volatile int use_offload = VOLATILE_INPUT;
    
    if (use_offload) {
        #pragma acc parallel loop copy vector(src[0:n]) copy vector(dest[0:n]) reduction(+:*sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 6.0f;
            *sum += src[i] * 5;
        }
    }
}

/* Code 5: gang+vector partitioned */
void test_gang_vector_partitioned(float *src, float *dest, int n, float *sum) {
    volatile int use_offload = VOLATILE_INPUT;
    
    if (use_offload) {
        #pragma acc parallel loop copy gang vector(src[0:n]) copy gang vector(dest[0:n]) reduction(+:*sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 7.0f;
            *sum += src[i] * 6;
        }
    }
}

/* Code 6: worker+vector partitioned */
void test_worker_vector_partitioned(float *src, float *dest, int n, float *sum) {
    volatile int use_offload = VOLATILE_INPUT;
    
    if (use_offload) {
        #pragma acc parallel loop copy worker vector(src[0:n]) copy worker vector(dest[0:n]) reduction(+:*sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 8.0f;
            *sum += src[i] * 7;
        }
    }
}

/* Code 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(float *src, float *dest, int n, float *sum) {
    volatile int use_offload = VOLATILE_INPUT;
    
    if (use_offload) {
        #pragma acc parallel loop copy gang worker vector(src[0:n]) copy gang worker vector(dest[0:n]) reduction(+:*sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 9.0f;
            *sum += src[i] * 8;
        }
    }
}

/* Additional test using kernels construct with explicit loop directives */
void test_kernels_mixed_partitions(float *src, float *dest1, float *dest2, int n, float *sum) {
    volatile int use_offload = VOLATILE_INPUT;
    
    if (use_offload) {
        #pragma acc kernels copyin(src[0:n]) create(dest1[0:n], dest2[0:n]) reduction(+:*sum)
        {
            /* Mix different partition types within same kernels region */
            #pragma acc loop gang worker
            for (int i = 0; i < n; i++) {
                dest1[i] = src[i] * 10.0f;
            }
            
            #pragma acc loop vector
            for (int i = 0; i < n; i++) {
                dest2[i] = src[i] * 11.0f;
                *sum += src[i] * 9;
            }
        }
    }
}

/* Test with nested loops to engage complex partitioning */
void test_nested_loops_partitioned(float *src, float *dest, int n, float *sum) {
    volatile int use_offload = VOLATILE_INPUT;
    int side = 32;  /* For 32x32 grid */
    
    if (use_offload && n >= side * side) {
        #pragma acc parallel loop copy gang worker(src[0:n]) copy gang worker vector(dest[0:n]) reduction(+:*sum)
        for (int i = 0; i < side; i++) {
            #pragma acc loop vector
            for (int j = 0; j < side; j++) {
                int idx = i * side + j;
                dest[idx] = src[idx] * (i + j + 1.0f);
                *sum += src[idx];
            }
        }
    }
}

int main() {
    float *src = (float*)malloc(N * sizeof(float));
    float *dest = (float*)malloc(N * sizeof(float));
    float *dest2 = (float*)malloc(N * sizeof(float));
    float total_sum = 0.0f;
    
    /* Initialize source array with pattern */
    for (int i = 0; i < N; i++) {
        src[i] = (i % 100) * 0.1f;
    }
    
    /* Clear destination arrays */
    memset(dest, 0, N * sizeof(float));
    memset(dest2, 0, N * sizeof(float));
    
    printf("Starting OpenACC partition coverage tests...\n");
    
    /* Test all partition codes 0-7 */
    float sum = 0.0f;
    
    test_gang_redundant(src, dest, N, &sum);
    total_sum += sum + dest[N/2];
    
    sum = 0.0f;
    test_gang_partitioned(src, dest, N, &sum);
    total_sum += sum + dest[N/3];
    
    sum = 0.0f;
    test_worker_partitioned(src, dest, N, &sum);
    total_sum += sum + dest[N/4];
    
    sum = 0.0f;
    test_gang_worker_partitioned(src, dest, N, &sum);
    total_sum += sum + dest[N/5];
    
    sum = 0.0f;
    test_vector_partitioned(src, dest, N, &sum);
    total_sum += sum + dest[N/6];
    
    sum = 0.0f;
    test_gang_vector_partitioned(src, dest, N, &sum);
    total_sum += sum + dest[N/7];
    
    sum = 0.0f;
    test_worker_vector_partitioned(src, dest, N, &sum);
    total_sum += sum + dest[N/8];
    
    sum = 0.0f;
    test_fully_partitioned(src, dest, N, &sum);
    total_sum += sum + dest[N/9];
    
    /* Test mixed partitions in kernels region */
    sum = 0.0f;
    test_kernels_mixed_partitions(src, dest, dest2, N, &sum);
    total_sum += sum + dest[N/10] + dest2[N/11];
    
    /* Test nested loops */
    sum = 0.0f;
    test_nested_loops_partitioned(src, dest, N, &sum);
    total_sum += sum + dest[N/12];
    
    /* Print checksum to prevent dead code elimination */
    printf("Final checksum: %f\n", total_sum);
    
    /* Verify some results */
    int verify_idx = N/2;
    printf("Sample verification - src[%d] = %f, dest[%d] = %f\n", 
           verify_idx, src[verify_idx], verify_idx, dest[verify_idx]);
    
    free(src);
    free(dest);
    free(dest2);
    
    return 0;
}
