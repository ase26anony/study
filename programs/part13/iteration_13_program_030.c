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
    printf("Testing gang redundant pattern...\n");
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
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_gang_partitioned() {
    printf("Testing gang partitioned pattern...\n");
    int A[N], B[N];
    
    for (int i = 0; i < N; i++) {
        A[i] = i;
        B[i] = 0;
    }
    
    #pragma acc parallel loop gang copyin(A[0:N]) copyout(B[0:N])
    for (int i = 0; i < N; i++) {
        B[i] = A[i] * 3;
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (B[i] != i * 3) errors++;
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_worker_partitioned() {
    printf("Testing worker partitioned pattern...\n");
    float data[M];
    
    #pragma acc parallel loop worker copyout(data[0:M])
    for (int i = 0; i < M; i++) {
        data[i] = i * 1.5f;
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < M; i++) {
        if (data[i] != i * 1.5f) errors++;
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_gang_worker_partitioned() {
    printf("Testing gang+worker partitioned pattern...\n");
    int matrix[N][M];
    
    #pragma acc parallel loop gang worker collapse(2) copyout(matrix[0:N][0:M])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            matrix[i][j] = i * 1000 + j;
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            if (matrix[i][j] != i * 1000 + j) errors++;
        }
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_vector_partitioned() {
    printf("Testing vector partitioned pattern...\n");
    double vec[P];
    
    #pragma acc parallel loop vector copyout(vec[0:P])
    for (int i = 0; i < P; i++) {
        vec[i] = i * 2.5;
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < P; i++) {
        if (vec[i] != i * 2.5) errors++;
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_gang_vector_partitioned() {
    printf("Testing gang+vector partitioned pattern...\n");
    int data[N];
    
    #pragma acc parallel loop gang vector copyout(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = i * i;
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (data[i] != i * i) errors++;
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_worker_vector_partitioned() {
    printf("Testing worker+vector partitioned pattern...\n");
    float data[M];
    
    #pragma acc parallel loop worker vector copyout(data[0:M])
    for (int i = 0; i < M; i++) {
        data[i] = i / 2.0f;
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < M; i++) {
        if (data[i] != i / 2.0f) errors++;
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_fully_partitioned() {
    printf("Testing fully partitioned pattern...\n");
    int result = 0;
    int data[N];
    
    // Initialize data
    for (int i = 0; i < N; i++) {
        data[i] = i + 1;
    }
    
    #pragma acc parallel loop gang worker vector reduction(+:result) copyin(data[0:N])
    for (int i = 0; i < N; i++) {
        result += data[i];
    }
    
    // Verify (sum of 1..N)
    int expected = N * (N + 1) / 2;
    if (result != expected) {
        printf("  Error: expected %d, got %d\n", expected, result);
    }
}

void test_nested_parallelism() {
    printf("Testing nested parallelism...\n");
    int outer[N], inner[M];
    
    #pragma acc parallel copyout(outer[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            outer[i] = i * 10;
            
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                // This nested structure may trigger complex partitioning
                if (j == 0) {
                    outer[i] += 1;
                }
            }
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (outer[i] != i * 10 + 1) errors++;
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_combined_directives() {
    printf("Testing combined directives...\n");
    int tile_data[N][M];
    
    #pragma acc parallel loop tile(32, 16) copyout(tile_data[0:N][0:M])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            tile_data[i][j] = (i << 16) | j;
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            if (tile_data[i][j] != ((i << 16) | j)) errors++;
        }
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_runtime_dependent_partitioning() {
    printf("Testing runtime-dependent partitioning...\n");
    int data[N];
    int chunk = N / 4;
    
    // Dynamic partitioning based on runtime value
    #pragma acc parallel loop gang(num_gangs:4) copyout(data[0:N])
    for (int i = 0; i < N; i++) {
        int gang_id = i / chunk;
        data[i] = gang_id * 1000 + i;
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        int gang_id = i / chunk;
        if (data[i] != gang_id * 1000 + i) errors++;
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_multi_device_context() {
    printf("Testing multi-device context...\n");
    
    // Try to create context on different device types
    acc_device_t dev_type = acc_get_device_type();
    printf("  Current device type: %d\n", dev_type);
    
    // Switch device (if multiple available)
    int num_devices = acc_get_num_devices(dev_type);
    printf("  Number of devices: %d\n", num_devices);
    
    if (num_devices > 1) {
        acc_set_device_num(1, dev_type);
        printf("  Switched to device 1\n");
    }
    
    // Execute a kernel on potentially different device
    int test_data[10];
    #pragma acc parallel loop copyout(test_data[0:10])
    for (int i = 0; i < 10; i++) {
        test_data[i] = i * 100;
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < 10; i++) {
        if (test_data[i] != i * 100) errors++;
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_async_operations() {
    printf("Testing async operations...\n");
    int data[N];
    int async_id = 1;
    
    #pragma acc parallel loop async(async_id) copyout(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = i * 7;
    }
    
    #pragma acc wait(async_id)
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (data[i] != i * 7) errors++;
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_complex_data_structures() {
    printf("Testing complex data structures...\n");
    
    typedef struct {
        int x[N];
        float y[M];
        double z[P];
    } ComplexData;
    
    ComplexData cd;
    
    #pragma acc parallel loop gang copyout(cd.x[0:N])
    for (int i = 0; i < N; i++) {
        cd.x[i] = i * 11;
    }
    
    #pragma acc parallel loop worker copyout(cd.y[0:M])
    for (int i = 0; i < M; i++) {
        cd.y[i] = i * 3.14f;
    }
    
    #pragma acc parallel loop vector copyout(cd.z[0:P])
    for (int i = 0; i < P; i++) {
        cd.z[i] = i * 2.71828;
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (cd.x[i] != i * 11) errors++;
    }
    for (int i = 0; i < M; i++) {
        if (cd.y[i] != i * 3.14f) errors++;
    }
    for (int i = 0; i < P; i++) {
        if (cd.z[i] != i * 2.71828) errors++;
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

int main() {
    printf("Starting OpenACC partition coverage test...\n");
    
    // Enable debug output to trigger partition string mapping
    // This is done via environment variable: ACC_DEBUG=1
    
    // Test all partition patterns
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
    test_runtime_dependent_partitioning();
    test_multi_device_context();
    test_async_operations();
    test_complex_data_structures();
    
    printf("\nAll tests completed.\n");
    printf("Note: To see partition debug output, run with: ACC_DEBUG=1 ./test_partition\n");
    
    return 0;
}
