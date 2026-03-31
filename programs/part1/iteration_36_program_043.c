#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <openacc.h>

// Volatile variables to prevent optimization
volatile int force_runtime = 1;
volatile int partition_code = 0;

// Function that could trigger the string mapping logic
void debug_partition_info(int code) {
    // This mimics the internal compiler logic
    // The compiler might generate similar code when debugging is enabled
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
    
    // Use asm to prevent dead code elimination
    asm volatile("" : : "r"(desc));
    
    // Conditional error that might use the description
    if (code < 0 || code > 7) {
        fprintf(stderr, "Invalid partition code: %d (%s)\n", code, desc);
    }
}

// Test case 0: Gang redundant (single gang)
void test_gang_redundant() {
    const int N = 1000;
    int* data = (int*)malloc(N * sizeof(int));
    
    #pragma acc parallel copy(data[0:N]) num_gangs(1) num_workers(1) vector_length(1)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            data[i] = i * 2;
        }
    }
    
    // Verify results
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (data[i] != i * 2) errors++;
    }
    
    // Trigger potential debug output
    if (errors > 0) debug_partition_info(0);
    
    free(data);
}

// Test case 1: Gang partitioned
void test_gang_partitioned() {
    const int N = 10000;
    float* a = (float*)malloc(N * sizeof(float));
    float* b = (float*)malloc(N * sizeof(float));
    
    // Initialize with runtime values
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
    
    // Check results
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (b[i] != a[i] * 2.0f) errors++;
    }
    
    if (errors > 0) debug_partition_info(1);
    
    free(a);
    free(b);
}

// Test case 2: Worker partitioned
void test_worker_partitioned() {
    const int N = 5000;
    double* arr = (double*)malloc(N * sizeof(double));
    
    // Runtime-dependent initialization
    int seed = force_runtime ? 42 : 0;
    for (int i = 0; i < N; i++) {
        arr[i] = (double)((i + seed) % 50);
    }
    
    #pragma acc parallel copy(arr[0:N]) num_gangs(1) num_workers(8) vector_length(1)
    {
        #pragma acc loop worker independent
        for (int i = 0; i < N; i++) {
            arr[i] = arr[i] * 3.0 + 1.0;
        }
    }
    
    // Simple verification
    int errors = 0;
    for (int i = 0; i < 10; i++) {
        double expected = ((i + seed) % 50) * 3.0 + 1.0;
        if (arr[i] != expected) errors++;
    }
    
    if (errors > 0) debug_partition_info(2);
    
    free(arr);
}

// Test case 3: Gang+Worker partitioned
void test_gang_worker_partitioned() {
    const int N = 20000;
    int* matrix = (int*)malloc(N * sizeof(int));
    
    // Complex initialization to prevent optimization
    for (int i = 0; i < N; i++) {
        matrix[i] = (i * 17) % 256;
    }
    
    #pragma acc parallel copy(matrix[0:N]) num_gangs(16) num_workers(4) vector_length(1)
    {
        #pragma acc loop gang worker independent collapse(2)
        for (int i = 0; i < N/100; i++) {
            for (int j = 0; j < 100; j++) {
                int idx = i * 100 + j;
                matrix[idx] = matrix[idx] * 2 + (i % 16);
            }
        }
    }
    
    // Partial verification
    int errors = 0;
    for (int i = 0; i < 100; i++) {
        int expected = ((i * 17) % 256) * 2 + (0 % 16);
        if (matrix[i] != expected) errors++;
    }
    
    if (errors > 0) debug_partition_info(3);
    
    free(matrix);
}

// Test case 4: Vector partitioned
void test_vector_partitioned() {
    const int N = 8192;
    float* data = (float*)malloc(N * sizeof(float));
    
    for (int i = 0; i < N; i++) {
        data[i] = (float)i;
    }
    
    #pragma acc parallel copy(data[0:N]) num_gangs(1) num_workers(1) vector_length(128)
    {
        #pragma acc loop vector independent
        for (int i = 0; i < N; i++) {
            data[i] = sqrtf(data[i] * 2.0f);
        }
    }
    
    int errors = 0;
    for (int i = 0; i < 10; i++) {
        float expected = sqrtf((float)i * 2.0f);
        if (fabs(data[i] - expected) > 0.001f) errors++;
    }
    
    if (errors > 0) debug_partition_info(4);
    
    free(data);
}

// Test case 5: Gang+Vector partitioned
void test_gang_vector_partitioned() {
    const int N = 16384;
    double* arr1 = (double*)malloc(N * sizeof(double));
    double* arr2 = (double*)malloc(N * sizeof(double));
    
    for (int i = 0; i < N; i++) {
        arr1[i] = (double)(i % 1000);
        arr2[i] = 0.0;
    }
    
    #pragma acc parallel copyin(arr1[0:N]) copyout(arr2[0:N]) \
                num_gangs(32) num_workers(1) vector_length(64)
    {
        #pragma acc loop gang vector independent
        for (int i = 0; i < N; i++) {
            arr2[i] = arr1[i] * arr1[i] + 1.0;
        }
    }
    
    int errors = 0;
    for (int i = 0; i < 100; i++) {
        double expected = (double)(i % 1000) * (double)(i % 1000) + 1.0;
        if (fabs(arr2[i] - expected) > 0.0001) errors++;
    }
    
    if (errors > 0) debug_partition_info(5);
    
    free(arr1);
    free(arr2);
}

// Test case 6: Worker+Vector partitioned
void test_worker_vector_partitioned() {
    const int N = 12288;
    int* data = (int*)malloc(N * sizeof(int));
    
    for (int i = 0; i < N; i++) {
        data[i] = i;
    }
    
    #pragma acc parallel copy(data[0:N]) num_gangs(1) num_workers(6) vector_length(32)
    {
        #pragma acc loop worker vector independent
        for (int i = 0; i < N; i++) {
            data[i] = data[i] * 3 - 2;
        }
    }
    
    int errors = 0;
    for (int i = 0; i < 50; i++) {
        if (data[i] != i * 3 - 2) errors++;
    }
    
    if (errors > 0) debug_partition_info(6);
    
    free(data);
}

// Test case 7: Fully partitioned (Gang+Worker+Vector)
void test_fully_partitioned() {
    const int N = 32768;
    float* a = (float*)malloc(N * sizeof(float));
    float* b = (float*)malloc(N * sizeof(float));
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)(i % 500);
        b[i] = 0.0f;
    }
    
    #pragma acc parallel copyin(a[0:N]) copyout(b[0:N]) \
                num_gangs(64) num_workers(2) vector_length(16)
    {
        #pragma acc loop gang worker vector independent
        for (int i = 0; i < N; i++) {
            b[i] = a[i] * 0.5f + (float)(i % 10);
        }
    }
    
    int errors = 0;
    for (int i = 0; i < 100; i++) {
        float expected = (float)(i % 500) * 0.5f + (float)(i % 10);
        if (fabs(b[i] - expected) > 0.001f) errors++;
    }
    
    if (errors > 0) debug_partition_info(7);
    
    free(a);
    free(b);
}

// Test invalid partition codes (default case)
void test_invalid_partitions() {
    // Force generation of invalid codes through boundary conditions
    int invalid_codes[] = {-1, 8, 255, 1000};
    
    for (int i = 0; i < 4; i++) {
        partition_code = invalid_codes[i];
        debug_partition_info(partition_code);
    }
    
    // Also test with OpenMP to trigger different code paths
    #pragma omp target teams distribute parallel for map(tofrom: invalid_codes[0:4])
    for (int i = 0; i < 4; i++) {
        invalid_codes[i] = invalid_codes[i] * 2;
    }
}

// OpenMP version to trigger alternative code paths
void test_omp_partitioning() {
    const int N = 10000;
    int* data = (int*)malloc(N * sizeof(int));
    
    // OpenMP target with various team configurations
    #pragma omp target teams distribute parallel for \
                num_teams(32) thread_limit(64) map(tofrom: data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = omp_get_team_num() * 1000 + omp_get_thread_num();
    }
    
    // Nested parallelism for complex partitioning
    #pragma omp target teams distribute parallel for simd \
                num_teams(16) thread_limit(128) simdlen(8) \
                map(tofrom: data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] += i;
    }
    
    free(data);
}

int main() {
    printf("Testing OpenACC/OpenMP partition coverage...\n");
    
    // Test all valid partition cases
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    
    // Test invalid cases
    test_invalid_partitions();
    
    // Test OpenMP variants
    test_omp_partitioning();
    
    printf("All tests completed.\n");
    
    // Force runtime evaluation of partition codes
    if (force_runtime) {
        for (int code = -2; code <= 10; code++) {
            debug_partition_info(code);
        }
    }
    
    return 0;
}
