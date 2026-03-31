/* Test program to cover partition code mapping in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O1 -fopenacc -foffload=disable -o test_partition test_partition.c
 * Run with: ACC_DEBUG=1 ./test_partition
 */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define N 1024
#define M 256
#define P 128

void test_gang_redundant() {
    printf("Testing gang redundant partitioning...\n");
    int data[N];
    
    #pragma acc parallel loop gang copyout(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = i * 2;
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (data[i] != i * 2) errors++;
    }
    if (errors) printf("  Errors: %d\n", errors);
}

void test_gang_partitioned() {
    printf("Testing gang partitioned...\n");
    int data[N];
    
    #pragma acc parallel loop gang copyout(data[0:N]) num_gangs(4)
    for (int i = 0; i < N; i++) {
        data[i] = i + 1;
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (data[i] != i + 1) errors++;
    }
    if (errors) printf("  Errors: %d\n", errors);
}

void test_worker_partitioned() {
    printf("Testing worker partitioned...\n");
    int data[M];
    
    #pragma acc parallel loop worker copyout(data[0:M]) num_workers(2)
    for (int i = 0; i < M; i++) {
        data[i] = i * 3;
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < M; i++) {
        if (data[i] != i * 3) errors++;
    }
    if (errors) printf("  Errors: %d\n", errors);
}

void test_gang_worker_partitioned() {
    printf("Testing gang+worker partitioned...\n");
    int data[N][M];
    
    #pragma acc parallel loop gang worker copyout(data[0:N][0:M]) \
        num_gangs(2) num_workers(4)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            data[i][j] = i * M + j;
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            if (data[i][j] != i * M + j) errors++;
        }
    }
    if (errors) printf("  Errors: %d\n", errors);
}

void test_vector_partitioned() {
    printf("Testing vector partitioned...\n");
    float data[P];
    
    #pragma acc parallel loop vector copyout(data[0:P]) vector_length(32)
    for (int i = 0; i < P; i++) {
        data[i] = i * 1.5f;
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < P; i++) {
        if (data[i] != i * 1.5f) errors++;
    }
    if (errors) printf("  Errors: %d\n", errors);
}

void test_gang_vector_partitioned() {
    printf("Testing gang+vector partitioned...\n");
    int data[N];
    
    #pragma acc parallel loop gang vector copyout(data[0:N]) \
        num_gangs(4) vector_length(16)
    for (int i = 0; i < N; i++) {
        data[i] = i * 4;
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (data[i] != i * 4) errors++;
    }
    if (errors) printf("  Errors: %d\n", errors);
}

void test_worker_vector_partitioned() {
    printf("Testing worker+vector partitioned...\n");
    float data[M];
    
    #pragma acc parallel loop worker vector copyout(data[0:M]) \
        num_workers(2) vector_length(8)
    for (int i = 0; i < M; i++) {
        data[i] = i * 2.5f;
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < M; i++) {
        if (data[i] != i * 2.5f) errors++;
    }
    if (errors) printf("  Errors: %d\n", errors);
}

void test_fully_partitioned() {
    printf("Testing fully partitioned...\n");
    int data[N][M];
    int sum = 0;
    
    #pragma acc parallel loop gang worker vector reduction(+:sum) \
        copyout(data[0:N][0:M]) num_gangs(2) num_workers(4) vector_length(16)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            data[i][j] = i + j;
            sum += data[i][j];
        }
    }
    
    // Verify
    int expected_sum = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            expected_sum += i + j;
        }
    }
    
    if (sum != expected_sum) {
        printf("  Reduction error: got %d, expected %d\n", sum, expected_sum);
    }
}

void test_nested_parallelism() {
    printf("Testing nested parallelism...\n");
    int data[N];
    
    #pragma acc parallel copyout(data[0:N]) num_gangs(2)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            data[i] = i;
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (data[i] != i) errors++;
    }
    if (errors) printf("  Errors: %d\n", errors);
}

void test_combined_directives() {
    printf("Testing combined directives...\n");
    int data[N][M];
    
    #pragma acc kernels copyout(data[0:N][0:M])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                data[i][j] = i * j;
            }
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            if (data[i][j] != i * j) errors++;
        }
    }
    if (errors) printf("  Errors: %d\n", errors);
}

void test_runtime_partitioning() {
    printf("Testing runtime-dependent partitioning...\n");
    int data[N];
    int chunk = N / 4;
    
    // Runtime-dependent loop bounds
    #pragma acc parallel loop gang copyout(data[0:N]) num_gangs(4)
    for (int i = chunk; i < 2 * chunk; i++) {
        data[i] = i * 2;
    }
    
    // Verify
    int errors = 0;
    for (int i = chunk; i < 2 * chunk; i++) {
        if (data[i] != i * 2) errors++;
    }
    if (errors) printf("  Errors: %d\n", errors);
}

void test_multi_device() {
    printf("Testing multi-device scenarios...\n");
    
    // Try to set different device types
    acc_device_t dev_type = acc_get_device_type();
    printf("  Current device type: %d\n", dev_type);
    
    // Create data on device
    int *d_data = (int*)acc_malloc(N * sizeof(int));
    
    if (d_data) {
        #pragma acc parallel loop present(d_data[0:N]) num_gangs(2)
        for (int i = 0; i < N; i++) {
            d_data[i] = i;
        }
        
        acc_free(d_data);
    }
}

void test_varied_access_patterns() {
    printf("Testing varied access patterns...\n");
    int src[N], dest[N];
    int indices[N];
    
    // Initialize
    for (int i = 0; i < N; i++) {
        src[i] = i;
        indices[i] = (i * 7) % N;  // Strided access pattern
    }
    
    #pragma acc parallel loop gang vector copyin(src[0:N], indices[0:N]) \
        copyout(dest[0:N]) num_gangs(4) vector_length(8)
    for (int i = 0; i < N; i++) {
        dest[i] = src[indices[i]] * 2;
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (dest[i] != src[indices[i]] * 2) errors++;
    }
    if (errors) printf("  Errors: %d\n", errors);
}

void test_private_firstprivate() {
    printf("Testing private/firstprivate clauses...\n");
    int data[N];
    int private_var = 42;
    
    #pragma acc parallel loop gang private(private_var) copyout(data[0:N]) \
        num_gangs(4)
    for (int i = 0; i < N; i++) {
        private_var = i;
        data[i] = private_var + 1;
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (data[i] != i + 1) errors++;
    }
    if (errors) printf("  Errors: %d\n", errors);
}

int main() {
    printf("Starting OpenACC partition coverage test...\n\n");
    
    // Enable debug output to trigger partition string mapping
    char *debug_env = getenv("ACC_DEBUG");
    if (!debug_env) {
        printf("Note: Set ACC_DEBUG=1 for verbose output\n");
    }
    
    // Test all partition types
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    
    // Additional tests to explore different code paths
    test_nested_parallelism();
    test_combined_directives();
    test_runtime_partitioning();
    test_multi_device();
    test_varied_access_patterns();
    test_private_firstprivate();
    
    printf("\nAll tests completed.\n");
    
    // Try to trigger potential error/illegal case
    // by using invalid device operations
    printf("\nTesting error conditions...\n");
    int *invalid_ptr = NULL;
    
    // This might trigger error handling paths
    #pragma acc parallel present(invalid_ptr[0:1])
    {
        // Empty - should fail if invalid_ptr is not present
    }
    
    return 0;
}
