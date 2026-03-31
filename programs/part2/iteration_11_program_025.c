/* test_omp_acc_partitions.c
 * Comprehensive test for OpenACC data partition codes 0-7
 * Compile with: gcc -O2 -fopenacc -fdump-tree-omplower -fdump-tree-optimized test.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 32

/* Test 0: gang redundant */
void test_gang_redundant(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n], dest[0:n]) copy(sum) \
        present_or_copy(src[0:n], dest[0:n])
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
    
    #pragma acc parallel copy(src[0:n]) create(dest[0:n]) copy(sum) \
        copy gang(src[0:n])
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
    
    #pragma acc parallel copy(src[0:n], dest[0:n]) copy(sum) \
        copy worker(src[0:n])
    {
        #pragma acc loop gang worker reduction(+:*sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 4.0f;
            *sum += dest[i];
        }
    }
}

/* Test 3: gang+worker partitioned */
void test_gang_worker_partitioned(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n], dest[0:n]) copy(sum) \
        copy gang worker(src[0:n])
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
    
    #pragma acc parallel copy(src[0:n], dest[0:n]) copy(sum) \
        copy vector(src[0:n])
    {
        #pragma acc loop gang vector reduction(+:*sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 6.0f;
            *sum += dest[i];
        }
    }
}

/* Test 5: gang+vector partitioned */
void test_gang_vector_partitioned(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n], dest[0:n]) copy(sum) \
        copy gang vector(src[0:n])
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
    
    #pragma acc parallel copy(src[0:n], dest[0:n]) copy(sum) \
        copy worker vector(src[0:n])
    {
        #pragma acc loop gang worker vector reduction(+:*sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 8.0f;
            *sum += dest[i];
        }
    }
}

/* Test 7: fully partitioned */
void test_fully_partitioned(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n], dest[0:n]) copy(sum) \
        copy gang worker vector(src[0:n])
    {
        #pragma acc loop gang worker vector reduction(+:*sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 9.0f;
            *sum += dest[i];
        }
    }
}

/* Additional test with nested loops to trigger complex partitioning */
void test_nested_partitions(float *src, float *dest, int n, int m, float *sum) {
    *sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n*m], dest[0:n*m]) copy(sum) \
        copy gang worker(src[0:n*m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector reduction(+:*sum)
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                dest[idx] = src[idx] * (i + j + 1.0f);
                *sum += dest[idx];
            }
        }
    }
}

/* Test with data region for different partition modes */
void test_data_region_partitions(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    
    #pragma acc data copyin(src[0:n]) copyout(dest[0:n]) copy(sum) \
        copy gang worker vector(src[0:n])
    {
        #pragma acc parallel present(src, dest, sum)
        {
            #pragma acc loop gang worker vector reduction(+:*sum)
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] * 10.0f;
                *sum += dest[i];
            }
        }
    }
}

/* Main function that calls all tests */
int main() {
    float *src = (float*)malloc(N * sizeof(float));
    float *dest = (float*)malloc(N * sizeof(float));
    float sums[10] = {0};
    float total_sum = 0.0f;
    
    /* Initialize source array with pattern */
    for (int i = 0; i < N; i++) {
        src[i] = (float)(i % 100) * 0.1f;
    }
    
    /* Use volatile to prevent compile-time elimination */
    volatile int test_selector = 0;
    
    /* Test all 8 partition codes systematically */
    if (test_selector == 0) {
        test_gang_redundant(src, dest, N, &sums[0]);
        printf("Test 0 (gang redundant) sum: %f\n", sums[0]);
        total_sum += sums[0];
    }
    
    memset(dest, 0, N * sizeof(float));
    test_gang_partitioned(src, dest, N, &sums[1]);
    printf("Test 1 (gang partitioned) sum: %f\n", sums[1]);
    total_sum += sums[1];
    
    memset(dest, 0, N * sizeof(float));
    test_worker_partitioned(src, dest, N, &sums[2]);
    printf("Test 2 (worker partitioned) sum: %f\n", sums[2]);
    total_sum += sums[2];
    
    memset(dest, 0, N * sizeof(float));
    test_gang_worker_partitioned(src, dest, N, &sums[3]);
    printf("Test 3 (gang+worker partitioned) sum: %f\n", sums[3]);
    total_sum += sums[3];
    
    memset(dest, 0, N * sizeof(float));
    test_vector_partitioned(src, dest, N, &sums[4]);
    printf("Test 4 (vector partitioned) sum: %f\n", sums[4]);
    total_sum += sums[4];
    
    memset(dest, 0, N * sizeof(float));
    test_gang_vector_partitioned(src, dest, N, &sums[5]);
    printf("Test 5 (gang+vector partitioned) sum: %f\n", sums[5]);
    total_sum += sums[5];
    
    memset(dest, 0, N * sizeof(float));
    test_worker_vector_partitioned(src, dest, N, &sums[6]);
    printf("Test 6 (worker+vector partitioned) sum: %f\n", sums[6]);
    total_sum += sums[6];
    
    memset(dest, 0, N * sizeof(float));
    test_fully_partitioned(src, dest, N, &sums[7]);
    printf("Test 7 (fully partitioned) sum: %f\n", sums[7]);
    total_sum += sums[7];
    
    /* Additional tests with different patterns */
    float *src2d = (float*)malloc(N * M * sizeof(float));
    float *dest2d = (float*)malloc(N * M * sizeof(float));
    
    for (int i = 0; i < N * M; i++) {
        src2d[i] = (float)(i % 50) * 0.05f;
    }
    
    test_nested_partitions(src2d, dest2d, N, M, &sums[8]);
    printf("Test nested partitions sum: %f\n", sums[8]);
    total_sum += sums[8];
    
    memset(dest, 0, N * sizeof(float));
    test_data_region_partitions(src, dest, N, &sums[9]);
    printf("Test data region partitions sum: %f\n", sums[9]);
    total_sum += sums[9];
    
    /* Final checksum */
    printf("\nTotal checksum: %f\n", total_sum);
    
    /* Verify some results */
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (dest[i] != src[i] * 10.0f) {
            errors++;
            if (errors < 5) {
                printf("Mismatch at %d: %f != %f\n", i, dest[i], src[i] * 10.0f);
            }
        }
    }
    
    if (errors > 0) {
        printf("Found %d errors in final array\n", errors);
    } else {
        printf("All tests completed successfully\n");
    }
    
    free(src);
    free(dest);
    free(src2d);
    free(dest2d);
    
    return 0;
}
