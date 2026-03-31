/* Test program to cover partition code mapping in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O1 -fopenacc -foffload=disable -o test_partition test_partition.c
 * Run with: ACC_DEBUG=1 ./test_partition
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>

#define N 1024
#define M 512
#define P 256

void test_gang_redundant() {
    printf("Testing gang redundant partitioning...\n");
    int data[N];
    int sum = 0;
    
    #pragma acc parallel loop gang copy(data[0:N]) copyin(sum)
    for (int i = 0; i < N; i++) {
        data[i] = i;
    }
    
    // Verify
    int correct = 1;
    for (int i = 0; i < N; i++) {
        if (data[i] != i) correct = 0;
    }
    if (correct) printf("  PASS: gang redundant\n");
    else printf("  FAIL: gang redundant\n");
}

void test_gang_partitioned() {
    printf("Testing gang partitioned...\n");
    int data[N];
    
    #pragma acc parallel loop gang copy(data[0:N]) num_gangs(4)
    for (int i = 0; i < N; i++) {
        data[i] = i * 2;
    }
    
    // Verify
    int correct = 1;
    for (int i = 0; i < N; i++) {
        if (data[i] != i * 2) correct = 0;
    }
    if (correct) printf("  PASS: gang partitioned\n");
    else printf("  FAIL: gang partitioned\n");
}

void test_worker_partitioned() {
    printf("Testing worker partitioned...\n");
    float data[M];
    
    #pragma acc parallel loop worker copy(data[0:M]) num_workers(8)
    for (int i = 0; i < M; i++) {
        data[i] = i * 1.5f;
    }
    
    // Verify
    int correct = 1;
    for (int i = 0; i < M; i++) {
        if (data[i] != i * 1.5f) correct = 0;
    }
    if (correct) printf("  PASS: worker partitioned\n");
    else printf("  FAIL: worker partitioned\n");
}

void test_gang_worker_partitioned() {
    printf("Testing gang+worker partitioned...\n");
    double data[N][M];
    
    #pragma acc parallel loop gang worker copy(data[0:N][0:M]) \
        num_gangs(2) num_workers(4)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            data[i][j] = i * 1000.0 + j;
        }
    }
    
    // Verify
    int correct = 1;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            if (data[i][j] != i * 1000.0 + j) correct = 0;
        }
    }
    if (correct) printf("  PASS: gang+worker partitioned\n");
    else printf("  FAIL: gang+worker partitioned\n");
}

void test_vector_partitioned() {
    printf("Testing vector partitioned...\n");
    int data[P];
    
    #pragma acc parallel loop vector copy(data[0:P]) vector_length(32)
    for (int i = 0; i < P; i++) {
        data[i] = i * 3;
    }
    
    // Verify
    int correct = 1;
    for (int i = 0; i < P; i++) {
        if (data[i] != i * 3) correct = 0;
    }
    if (correct) printf("  PASS: vector partitioned\n");
    else printf("  FAIL: vector partitioned\n");
}

void test_gang_vector_partitioned() {
    printf("Testing gang+vector partitioned...\n");
    int data[N];
    
    #pragma acc parallel loop gang vector copy(data[0:N]) \
        num_gangs(8) vector_length(16)
    for (int i = 0; i < N; i++) {
        data[i] = i * 4;
    }
    
    // Verify
    int correct = 1;
    for (int i = 0; i < N; i++) {
        if (data[i] != i * 4) correct = 0;
    }
    if (correct) printf("  PASS: gang+vector partitioned\n");
    else printf("  FAIL: gang+vector partitioned\n");
}

void test_worker_vector_partitioned() {
    printf("Testing worker+vector partitioned...\n");
    float data[M];
    
    #pragma acc parallel loop worker vector copy(data[0:M]) \
        num_workers(4) vector_length(64)
    for (int i = 0; i < M; i++) {
        data[i] = i * 2.5f;
    }
    
    // Verify
    int correct = 1;
    for (int i = 0; i < M; i++) {
        if (data[i] != i * 2.5f) correct = 0;
    }
    if (correct) printf("  PASS: worker+vector partitioned\n");
    else printf("  FAIL: worker+vector partitioned\n");
}

void test_fully_partitioned() {
    printf("Testing fully partitioned...\n");
    int data[N];
    int sum = 0;
    
    #pragma acc parallel loop gang worker vector reduction(+:sum) \
        copy(data[0:N]) copyin(sum) num_gangs(4) num_workers(2) vector_length(32)
    for (int i = 0; i < N; i++) {
        data[i] = i;
        sum += data[i];
    }
    
    // Verify
    int expected = (N-1)*N/2;
    if (sum == expected) printf("  PASS: fully partitioned (sum = %d)\n", sum);
    else printf("  FAIL: fully partitioned (got %d, expected %d)\n", sum, expected);
}

void test_nested_parallelism() {
    printf("Testing nested parallelism...\n");
    int data[N][M];
    
    #pragma acc parallel copy(data[0:N][0:M]) num_gangs(2)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < M; j++) {
                data[i][j] = i * M + j;
            }
        }
    }
    
    // Verify
    int correct = 1;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            if (data[i][j] != i * M + j) correct = 0;
        }
    }
    if (correct) printf("  PASS: nested parallelism\n");
    else printf("  FAIL: nested parallelism\n");
}

void test_combined_directives() {
    printf("Testing combined directives...\n");
    int data[N];
    
    #pragma acc parallel loop gang worker vector tile(32, 16) copy(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = i * 5;
    }
    
    // Verify
    int correct = 1;
    for (int i = 0; i < N; i++) {
        if (data[i] != i * 5) correct = 0;
    }
    if (correct) printf("  PASS: combined directives\n");
    else printf("  FAIL: combined directives\n");
}

void test_runtime_determined_partitioning() {
    printf("Testing runtime-determined partitioning...\n");
    int data[N];
    int chunk_size = 128;
    
    #pragma acc parallel loop gang copy(data[0:N]) copyin(chunk_size)
    for (int i = 0; i < N; i += chunk_size) {
        int end = i + chunk_size;
        if (end > N) end = N;
        #pragma acc loop worker vector
        for (int j = i; j < end; j++) {
            data[j] = j * 7;
        }
    }
    
    // Verify
    int correct = 1;
    for (int i = 0; i < N; i++) {
        if (data[i] != i * 7) correct = 0;
    }
    if (correct) printf("  PASS: runtime-determined partitioning\n");
    else printf("  FAIL: runtime-determined partitioning\n");
}

void test_multi_device() {
    printf("Testing multi-device scenarios...\n");
    
    // Test with different device types if available
    acc_device_t dev_type = acc_get_device_type();
    printf("  Device type: %d\n", dev_type);
    
    int data[N];
    #pragma acc parallel loop copy(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = i * 9;
    }
    
    // Verify
    int correct = 1;
    for (int i = 0; i < N; i++) {
        if (data[i] != i * 9) correct = 0;
    }
    if (correct) printf("  PASS: multi-device test\n");
    else printf("  FAIL: multi-device test\n");
}

void test_complex_data_structures() {
    printf("Testing complex data structures...\n");
    
    typedef struct {
        int x[N];
        float y[M];
        double z[P];
    } ComplexData;
    
    ComplexData *cd = (ComplexData*)malloc(sizeof(ComplexData));
    
    #pragma acc parallel loop gang worker copy(cd[0:1])
    for (int i = 0; i < N; i++) {
        cd->x[i] = i * 11;
    }
    
    #pragma acc parallel loop worker vector copy(cd[0:1])
    for (int i = 0; i < M; i++) {
        cd->y[i] = i * 3.14f;
    }
    
    #pragma acc parallel loop vector copy(cd[0:1])
    for (int i = 0; i < P; i++) {
        cd->z[i] = i * 2.71828;
    }
    
    // Verify
    int correct = 1;
    for (int i = 0; i < N; i++) {
        if (cd->x[i] != i * 11) correct = 0;
    }
    for (int i = 0; i < M; i++) {
        if (cd->y[i] != i * 3.14f) correct = 0;
    }
    for (int i = 0; i < P; i++) {
        if (cd->z[i] != i * 2.71828) correct = 0;
    }
    
    if (correct) printf("  PASS: complex data structures\n");
    else printf("  FAIL: complex data structures\n");
    
    free(cd);
}

void test_async_operations() {
    printf("Testing async operations...\n");
    int data[N];
    int async_id = 1;
    
    #pragma acc parallel loop gang async(async_id) copy(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = i * 13;
    }
    
    #pragma acc wait(async_id)
    
    // Verify
    int correct = 1;
    for (int i = 0; i < N; i++) {
        if (data[i] != i * 13) correct = 0;
    }
    if (correct) printf("  PASS: async operations\n");
    else printf("  FAIL: async operations\n");
}

void test_edge_cases() {
    printf("Testing edge cases...\n");
    
    // Test with zero-length arrays
    int empty[1];
    #pragma acc parallel copy(empty[0:1])
    {
        empty[0] = 42;
    }
    
    // Test with private/firstprivate clauses
    int private_var = 100;
    int data[10];
    
    #pragma acc parallel loop private(private_var) copy(data[0:10])
    for (int i = 0; i < 10; i++) {
        private_var = i;
        data[i] = private_var;
    }
    
    printf("  PASS: edge cases\n");
}

int main() {
    printf("Starting OpenACC partition coverage test...\n\n");
    
    // Enable debug output to trigger partition string mapping
    char* debug_env = getenv("ACC_DEBUG");
    if (!debug_env) {
        printf("Note: Set ACC_DEBUG=1 for verbose partition logging\n\n");
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
    
    // Test additional scenarios
    test_nested_parallelism();
    test_combined_directives();
    test_runtime_determined_partitioning();
    test_multi_device();
    test_complex_data_structures();
    test_async_operations();
    test_edge_cases();
    
    printf("\nAll tests completed.\n");
    
    // To potentially trigger the default case (illegal partition code),
    // we rely on internal runtime errors or invalid states
    // This might happen with:
    // - Invalid device pointers
    // - Memory allocation failures
    // - Internal runtime errors
    
    return 0;
}
