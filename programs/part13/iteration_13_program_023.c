/* Test program to cover partition code mapping in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O0 -fopenacc -foffload=disable -o test_partition test_partition.c
 * Run with: ACC_DEBUG=1 ./test_partition
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>

#define N 1024
#define M 256
#define P 128

void test_gang_redundant() {
    printf("Testing gang redundant partitioning...\n");
    int data[N];
    
    #pragma acc parallel copyout(data) num_gangs(4)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            data[i] = i * 2;
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (data[i] != i * 2) errors++;
    }
    printf("  Gang redundant: %s\n", errors == 0 ? "PASS" : "FAIL");
}

void test_gang_partitioned() {
    printf("Testing gang partitioned...\n");
    int data[N];
    int sum = 0;
    
    #pragma acc parallel copy(data) copy(sum) reduction(+:sum) num_gangs(8)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            data[i] = i;
            sum += i;
        }
    }
    
    int expected = (N-1)*N/2;
    printf("  Gang partitioned: %s (sum=%d, expected=%d)\n", 
           sum == expected ? "PASS" : "FAIL", sum, expected);
}

void test_worker_partitioned() {
    printf("Testing worker partitioned...\n");
    int data[N];
    
    #pragma acc parallel copyout(data) num_workers(4) vector_length(32)
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            data[i] = i * 3;
        }
    }
    
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (data[i] != i * 3) errors++;
    }
    printf("  Worker partitioned: %s\n", errors == 0 ? "PASS" : "FAIL");
}

void test_gang_worker_partitioned() {
    printf("Testing gang+worker partitioned...\n");
    int data[N][M];
    
    #pragma acc parallel copyout(data) num_gangs(4) num_workers(4)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                data[i][j] = i * M + j;
            }
        }
    }
    
    int errors = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            if (data[i][j] != i * M + j) errors++;
        }
    }
    printf("  Gang+worker partitioned: %s\n", errors == 0 ? "PASS" : "FAIL");
}

void test_vector_partitioned() {
    printf("Testing vector partitioned...\n");
    float data[N];
    
    #pragma acc parallel copyout(data) vector_length(64)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            data[i] = i * 1.5f;
        }
    }
    
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (data[i] != i * 1.5f) errors++;
    }
    printf("  Vector partitioned: %s\n", errors == 0 ? "PASS" : "FAIL");
}

void test_gang_vector_partitioned() {
    printf("Testing gang+vector partitioned...\n");
    int data[N];
    
    #pragma acc parallel copyout(data) num_gangs(4) vector_length(32)
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            data[i] = i * 4;
        }
    }
    
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (data[i] != i * 4) errors++;
    }
    printf("  Gang+vector partitioned: %s\n", errors == 0 ? "PASS" : "FAIL");
}

void test_worker_vector_partitioned() {
    printf("Testing worker+vector partitioned...\n");
    float data[N][M];
    
    #pragma acc parallel copyout(data) num_workers(4) vector_length(32)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                data[i][j] = (i + j) * 1.1f;
            }
        }
    }
    
    int errors = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            if (data[i][j] != (i + j) * 1.1f) errors++;
        }
    }
    printf("  Worker+vector partitioned: %s\n", errors == 0 ? "PASS" : "FAIL");
}

void test_fully_partitioned() {
    printf("Testing fully partitioned...\n");
    int data[N][M][P];
    long long total_sum = 0;
    
    #pragma acc parallel copyout(data) copy(total_sum) \
                num_gangs(4) num_workers(4) vector_length(32) \
                reduction(+:total_sum)
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    int val = i * M * P + j * P + k;
                    data[i][j][k] = val;
                    total_sum += val;
                }
            }
        }
    }
    
    // Verify a few random positions
    int errors = 0;
    for (int sample = 0; sample < 100; sample++) {
        int i = rand() % N;
        int j = rand() % M;
        int k = rand() % P;
        int expected = i * M * P + j * P + k;
        if (data[i][j][k] != expected) errors++;
    }
    
    long long expected_sum = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                expected_sum += i * M * P + j * P + k;
            }
        }
    }
    
    printf("  Fully partitioned: %s (sum=%lld, expected=%lld)\n",
           errors == 0 && total_sum == expected_sum ? "PASS" : "FAIL",
           total_sum, expected_sum);
}

void test_nested_parallelism() {
    printf("Testing nested parallelism...\n");
    int data[N];
    
    #pragma acc parallel copyout(data) num_gangs(2)
    {
        // Outer gang level
        #pragma acc loop gang
        for (int i = 0; i < N/2; i++) {
            // Inner worker+vector level
            #pragma acc loop worker vector
            for (int j = 0; j < 2; j++) {
                int idx = i * 2 + j;
                if (idx < N) {
                    data[idx] = idx * 5;
                }
            }
        }
    }
    
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (data[i] != i * 5) errors++;
    }
    printf("  Nested parallelism: %s\n", errors == 0 ? "PASS" : "FAIL");
}

void test_combined_directives() {
    printf("Testing combined directives...\n");
    int data[N][M];
    
    // Using kernels directive with different loop mappings
    #pragma acc kernels copyout(data)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < M; j++) {
                data[i][j] = i * 1000 + j;
            }
        }
    }
    
    int errors = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            if (data[i][j] != i * 1000 + j) errors++;
        }
    }
    printf("  Combined directives: %s\n", errors == 0 ? "PASS" : "FAIL");
}

void test_runtime_determined_partitioning() {
    printf("Testing runtime-determined partitioning...\n");
    int data[N];
    int chunk_size = N / 4;
    
    // Use runtime variable to determine loop bounds
    #pragma acc parallel copyout(data) num_gangs(4)
    {
        #pragma acc loop gang
        for (int gang = 0; gang < 4; gang++) {
            int start = gang * chunk_size;
            int end = (gang == 3) ? N : (gang + 1) * chunk_size;
            
            #pragma acc loop worker vector
            for (int i = start; i < end; i++) {
                data[i] = i * 7;
            }
        }
    }
    
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (data[i] != i * 7) errors++;
    }
    printf("  Runtime-determined: %s\n", errors == 0 ? "PASS" : "FAIL");
}

void test_mixed_data_clauses() {
    printf("Testing mixed data clauses...\n");
    int src[N], dst[N];
    
    for (int i = 0; i < N; i++) {
        src[i] = i;
        dst[i] = 0;
    }
    
    // Use different data clauses to trigger different broadcast patterns
    #pragma acc data copyin(src) copyout(dst)
    {
        #pragma acc parallel num_gangs(4) num_workers(2) vector_length(16)
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < N; i++) {
                dst[i] = src[i] * 2;
            }
        }
    }
    
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (dst[i] != i * 2) errors++;
    }
    printf("  Mixed data clauses: %s\n", errors == 0 ? "PASS" : "FAIL");
}

void test_async_operations() {
    printf("Testing async operations...\n");
    int data[N];
    acc_device_t dev_type = acc_get_device_type();
    
    // Use async to potentially trigger different code paths
    #pragma acc parallel copyout(data) async(1) num_gangs(4)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            data[i] = i * 9;
        }
    }
    
    #pragma acc wait(1)
    
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (data[i] != i * 9) errors++;
    }
    printf("  Async operations: %s\n", errors == 0 ? "PASS" : "FAIL");
}

int main() {
    printf("Starting OpenACC partition coverage test...\n");
    
    // Enable debug output to increase likelihood of calling the mapping function
    char* debug_env = getenv("ACC_DEBUG");
    if (!debug_env) {
        printf("Note: Set ACC_DEBUG=1 for verbose output that may trigger more coverage\n");
    }
    
    // Test each partitioning type
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    
    // Test more complex scenarios
    test_nested_parallelism();
    test_combined_directives();
    test_runtime_determined_partitioning();
    test_mixed_data_clauses();
    test_async_operations();
    
    printf("\nAll tests completed.\n");
    
    // Try to trigger potential error/illegal case
    // Note: This might not directly hit the default case in the mapping function,
    // but could trigger other error paths
    printf("\nAttempting to trigger error conditions...\n");
    
    // Invalid device number (might trigger error handling)
    int original_device = acc_get_device_num(acc_get_device_type());
    acc_set_device_num(999, acc_get_device_type());  // Invalid device number
    acc_set_device_num(original_device, acc_get_device_type());  // Restore
    
    printf("Test program finished.\n");
    return 0;
}
