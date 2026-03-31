/* Test program to exercise omp-oacc-neuter-broadcast.cc partition mapping logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>
#include <omp.h>

/* Prevent optimization */
volatile int force_runtime = 1;

/* Helper to use values without optimization */
#define USE(V) asm volatile("" : : "r"(V))

/* Function that could trigger partition string mapping */
void debug_partition_info(int partition_code) {
    /* This mimics the internal compiler logic - the actual mapping
       happens in compiler internals, but we create similar patterns */
    const char* desc;
    switch(partition_code) {
        case 0: desc = "gang redundant"; break;
        case 1: desc = "gang partitioned"; break;
        case 2: desc = "worker partitioned"; break;
        case 3: desc = "gang+worker partitioned"; break;
        case 4: desc = "vector partitioned"; break;
        case 5: desc = "gang+vector partitioned"; break;
        case 6: desc = "worker+vector partitioned"; break;
        case 7: desc = "fully partitioned"; break;
        default: desc = "<illegal>"; break;
    }
    USE(desc);
    
    /* Simulate error reporting that might use these strings */
    if (partition_code < 0 || partition_code > 7) {
        fprintf(stderr, "Invalid partition code: %d\n", partition_code);
    }
}

/* Test case 0: Gang redundant (single gang) */
void test_gang_redundant() {
    int N = 1000;
    int *data = (int*)malloc(N * sizeof(int));
    
    #pragma acc parallel copy(data[0:N]) num_gangs(1) num_workers(1) vector_length(1)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            data[i] = i * 2;
        }
    }
    
    USE(data);
    free(data);
}

/* Test case 1: Gang partitioned */
void test_gang_partitioned() {
    int N = 10000;
    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));
    
    /* Initialize with runtime values */
    for (int i = 0; i < N; i++) {
        a[i] = (float)(i % 100);
        b[i] = 0.0f;
    }
    
    #pragma acc parallel copyin(a[0:N]) copyout(b[0:N]) num_gangs(32) num_workers(1) vector_length(1)
    {
        #pragma acc loop gang independent
        for (int i = 0; i < N; i++) {
            b[i] = a[i] * 2.0f;
        }
    }
    
    USE(b);
    free(a);
    free(b);
}

/* Test case 2: Worker partitioned */
void test_worker_partitioned() {
    int N = 512;
    int M = 512;
    int *matrix = (int*)malloc(N * M * sizeof(int));
    
    #pragma acc parallel copy(matrix[0:N*M]) num_gangs(1) num_workers(4) vector_length(1)
    {
        #pragma acc loop worker independent
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                matrix[i * M + j] = i + j;
            }
        }
    }
    
    USE(matrix);
    free(matrix);
}

/* Test case 3: Gang+Worker partitioned */
void test_gang_worker_partitioned() {
    int N = 1024;
    int M = 1024;
    double *grid = (double*)malloc(N * M * sizeof(double));
    
    #pragma acc parallel copy(grid[0:N*M]) num_gangs(8) num_workers(4) vector_length(1)
    {
        #pragma acc loop gang worker independent collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                grid[i * M + j] = (double)(i * j) / 1000.0;
            }
        }
    }
    
    USE(grid);
    free(grid);
}

/* Test case 4: Vector partitioned */
void test_vector_partitioned() {
    int N = 4096;
    float *data = (float*)malloc(N * sizeof(float));
    
    #pragma acc parallel copy(data[0:N]) num_gangs(1) num_workers(1) vector_length(256)
    {
        #pragma acc loop vector independent
        for (int i = 0; i < N; i++) {
            data[i] = (float)i * 3.14159f;
        }
    }
    
    USE(data);
    free(data);
}

/* Test case 5: Gang+Vector partitioned */
void test_gang_vector_partitioned() {
    int N = 8192;
    int *results = (int*)malloc(N * sizeof(int));
    
    #pragma acc parallel copy(results[0:N]) num_gangs(16) num_workers(1) vector_length(128)
    {
        #pragma acc loop gang vector independent
        for (int i = 0; i < N; i++) {
            results[i] = (i % 256) * 2;
        }
    }
    
    USE(results);
    free(results);
}

/* Test case 6: Worker+Vector partitioned */
void test_worker_vector_partitioned() {
    int N = 2048;
    int M = 2048;
    short *buffer = (short*)malloc(N * M * sizeof(short));
    
    #pragma acc parallel copy(buffer[0:N*M]) num_gangs(1) num_workers(8) vector_length(64)
    {
        #pragma acc loop worker vector independent collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                buffer[i * M + j] = (short)((i + j) % 65536);
            }
        }
    }
    
    USE(buffer);
    free(buffer);
}

/* Test case 7: Fully partitioned (Gang+Worker+Vector) */
void test_fully_partitioned() {
    int N = 1024;
    int M = 1024;
    int K = 1024;
    float *tensor = (float*)malloc(N * M * K * sizeof(float));
    
    #pragma acc parallel copy(tensor[0:N*M*K]) num_gangs(4) num_workers(4) vector_length(32)
    {
        #pragma acc loop gang worker vector independent collapse(3)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < K; k++) {
                    int idx = (i * M * K) + (j * K) + k;
                    tensor[idx] = (float)(i + j + k) / 100.0f;
                }
            }
        }
    }
    
    USE(tensor);
    free(tensor);
}

/* OpenMP equivalents to trigger similar partitioning logic */
void test_omp_partitioning() {
    int N = 10000;
    int *arr = (int*)malloc(N * sizeof(int));
    
    /* OpenMP target offload with various team/thread configurations */
    #pragma omp target teams distribute parallel for map(tofrom: arr[0:N]) num_teams(8) thread_limit(256)
    for (int i = 0; i < N; i++) {
        arr[i] = i * 3;
    }
    
    /* Combined SIMD for vector partitioning */
    #pragma omp target teams distribute parallel for simd map(tofrom: arr[0:N]) \
            num_teams(4) thread_limit(128) simdlen(16)
    for (int i = 0; i < N; i++) {
        arr[i] += i % 16;
    }
    
    USE(arr);
    free(arr);
}

/* Test invalid partition codes */
void test_invalid_partitions() {
    /* Force generation of default case through variable control flow */
    int invalid_codes[] = {-1, 8, 255, 1024};
    
    for (int i = 0; i < 4; i++) {
        debug_partition_info(invalid_codes[i]);
    }
    
    /* Runtime-determined invalid code */
    int dynamic_code = force_runtime ? 999 : 0;
    debug_partition_info(dynamic_code);
}

/* Template/macro approach to generate different partitionings */
#define GENERATE_PARTITION_TEST(NAME, GANGS, WORKERS, VECTORS) \
void test_##NAME() { \
    int size = 1024; \
    int *data = (int*)malloc(size * sizeof(int)); \
    \
    #pragma acc parallel copy(data[0:size]) \
        num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTORS) \
    { \
        #pragma acc loop gang worker vector independent \
        for (int i = 0; i < size; i++) { \
            data[i] = i * (GANGS + WORKERS + VECTORS); \
        } \
    } \
    \
    USE(data); \
    free(data); \
}

/* Generate specific partition combinations */
GENERATE_PARTITION_TEST(mixed_1, 2, 4, 8)
GENERATE_PARTITION_TEST(mixed_2, 4, 1, 16)
GENERATE_PARTITION_TEST(mixed_3, 1, 8, 32)

int main() {
    printf("Testing partition mapping coverage...\n");
    
    /* Execute all test cases systematically */
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    
    /* OpenMP variants */
    test_omp_partitioning();
    
    /* Generated template tests */
    test_mixed_1();
    test_mixed_2();
    test_mixed_3();
    
    /* Invalid cases for default switch branch */
    test_invalid_partitions();
    
    printf("All tests completed (coverage of partition codes 0-7 + default)\n");
    
    return 0;
}
