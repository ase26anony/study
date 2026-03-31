/* Test program to cover all partition mapping cases in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partitions test_partitions.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define M 128
#define P 32

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

/* Test case 0: gang redundant - scalar reduction */
void test_gang_redundant() {
    printf("Testing case 0: gang redundant\n");
    float sum = 0.0f;
    float arr[N];
    
    /* Initialize array */
    for (int i = 0; i < N; i++) {
        arr[i] = 1.0f;
    }
    
    #pragma acc parallel copyin(arr[0:N]) reduction(+:sum)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            sum += arr[i];
        }
    }
    
    printf("  Sum = %f (expected %d)\n", sum, N);
    assert(sum == (float)N);
}

/* Test case 1: gang partitioned - array distributed across gangs */
void test_gang_partitioned() {
    printf("Testing case 1: gang partitioned\n");
    float arr[N], result[N];
    
    /* Initialize arrays */
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
    
    /* Verify results */
    for (int i = 0; i < N; i++) {
        if (result[i] != arr[i] * 2.0f) {
            printf("  Verification failed at index %d\n", i);
            exit(1);
        }
    }
    printf("  Gang partitioned test passed\n");
}

/* Test case 2: worker partitioned - worker-level distribution */
void test_worker_partitioned() {
    printf("Testing case 2: worker partitioned\n");
    float arr[M], result[M];
    
    /* Initialize arrays */
    for (int i = 0; i < M; i++) {
        arr[i] = (float)i;
        result[i] = 0.0f;
    }
    
    #pragma acc parallel copyin(arr[0:M]) copyout(result[0:M]) num_workers(4)
    {
        #pragma acc loop worker
        for (int i = 0; i < M; i++) {
            result[i] = arr[i] + 100.0f;
        }
    }
    
    /* Verify results */
    for (int i = 0; i < M; i++) {
        if (result[i] != arr[i] + 100.0f) {
            printf("  Verification failed at index %d\n", i);
            exit(1);
        }
    }
    printf("  Worker partitioned test passed\n");
}

/* Test case 3: gang+worker partitioned - nested distribution */
void test_gang_worker_partitioned() {
    printf("Testing case 3: gang+worker partitioned\n");
    float arr[N][M], result[N][M];
    
    /* Initialize 2D array */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr[i][j] = (float)(i * M + j);
            result[i][j] = 0.0f;
        }
    }
    
    #pragma acc parallel copyin(arr[0:N][0:M]) copyout(result[0:N][0:M])
    {
        #pragma acc loop gang worker collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                result[i][j] = arr[i][j] * 3.0f;
            }
        }
    }
    
    /* Verify results */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            if (result[i][j] != arr[i][j] * 3.0f) {
                printf("  Verification failed at [%d][%d]\n", i, j);
                exit(1);
            }
        }
    }
    printf("  Gang+worker partitioned test passed\n");
}

/* Test case 4: vector partitioned - SIMD-style operations */
void test_vector_partitioned() {
    printf("Testing case 4: vector partitioned\n");
    float arr[N], result[N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr[i] = (float)i;
        result[i] = 0.0f;
    }
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N]) vector_length(32)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] * arr[i];  /* Square operation */
        }
    }
    
    /* Verify results */
    for (int i = 0; i < N; i++) {
        if (result[i] != arr[i] * arr[i]) {
            printf("  Verification failed at index %d\n", i);
            exit(1);
        }
    }
    printf("  Vector partitioned test passed\n");
}

/* Test case 5: gang+vector partitioned */
void test_gang_vector_partitioned() {
    printf("Testing case 5: gang+vector partitioned\n");
    float arr[N], result[N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr[i] = (float)i;
        result[i] = 0.0f;
    }
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            result[i] = sqrtf(arr[i]);  /* Square root operation */
        }
    }
    
    /* Verify approximate results */
    for (int i = 0; i < N; i++) {
        float expected = sqrtf(arr[i]);
        if (fabs(result[i] - expected) > 0.001f) {
            printf("  Verification failed at index %d: got %f, expected %f\n", 
                   i, result[i], expected);
            exit(1);
        }
    }
    printf("  Gang+vector partitioned test passed\n");
}

/* Test case 6: worker+vector partitioned */
void test_worker_vector_partitioned() {
    printf("Testing case 6: worker+vector partitioned\n");
    float arr[M], result[M];
    
    /* Initialize arrays */
    for (int i = 0; i < M; i++) {
        arr[i] = (float)i;
        result[i] = 0.0f;
    }
    
    #pragma acc parallel copyin(arr[0:M]) copyout(result[0:M]) \
        num_workers(4) vector_length(16)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < M; i++) {
            result[i] = sinf(arr[i] * 0.1f);  /* Sine operation */
        }
    }
    
    /* Verify approximate results */
    for (int i = 0; i < M; i++) {
        float expected = sinf(arr[i] * 0.1f);
        if (fabs(result[i] - expected) > 0.001f) {
            printf("  Verification failed at index %d: got %f, expected %f\n", 
                   i, result[i], expected);
            exit(1);
        }
    }
    printf("  Worker+vector partitioned test passed\n");
}

/* Test case 7: fully partitioned - all three levels */
void test_fully_partitioned() {
    printf("Testing case 7: fully partitioned\n");
    float arr[N][M][P], result[N][M][P];
    
    /* Initialize 3D array */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr[i][j][k] = (float)(i * M * P + j * P + k);
                result[i][j][k] = 0.0f;
            }
        }
    }
    
    #pragma acc parallel copyin(arr[0:N][0:M][0:P]) copyout(result[0:N][0:M][0:P])
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    result[i][j][k] = arr[i][j][k] + 1.0f;
                }
            }
        }
    }
    
    /* Verify results */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                if (result[i][j][k] != arr[i][j][k] + 1.0f) {
                    printf("  Verification failed at [%d][%d][%d]\n", i, j, k);
                    exit(1);
                }
            }
        }
    }
    printf("  Fully partitioned test passed\n");
}

/* Test default case - if compiler testing interfaces were available */
void test_default_case() {
    printf("Testing default case (simulated)\n");
    printf("  Note: To trigger default case, would need compiler testing hooks\n");
    printf("  such as __builtin_acc_partition_code(8) or similar\n");
}

int main() {
    printf("Starting partition mapping coverage tests...\n\n");
    
    /* Execute all test cases */
    test_gang_redundant();
    printf("\n");
    
    test_gang_partitioned();
    printf("\n");
    
    test_worker_partitioned();
    printf("\n");
    
    test_gang_worker_partitioned();
    printf("\n");
    
    test_vector_partitioned();
    printf("\n");
    
    test_gang_vector_partitioned();
    printf("\n");
    
    test_worker_vector_partitioned();
    printf("\n");
    
    test_fully_partitioned();
    printf("\n");
    
    test_default_case();
    printf("\n");
    
    printf("All partition mapping tests completed successfully!\n");
    printf("Expected to trigger all 8 partition codes (0-7) during compilation.\n");
    
    return 0;
}
