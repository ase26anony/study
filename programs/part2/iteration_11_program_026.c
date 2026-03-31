/* test_openacc_partitions.c - Cover all data partition mapping codes in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 32

/* Test function for each partition code */

/* Code 0: gang redundant */
void test_gang_redundant(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum) /* gang redundant */
    {
        #pragma acc loop gang reduction(+:sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 2.0f;
            *sum += dest[i];
        }
    }
}

/* Code 1: gang partitioned */
void test_gang_partitioned(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum)
    {
        #pragma acc loop gang reduction(+:sum) gang
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 3.0f;
            *sum += dest[i];
        }
    }
}

/* Code 2: worker partitioned */
void test_worker_partitioned(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum)
    {
        #pragma acc loop worker reduction(+:sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 4.0f;
            *sum += dest[i];
        }
    }
}

/* Code 3: gang+worker partitioned */
void test_gang_worker_partitioned(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum)
    {
        #pragma acc loop gang worker reduction(+:sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 5.0f;
            *sum += dest[i];
        }
    }
}

/* Code 4: vector partitioned */
void test_vector_partitioned(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum)
    {
        #pragma acc loop vector reduction(+:sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 6.0f;
            *sum += dest[i];
        }
    }
}

/* Code 5: gang+vector partitioned */
void test_gang_vector_partitioned(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum)
    {
        #pragma acc loop gang vector reduction(+:sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 7.0f;
            *sum += dest[i];
        }
    }
}

/* Code 6: worker+vector partitioned */
void test_worker_vector_partitioned(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum)
    {
        #pragma acc loop worker vector reduction(+:sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 8.0f;
            *sum += dest[i];
        }
    }
}

/* Code 7: fully partitioned */
void test_fully_partitioned(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum)
    {
        #pragma acc loop gang worker vector reduction(+:sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 9.0f;
            *sum += dest[i];
        }
    }
}

/* Additional tests with explicit data clauses for partition modifiers */

void test_explicit_gang_partition(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    /* Explicit gang partition on data clause */
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum)
    {
        #pragma acc loop gang reduction(+:sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] + 1.0f;
            *sum += dest[i];
        }
    }
}

void test_explicit_worker_partition(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    /* Explicit worker partition on data clause */
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum)
    {
        #pragma acc loop worker reduction(+:sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] + 2.0f;
            *sum += dest[i];
        }
    }
}

void test_explicit_vector_partition(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    /* Explicit vector partition on data clause */
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum)
    {
        #pragma acc loop vector reduction(+:sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] + 3.0f;
            *sum += dest[i];
        }
    }
}

/* Test with nested loops to trigger complex partitioning */
void test_nested_partitioning(float *src, float *dest, int n, int m, float *sum) {
    *sum = 0.0f;
    #pragma acc parallel copy(src[0:n*m]) copy(dest[0:n*m]) copy(sum)
    {
        #pragma acc loop gang reduction(+:sum)
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                dest[idx] = src[idx] * (i + j);
                *sum += dest[idx];
            }
        }
    }
}

/* Test with kernels construct and data clauses */
void test_kernels_partition(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    #pragma acc data copy(src[0:n], dest[0:n], sum)
    {
        #pragma acc kernels
        {
            #pragma acc loop gang worker vector reduction(+:sum)
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] * 10.0f;
                *sum += dest[i];
            }
        }
    }
}

/* Main function with volatile control to prevent optimization */
int main() {
    const int n = N;
    const int m = M;
    float *src = (float*)malloc(n * m * sizeof(float));
    float *dest = (float*)malloc(n * m * sizeof(float));
    float sums[12] = {0};
    
    /* Initialize source array */
    for (int i = 0; i < n * m; i++) {
        src[i] = (float)(i % 100) * 0.1f;
    }
    
    /* Use volatile to prevent compile-time elimination */
    volatile int run_all = 1;
    
    if (run_all) {
        /* Test all partition codes */
        test_gang_redundant(src, dest, n, &sums[0]);
        test_gang_partitioned(src, dest, n, &sums[1]);
        test_worker_partitioned(src, dest, n, &sums[2]);
        test_gang_worker_partitioned(src, dest, n, &sums[3]);
        test_vector_partitioned(src, dest, n, &sums[4]);
        test_gang_vector_partitioned(src, dest, n, &sums[5]);
        test_worker_vector_partitioned(src, dest, n, &sums[6]);
        test_fully_partitioned(src, dest, n, &sums[7]);
        
        /* Additional explicit partition tests */
        test_explicit_gang_partition(src, dest, n, &sums[8]);
        test_explicit_worker_partition(src, dest, n, &sums[9]);
        test_explicit_vector_partition(src, dest, n, &sums[10]);
        
        /* Complex partitioning tests */
        test_nested_partitioning(src, dest, n, m, &sums[11]);
        test_kernels_partition(src, dest, n, &sums[0]); /* Reuse sums[0] */
    }
    
    /* Compute final checksum to ensure code isn't eliminated */
    float final_checksum = 0.0f;
    for (int i = 0; i < 12; i++) {
        final_checksum += sums[i];
    }
    
    /* Also checksum the destination array */
    float array_checksum = 0.0f;
    for (int i = 0; i < n * m; i++) {
        array_checksum += dest[i];
    }
    
    printf("Final checksum: %f\n", final_checksum);
    printf("Array checksum: %f\n", array_checksum);
    
    free(src);
    free(dest);
    
    return 0;
}
