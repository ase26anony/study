/* Test program to cover partition code mapping in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O0 -fopenacc -foffload=disable -o test_partition test_partition.c
 * Run with: ACC_DEBUG=1 ./test_partition
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>

#define N 1024
#define M 128
#define P 64

void test_gang_redundant() {
    printf("Testing gang redundant (case 0)...\n");
    int scalar = 42;
    int arr[N];
    
    #pragma acc parallel copyout(arr[0:N]) copy(scalar) num_gangs(4)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            arr[i] = scalar + i;
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (arr[i] != 42 + i) errors++;
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_gang_partitioned() {
    printf("Testing gang partitioned (case 1)...\n");
    int arr[N];
    
    #pragma acc parallel copyout(arr[0:N]) num_gangs(8)
    {
        #pragma acc loop gang independent
        for (int i = 0; i < N; i++) {
            arr[i] = i * 2;
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (arr[i] != i * 2) errors++;
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_worker_partitioned() {
    printf("Testing worker partitioned (case 2)...\n");
    int arr[N];
    
    #pragma acc parallel copyout(arr[0:N]) num_workers(4) vector_length(32)
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            arr[i] = i + 1000;
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (arr[i] != i + 1000) errors++;
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_gang_worker_partitioned() {
    printf("Testing gang+worker partitioned (case 3)...\n");
    int matrix[M][P];
    
    #pragma acc parallel copyout(matrix[0:M][0:P]) num_gangs(4) num_workers(2)
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker
            for (int j = 0; j < P; j++) {
                matrix[i][j] = i * P + j;
            }
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            if (matrix[i][j] != i * P + j) errors++;
        }
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_vector_partitioned() {
    printf("Testing vector partitioned (case 4)...\n");
    float arr[N];
    
    #pragma acc parallel copyout(arr[0:N]) vector_length(64)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            arr[i] = (float)i / 2.0f;
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (arr[i] != (float)i / 2.0f) errors++;
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_gang_vector_partitioned() {
    printf("Testing gang+vector partitioned (case 5)...\n");
    int arr[N];
    
    #pragma acc parallel copyout(arr[0:N]) num_gangs(4) vector_length(32)
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            arr[i] = i * 3;
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (arr[i] != i * 3) errors++;
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_worker_vector_partitioned() {
    printf("Testing worker+vector partitioned (case 6)...\n");
    float arr[N];
    
    #pragma acc parallel copyout(arr[0:N]) num_workers(2) vector_length(64)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < N; i++) {
            arr[i] = (float)i * 1.5f;
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (arr[i] != (float)i * 1.5f) errors++;
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_fully_partitioned() {
    printf("Testing fully partitioned (case 7)...\n");
    int matrix[M][P];
    int sum = 0;
    
    #pragma acc parallel copyout(matrix[0:M][0:P]) reduction(+:sum) \
                num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang worker vector collapse(2)
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < P; j++) {
                matrix[i][j] = i + j;
                sum += matrix[i][j];
            }
        }
    }
    
    // Verify
    int errors = 0;
    int expected_sum = 0;
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            if (matrix[i][j] != i + j) errors++;
            expected_sum += i + j;
        }
    }
    if (sum != expected_sum) errors++;
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_nested_parallelism() {
    printf("Testing nested parallelism for varied partitioning...\n");
    int arr[N];
    
    #pragma acc parallel copyout(arr[0:N]) num_gangs(2)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            int temp = i;
            #pragma acc parallel loop worker vector reduction(+:temp) num_workers(2) vector_length(16)
            for (int j = 0; j < 10; j++) {
                temp += j;
            }
            arr[i] = temp;
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        int expected = i + 45; // sum of 0..9
        if (arr[i] != expected) errors++;
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_combined_directives() {
    printf("Testing combined directives...\n");
    int arr[N];
    
    #pragma acc kernels copyout(arr[0:N])
    {
        #pragma acc loop gang worker vector tile(32, 4)
        for (int i = 0; i < N; i++) {
            arr[i] = i * i;
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (arr[i] != i * i) errors++;
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_runtime_determined() {
    printf("Testing runtime-determined partitioning...\n");
    int arr[N];
    int use_vector = 1;  // Runtime value
    
    if (use_vector) {
        #pragma acc parallel copyout(arr[0:N]) vector_length(32)
        {
            #pragma acc loop vector
            for (int i = 0; i < N; i++) {
                arr[i] = i + 500;
            }
        }
    } else {
        #pragma acc parallel copyout(arr[0:N]) num_gangs(4)
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                arr[i] = i + 1000;
            }
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (arr[i] != i + 500) errors++;
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_multi_device() {
    printf("Testing multi-device scenarios...\n");
    
    // Try to set different device types
    acc_device_t dev_type = acc_get_device_type();
    printf("  Current device type: %d\n", dev_type);
    
    // Test with async
    int arr[N];
    
    #pragma acc parallel copyout(arr[0:N]) async(1) num_gangs(4) num_workers(2)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            arr[i] = i * 10;
        }
    }
    
    #pragma acc wait(1)
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (arr[i] != i * 10) errors++;
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_data_clause_variations() {
    printf("Testing data clause variations...\n");
    int a[N], b[N], c[N];
    
    // Initialize on host
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = i * 2;
    }
    
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc parallel num_gangs(8) vector_length(32)
        {
            #pragma acc loop gang vector
            for (int i = 0; i < N; i++) {
                c[i] = a[i] + b[i];
            }
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (c[i] != a[i] + b[i]) errors++;
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_complex_access_patterns() {
    printf("Testing complex access patterns...\n");
    int src[N], indices[N], dest[N];
    
    // Create index mapping
    for (int i = 0; i < N; i++) {
        src[i] = i * 3;
        indices[i] = (i * 7) % N;  // Scatter pattern
    }
    
    #pragma acc data copyin(src[0:N], indices[0:N]) copyout(dest[0:N])
    {
        #pragma acc parallel num_gangs(4) num_workers(4) vector_length(16)
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < N; i++) {
                dest[i] = src[indices[i]];
            }
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (dest[i] != src[indices[i]]) errors++;
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

int main() {
    printf("Starting partition code coverage test...\n");
    
    // Enable debug output to trigger logging paths
    char* debug_env = getenv("ACC_DEBUG");
    if (!debug_env) {
        printf("Set ACC_DEBUG=1 for detailed partition logging\n");
    }
    
    // Test all partition cases
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    
    // Additional tests to explore different paths
    test_nested_parallelism();
    test_combined_directives();
    test_runtime_determined();
    test_multi_device();
    test_data_clause_variations();
    test_complex_access_patterns();
    
    printf("\nAll tests completed.\n");
    printf("Note: To see partition code logging, compile with debug flags and run with ACC_DEBUG=1\n");
    
    return 0;
}
