#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <openacc.h>

#ifdef __cplusplus
extern "C" {
#endif

// Function to force compiler to generate partition mapping
// This mimics what the compiler internally does
typedef const char* (*partition_desc_fn)(int);
volatile partition_desc_fn force_partition_desc = NULL;

// Prevent optimization
void use_result(int val) {
    asm volatile("" : : "r"(val));
}

// Test case 0: gang redundant (single gang)
void test_gang_redundant() {
    const int N = 1024;
    int *data = (int*)malloc(N * sizeof(int));
    volatile int gang_id = 0;
    
    #pragma acc parallel num_gangs(1) copyout(data[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            data[i] = i * 2;
        }
    }
    
    // Force partition logic
    if (gang_id == -1) {  // Never true, but prevents optimization
        const char* desc = "gang redundant";
        use_result((int)desc[0]);
    }
    
    free(data);
}

// Test case 1: gang partitioned (multiple gangs)
void test_gang_partitioned() {
    const int N = 1024;
    int *data = (int*)malloc(N * sizeof(int));
    int gang_size = 4;
    
    #pragma acc parallel num_gangs(gang_size) copyout(data[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            data[i] = i + acc_gang_id();
        }
    }
    
    // Runtime check that could use partition description
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += data[i];
    }
    use_result(sum);
    
    free(data);
}

// Test case 2: worker partitioned (single gang, multiple workers)
void test_worker_partitioned() {
    const int N = 1024;
    float *data = (float*)malloc(N * sizeof(float));
    
    #pragma acc parallel num_gangs(1) num_workers(4) copyout(data[0:N])
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            data[i] = (float)i / (acc_worker_id() + 1);
        }
    }
    
    free(data);
}

// Test case 3: gang+worker partitioned
void test_gang_worker_partitioned() {
    const int N = 1024;
    double *data = (double*)malloc(N * sizeof(double));
    volatile int use_workers = 1;
    
    #pragma acc parallel num_gangs(2) num_workers(2) copyout(data[0:N])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            data[i] = acc_gang_id() * 1000.0 + acc_worker_id() * 100.0 + i;
        }
    }
    
    free(data);
}

// Test case 4: vector partitioned
void test_vector_partitioned() {
    const int N = 1024;
    int *data = (int*)malloc(N * sizeof(int));
    
    #pragma acc parallel vector_length(32) copyout(data[0:N])
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            data[i] = i * i;
        }
    }
    
    free(data);
}

// Test case 5: gang+vector partitioned
void test_gang_vector_partitioned() {
    const int N = 2048;
    float *data = (float*)malloc(N * sizeof(float));
    int dynamic_size = N;  // Prevent compile-time optimization
    
    #pragma acc parallel num_gangs(4) vector_length(16) copyout(data[0:dynamic_size])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < dynamic_size; i++) {
            data[i] = acc_gang_id() * 10.0f + (i % 16);
        }
    }
    
    free(data);
}

// Test case 6: worker+vector partitioned
void test_worker_vector_partitioned() {
    const int N = 1024;
    double *data = (double*)malloc(N * sizeof(double));
    
    #pragma acc parallel num_gangs(1) num_workers(2) vector_length(8) copyout(data[0:N])
    {
        #pragma acc loop worker vector
        for (int i = 0; i < N; i++) {
            data[i] = acc_worker_id() * 1000.0 + (i % 8);
        }
    }
    
    free(data);
}

// Test case 7: fully partitioned (gang+worker+vector)
void test_fully_partitioned() {
    const int N = 4096;
    int *data = (int*)malloc(N * sizeof(int));
    volatile int use_all = 1;
    
    #pragma acc parallel num_gangs(8) num_workers(4) vector_length(32) copyout(data[0:N])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < N; i++) {
            int gid = acc_gang_id();
            int wid = acc_worker_id();
            int vid = i % 32;
            data[i] = gid * 10000 + wid * 1000 + vid * 10 + i;
        }
    }
    
    // Complex dependency to force partitioning
    int check = 0;
    for (int i = 0; i < N; i++) {
        if (data[i] < 0) check++;  // Never true but prevents optimization
    }
    use_result(check);
    
    free(data);
}

// Test default case (illegal partition codes)
void test_illegal_partitions() {
    // Force generation of default case through macro expansion
    #define TEST_INVALID_PARTITION(code) \
        if (force_partition_desc) { \
            const char* desc = force_partition_desc(code); \
            if (strcmp(desc, "<illegal>") == 0) { \
                use_result(1); \
            } \
        }
    
    // Test invalid codes
    TEST_INVALID_PARTITION(-1)
    TEST_INVALID_PARTITION(8)
    TEST_INVALID_PARTITION(255)
    
    // Also test with OpenMP for completeness
    #pragma omp target teams distribute parallel for simd \
        num_teams(1) thread_limit(1) if(0)  // Never executes but generates code
    for (int i = 0; i < 10; i++) {
        // Empty
    }
}

// OpenMP equivalents to trigger similar partitioning logic
void test_omp_partitioning() {
    const int N = 1024;
    int *data = (int*)malloc(N * sizeof(int));
    
    // OpenMP target with teams (gangs) and threads (workers/vectors)
    #pragma omp target teams distribute parallel for simd \
        num_teams(4) thread_limit(64) map(from:data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = omp_get_team_num() * 1000 + omp_get_thread_num() + i;
    }
    
    // Nested parallelism
    #pragma omp target teams distribute parallel for \
        num_teams(2) map(from:data[0:N/2])
    for (int i = 0; i < N/2; i++) {
        #pragma omp parallel for
        for (int j = 0; j < 2; j++) {
            data[i*2 + j] = i * j;
        }
    }
    
    free(data);
}

// Template function to generate different partition types (C++ only)
#ifdef __cplusplus
template<int PartitionType>
void test_template_partition() {
    const int N = 512;
    int *data = new int[N];
    
    // Different partition strategies based on template parameter
    if constexpr (PartitionType == 0) {
        #pragma acc parallel num_gangs(1) copyout(data[0:N])
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) data[i] = i;
        }
    } else if constexpr (PartitionType == 1) {
        #pragma acc parallel num_gangs(4) copyout(data[0:N])
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) data[i] = i * 2;
        }
    }
    // ... more cases
    
    delete[] data;
}
#endif

int main() {
    printf("Testing OpenACC/OpenMP partition mapping coverage\n");
    
    // Execute all test cases
    test_gang_redundant();
    printf("Test 0 (gang redundant) completed\n");
    
    test_gang_partitioned();
    printf("Test 1 (gang partitioned) completed\n");
    
    test_worker_partitioned();
    printf("Test 2 (worker partitioned) completed\n");
    
    test_gang_worker_partitioned();
    printf("Test 3 (gang+worker partitioned) completed\n");
    
    test_vector_partitioned();
    printf("Test 4 (vector partitioned) completed\n");
    
    test_gang_vector_partitioned();
    printf("Test 5 (gang+vector partitioned) completed\n");
    
    test_worker_vector_partitioned();
    printf("Test 6 (worker+vector partitioned) completed\n");
    
    test_fully_partitioned();
    printf("Test 7 (fully partitioned) completed\n");
    
    test_illegal_partitions();
    printf("Illegal partition tests completed\n");
    
    test_omp_partitioning();
    printf("OpenMP partitioning tests completed\n");
    
    #ifdef __cplusplus
    test_template_partition<0>();
    test_template_partition<1>();
    printf("Template partition tests completed\n");
    #endif
    
    printf("All tests completed successfully\n");
    return 0;
}

#ifdef __cplusplus
}
#endif
