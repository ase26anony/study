/* test_partition_codes.c - Cover GCC omp-oacc-neuter-broadcast.cc partition string mapping */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define GANG_SIZE 32
#define WORKER_SIZE 4
#define VECTOR_SIZE 32

/* Function prototypes for different partition schemes */
void test_gang_redundant(float *src, float *dest, int n, float *sum);
void test_gang_partitioned(float *src, float *dest, int n, float *sum);
void test_worker_partitioned(float *src, float *dest, int n, float *sum);
void test_gang_worker_partitioned(float *src, float *dest, int n, float *sum);
void test_vector_partitioned(float *src, float *dest, int n, float *sum);
void test_gang_vector_partitioned(float *src, float *dest, int n, float *sum);
void test_worker_vector_partitioned(float *src, float *dest, int n, float *sum);
void test_fully_partitioned(float *src, float *dest, int n, float *sum);

/* Volatile variable to prevent compile-time elimination */
volatile int use_partition = 0;

int main() {
    float *src = (float*)malloc(N * sizeof(float));
    float *dest = (float*)malloc(N * sizeof(float));
    float total_sum = 0.0f;
    
    /* Initialize source array with patterned data */
    for (int i = 0; i < N; i++) {
        src[i] = (float)(i % 100) * 0.1f;
    }
    
    printf("Testing OpenACC partition codes 0-7...\n");
    
    /* Test all 8 partition cases */
    for (int p = 0; p < 8; p++) {
        use_partition = p; /* Force runtime evaluation */
        memset(dest, 0, N * sizeof(float));
        float part_sum = 0.0f;
        
        switch (p) {
            case 0:
                test_gang_redundant(src, dest, N, &part_sum);
                break;
            case 1:
                test_gang_partitioned(src, dest, N, &part_sum);
                break;
            case 2:
                test_worker_partitioned(src, dest, N, &part_sum);
                break;
            case 3:
                test_gang_worker_partitioned(src, dest, N, &part_sum);
                break;
            case 4:
                test_vector_partitioned(src, dest, N, &part_sum);
                break;
            case 5:
                test_gang_vector_partitioned(src, dest, N, &part_sum);
                break;
            case 6:
                test_worker_vector_partitioned(src, dest, N, &part_sum);
                break;
            case 7:
                test_fully_partitioned(src, dest, N, &part_sum);
                break;
        }
        
        /* Verify computation */
        float verify_sum = 0.0f;
        for (int i = 0; i < N; i++) {
            verify_sum += dest[i];
        }
        
        printf("Partition %d: sum = %f (offload), %f (host), diff = %f\n",
               p, part_sum, verify_sum, part_sum - verify_sum);
        total_sum += part_sum;
    }
    
    printf("Total checksum: %f\n", total_sum);
    
    free(src);
    free(dest);
    return 0;
}

/* Case 0: gang redundant */
void test_gang_redundant(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(sum[0:1]) \
                         num_gangs(GANG_SIZE) num_workers(1) vector_length(VECTOR_SIZE)
    {
        #pragma acc loop gang reduction(+:*sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 2.0f;
            *sum += dest[i];
        }
    }
}

/* Case 1: gang partitioned */
void test_gang_partitioned(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copyout(dest[0:n], sum[0:1]) \
                         num_gangs(GANG_SIZE) num_workers(1) vector_length(VECTOR_SIZE)
    {
        #pragma acc loop gang gang(dest) reduction(+:*sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 3.0f;
            *sum += dest[i];
        }
    }
}

/* Case 2: worker partitioned */
void test_worker_partitioned(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copyout(dest[0:n], sum[0:1]) \
                         num_gangs(GANG_SIZE) num_workers(WORKER_SIZE) vector_length(VECTOR_SIZE)
    {
        #pragma acc loop gang worker worker(dest) reduction(+:*sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 4.0f;
            *sum += dest[i];
        }
    }
}

/* Case 3: gang+worker partitioned */
void test_gang_worker_partitioned(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copyout(dest[0:n], sum[0:1]) \
                         num_gangs(GANG_SIZE) num_workers(WORKER_SIZE) vector_length(VECTOR_SIZE)
    {
        #pragma acc loop gang worker gang worker(dest) reduction(+:*sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 5.0f;
            *sum += dest[i];
        }
    }
}

/* Case 4: vector partitioned */
void test_vector_partitioned(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copyout(dest[0:n], sum[0:1]) \
                         num_gangs(1) num_workers(1) vector_length(VECTOR_SIZE)
    {
        #pragma acc loop vector vector(dest) reduction(+:*sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 6.0f;
            *sum += dest[i];
        }
    }
}

/* Case 5: gang+vector partitioned */
void test_gang_vector_partitioned(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copyout(dest[0:n], sum[0:1]) \
                         num_gangs(GANG_SIZE) num_workers(1) vector_length(VECTOR_SIZE)
    {
        #pragma acc loop gang vector gang vector(dest) reduction(+:*sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 7.0f;
            *sum += dest[i];
        }
    }
}

/* Case 6: worker+vector partitioned */
void test_worker_vector_partitioned(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copyout(dest[0:n], sum[0:1]) \
                         num_gangs(1) num_workers(WORKER_SIZE) vector_length(VECTOR_SIZE)
    {
        #pragma acc loop worker vector worker vector(dest) reduction(+:*sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 8.0f;
            *sum += dest[i];
        }
    }
}

/* Case 7: fully partitioned */
void test_fully_partitioned(float *src, float *dest, int n, float *sum) {
    *sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copyout(dest[0:n], sum[0:1]) \
                         num_gangs(GANG_SIZE) num_workers(WORKER_SIZE) vector_length(VECTOR_SIZE)
    {
        #pragma acc loop gang worker vector gang worker vector(dest) reduction(+:*sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 9.0f;
            *sum += dest[i];
        }
    }
}
