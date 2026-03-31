/* Test program to cover partition code mapping in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O0 -fopenacc -foffload=disable -o test_partition test_partition.c
 * Run with: ACC_DEBUG=1 ./test_partition
 */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define N 1024
#define M 128
#define P 64

void test_gang_redundant() {
    printf("Testing gang redundant (case 0)...\n");
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
    printf("Testing gang partitioned (case 1)...\n");
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
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_worker_partitioned() {
    printf("Testing worker partitioned (case 2)...\n");
    int data[M];
    
    #pragma acc parallel loop worker copyout(data[0:M]) num_workers(4)
    for (int i = 0; i < M; i++) {
        data[i] = i * 3;
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < M; i++) {
        if (data[i] != i * 3) errors++;
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_gang_worker_partitioned() {
    printf("Testing gang+worker partitioned (case 3)...\n");
    int data[N][M];
    
    #pragma acc parallel loop gang worker collapse(2) copyout(data[0:N][0:M]) \
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
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_vector_partitioned() {
    printf("Testing vector partitioned (case 4)...\n");
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
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_gang_vector_partitioned() {
    printf("Testing gang+vector partitioned (case 5)...\n");
    int data[N];
    
    #pragma acc parallel loop gang vector copyout(data[0:N]) \
        num_gangs(4) vector_length(32)
    for (int i = 0; i < N; i++) {
        data[i] = i * 4;
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (data[i] != i * 4) errors++;
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_worker_vector_partitioned() {
    printf("Testing worker+vector partitioned (case 6)...\n");
    float data[M];
    
    #pragma acc parallel loop worker vector copyout(data[0:M]) \
        num_workers(4) vector_length(16)
    for (int i = 0; i < M; i++) {
        data[i] = i * 2.5f;
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < M; i++) {
        if (data[i] != i * 2.5f) errors++;
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_fully_partitioned() {
    printf("Testing fully partitioned (case 7)...\n");
    int data[N][M];
    int sum = 0;
    
    #pragma acc parallel loop gang worker vector collapse(2) \
        copyout(data[0:N][0:M]) reduction(+:sum) \
        num_gangs(2) num_workers(2) vector_length(32)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            data[i][j] = i + j;
            sum += data[i][j];
        }
    }
    
    // Verify
    int errors = 0;
    int expected_sum = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            if (data[i][j] != i + j) errors++;
            expected_sum += i + j;
        }
    }
    if (errors > 0) printf("  Data errors: %d\n", errors);
    if (sum != expected_sum) printf("  Reduction error: got %d, expected %d\n", sum, expected_sum);
}

void test_nested_parallelism() {
    printf("Testing nested parallelism...\n");
    int data[N];
    
    #pragma acc parallel copyout(data[0:N]) num_gangs(2)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            data[i] = i * 2;
        }
        
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            data[i] += 1;
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (data[i] != i * 2 + 1) errors++;
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_conditional_partitioning() {
    printf("Testing conditional partitioning...\n");
    int data[N];
    int use_worker = 1;  // Runtime value
    
    if (use_worker) {
        #pragma acc parallel loop worker copyout(data[0:N]) num_workers(4)
        for (int i = 0; i < N; i++) {
            data[i] = i * 3;
        }
    } else {
        #pragma acc parallel loop gang copyout(data[0:N]) num_gangs(4)
        for (int i = 0; i < N; i++) {
            data[i] = i * 2;
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (data[i] != i * 3) errors++;
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_multi_device() {
    printf("Testing multi-device scenarios...\n");
    
    // Try to set different device types
    acc_device_t dev_type = acc_get_device_type();
    printf("  Current device type: %d\n", dev_type);
    
    // Test with async
    int data[N];
    
    #pragma acc parallel loop async(1) copyout(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = i;
    }
    
    #pragma acc wait(1)
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (data[i] != i) errors++;
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_invalid_partition() {
    printf("Testing potential invalid partition (default case)...\n");
    
    // Try to create a situation that might trigger invalid partition codes
    // by using unusual data mappings and device operations
    
    int *d_ptr = NULL;
    int data[N];
    
    // Allocate device memory
    d_ptr = (int *)acc_malloc(N * sizeof(int));
    
    if (d_ptr) {
        // Use deviceptr with manual copy
        #pragma acc parallel loop deviceptr(d_ptr)
        for (int i = 0; i < N; i++) {
            d_ptr[i] = i;
        }
        
        // Copy back
        #pragma acc update host(d_ptr[0:N])
        
        // Verify
        int errors = 0;
        for (int i = 0; i < N; i++) {
            if (d_ptr[i] != i) errors++;
        }
        if (errors > 0) printf("  Errors: %d\n", errors);
        
        acc_free(d_ptr);
    }
}

int main() {
    printf("Starting partition code coverage test...\n");
    
    // Enable debug output to trigger string mapping calls
    // This should be done via environment variable: ACC_DEBUG=1
    
    // Test all partition cases
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    
    // Additional tests for varied execution paths
    test_nested_parallelism();
    test_conditional_partitioning();
    test_multi_device();
    test_invalid_partition();
    
    printf("All tests completed.\n");
    
    return 0;
}
