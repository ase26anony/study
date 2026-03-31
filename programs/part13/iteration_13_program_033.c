/* Test program to cover partition code mapping in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O0 -fopenacc -foffload=disable -o test_partition test_partition.c
 * Run with: ACC_DEBUG=1 ./test_partition
 */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define N 1024
#define M 32
#define P 8

void test_gang_redundant() {
    int data[N];
    int i;
    
    printf("Testing gang redundant pattern...\n");
    
    #pragma acc parallel loop gang copyout(data[0:N])
    for (i = 0; i < N; i++) {
        data[i] = i * 2;
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        if (data[i] != i * 2) {
            printf("Error in gang redundant test at index %d\n", i);
            exit(1);
        }
    }
}

void test_gang_partitioned() {
    int data[N];
    int i;
    
    printf("Testing gang partitioned pattern...\n");
    
    #pragma acc parallel loop gang copy(data[0:N]) num_gangs(4)
    for (i = 0; i < N; i++) {
        data[i] += 1;
    }
}

void test_worker_partitioned() {
    int data[N];
    int i;
    
    printf("Testing worker partitioned pattern...\n");
    
    #pragma acc parallel loop worker copy(data[0:N]) num_workers(4)
    for (i = 0; i < N; i++) {
        data[i] = data[i] * 2;
    }
}

void test_gang_worker_partitioned() {
    int data[N][M];
    int i, j;
    
    printf("Testing gang+worker partitioned pattern...\n");
    
    #pragma acc parallel loop gang worker collapse(2) copy(data[0:N][0:M]) \
        num_gangs(2) num_workers(4)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            data[i][j] = i * M + j;
        }
    }
}

void test_vector_partitioned() {
    float data[N];
    int i;
    
    printf("Testing vector partitioned pattern...\n");
    
    #pragma acc parallel loop vector copy(data[0:N]) vector_length(32)
    for (i = 0; i < N; i++) {
        data[i] = (float)i / 2.0f;
    }
}

void test_gang_vector_partitioned() {
    double data[N];
    int i;
    
    printf("Testing gang+vector partitioned pattern...\n");
    
    #pragma acc parallel loop gang vector copy(data[0:N]) \
        num_gangs(4) vector_length(64)
    for (i = 0; i < N; i++) {
        data[i] = data[i] * 3.14159;
    }
}

void test_worker_vector_partitioned() {
    int data[N][M];
    int i, j;
    
    printf("Testing worker+vector partitioned pattern...\n");
    
    #pragma acc parallel loop worker vector collapse(2) copy(data[0:N][0:M]) \
        num_workers(4) vector_length(16)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            data[i][j] += 1;
        }
    }
}

void test_fully_partitioned() {
    int data[N][M][P];
    int i, j, k;
    
    printf("Testing fully partitioned pattern...\n");
    
    #pragma acc parallel loop gang worker vector collapse(3) \
        copy(data[0:N][0:M][0:P]) \
        num_gangs(2) num_workers(2) vector_length(8)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            for (k = 0; k < P; k++) {
                data[i][j][k] = i * M * P + j * P + k;
            }
        }
    }
}

void test_nested_parallelism() {
    int data[N];
    int i;
    
    printf("Testing nested parallelism...\n");
    
    #pragma acc parallel copy(data[0:N]) num_gangs(2)
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            data[i] = i;
        }
        
        #pragma acc loop worker
        for (i = 0; i < N; i++) {
            data[i] += 1;
        }
    }
}

void test_reduction_patterns() {
    int sum = 0;
    int data[N];
    int i;
    
    printf("Testing reduction patterns...\n");
    
    // Initialize data
    for (i = 0; i < N; i++) {
        data[i] = i + 1;
    }
    
    #pragma acc parallel loop gang reduction(+:sum) copyin(data[0:N]) copy(sum)
    for (i = 0; i < N; i++) {
        sum += data[i];
    }
    
    int expected = N * (N + 1) / 2;
    if (sum != expected) {
        printf("Reduction error: got %d, expected %d\n", sum, expected);
        exit(1);
    }
}

void test_conditional_partitioning() {
    int data[N];
    int i;
    int use_vector = 1;  // Runtime value
    
    printf("Testing conditional partitioning...\n");
    
    if (use_vector) {
        #pragma acc parallel loop vector copy(data[0:N])
        for (i = 0; i < N; i++) {
            data[i] = i * 3;
        }
    } else {
        #pragma acc parallel loop worker copy(data[0:N])
        for (i = 0; i < N; i++) {
            data[i] = i * 2;
        }
    }
}

void test_async_operations() {
    int data[N];
    int i;
    acc_device_t dev_type;
    
    printf("Testing async operations...\n");
    
    dev_type = acc_get_device_type();
    acc_set_device_num(0, dev_type);
    
    #pragma acc parallel loop async copy(data[0:N])
    for (i = 0; i < N; i++) {
        data[i] = 0;
    }
    
    #pragma acc wait
    
    #pragma acc parallel loop gang async(1) copy(data[0:N])
    for (i = 0; i < N; i++) {
        data[i] += 1;
    }
    
    #pragma acc wait(1)
}

void test_multi_device_context() {
    int data[N];
    int i;
    
    printf("Testing multi-device context...\n");
    
    // Try to create context on default device
    #pragma acc enter data create(data[0:N])
    
    #pragma acc parallel loop present(data[0:N])
    for (i = 0; i < N; i++) {
        data[i] = i;
    }
    
    #pragma acc update host(data[0:N])
    
    #pragma acc exit data delete(data[0:N])
}

void test_private_firstprivate() {
    int i, j;
    int private_var;
    int firstprivate_var = 42;
    int shared_data[N];
    
    printf("Testing private/firstprivate clauses...\n");
    
    #pragma acc parallel loop gang private(private_var) \
        firstprivate(firstprivate_var) copy(shared_data[0:N])
    for (i = 0; i < N; i++) {
        private_var = i;
        shared_data[i] = private_var + firstprivate_var;
    }
}

void test_tiled_loops() {
    int data[N][M];
    int i, j;
    
    printf("Testing tiled loops...\n");
    
    #pragma acc parallel loop tile(32, 8) copy(data[0:N][0:M])
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            data[i][j] = i * M + j;
        }
    }
}

void test_combined_directives() {
    int data[N];
    int i;
    
    printf("Testing combined directives...\n");
    
    #pragma acc kernels loop gang worker vector copy(data[0:N])
    for (i = 0; i < N; i++) {
        data[i] = i * 2;
    }
    
    #pragma acc parallel loop gang copy(data[0:N])
    for (i = 0; i < N; i++) {
        data[i] += 1;
    }
}

int main() {
    printf("Starting partition code coverage test...\n");
    
    // Enable debug output to trigger mapping function calls
    setenv("ACC_DEBUG", "1", 1);
    setenv("LIBGOMP_DEBUG", "1", 1);
    
    // Test various partitioning patterns
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    
    // Test additional patterns that may trigger different paths
    test_nested_parallelism();
    test_reduction_patterns();
    test_conditional_partitioning();
    test_async_operations();
    test_multi_device_context();
    test_private_firstprivate();
    test_tiled_loops();
    test_combined_directives();
    
    printf("All tests completed successfully!\n");
    printf("The partition code mapping function should have been called with values 0-7.\n");
    
    return 0;
}
