/* Test program to cover all partition mapping cases in omp-oacc-neuter-broadcast.cc
 * Lines 335-343: case 0-7 partition string mappings
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
int verify_array(float *arr, int size, float expected) {
    for (int i = 0; i < size; i++) {
        if (arr[i] != expected) {
            printf("Verification failed at index %d: got %f, expected %f\n", 
                   i, arr[i], expected);
            return 0;
        }
    }
    return 1;
}

/* Test 0: gang redundant - scalar reductions, no data partitioning across gangs */
void test_gang_redundant() {
    printf("Testing case 0: gang redundant\n");
    float sum = 0.0f;
    float data[N];
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        data[i] = 1.0f;
    }
    
    /* This should trigger gang redundant partitioning */
    #pragma acc parallel copyin(data[0:N]) reduction(+:sum)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            sum += data[i];
        }
    }
    
    printf("  Sum = %f (expected %d)\n", sum, N);
    assert(sum == (float)N);
}

/* Test 1: gang partitioned - array data distributed across gangs */
void test_gang_partitioned() {
    printf("Testing case 1: gang partitioned\n");
    float arr[N];
    float result[N];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        arr[i] = (float)i;
        result[i] = 0.0f;
    }
    
    /* Each gang processes contiguous chunks */
    #pragma acc parallel loop gang copy(arr[0:N]) copyout(result[0:N])
    for (int i = 0; i < N; i++) {
        result[i] = arr[i] * 2.0f;
    }
    
    /* Verify */
    for (int i = 0; i < N; i++) {
        assert(result[i] == (float)i * 2.0f);
    }
    printf("  Gang partitioned test passed\n");
}

/* Test 2: worker partitioned - worker-level distribution */
void test_worker_partitioned() {
    printf("Testing case 2: worker partitioned\n");
    float arr[M];
    float result[M];
    
    for (int i = 0; i < M; i++) {
        arr[i] = (float)i;
        result[i] = 0.0f;
    }
    
    /* Worker-level partitioning */
    #pragma acc parallel loop worker num_workers(4) copy(arr[0:M]) copyout(result[0:M])
    for (int i = 0; i < M; i++) {
        result[i] = arr[i] + 100.0f;
    }
    
    /* Verify */
    for (int i = 0; i < M; i++) {
        assert(result[i] == (float)i + 100.0f);
    }
    printf("  Worker partitioned test passed\n");
}

/* Test 3: gang+worker partitioned - nested gang and worker distribution */
void test_gang_worker_partitioned() {
    printf("Testing case 3: gang+worker partitioned\n");
    float matrix[N][M];
    float result[N][M];
    
    /* Initialize 2D array */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            matrix[i][j] = (float)(i * M + j);
            result[i][j] = 0.0f;
        }
    }
    
    /* Combined gang and worker partitioning */
    #pragma acc parallel loop gang worker collapse(2) copyin(matrix[0:N][0:M]) copyout(result[0:N][0:M])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            result[i][j] = matrix[i][j] * 3.0f;
        }
    }
    
    /* Verify */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            assert(result[i][j] == (float)(i * M + j) * 3.0f);
        }
    }
    printf("  Gang+worker partitioned test passed\n");
}

/* Test 4: vector partitioned - vector-level SIMD operations */
void test_vector_partitioned() {
    printf("Testing case 4: vector partitioned\n");
    float vec[N];
    float result[N];
    
    for (int i = 0; i < N; i++) {
        vec[i] = (float)i;
        result[i] = 0.0f;
    }
    
    /* Vector-level partitioning */
    #pragma acc parallel loop vector vector_length(32) copy(vec[0:N]) copyout(result[0:N])
    for (int i = 0; i < N; i++) {
        result[i] = vec[i] * vec[i];  /* Square operation */
    }
    
    /* Verify */
    for (int i = 0; i < N; i++) {
        assert(result[i] == (float)(i * i));
    }
    printf("  Vector partitioned test passed\n");
}

/* Test 5: gang+vector partitioned - gang and vector without workers */
void test_gang_vector_partitioned() {
    printf("Testing case 5: gang+vector partitioned\n");
    float arr[N];
    float result[N];
    
    for (int i = 0; i < N; i++) {
        arr[i] = (float)i;
        result[i] = 0.0f;
    }
    
    /* Gang and vector partitioning */
    #pragma acc parallel loop gang vector copy(arr[0:N]) copyout(result[0:N])
    for (int i = 0; i < N; i++) {
        result[i] = arr[i] * 4.0f + 1.0f;
    }
    
    /* Verify */
    for (int i = 0; i < N; i++) {
        assert(result[i] == (float)i * 4.0f + 1.0f);
    }
    printf("  Gang+vector partitioned test passed\n");
}

/* Test 6: worker+vector partitioned - worker and vector combination */
void test_worker_vector_partitioned() {
    printf("Testing case 6: worker+vector partitioned\n");
    float arr[M];
    float result[M];
    
    for (int i = 0; i < M; i++) {
        arr[i] = (float)i;
        result[i] = 0.0f;
    }
    
    /* Worker and vector partitioning */
    #pragma acc parallel loop worker vector num_workers(4) vector_length(16) \
        copy(arr[0:M]) copyout(result[0:M])
    for (int i = 0; i < M; i++) {
        result[i] = arr[i] / 2.0f;
    }
    
    /* Verify */
    for (int i = 0; i < M; i++) {
        assert(result[i] == (float)i / 2.0f);
    }
    printf("  Worker+vector partitioned test passed\n");
}

/* Test 7: fully partitioned - all three levels (gang, worker, vector) */
void test_fully_partitioned() {
    printf("Testing case 7: fully partitioned\n");
    float cube[N][M][P];
    float result[N][M][P];
    
    /* Initialize 3D array */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                cube[i][j][k] = (float)(i * M * P + j * P + k);
                result[i][j][k] = 0.0f;
            }
        }
    }
    
    /* Fully partitioned with all three levels */
    #pragma acc parallel loop gang worker vector collapse(3) \
        copyin(cube[0:N][0:M][0:P]) copyout(result[0:N][0:M][0:P])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                result[i][j][k] = cube[i][j][k] + 1000.0f;
            }
        }
    }
    
    /* Verify */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                float expected = (float)(i * M * P + j * P + k) + 1000.0f;
                assert(result[i][j][k] == expected);
            }
        }
    }
    printf("  Fully partitioned test passed\n");
}

/* Test default case - if compiler testing interfaces were available */
void test_default_case() {
    printf("Testing default case (if compiler hooks available)\n");
    /* In a real compiler test harness, we might call internal functions
     * with invalid partition codes to trigger the default case.
     * For this standalone test, we'll just note it's not directly testable.
     */
    printf("  Note: Default case requires compiler internal testing hooks\n");
}

int main() {
    printf("Starting partition mapping coverage tests...\n\n");
    
    /* Run all partition tests */
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    
    /* Note about default case */
    test_default_case();
    
    printf("\nAll partition tests completed successfully!\n");
    printf("This should trigger all 8 partition code mappings (0-7) in the compiler.\n");
    
    return 0;
}
