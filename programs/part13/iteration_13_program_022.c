/* Test program to cover partition code mapping in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O0 -fopenacc -foffload=disable -o test_partition test_partition.c
 * Run with: ACC_DEBUG=1 ./test_partition
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>

#define N 1024
#define M 32
#define P 8

void test_gang_redundant() {
    printf("Testing gang redundant (case 0)...\n");
    int data[N];
    int i;
    
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
    printf("Testing gang partitioned (case 1)...\n");
    int data[N];
    int i;
    
    #pragma acc parallel loop gang vector copyout(data[0:N])
    for (i = 0; i < N; i++) {
        data[i] = i + 1;
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        if (data[i] != i + 1) {
            printf("Error in gang partitioned test at index %d\n", i);
            exit(1);
        }
    }
}

void test_worker_partitioned() {
    printf("Testing worker partitioned (case 2)...\n");
    int data[N];
    int i;
    
    #pragma acc parallel loop worker copyout(data[0:N])
    for (i = 0; i < N; i++) {
        data[i] = i * 3;
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        if (data[i] != i * 3) {
            printf("Error in worker partitioned test at index %d\n", i);
            exit(1);
        }
    }
}

void test_gang_worker_partitioned() {
    printf("Testing gang+worker partitioned (case 3)...\n");
    int data[N][M];
    int i, j;
    
    #pragma acc parallel loop gang worker collapse(2) copyout(data[0:N][0:M])
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            data[i][j] = i * M + j;
        }
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            if (data[i][j] != i * M + j) {
                printf("Error in gang+worker partitioned test at [%d][%d]\n", i, j);
                exit(1);
            }
        }
    }
}

void test_vector_partitioned() {
    printf("Testing vector partitioned (case 4)...\n");
    int data[N];
    int i;
    
    #pragma acc parallel loop vector copyout(data[0:N])
    for (i = 0; i < N; i++) {
        data[i] = i * 4;
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        if (data[i] != i * 4) {
            printf("Error in vector partitioned test at index %d\n", i);
            exit(1);
        }
    }
}

void test_gang_vector_partitioned() {
    printf("Testing gang+vector partitioned (case 5)...\n");
    int data[N][P];
    int i, j;
    
    #pragma acc parallel loop gang vector collapse(2) copyout(data[0:N][0:P])
    for (i = 0; i < N; i++) {
        for (j = 0; j < P; j++) {
            data[i][j] = i * P + j + 100;
        }
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        for (j = 0; j < P; j++) {
            if (data[i][j] != i * P + j + 100) {
                printf("Error in gang+vector partitioned test at [%d][%d]\n", i, j);
                exit(1);
            }
        }
    }
}

void test_worker_vector_partitioned() {
    printf("Testing worker+vector partitioned (case 6)...\n");
    int data[M][P];
    int i, j;
    
    #pragma acc parallel loop worker vector collapse(2) copyout(data[0:M][0:P])
    for (i = 0; i < M; i++) {
        for (j = 0; j < P; j++) {
            data[i][j] = i * P + j + 200;
        }
    }
    
    // Verify
    for (i = 0; i < M; i++) {
        for (j = 0; j < P; j++) {
            if (data[i][j] != i * P + j + 200) {
                printf("Error in worker+vector partitioned test at [%d][%d]\n", i, j);
                exit(1);
            }
        }
    }
}

void test_fully_partitioned() {
    printf("Testing fully partitioned (case 7)...\n");
    int data[N][M][P];
    int i, j, k;
    
    #pragma acc parallel loop gang worker vector collapse(3) copyout(data[0:N][0:M][0:P])
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            for (k = 0; k < P; k++) {
                data[i][j][k] = i * M * P + j * P + k;
            }
        }
    }
    
    // Verify a subset
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 5; j++) {
            for (k = 0; k < 3; k++) {
                if (data[i][j][k] != i * M * P + j * P + k) {
                    printf("Error in fully partitioned test at [%d][%d][%d]\n", i, j, k);
                    exit(1);
                }
            }
        }
    }
}

void test_nested_parallelism() {
    printf("Testing nested parallelism for varied partitioning...\n");
    int data[N];
    int i;
    
    #pragma acc parallel copyout(data[0:N])
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            data[i] = i * 5;
        }
        
        #pragma acc loop worker
        for (i = 0; i < N; i++) {
            data[i] += 1;
        }
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        if (data[i] != i * 5 + 1) {
            printf("Error in nested parallelism test at index %d\n", i);
            exit(1);
        }
    }
}

void test_reductions() {
    printf("Testing reductions with different partitioning...\n");
    int data[N];
    int sum = 0, expected_sum = 0;
    int i;
    
    for (i = 0; i < N; i++) {
        data[i] = i + 1;
        expected_sum += i + 1;
    }
    
    #pragma acc parallel loop gang reduction(+:sum) copyin(data[0:N])
    for (i = 0; i < N; i++) {
        sum += data[i];
    }
    
    if (sum != expected_sum) {
        printf("Error in reduction test: got %d, expected %d\n", sum, expected_sum);
        exit(1);
    }
}

void test_conditional_partitioning() {
    printf("Testing conditional partitioning...\n");
    int data[N];
    int i;
    int use_vector = 1;
    
    #pragma acc parallel loop copyout(data[0:N])
    for (i = 0; i < N; i++) {
        if (use_vector) {
            #pragma acc loop vector
            for (int j = 0; j < 4; j++) {
                data[i] = i * 10 + j;
            }
        } else {
            data[i] = i * 10;
        }
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        if (data[i] != i * 10 + 3) {  // Last iteration of inner loop
            printf("Error in conditional partitioning test at index %d\n", i);
            exit(1);
        }
    }
}

void test_async_operations() {
    printf("Testing async operations...\n");
    int data[N];
    int i;
    int async_id = 1;
    
    #pragma acc parallel loop async(async_id) copyout(data[0:N])
    for (i = 0; i < N; i++) {
        data[i] = i * 7;
    }
    
    #pragma acc wait(async_id)
    
    // Verify
    for (i = 0; i < N; i++) {
        if (data[i] != i * 7) {
            printf("Error in async operations test at index %d\n", i);
            exit(1);
        }
    }
}

void test_multi_device() {
    printf("Testing multi-device operations...\n");
    
    // Try to set device (may trigger different paths)
    acc_set_device_type(acc_device_default);
    int dev_num = acc_get_device_num(acc_device_default);
    printf("Using device number: %d\n", dev_num);
    
    // Test with device data
    int data[N];
    int i;
    
    #pragma acc data create(data[0:N])
    {
        #pragma acc parallel loop present(data)
        for (i = 0; i < N; i++) {
            data[i] = i * 11;
        }
        
        #pragma acc update host(data[0:N])
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        if (data[i] != i * 11) {
            printf("Error in multi-device test at index %d\n", i);
            exit(1);
        }
    }
}

void test_invalid_conditions() {
    printf("Testing potential invalid conditions...\n");
    
    // This might trigger error paths that could hit default case
    // Note: We're not actually causing errors, just testing edge cases
    
    int data[10];
    int i;
    
    // Test with very small loop
    #pragma acc parallel loop copyout(data[0:1])
    for (i = 0; i < 1; i++) {
        data[i] = 42;
    }
    
    if (data[0] != 42) {
        printf("Error in invalid conditions test\n");
        exit(1);
    }
}

int main() {
    printf("Starting partition code coverage test...\n");
    
    // Enable debug output to increase chance of calling the mapping function
    setenv("ACC_DEBUG", "1", 1);
    
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
    test_reductions();
    test_conditional_partitioning();
    test_async_operations();
    test_multi_device();
    test_invalid_conditions();
    
    printf("All tests passed successfully!\n");
    printf("Note: To see debug output including partition codes, run with ACC_DEBUG=2 or higher\n");
    
    return 0;
}
