/* Test program to exercise all OpenACC partition mapping cases
 * Target: omp-oacc-neuter-broadcast.cc lines 335-343
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partitions test_partitions.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define M 64
#define P 32

/* Helper function to verify results */
int verify_array(float *arr, int size, float expected) {
    for (int i = 0; i < size; i++) {
        if (arr[i] != expected) {
            return 0;
        }
    }
    return 1;
}

/* Test case 0: gang redundant - scalar reduction */
void test_gang_redundant() {
    printf("Testing case 0: gang redundant\n");
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

/* Test case 1: gang partitioned - array distributed across gangs */
void test_gang_partitioned() {
    printf("Testing case 1: gang partitioned\n");
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
    
    int correct = 1;
    for (int i = 0; i < N; i++) {
        if (result[i] != arr[i] * 2.0f) {
            correct = 0;
            break;
        }
    }
    printf("  Result: %s\n", correct ? "PASS" : "FAIL");
    assert(correct);
}

/* Test case 2: worker partitioned - worker-level distribution */
void test_worker_partitioned() {
    printf("Testing case 2: worker partitioned\n");
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
            result[i] = arr[i] + 100.0f;
        }
    }
    
    int correct = 1;
    for (int i = 0; i < N; i++) {
        if (result[i] != arr[i] + 100.0f) {
            correct = 0;
            break;
        }
    }
    printf("  Result: %s\n", correct ? "PASS" : "FAIL");
    assert(correct);
}

/* Test case 3: gang+worker partitioned - nested gang/worker distribution */
void test_gang_worker_partitioned() {
    printf("Testing case 3: gang+worker partitioned\n");
    float arr[M][N];
    float result[M][N];
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr[i][j] = (float)(i * N + j);
            result[i][j] = 0.0f;
        }
    }
    
    #pragma acc parallel copyin(arr[0:M][0:N]) copyout(result[0:M][0:N])
    {
        #pragma acc loop gang worker collapse(2)
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < N; j++) {
                result[i][j] = arr[i][j] * 3.0f;
            }
        }
    }
    
    int correct = 1;
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            if (result[i][j] != arr[i][j] * 3.0f) {
                correct = 0;
                break;
            }
        }
        if (!correct) break;
    }
    printf("  Result: %s\n", correct ? "PASS" : "FAIL");
    assert(correct);
}

/* Test case 4: vector partitioned - SIMD-style vector operations */
void test_vector_partitioned() {
    printf("Testing case 4: vector partitioned\n");
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
    
    int correct = 1;
    for (int i = 0; i < N; i++) {
        if (result[i] != arr[i] / 2.0f) {
            correct = 0;
            break;
        }
    }
    printf("  Result: %s\n", correct ? "PASS" : "FAIL");
    assert(correct);
}

/* Test case 5: gang+vector partitioned - gang and vector without workers */
void test_gang_vector_partitioned() {
    printf("Testing case 5: gang+vector partitioned\n");
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
            result[i] = arr[i] * arr[i];
        }
    }
    
    int correct = 1;
    for (int i = 0; i < N; i++) {
        if (result[i] != arr[i] * arr[i]) {
            correct = 0;
            break;
        }
    }
    printf("  Result: %s\n", correct ? "PASS" : "FAIL");
    assert(correct);
}

/* Test case 6: worker+vector partitioned - worker and vector combination */
void test_worker_vector_partitioned() {
    printf("Testing case 6: worker+vector partitioned\n");
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
            result[i] = sqrtf(arr[i] + 1.0f);
        }
    }
    
    int correct = 1;
    for (int i = 0; i < N; i++) {
        float expected = sqrtf(arr[i] + 1.0f);
        if (fabsf(result[i] - expected) > 0.0001f) {
            correct = 0;
            break;
        }
    }
    printf("  Result: %s\n", correct ? "PASS" : "FAIL");
    assert(correct);
}

/* Test case 7: fully partitioned - gang, worker, and vector all active */
void test_fully_partitioned() {
    printf("Testing case 7: fully partitioned\n");
    float arr[M][N][P];
    float result[M][N][P];
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            for (int k = 0; k < P; k++) {
                arr[i][j][k] = (float)(i * N * P + j * P + k);
                result[i][j][k] = 0.0f;
            }
        }
    }
    
    #pragma acc parallel copyin(arr[0:M][0:N][0:P]) copyout(result[0:M][0:N][0:P])
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < N; j++) {
                for (int k = 0; k < P; k++) {
                    result[i][j][k] = arr[i][j][k] + 1000.0f;
                }
            }
        }
    }
    
    int correct = 1;
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            for (int k = 0; k < P; k++) {
                if (result[i][j][k] != arr[i][j][k] + 1000.0f) {
                    correct = 0;
                    break;
                }
            }
            if (!correct) break;
        }
        if (!correct) break;
    }
    printf("  Result: %s\n", correct ? "PASS" : "FAIL");
    assert(correct);
}

/* Test default case - invalid partition code (if compiler hooks available) */
void test_default_case() {
    printf("Testing default case: <illegal>\n");
    printf("  Note: Default case typically triggered by invalid partition codes\n");
    printf("  in compiler internals. This may not be directly exercisable from user code.\n");
    
    /* Attempt to create ambiguous partitioning scenario */
    float arr[N];
    float result[N];
    
    for (int i = 0; i < N; i++) {
        arr[i] = (float)i;
        result[i] = 0.0f;
    }
    
    /* Complex nested directives that might confuse partitioning analysis */
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < 1; j++) {  /* Single iteration to create edge case */
                result[i] = arr[i] * 2.0f;
            }
        }
    }
    
    printf("  Edge case test completed\n");
}

int main() {
    printf("=== OpenACC Partition Mapping Test ===\n");
    printf("Targeting omp-oacc-neuter-broadcast.cc lines 335-343\n\n");
    
    /* Run all test cases */
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    test_default_case();
    
    printf("\n=== All tests completed successfully ===\n");
    return 0;
}
