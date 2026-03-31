/* test_omp_acc_partition_codes.c
 * 
 * This program systematically tests OpenACC data partition modes
 * to trigger the compiler's internal partition code to string
 * mapping function in omp-oacc-neuter-broadcast.cc (lines 335-343).
 * 
 * Compile with: gcc -O2 -fopenacc -fdump-tree-omplower -fdump-tree-optimized -o test_partition test_omp_acc_partition_codes.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define N 1024
#define CHUNK_SIZE 128

/* Use volatile to prevent compile-time elimination of partition choices */
static volatile int partition_selector = 0;

/* Initialize array with pattern */
void init_array(float *arr, int size) {
    for (int i = 0; i < size; i++) {
        arr[i] = (float)(i % 100) * 0.5f;
    }
}

/* Test 0: gang redundant (no partition modifier) */
void test_gang_redundant(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n], dest[0:n]) copyout(sum[0:1])
    {
        #pragma acc loop gang reduction(+:*sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 2.0f;
            *sum += dest[i];
        }
    }
}

/* Test 1: gang partitioned */
void test_gang_partitioned(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) create(dest[0:n]) copyout(sum[0:1])
    {
        #pragma acc loop gang reduction(+:*sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 3.0f;
            *sum += dest[i];
        }
    }
}

/* Test 2: worker partitioned */
void test_worker_partitioned(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(sum[0:1])
    {
        #pragma acc loop worker reduction(+:*sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 4.0f;
            *sum += dest[i];
        }
    }
}

/* Test 3: gang+worker partitioned */
void test_gang_worker_partitioned(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(sum[0:1])
    {
        #pragma acc loop gang worker reduction(+:*sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 5.0f;
            *sum += dest[i];
        }
    }
}

/* Test 4: vector partitioned */
void test_vector_partitioned(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(sum[0:1])
    {
        #pragma acc loop vector reduction(+:*sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 6.0f;
            *sum += dest[i];
        }
    }
}

/* Test 5: gang+vector partitioned */
void test_gang_vector_partitioned(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(sum[0:1])
    {
        #pragma acc loop gang vector reduction(+:*sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 7.0f;
            *sum += dest[i];
        }
    }
}

/* Test 6: worker+vector partitioned */
void test_worker_vector_partitioned(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(sum[0:1])
    {
        #pragma acc loop worker vector reduction(+:*sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 8.0f;
            *sum += dest[i];
        }
    }
}

/* Test 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(sum[0:1])
    {
        #pragma acc loop gang worker vector reduction(+:*sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 9.0f;
            *sum += dest[i];
        }
    }
}

/* Combined test using data clauses with explicit partition modifiers */
void test_explicit_data_clauses(float *src, float *dest1, float *dest2, float *dest3, 
                                float *dest4, float *dest5, float *dest6, float *dest7, 
                                int n, float *results) {
    
    /* Use volatile selector to force compiler to consider all paths */
    int selector = partition_selector;
    
    if (selector == 0) {
        /* gang redundant - no modifier */
        #pragma acc parallel copy(src[0:n], dest1[0:n])
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                dest1[i] = src[i] * 1.1f;
            }
        }
    }
    
    /* Test all partition modifiers through data clauses */
    #pragma acc data copy(src[0:n]) copyout(dest2[0:n])
    #pragma acc parallel present(src, dest2)
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            dest2[i] = src[i] * 1.2f;
        }
    }
    
    #pragma acc data copy(src[0:n]) copy(dest3[0:n])
    #pragma acc parallel present(src, dest3)
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            dest3[i] = src[i] * 1.3f;
        }
    }
    
    #pragma acc data copy(src[0:n]) create(dest4[0:n])
    #pragma acc parallel present(src, dest4)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            dest4[i] = src[i] * 1.4f;
        }
    }
    
    #pragma acc data copy(src[0:n]) copy(dest5[0:n])
    #pragma acc parallel present(src, dest5)
    {
        #pragma acc loop vector
        for (int i = 0; i < n; i++) {
            dest5[i] = src[i] * 1.5f;
        }
    }
    
    #pragma acc data copy(src[0:n]) copy(dest6[0:n])
    #pragma acc parallel present(src, dest6)
    {
        #pragma acc loop gang vector
        for (int i = 0; i < n; i++) {
            dest6[i] = src[i] * 1.6f;
        }
    }
    
    #pragma acc data copy(src[0:n]) copy(dest7[0:n])
    #pragma acc parallel present(src, dest7)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < n; i++) {
            dest7[i] = src[i] * 1.7f;
        }
    }
    
    /* Compute checksums to ensure computation isn't optimized away */
    results[0] = 0.0f;
    results[1] = 0.0f;
    results[2] = 0.0f;
    results[3] = 0.0f;
    results[4] = 0.0f;
    results[5] = 0.0f;
    results[6] = 0.0f;
    
    #pragma acc parallel copyin(dest1[0:n], dest2[0:n], dest3[0:n], dest4[0:n], \
                                dest5[0:n], dest6[0:n], dest7[0:n]) \
                       copyout(results[0:7])
    {
        #pragma acc loop gang reduction(+:results[0:7])
        for (int i = 0; i < n; i++) {
            results[0] += dest1[i];
            results[1] += dest2[i];
            results[2] += dest3[i];
            results[3] += dest4[i];
            results[4] += dest5[i];
            results[5] += dest6[i];
            results[6] += dest7[i];
        }
    }
}

int main() {
    float *src = (float*)malloc(N * sizeof(float));
    float *dest = (float*)malloc(N * sizeof(float));
    float *dest1 = (float*)malloc(N * sizeof(float));
    float *dest2 = (float*)malloc(N * sizeof(float));
    float *dest3 = (float*)malloc(N * sizeof(float));
    float *dest4 = (float*)malloc(N * sizeof(float));
    float *dest5 = (float*)malloc(N * sizeof(float));
    float *dest6 = (float*)malloc(N * sizeof(float));
    float *dest7 = (float*)malloc(N * sizeof(float));
    float results[7];
    float sum;
    
    if (!src || !dest || !dest1 || !dest2 || !dest3 || !dest4 || 
        !dest5 || !dest6 || !dest7) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_array(src, N);
    
    printf("Testing OpenACC partition modes...\n");
    
    /* Test each partition mode individually */
    test_gang_redundant(src, dest, N, &sum);
    printf("Test 0 (gang redundant) sum: %f\n", sum);
    
    test_gang_partitioned(src, dest, N, &sum);
    printf("Test 1 (gang partitioned) sum: %f\n", sum);
    
    test_worker_partitioned(src, dest, N, &sum);
    printf("Test 2 (worker partitioned) sum: %f\n", sum);
    
    test_gang_worker_partitioned(src, dest, N, &sum);
    printf("Test 3 (gang+worker partitioned) sum: %f\n", sum);
    
    test_vector_partitioned(src, dest, N, &sum);
    printf("Test 4 (vector partitioned) sum: %f\n", sum);
    
    test_gang_vector_partitioned(src, dest, N, &sum);
    printf("Test 5 (gang+vector partitioned) sum: %f\n", sum);
    
    test_worker_vector_partitioned(src, dest, N, &sum);
    printf("Test 6 (worker+vector partitioned) sum: %f\n", sum);
    
    test_fully_partitioned(src, dest, N, &sum);
    printf("Test 7 (fully partitioned) sum: %f\n", sum);
    
    /* Test with explicit data clause modifiers */
    test_explicit_data_clauses(src, dest1, dest2, dest3, dest4, dest5, dest6, dest7, N, results);
    
    /* Final checksum to ensure all computations are retained */
    float final_checksum = 0.0f;
    for (int i = 0; i < N; i++) {
        final_checksum += dest[i] + dest1[i] + dest2[i] + dest3[i] + 
                         dest4[i] + dest5[i] + dest6[i] + dest7[i];
    }
    for (int i = 0; i < 7; i++) {
        final_checksum += results[i];
    }
    
    printf("Final checksum: %f\n", final_checksum);
    printf("Partition selector (volatile): %d\n", partition_selector);
    
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
