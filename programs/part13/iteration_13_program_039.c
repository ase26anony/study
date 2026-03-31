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
    
    printf("Testing gang redundant...\n");
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
    
    printf("Testing gang partitioned...\n");
    #pragma acc parallel loop gang copy(data[0:N]) num_gangs(4)
    for (i = 0; i < N; i++) {
        data[i] += 1;
    }
}

void test_worker_partitioned() {
    int data[N];
    int i;
    
    printf("Testing worker partitioned...\n");
    #pragma acc parallel loop worker copy(data[0:N]) num_workers(4)
    for (i = 0; i < N; i++) {
        data[i] = data[i] * 2;
    }
}

void test_gang_worker_partitioned() {
    int data[N][M];
    int i, j;
    
    printf("Testing gang+worker partitioned...\n");
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
    
    printf("Testing vector partitioned...\n");
    #pragma acc parallel loop vector copy(data[0:N]) vector_length(32)
    for (i = 0; i < N; i++) {
        data[i] = (float)i / 2.0f;
    }
}

void test_gang_vector_partitioned() {
    double data[N][M];
    int i, j;
    
    printf("Testing gang+vector partitioned...\n");
    #pragma acc parallel loop gang vector collapse(2) copy(data[0:N][0:M]) \
        num_gangs(4) vector_length(16)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            data[i][j] = (double)(i * j) / 100.0;
        }
    }
}

void test_worker_vector_partitioned() {
    int data[N][M];
    int i, j;
    
    printf("Testing worker+vector partitioned...\n");
    #pragma acc parallel loop worker vector collapse(2) copy(data[0:N][0:M]) \
        num_workers(4) vector_length(8)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            data[i][j] = (i + j) % 256;
        }
    }
}

void test_fully_partitioned() {
    int data[N][M][P];
    int i, j, k;
    int sum = 0;
    
    printf("Testing fully partitioned...\n");
    #pragma acc parallel loop gang worker vector collapse(3) \
        copy(data[0:N][0:M][0:P]) reduction(+:sum) \
        num_gangs(2) num_workers(2) vector_length(4)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            for (k = 0; k < P; k++) {
                data[i][j][k] = 1;
                sum += 1;
            }
        }
    }
    
    printf("Reduction sum: %d (expected: %d)\n", sum, N * M * P);
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
            data[i] *= 2;
        }
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
            data[i] = i * 4;
        }
    }
}

void test_varied_access_patterns() {
    int src[N], dst[N], indices[N];
    int i;
    
    printf("Testing varied access patterns...\n");
    
    // Initialize on host
    for (i = 0; i < N; i++) {
        src[i] = i;
        indices[i] = (i * 7) % N;  // Non-sequential access pattern
    }
    
    #pragma acc parallel loop gang worker copyin(src[0:N], indices[0:N]) \
        copyout(dst[0:N]) num_gangs(4) num_workers(2)
    for (i = 0; i < N; i++) {
        dst[i] = src[indices[i]];
    }
}

void test_async_operations() {
    int data[N];
    int i;
    acc_handle_t async_handle;
    
    printf("Testing async operations...\n");
    
    #pragma acc parallel loop gang async copy(data[0:N]) num_gangs(2)
    for (i = 0; i < N; i++) {
        data[i] = i;
    }
    
    #pragma acc wait
}

void test_device_management() {
    int data[N];
    int i;
    
    printf("Testing device management...\n");
    
    // Try to set device (may affect partitioning decisions)
    acc_set_device_num(0, acc_device_default);
    
    #pragma acc parallel loop gang worker copy(data[0:N]) \
        device_type(acc_device_default) num_gangs(2) num_workers(2)
    for (i = 0; i < N; i++) {
        data[i] = i * 5;
    }
}

void test_multi_dimensional_partitioning() {
    int matrix[N][M];
    int i, j;
    
    printf("Testing multi-dimensional partitioning...\n");
    
    // Different partitioning for different dimensions
    #pragma acc parallel loop gang tile(32, 4) copy(matrix[0:N][0:M])
    for (i = 0; i < N; i++) {
        #pragma acc loop worker
        for (j = 0; j < M; j++) {
            matrix[i][j] = i * M + j;
        }
    }
}

void test_reduction_variants() {
    int data[N];
    int sum = 0, max = 0, min = 0;
    int i;
    
    printf("Testing reduction variants...\n");
    
    #pragma acc parallel loop gang reduction(+:sum) reduction(max:max) \
        reduction(min:min) copy(data[0:N]) copyout(sum, max, min)
    for (i = 0; i < N; i++) {
        data[i] = i - N/2;
        sum += data[i];
        if (data[i] > max) max = data[i];
        if (data[i] < min) min = data[i];
    }
    
    printf("Reductions: sum=%d, max=%d, min=%d\n", sum, max, min);
}

void test_firstprivate_private() {
    int base = 42;
    int result[N];
    int i;
    
    printf("Testing firstprivate/private...\n");
    
    #pragma acc parallel loop gang worker firstprivate(base) \
        private(i) copyout(result[0:N])
    for (i = 0; i < N; i++) {
        int local = base + i;
        result[i] = local;
    }
}

int main() {
    printf("Starting partition code coverage test...\n");
    
    // Enable debug output to trigger mapping function calls
    putenv("ACC_DEBUG=1");
    putenv("LIBGOMP_DEBUG=1");
    
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
    test_conditional_partitioning();
    test_varied_access_patterns();
    test_async_operations();
    test_device_management();
    test_multi_dimensional_partitioning();
    test_reduction_variants();
    test_firstprivate_private();
    
    printf("All tests completed successfully!\n");
    
    // Note: To potentially trigger the default case (<illegal>),
    // we would need to cause an internal error or use an invalid
    // partition code. This is harder to do from user code but
    // might occur with corrupted runtime state or invalid
    // compiler-generated code.
    
    return 0;
}
