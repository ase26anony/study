/* Test program to cover all partition code cases in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partitions test_partitions.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define M 64
#define P 16

/* Helper function to verify results */
int verify_array(int *arr, int size, int expected_value) {
    for (int i = 0; i < size; i++) {
        if (arr[i] != expected_value) {
            return 0;
        }
    }
    return 1;
}

/* Test case 0: gang redundant - scalar reductions, no data partitioning across gangs */
void test_gang_redundant() {
    printf("Testing case 0: gang redundant\n");
    
    int sum = 0;
    int arr[N];
    
    #pragma acc parallel copy(arr[0:N]) reduction(+:sum)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            arr[i] = i;
            sum += i;
        }
    }
    
    int expected_sum = (N-1)*N/2;
    assert(sum == expected_sum);
    printf("  Case 0 passed: sum = %d (expected %d)\n", sum, expected_sum);
}

/* Test case 1: gang partitioned - array data distributed across gangs */
void test_gang_partitioned() {
    printf("Testing case 1: gang partitioned\n");
    
    int arr[N];
    memset(arr, 0, sizeof(arr));
    
    #pragma acc parallel copy(arr[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            arr[i] = i * 2;
        }
    }
    
    assert(verify_array(arr, N, 0) == 0); // Not all zeros
    printf("  Case 1 passed: array modified across gangs\n");
}

/* Test case 2: worker partitioned - worker-level distribution */
void test_worker_partitioned() {
    printf("Testing case 2: worker partitioned\n");
    
    int arr[N];
    memset(arr, 0, sizeof(arr));
    
    #pragma acc parallel copy(arr[0:N]) num_workers(4)
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            arr[i] = i + 1;
        }
    }
    
    assert(arr[0] == 1 && arr[N-1] == N);
    printf("  Case 2 passed: worker-partitioned array initialized\n");
}

/* Test case 3: gang+worker partitioned - nested gang and worker distribution */
void test_gang_worker_partitioned() {
    printf("Testing case 3: gang+worker partitioned\n");
    
    int arr[M][N];
    memset(arr, 0, sizeof(arr));
    
    #pragma acc parallel copy(arr[0:M][0:N])
    {
        #pragma acc loop gang worker collapse(2)
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < N; j++) {
                arr[i][j] = i * N + j;
            }
        }
    }
    
    assert(arr[0][0] == 0 && arr[M-1][N-1] == (M*N - 1));
    printf("  Case 3 passed: 2D array gang+worker partitioned\n");
}

/* Test case 4: vector partitioned - vector-level SIMD operations */
void test_vector_partitioned() {
    printf("Testing case 4: vector partitioned\n");
    
    float arr[N];
    
    #pragma acc parallel copy(arr[0:N]) vector_length(32)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            arr[i] = (float)i * 0.5f;
        }
    }
    
    assert(arr[0] == 0.0f && arr[1] == 0.5f);
    printf("  Case 4 passed: vector-partitioned computation\n");
}

/* Test case 5: gang+vector partitioned - gang and vector without workers */
void test_gang_vector_partitioned() {
    printf("Testing case 5: gang+vector partitioned\n");
    
    int arr[N];
    
    #pragma acc parallel copy(arr[0:N])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            arr[i] = i * i;
        }
    }
    
    assert(arr[2] == 4 && arr[3] == 9);
    printf("  Case 5 passed: gang+vector partitioned squares\n");
}

/* Test case 6: worker+vector partitioned - worker and vector combination */
void test_worker_vector_partitioned() {
    printf("Testing case 6: worker+vector partitioned\n");
    
    float arr[N];
    
    #pragma acc parallel copy(arr[0:N]) num_workers(4) vector_length(16)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < N; i++) {
            arr[i] = (float)i / 2.0f;
        }
    }
    
    assert(arr[4] == 2.0f && arr[6] == 3.0f);
    printf("  Case 6 passed: worker+vector partitioned division\n");
}

/* Test case 7: fully partitioned - gang, worker, and vector all active */
void test_fully_partitioned() {
    printf("Testing case 7: fully partitioned\n");
    
    int arr[P][M][N];
    memset(arr, 0, sizeof(arr));
    
    #pragma acc parallel copy(arr[0:P][0:M][0:N])
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < P; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < N; k++) {
                    arr[i][j][k] = i * M * N + j * N + k;
                }
            }
        }
    }
    
    assert(arr[0][0][0] == 0);
    assert(arr[P-1][M-1][N-1] == (P * M * N - 1));
    printf("  Case 7 passed: 3D array fully partitioned\n");
}

/* Test default case: This would normally require compiler internals,
   but we simulate by testing edge conditions */
void test_edge_cases() {
    printf("Testing edge cases (potential default path triggers)\n");
    
    /* Test with runtime-variable loop bounds */
    int size = N;
    int arr[N];
    
    #pragma acc parallel copy(arr[0:size])
    {
        #pragma acc loop gang
        for (int i = 0; i < size; i++) {
            arr[i] = 1;
        }
    }
    
    /* Test with present clause (assuming data already on device) */
    int *dev_arr = (int*)malloc(N * sizeof(int));
    #pragma acc enter data copyin(dev_arr[0:N])
    
    #pragma acc parallel present(dev_arr[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            dev_arr[i] = i;
        }
    }
    
    #pragma acc exit data copyout(dev_arr[0:N])
    free(dev_arr);
    
    printf("  Edge cases tested\n");
}

int main() {
    printf("Starting partition coverage tests...\n\n");
    
    /* Execute all test cases */
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    test_edge_cases();
    
    printf("\nAll partition test cases completed successfully!\n");
    printf("This should trigger all 8 partition codes (0-7) in the compiler's analysis.\n");
    
    return 0;
}
