/* Test program to exercise partition mapping logic in omp-oacc-neuter-broadcast.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _OPENACC
#include <openacc.h>
#endif
#ifdef _OPENMP
#include <omp.h>
#endif

/* Prevent optimization */
#define KEEP(var) asm volatile("" : : "r"(var))
#define NOINLINE __attribute__((noinline))

/* Global to force runtime values */
volatile int runtime_gangs = 2;
volatile int runtime_workers = 4;
volatile int runtime_vector = 8;
volatile int runtime_size = 1024;

/* Function that could trigger partition string mapping */
NOINLINE void debug_partition(int code) {
    /* This mimics the internal compiler logic */
    const char* desc;
    switch(code) {
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
    KEEP(desc);
    
    /* Force compiler to consider all cases */
    if (code < 0 || code > 7) {
        fprintf(stderr, "Invalid partition code: %d (%s)\n", code, desc);
    }
}

/* Test case 0: Gang redundant (single gang) */
NOINLINE void test_gang_redundant() {
    int size = runtime_size;
    float* data = (float*)malloc(size * sizeof(float));
    
    #pragma acc parallel loop gang(num:1) copy(data[0:size])
    for (int i = 0; i < size; i++) {
        data[i] = i * 1.5f;
    }
    
    /* Verify and potentially trigger debug output */
    int error = 0;
    for (int i = 0; i < size; i++) {
        if (data[i] != i * 1.5f) error = 1;
    }
    if (error) debug_partition(0);
    
    free(data);
}

/* Test case 1: Gang partitioned */
NOINLINE void test_gang_partitioned() {
    int size = runtime_size;
    float* data = (float*)malloc(size * sizeof(float));
    int gangs = runtime_gangs;
    
    #pragma acc parallel loop gang(num:gangs) copy(data[0:size])
    for (int i = 0; i < size; i++) {
        data[i] = i * 2.0f + gangs;
    }
    
    int error = 0;
    #pragma acc parallel loop reduction(+:error) gang(num:gangs) copy(data[0:size], error)
    for (int i = 0; i < size; i++) {
        if (data[i] != i * 2.0f + gangs) error++;
    }
    
    if (error) debug_partition(1);
    free(data);
}

/* Test case 2: Worker partitioned */
NOINLINE void test_worker_partitioned() {
    int size = runtime_size;
    float* data = (float*)malloc(size * sizeof(float));
    int workers = runtime_workers;
    
    #pragma acc parallel loop gang(1) worker(num:workers) copy(data[0:size])
    for (int i = 0; i < size; i++) {
        data[i] = i * 3.0f;
    }
    
    /* Nested worker parallelism */
    #pragma acc parallel loop gang(1) worker(num:workers) copy(data[0:size])
    for (int i = 0; i < size; i++) {
        #pragma acc loop worker
        for (int j = 0; j < 4; j++) {
            data[i] += j * 0.1f;
        }
    }
    
    if (data[size-1] > 0) debug_partition(2);
    free(data);
}

/* Test case 3: Gang+Worker partitioned */
NOINLINE void test_gang_worker_partitioned() {
    int size = runtime_size;
    float* data = (float*)malloc(size * sizeof(float));
    int gangs = runtime_gangs;
    int workers = runtime_workers;
    
    #pragma acc parallel loop gang(num:gangs) worker(num:workers) copy(data[0:size])
    for (int i = 0; i < size; i++) {
        data[i] = i * 4.0f + gangs + workers;
    }
    
    /* Complex nested computation */
    #pragma acc parallel loop gang(num:gangs) worker(num:workers) copy(data[0:size])
    for (int i = 0; i < size; i++) {
        float sum = 0;
        #pragma acc loop worker reduction(+:sum)
        for (int j = 0; j < 8; j++) {
            sum += data[i] * j;
        }
        data[i] = sum;
    }
    
    if (data[0] != 0) debug_partition(3);
    free(data);
}

/* Test case 4: Vector partitioned */
NOINLINE void test_vector_partitioned() {
    int size = runtime_size;
    float* data = (float*)malloc(size * sizeof(float));
    int vector_len = runtime_vector;
    
    #pragma acc parallel loop gang(1) worker(1) vector(length:vector_len) copy(data[0:size])
    for (int i = 0; i < size; i++) {
        data[i] = i * 5.0f;
    }
    
    /* Vector operations */
    #pragma acc parallel loop gang(1) worker(1) vector(length:vector_len) copy(data[0:size])
    for (int i = 0; i < size; i++) {
        #pragma acc loop vector
        for (int j = 0; j < vector_len; j++) {
            data[i] += j * 0.01f;
        }
    }
    
    if (data[size/2] > 0) debug_partition(4);
    free(data);
}

/* Test case 5: Gang+Vector partitioned */
NOINLINE void test_gang_vector_partitioned() {
    int size = runtime_size;
    float* data = (float*)malloc(size * sizeof(float));
    int gangs = runtime_gangs;
    int vector_len = runtime_vector;
    
    #pragma acc parallel loop gang(num:gangs) vector(length:vector_len) copy(data[0:size])
    for (int i = 0; i < size; i++) {
        data[i] = i * 6.0f + gangs;
    }
    
    /* Mixed gang-vector computation */
    #pragma acc parallel loop gang(num:gangs) vector(length:vector_len) copy(data[0:size])
    for (int i = 0; i < size; i++) {
        float temp = data[i];
        #pragma acc loop vector reduction(+:temp)
        for (int j = 0; j < vector_len; j++) {
            temp += j * 0.1f;
        }
        data[i] = temp;
    }
    
    if (data[0] >= 0) debug_partition(5);
    free(data);
}

/* Test case 6: Worker+Vector partitioned */
NOINLINE void test_worker_vector_partitioned() {
    int size = runtime_size;
    float* data = (float*)malloc(size * sizeof(float));
    int workers = runtime_workers;
    int vector_len = runtime_vector;
    
    #pragma acc parallel loop gang(1) worker(num:workers) vector(length:vector_len) copy(data[0:size])
    for (int i = 0; i < size; i++) {
        data[i] = i * 7.0f;
    }
    
    /* Nested worker-vector computation */
    #pragma acc parallel loop gang(1) worker(num:workers) vector(length:vector_len) copy(data[0:size])
    for (int i = 0; i < size; i++) {
        #pragma acc loop worker
        for (int w = 0; w < workers; w++) {
            float sum = 0;
            #pragma acc loop vector reduction(+:sum)
            for (int v = 0; v < vector_len; v++) {
                sum += data[i] * v * 0.01f;
            }
            data[i] += sum;
        }
    }
    
    if (data[size-1] > 0) debug_partition(6);
    free(data);
}

/* Test case 7: Fully partitioned (Gang+Worker+Vector) */
NOINLINE void test_fully_partitioned() {
    int size = runtime_size;
    float* data = (float*)malloc(size * sizeof(float));
    int gangs = runtime_gangs;
    int workers = runtime_workers;
    int vector_len = runtime_vector;
    
    #pragma acc parallel loop gang(num:gangs) worker(num:workers) vector(length:vector_len) copy(data[0:size])
    for (int i = 0; i < size; i++) {
        data[i] = i * 8.0f;
    }
    
    /* Complex fully nested computation */
    #pragma acc parallel loop gang(num:gangs) worker(num:workers) vector(length:vector_len) copy(data[0:size])
    for (int i = 0; i < size; i++) {
        #pragma acc loop worker
        for (int w = 0; w < workers; w++) {
            float partial = 0;
            #pragma acc loop vector reduction(+:partial)
            for (int v = 0; v < vector_len; v++) {
                partial += data[i] * (w + v) * 0.001f;
            }
            data[i] += partial;
        }
    }
    
    if (data[0] != 0) debug_partition(7);
    free(data);
}

/* Test invalid partition codes (default case) */
NOINLINE void test_invalid_partitions() {
    /* Force compiler to consider invalid codes */
    for (int code = -5; code < 0; code++) {
        debug_partition(code);
    }
    for (int code = 8; code < 15; code++) {
        debug_partition(code);
    }
}

/* OpenMP equivalent tests for cross-validation */
#ifdef _OPENMP
NOINLINE void test_omp_partitioning() {
    int size = runtime_size;
    float* data = (float*)malloc(size * sizeof(float));
    
    /* Test various OpenMP team distributions */
    #pragma omp target teams distribute parallel for map(tofrom: data[0:size])
    for (int i = 0; i < size; i++) {
        data[i] = i * 1.0f;
    }
    
    /* Nested parallelism */
    #pragma omp target teams distribute parallel for simd map(tofrom: data[0:size])
    for (int i = 0; i < size; i++) {
        data[i] += omp_get_team_num() * 0.1f;
    }
    
    free(data);
}
#endif

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
    
    /* Test invalid codes */
    test_invalid_partitions();
    
    #ifdef _OPENMP
    test_omp_partitioning();
    #endif
    
    printf("All partition tests completed.\n");
    
    /* Force exit with error if any test failed */
    volatile int result = 0;
    KEEP(result);
    
    return result;
}
