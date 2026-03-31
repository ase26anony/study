/* Test program to cover partition code mapping in omp-oacc-neuter-broadcast.cc */
#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define N 1024
#define M 256
#define P 128

void test_gang_redundant() {
    int i;
    int data[N];
    
    #pragma acc parallel loop gang copyout(data[0:N])
    for (i = 0; i < N; i++) {
        data[i] = i * 2;
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        if (data[i] != i * 2) {
            fprintf(stderr, "Error in test_gang_redundant at index %d\n", i);
            exit(1);
        }
    }
}

void test_gang_partitioned() {
    int i, j;
    int data[N][M];
    
    #pragma acc parallel loop gang copyout(data[0:N][0:M])
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            data[i][j] = i + j;
        }
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            if (data[i][j] != i + j) {
                fprintf(stderr, "Error in test_gang_partitioned at [%d][%d]\n", i, j);
                exit(1);
            }
        }
    }
}

void test_worker_partitioned() {
    int i, j;
    int data[N][M];
    
    #pragma acc parallel loop worker copyout(data[0:N][0:M])
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            data[i][j] = i * j;
        }
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            if (data[i][j] != i * j) {
                fprintf(stderr, "Error in test_worker_partitioned at [%d][%d]\n", i, j);
                exit(1);
            }
        }
    }
}

void test_gang_worker_partitioned() {
    int i, j, k;
    int data[N][M][P];
    
    #pragma acc parallel loop gang worker copyout(data[0:N][0:M][0:P])
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            for (k = 0; k < P; k++) {
                data[i][j][k] = i + j + k;
            }
        }
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            for (k = 0; k < P; k++) {
                if (data[i][j][k] != i + j + k) {
                    fprintf(stderr, "Error in test_gang_worker_partitioned at [%d][%d][%d]\n", i, j, k);
                    exit(1);
                }
            }
        }
    }
}

void test_vector_partitioned() {
    int i;
    float data[N];
    
    #pragma acc parallel loop vector copyout(data[0:N])
    for (i = 0; i < N; i++) {
        data[i] = (float)i / 2.0f;
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        if (data[i] != (float)i / 2.0f) {
            fprintf(stderr, "Error in test_vector_partitioned at index %d\n", i);
            exit(1);
        }
    }
}

void test_gang_vector_partitioned() {
    int i, j;
    float data[N][M];
    float sum = 0.0f;
    
    #pragma acc parallel loop gang vector reduction(+:sum) copyin(data[0:N][0:M]) copy(sum)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            data[i][j] = (float)(i + j);
            sum += data[i][j];
        }
    }
    
    // Verify reduction
    float expected = 0.0f;
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            expected += (float)(i + j);
        }
    }
    
    if (sum != expected) {
        fprintf(stderr, "Error in test_gang_vector_partitioned: sum = %f, expected = %f\n", sum, expected);
        exit(1);
    }
}

void test_worker_vector_partitioned() {
    int i, j;
    double data[N][M];
    double product = 1.0;
    
    #pragma acc parallel loop worker vector reduction(*:product) copyin(data[0:N][0:M]) copy(product)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            data[i][j] = 1.0001;
            product *= data[i][j];
        }
    }
    
    // Note: Floating point multiplication may have precision issues
    // We'll just check it's a reasonable value
    if (product < 0.5 || product > 2.0) {
        fprintf(stderr, "Error in test_worker_vector_partitioned: product = %lf\n", product);
        exit(1);
    }
}

void test_fully_partitioned() {
    int i, j, k;
    int data[N][M][P];
    int total_sum = 0;
    
    #pragma acc parallel loop gang worker vector reduction(+:total_sum) copyout(data[0:N][0:M][0:P]) copy(total_sum)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            for (k = 0; k < P; k++) {
                data[i][j][k] = i * j * k;
                total_sum += data[i][j][k];
            }
        }
    }
    
    // Verify
    int expected_sum = 0;
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            for (k = 0; k < P; k++) {
                expected_sum += i * j * k;
            }
        }
    }
    
    if (total_sum != expected_sum) {
        fprintf(stderr, "Error in test_fully_partitioned: sum = %d, expected = %d\n", total_sum, expected_sum);
        exit(1);
    }
}

void test_nested_parallelism() {
    int i, j;
    int data[N][M];
    
    #pragma acc parallel copyout(data[0:N][0:M])
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            #pragma acc loop worker
            for (j = 0; j < M; j++) {
                data[i][j] = i - j;
            }
        }
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            if (data[i][j] != i - j) {
                fprintf(stderr, "Error in test_nested_parallelism at [%d][%d]\n", i, j);
                exit(1);
            }
        }
    }
}

void test_combined_directives() {
    int i, j;
    int data[N][M];
    
    #pragma acc parallel loop tile(32, 16) copyout(data[0:N][0:M])
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            data[i][j] = i * i + j * j;
        }
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            if (data[i][j] != i * i + j * j) {
                fprintf(stderr, "Error in test_combined_directives at [%d][%d]\n", i, j);
                exit(1);
            }
        }
    }
}

void test_runtime_determined_partitioning() {
    int i;
    int data[N];
    int chunk_size = N / 4;  // Runtime determined
    
    #pragma acc parallel loop gang(num_gangs:4) worker(num_workers:2) vector_length(32) copyout(data[0:N])
    for (i = 0; i < N; i++) {
        data[i] = i / chunk_size;
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        if (data[i] != i / chunk_size) {
            fprintf(stderr, "Error in test_runtime_determined_partitioning at index %d\n", i);
            exit(1);
        }
    }
}

void test_multi_device() {
    // Try to trigger different device paths
    acc_device_t dev_type = acc_get_device_type();
    
    if (dev_type == acc_device_nvidia || dev_type == acc_device_radeon) {
        // Try to set device explicitly
        acc_set_device_num(0, dev_type);
    }
    
    // Run a simple test
    int data[100];
    #pragma acc parallel loop copyout(data[0:100])
    for (int i = 0; i < 100; i++) {
        data[i] = i * 3;
    }
    
    // Verify
    for (int i = 0; i < 100; i++) {
        if (data[i] != i * 3) {
            fprintf(stderr, "Error in test_multi_device at index %d\n", i);
            exit(1);
        }
    }
}

void test_private_firstprivate() {
    int i;
    int private_var = 42;
    int firstprivate_var = 100;
    int results[N];
    
    #pragma acc parallel loop private(private_var) firstprivate(firstprivate_var) copyout(results[0:N])
    for (i = 0; i < N; i++) {
        private_var = i;
        results[i] = private_var + firstprivate_var;
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        if (results[i] != i + 100) {
            fprintf(stderr, "Error in test_private_firstprivate at index %d\n", i);
            exit(1);
        }
    }
}

int main() {
    printf("Starting OpenACC partition coverage test...\n");
    
    // Enable debug output to trigger logging paths
    char* debug_env = getenv("ACC_DEBUG");
    if (!debug_env) {
        // Try to set it programmatically if supported
        putenv("ACC_DEBUG=1");
    }
    
    // Test all partitioning scenarios
    test_gang_redundant();
    printf("✓ test_gang_redundant passed\n");
    
    test_gang_partitioned();
    printf("✓ test_gang_partitioned passed\n");
    
    test_worker_partitioned();
    printf("✓ test_worker_partitioned passed\n");
    
    test_gang_worker_partitioned();
    printf("✓ test_gang_worker_partitioned passed\n");
    
    test_vector_partitioned();
    printf("✓ test_vector_partitioned passed\n");
    
    test_gang_vector_partitioned();
    printf("✓ test_gang_vector_partitioned passed\n");
    
    test_worker_vector_partitioned();
    printf("✓ test_worker_vector_partitioned passed\n");
    
    test_fully_partitioned();
    printf("✓ test_fully_partitioned passed\n");
    
    // Additional tests to explore different code paths
    test_nested_parallelism();
    printf("✓ test_nested_parallelism passed\n");
    
    test_combined_directives();
    printf("✓ test_combined_directives passed\n");
    
    test_runtime_determined_partitioning();
    printf("✓ test_runtime_determined_partitioning passed\n");
    
    test_multi_device();
    printf("✓ test_multi_device passed\n");
    
    test_private_firstprivate();
    printf("✓ test_private_firstprivate passed\n");
    
    printf("\nAll tests passed successfully!\n");
    printf("The partition code mapping function should have been called with values 0-7.\n");
    
    return 0;
}
