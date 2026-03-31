/* Test program to cover all partition mapping cases in omp-oacc-neuter-broadcast.cc
 * Lines 335-343: Partition code to string mapping
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o partition_test partition_test.c
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

/* Test Case 0: gang redundant - scalar reduction */
void test_gang_redundant() {
    printf("Testing Case 0: gang redundant\n");
    float sum = 0.0f;
    float data[N];
    
    for (int i = 0; i < N; i++) {
        data[i] = 1.0f;
    }
    
    #pragma acc parallel copyin(data[0:N]) reduction(+:sum)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            sum += data[i];
        }
    }
    
    printf("  Sum = %f (expected %f)\n", sum, (float)N);
    assert(sum == (float)N);
}

/* Test Case 1: gang partitioned - array distributed across gangs */
void test_gang_partitioned() {
    printf("Testing Case 1: gang partitioned\n");
    float arr[N];
    float result[N];
    
    for (int i = 0; i < N; i++) {
        arr[i] = (float)i;
        result[i] = 0.0f;
    }
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] * 2.0f;
        }
    }
    
    int success = 1;
    for (int i = 0; i < N; i++) {
        if (result[i] != (float)i * 2.0f) {
            success = 0;
            break;
        }
    }
    assert(success);
    printf("  Gang partitioned test passed\n");
}

/* Test Case 2: worker partitioned - worker-level distribution */
void test_worker_partitioned() {
    printf("Testing Case 2: worker partitioned\n");
    float arr[N];
    float result[N];
    
    for (int i = 0; i < N; i++) {
        arr[i] = (float)i;
        result[i] = 0.0f;
    }
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N]) num_workers(4)
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] + 10.0f;
        }
    }
    
    int success = 1;
    for (int i = 0; i < N; i++) {
        if (result[i] != (float)i + 10.0f) {
            success = 0;
            break;
        }
    }
    assert(success);
    printf("  Worker partitioned test passed\n");
}

/* Test Case 3: gang+worker partitioned - nested gang/worker distribution */
void test_gang_worker_partitioned() {
    printf("Testing Case 3: gang+worker partitioned\n");
    float matrix[M][M];
    float result[M][M];
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            matrix[i][j] = (float)(i * M + j);
            result[i][j] = 0.0f;
        }
    }
    
    #pragma acc parallel copyin(matrix) copyout(result)
    {
        #pragma acc loop gang worker collapse(2)
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                result[i][j] = matrix[i][j] * 3.0f;
            }
        }
    }
    
    int success = 1;
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            if (result[i][j] != (float)(i * M + j) * 3.0f) {
                success = 0;
                break;
            }
        }
    }
    assert(success);
    printf("  Gang+worker partitioned test passed\n");
}

/* Test Case 4: vector partitioned - vector-level SIMD operations */
void test_vector_partitioned() {
    printf("Testing Case 4: vector partitioned\n");
    float arr[N];
    float result[N];
    
    for (int i = 0; i < N; i++) {
        arr[i] = (float)i;
        result[i] = 0.0f;
    }
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N]) vector_length(32)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] / 2.0f;
        }
    }
    
    int success = 1;
    for (int i = 0; i < N; i++) {
        if (result[i] != (float)i / 2.0f) {
            success = 0;
            break;
        }
    }
    assert(success);
    printf("  Vector partitioned test passed\n");
}

/* Test Case 5: gang+vector partitioned - gang and vector without workers */
void test_gang_vector_partitioned() {
    printf("Testing Case 5: gang+vector partitioned\n");
    float arr[N];
    float result[N];
    
    for (int i = 0; i < N; i++) {
        arr[i] = (float)i;
        result[i] = 0.0f;
    }
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] * arr[i];  // Square operation
        }
    }
    
    int success = 1;
    for (int i = 0; i < N; i++) {
        if (result[i] != (float)(i * i)) {
            success = 0;
            break;
        }
    }
    assert(success);
    printf("  Gang+vector partitioned test passed\n");
}

/* Test Case 6: worker+vector partitioned - worker and vector combination */
void test_worker_vector_partitioned() {
    printf("Testing Case 6: worker+vector partitioned\n");
    float arr[N];
    float result[N];
    
    for (int i = 0; i < N; i++) {
        arr[i] = (float)i;
        result[i] = 0.0f;
    }
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N]) num_workers(4) vector_length(16)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < N; i++) {
            result[i] = sqrtf(arr[i]);  // Use math function
        }
    }
    
    int success = 1;
    for (int i = 0; i < N; i++) {
        if (fabsf(result[i] - sqrtf((float)i)) > 0.001f) {
            success = 0;
            break;
        }
    }
    assert(success);
    printf("  Worker+vector partitioned test passed\n");
}

/* Test Case 7: fully partitioned - all three levels (gang, worker, vector) */
void test_fully_partitioned() {
    printf("Testing Case 7: fully partitioned\n");
    float cube[P][M][M];
    float result[P][M][M];
    
    for (int i = 0; i < P; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < M; k++) {
                cube[i][j][k] = (float)(i * M * M + j * M + k);
                result[i][j][k] = 0.0f;
            }
        }
    }
    
    #pragma acc parallel copyin(cube) copyout(result)
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < P; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < M; k++) {
                    result[i][j][k] = cube[i][j][k] + 100.0f;
                }
            }
        }
    }
    
    int success = 1;
    for (int i = 0; i < P; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < M; k++) {
                float expected = (float)(i * M * M + j * M + k) + 100.0f;
                if (fabsf(result[i][j][k] - expected) > 0.001f) {
                    success = 0;
                    break;
                }
            }
        }
    }
    assert(success);
    printf("  Fully partitioned test passed\n");
}

/* Test default case (if compiler testing interfaces were available) */
void test_default_case() {
    printf("Testing default case (simulated)\n");
    printf("  Note: To trigger default case, would need compiler testing hooks\n");
    printf("  that pass invalid partition codes (>7) to the mapping function.\n");
    printf("  This typically requires internal compiler debugging interfaces.\n");
}

int main() {
    printf("Starting partition mapping coverage tests...\n\n");
    
    /* Execute all test cases to trigger different partition mappings */
    test_gang_redundant();           /* Case 0 */
    test_gang_partitioned();         /* Case 1 */
    test_worker_partitioned();       /* Case 2 */
    test_gang_worker_partitioned();  /* Case 3 */
    test_vector_partitioned();       /* Case 4 */
    test_gang_vector_partitioned();  /* Case 5 */
    test_worker_vector_partitioned();/* Case 6 */
    test_fully_partitioned();        /* Case 7 */
    
    /* Note about default case */
    test_default_case();
    
    printf("\nAll partition mapping tests completed successfully!\n");
    printf("This test program should trigger all 8 partition codes (0-7)\n");
    printf("in the omp-oacc-neuter-broadcast.cc mapping function.\n");
    
    return 0;
}
