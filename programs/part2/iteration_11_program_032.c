/* test_openacc_partitions.c
 * 
 * This program systematically tests all 8 OpenACC data partition modes
 * to trigger coverage of the partition code string mapping function
 * in omp-oacc-neuter-broadcast.cc lines 335-343.
 *
 * Compile with: gcc -O2 -fopenacc -fdump-tree-omplower -fdump-tree-optimized test_openacc_partitions.c -o test_partitions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define NUM_CASES 8

/* Global volatile variable to prevent compile-time elimination */
volatile int use_partition_mode = 0;

/* Function prototypes for each partition mode */
void test_gang_redundant(float *src, float *dest, int n, float *reduction);
void test_gang_partitioned(float *src, float *dest, int n, float *reduction);
void test_worker_partitioned(float *src, float *dest, int n, float *reduction);
void test_gang_worker_partitioned(float *src, float *dest, int n, float *reduction);
void test_vector_partitioned(float *src, float *dest, int n, float *reduction);
void test_gang_vector_partitioned(float *src, float *dest, int n, float *reduction);
void test_worker_vector_partitioned(float *src, float *dest, int n, float *reduction);
void test_fully_partitioned(float *src, float *dest, int n, float *reduction);

/* Test case 0: gang redundant */
void test_gang_redundant(float *src, float *dest, int n, float *reduction)
{
    float local_sum = 0.0f;
    
    /* No partition modifier - should generate code 0 */
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

/* Test case 1: gang partitioned */
void test_gang_partitioned(float *src, float *dest, int n, float *reduction)
{
    float local_sum = 0.0f;
    
    /* Explicit gang partition - should generate code 1 */
    #pragma acc parallel copy gang(src[0:n], dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 3.0f;
            local_sum += src[i] * 2;
        }
    }
    
    *reduction = local_sum;
}

/* Test case 2: worker partitioned */
void test_worker_partitioned(float *src, float *dest, int n, float *reduction)
{
    float local_sum = 0.0f;
    
    /* Explicit worker partition - should generate code 2 */
    #pragma acc parallel copy worker(src[0:n], dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 4.0f;
            local_sum += src[i] * 3;
        }
    }
    
    *reduction = local_sum;
}

/* Test case 3: gang+worker partitioned */
void test_gang_worker_partitioned(float *src, float *dest, int n, float *reduction)
{
    float local_sum = 0.0f;
    
    /* Explicit gang+worker partition - should generate code 3 */
    #pragma acc parallel copy gang worker(src[0:n], dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 5.0f;
            local_sum += src[i] * 4;
        }
    }
    
    *reduction = local_sum;
}

/* Test case 4: vector partitioned */
void test_vector_partitioned(float *src, float *dest, int n, float *reduction)
{
    float local_sum = 0.0f;
    
    /* Explicit vector partition - should generate code 4 */
    #pragma acc parallel copy vector(src[0:n], dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 6.0f;
            local_sum += src[i] * 5;
        }
    }
    
    *reduction = local_sum;
}

/* Test case 5: gang+vector partitioned */
void test_gang_vector_partitioned(float *src, float *dest, int n, float *reduction)
{
    float local_sum = 0.0f;
    
    /* Explicit gang+vector partition - should generate code 5 */
    #pragma acc parallel copy gang vector(src[0:n], dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 7.0f;
            local_sum += src[i] * 6;
        }
    }
    
    *reduction = local_sum;
}

/* Test case 6: worker+vector partitioned */
void test_worker_vector_partitioned(float *src, float *dest, int n, float *reduction)
{
    float local_sum = 0.0f;
    
    /* Explicit worker+vector partition - should generate code 6 */
    #pragma acc parallel copy worker vector(src[0:n], dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 8.0f;
            local_sum += src[i] * 7;
        }
    }
    
    *reduction = local_sum;
}

/* Test case 7: fully partitioned */
void test_fully_partitioned(float *src, float *dest, int n, float *reduction)
{
    float local_sum = 0.0f;
    
    /* Explicit gang+worker+vector partition - should generate code 7 */
    #pragma acc parallel copy gang worker vector(src[0:n], dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 9.0f;
            local_sum += src[i] * 8;
        }
    }
    
    *reduction = local_sum;
}

/* Alternative approach using kernels construct for variety */
void test_kernels_partition(float *src, float *dest, int n, float *reduction)
{
    float local_sum = 0.0f;
    
    /* Using kernels construct with create clause and different partition modes */
    #pragma acc kernels create gang worker(src[0:n]) copyout(dest[0:n], local_sum)
    {
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 10.0f;
            local_sum += src[i] * 9;
        }
    }
    
    *reduction = local_sum;
}

/* Test with present clause to trigger different code paths */
void test_present_partition(float *src, float *dest, int n, float *reduction)
{
    float local_sum = 0.0f;
    
    /* First copy data to device */
    #pragma acc enter data copyin(src[0:n], dest[0:n])
    
    /* Then use present clause with partition modifiers */
    #pragma acc parallel present gang vector(src[0:n], dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 11.0f;
            local_sum += src[i] * 10;
        }
    }
    
    #pragma acc exit data delete(src[0:n], dest[0:n])
    
    *reduction = local_sum;
}

int main()
{
    float *src = (float *)malloc(N * sizeof(float));
    float *dest = (float *)malloc(N * sizeof(float));
    float reductions[NUM_CASES];
    float final_checksum = 0.0f;
    
    if (!src || !dest) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize source array with patterned data */
    for (int i = 0; i < N; i++) {
        src[i] = (float)(i % 100) * 0.1f;
    }
    
    /* Clear destination arrays */
    memset(dest, 0, N * sizeof(float));
    
    /* Test all partition modes using volatile to prevent elimination */
    for (int mode = 0; mode < NUM_CASES; mode++) {
        use_partition_mode = mode;
        
        /* Clear destination for each test */
        memset(dest, 0, N * sizeof(float));
        
        switch (use_partition_mode) {
            case 0:
                test_gang_redundant(src, dest, N, &reductions[0]);
                break;
            case 1:
                test_gang_partitioned(src, dest, N, &reductions[1]);
                break;
            case 2:
                test_worker_partitioned(src, dest, N, &reductions[2]);
                break;
            case 3:
                test_gang_worker_partitioned(src, dest, N, &reductions[3]);
                break;
            case 4:
                test_vector_partitioned(src, dest, N, &reductions[4]);
                break;
            case 5:
                test_gang_vector_partitioned(src, dest, N, &reductions[5]);
                break;
            case 6:
                test_worker_vector_partitioned(src, dest, N, &reductions[6]);
                break;
            case 7:
                test_fully_partitioned(src, dest, N, &reductions[7]);
                break;
        }
        
        /* Compute partial checksum from destination array */
        float partial_sum = 0.0f;
        for (int i = 0; i < N; i++) {
            partial_sum += dest[i];
        }
        final_checksum += partial_sum + reductions[mode];
        
        /* Print something to prevent dead code elimination */
        printf("Mode %d: dest[0]=%.2f, reduction=%.2f\n", 
               mode, dest[0], reductions[mode]);
    }
    
    /* Additional tests for variety */
    memset(dest, 0, N * sizeof(float));
    float extra_reduction;
    test_kernels_partition(src, dest, N, &extra_reduction);
    final_checksum += extra_reduction;
    
    memset(dest, 0, N * sizeof(float));
    test_present_partition(src, dest, N, &extra_reduction);
    final_checksum += extra_reduction;
    
    /* Final output to ensure observable side effect */
    printf("Final checksum: %.6f\n", final_checksum);
    
    free(src);
    free(dest);
    
    return 0;
}
