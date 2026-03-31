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
    printf("Testing gang redundant (code 0)...\n");
    int scalar = 42;
    int arr[N];
    
    #pragma acc parallel copyout(arr[0:N]) copy(scalar)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            arr[i] = scalar + i;  // scalar is gang-redundant
        }
    }
    
    // Verify
    for (int i = 0; i < N; i++) {
        if (arr[i] != 42 + i) {
            printf("Error in gang_redundant at index %d: %d != %d\n", 
                   i, arr[i], 42 + i);
            exit(1);
        }
    }
}

void test_gang_partitioned() {
    printf("Testing gang partitioned (code 1)...\n");
    int arr[N];
    
    #pragma acc parallel loop gang copyout(arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] = i * 2;
    }
    
    // Verify
    for (int i = 0; i < N; i++) {
        if (arr[i] != i * 2) {
            printf("Error in gang_partitioned at index %d\n", i);
            exit(1);
        }
    }
}

void test_worker_partitioned() {
    printf("Testing worker partitioned (code 2)...\n");
    int arr[N];
    
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
                copyout(arr[0:N])
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            arr[i] = i * 3;
        }
    }
    
    // Verify
    for (int i = 0; i < N; i++) {
        if (arr[i] != i * 3) {
            printf("Error in worker_partitioned at index %d\n", i);
            exit(1);
        }
    }
}

void test_gang_worker_partitioned() {
    printf("Testing gang+worker partitioned (code 3)...\n");
    int arr[N][M];
    
    #pragma acc parallel num_gangs(4) num_workers(4) vector_length(1) \
                copyout(arr[0:N][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                arr[i][j] = i * M + j;
            }
        }
    }
    
    // Verify
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            if (arr[i][j] != i * M + j) {
                printf("Error in gang_worker_partitioned at [%d][%d]\n", i, j);
                exit(1);
            }
        }
    }
}

void test_vector_partitioned() {
    printf("Testing vector partitioned (code 4)...\n");
    int arr[N];
    
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(32) \
                copyout(arr[0:N])
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            arr[i] = i * 4;
        }
    }
    
    // Verify
    for (int i = 0; i < N; i++) {
        if (arr[i] != i * 4) {
            printf("Error in vector_partitioned at index %d\n", i);
            exit(1);
        }
    }
}

void test_gang_vector_partitioned() {
    printf("Testing gang+vector partitioned (code 5)...\n");
    int arr[N][P];
    
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(32) \
                copyout(arr[0:N][0:P])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < P; j++) {
                arr[i][j] = i * P + j;
            }
        }
    }
    
    // Verify
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < P; j++) {
            if (arr[i][j] != i * P + j) {
                printf("Error in gang_vector_partitioned at [%d][%d]\n", i, j);
                exit(1);
            }
        }
    }
}

void test_worker_vector_partitioned() {
    printf("Testing worker+vector partitioned (code 6)...\n");
    int arr[N][P];
    
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(32) \
                copyout(arr[0:N][0:P])
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < P; j++) {
                arr[i][j] = i * P + j;
            }
        }
    }
    
    // Verify
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < P; j++) {
            if (arr[i][j] != i * P + j) {
                printf("Error in worker_vector_partitioned at [%d][%d]\n", i, j);
                exit(1);
            }
        }
    }
}

void test_fully_partitioned() {
    printf("Testing fully partitioned (code 7)...\n");
    int arr[N][M][P];
    
    #pragma acc parallel num_gangs(4) num_workers(4) vector_length(32) \
                copyout(arr[0:N][0:M][0:P])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                #pragma acc loop vector
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] = (i * M * P) + (j * P) + k;
                }
            }
        }
    }
    
    // Verify
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                int expected = (i * M * P) + (j * P) + k;
                if (arr[i][j][k] != expected) {
                    printf("Error in fully_partitioned at [%d][%d][%d]\n", 
                           i, j, k);
                    exit(1);
                }
            }
        }
    }
}

void test_reduction_partitioning() {
    printf("Testing reduction with mixed partitioning...\n");
    int sum = 0;
    int arr[N];
    
    #pragma acc parallel loop gang worker vector reduction(+:sum) \
                copyout(arr[0:N]) copy(sum)
    for (int i = 0; i < N; i++) {
        arr[i] = i;
        sum += i;
    }
    
    int expected = (N - 1) * N / 2;
    if (sum != expected) {
        printf("Reduction error: %d != %d\n", sum, expected);
        exit(1);
    }
}

void test_nested_parallelism() {
    printf("Testing nested parallelism...\n");
    int arr[N][M];
    
    #pragma acc parallel num_gangs(2) copyout(arr[0:N][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc parallel loop worker vector
            for (int j = 0; j < M; j++) {
                arr[i][j] = i * M + j;
            }
        }
    }
    
    // Verify
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            if (arr[i][j] != i * M + j) {
                printf("Error in nested_parallelism at [%d][%d]\n", i, j);
                exit(1);
            }
        }
    }
}

void test_conditional_partitioning() {
    printf("Testing conditional partitioning...\n");
    int arr[N];
    int use_vector = 1;  // Runtime value
    
    #pragma acc parallel copyout(arr[0:N])
    {
        if (use_vector) {
            #pragma acc loop vector
            for (int i = 0; i < N; i++) {
                arr[i] = i * 5;
            }
        } else {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                arr[i] = i * 6;
            }
        }
    }
    
    // Verify
    for (int i = 0; i < N; i++) {
        if (arr[i] != i * 5) {
            printf("Error in conditional_partitioning at index %d\n", i);
            exit(1);
        }
    }
}

void test_async_partitioning() {
    printf("Testing async partitioning...\n");
    int arr[N];
    int async_id = 1;
    
    #pragma acc parallel loop gang vector async(async_id) copyout(arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] = i * 7;
    }
    
    #pragma acc wait(async_id)
    
    // Verify
    for (int i = 0; i < N; i++) {
        if (arr[i] != i * 7) {
            printf("Error in async_partitioning at index %d\n", i);
            exit(1);
        }
    }
}

void test_device_management() {
    printf("Testing device management...\n");
    
    // Try to set device (may trigger different paths)
    acc_set_device_num(0, acc_device_default);
    
    int device_type = acc_get_device_type();
    printf("Device type: %d\n", device_type);
    
    // Create data on device
    int *d_arr = (int*)acc_malloc(N * sizeof(int));
    int h_arr[N];
    
    #pragma acc parallel loop present(d_arr[0:N])
    for (int i = 0; i < N; i++) {
        d_arr[i] = i * 8;
    }
    
    acc_memcpy_from_device(h_arr, d_arr, N * sizeof(int));
    
    // Verify
    for (int i = 0; i < N; i++) {
        if (h_arr[i] != i * 8) {
            printf("Error in device_management at index %d\n", i);
            exit(1);
        }
    }
    
    acc_free(d_arr);
}

void test_tiled_loops() {
    printf("Testing tiled loops...\n");
    int arr[N][M];
    
    #pragma acc parallel loop gang tile(32, 8) copyout(arr[0:N][0:M])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr[i][j] = i * M + j;
        }
    }
    
    // Verify
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            if (arr[i][j] != i * M + j) {
                printf("Error in tiled_loops at [%d][%d]\n", i, j);
                exit(1);
            }
        }
    }
}

int main() {
    printf("Starting partition code coverage test...\n");
    
    // Enable debug output to trigger mapping function calls
    setenv("ACC_DEBUG", "1", 1);
    setenv("LIBGOMP_DEBUG", "1", 1);
    
    // Test all partition codes
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    
    // Additional tests to exercise different paths
    test_reduction_partitioning();
    test_nested_parallelism();
    test_conditional_partitioning();
    test_async_partitioning();
    test_device_management();
    test_tiled_loops();
    
    printf("All tests passed successfully!\n");
    printf("Note: To see partition code mapping in action, run with ACC_DEBUG=2\n");
    
    return 0;
}
