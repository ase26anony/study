/* Test program to cover all partition mapping cases in omp-oacc-neuter-broadcast.cc
 * Lines 335-343: partition code to string mapping
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partitions test_partitions.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define M 128
#define P 32

/* Helper for verification */
static int verify_array(float *arr, int size, float expected) {
    for (int i = 0; i < size; i++) {
        if (arr[i] != expected) return 0;
    }
    return 1;
}

/* Test case 0: gang redundant - scalar reduction, no data partitioning across gangs */
void test_gang_redundant() {
    printf("Testing case 0 (gang redundant)...\n");
    float sum = 0.0f;
    float data[N];
    
    for (int i = 0; i < N; i++) data[i] = 1.0f;
    
    #pragma acc parallel copyin(data[0:N]) reduction(+:sum)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            sum += data[i];
        }
    }
    
    assert(sum == (float)N);
    printf("  Passed: sum = %f\n", sum);
}

/* Test case 1: gang partitioned - array distributed across gangs */
void test_gang_partitioned() {
    printf("Testing case 1 (gang partitioned)...\n");
    float arr[N];
    
    for (int i = 0; i < N; i++) arr[i] = 0.0f;
    
    #pragma acc parallel loop gang copy(arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] = i * 2.0f;
    }
    
    #pragma acc update self(arr[0:N])
    
    for (int i = 0; i < N; i++) {
        assert(arr[i] == i * 2.0f);
    }
    printf("  Passed: gang partitioned array update\n");
}

/* Test case 2: worker partitioned - worker-level distribution */
void test_worker_partitioned() {
    printf("Testing case 2 (worker partitioned)...\n");
    float arr[M];
    
    for (int i = 0; i < M; i++) arr[i] = 0.0f;
    
    #pragma acc parallel loop worker num_workers(4) copy(arr[0:M])
    for (int i = 0; i < M; i++) {
        arr[i] = (float)(i + 1);
    }
    
    #pragma acc update self(arr[0:M])
    
    for (int i = 0; i < M; i++) {
        assert(arr[i] == (float)(i + 1));
    }
    printf("  Passed: worker partitioned array\n");
}

/* Test case 3: gang+worker partitioned - nested gang/worker distribution */
void test_gang_worker_partitioned() {
    printf("Testing case 3 (gang+worker partitioned)...\n");
    float matrix[N][M];
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            matrix[i][j] = 0.0f;
        }
    }
    
    #pragma acc parallel loop gang worker collapse(2) copy(matrix[0:N][0:M])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            matrix[i][j] = (float)(i * M + j);
        }
    }
    
    #pragma acc update self(matrix[0:N][0:M])
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            assert(matrix[i][j] == (float)(i * M + j));
        }
    }
    printf("  Passed: gang+worker partitioned 2D array\n");
}

/* Test case 4: vector partitioned - SIMD-style vector operations */
void test_vector_partitioned() {
    printf("Testing case 4 (vector partitioned)...\n");
    float vec[N];
    
    for (int i = 0; i < N; i++) vec[i] = (float)i;
    
    #pragma acc parallel loop vector vector_length(32) copy(vec[0:N])
    for (int i = 0; i < N; i++) {
        vec[i] = vec[i] * 3.14f;
    }
    
    #pragma acc update self(vec[0:N])
    
    for (int i = 0; i < N; i++) {
        assert(vec[i] == (float)i * 3.14f);
    }
    printf("  Passed: vector partitioned operations\n");
}

/* Test case 5: gang+vector partitioned - gang and vector without workers */
void test_gang_vector_partitioned() {
    printf("Testing case 5 (gang+vector partitioned)...\n");
    float arr[N];
    
    for (int i = 0; i < N; i++) arr[i] = 0.0f;
    
    #pragma acc parallel loop gang vector copy(arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] = (float)(i % 10) + 1.0f;
    }
    
    #pragma acc update self(arr[0:N])
    
    for (int i = 0; i < N; i++) {
        assert(arr[i] == (float)(i % 10) + 1.0f);
    }
    printf("  Passed: gang+vector partitioned array\n");
}

/* Test case 6: worker+vector partitioned - worker and vector combination */
void test_worker_vector_partitioned() {
    printf("Testing case 6 (worker+vector partitioned)...\n");
    float arr[M];
    
    for (int i = 0; i < M; i++) arr[i] = (float)i;
    
    #pragma acc parallel loop worker vector num_workers(2) vector_length(16) copy(arr[0:M])
    for (int i = 0; i < M; i++) {
        arr[i] = arr[i] + 100.0f;
    }
    
    #pragma acc update self(arr[0:M])
    
    for (int i = 0; i < M; i++) {
        assert(arr[i] == (float)i + 100.0f);
    }
    printf("  Passed: worker+vector partitioned array\n");
}

/* Test case 7: fully partitioned - gang, worker, and vector all active */
void test_fully_partitioned() {
    printf("Testing case 7 (fully partitioned)...\n");
    float cube[N][M][P];
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                cube[i][j][k] = 0.0f;
            }
        }
    }
    
    #pragma acc parallel loop gang worker vector collapse(3) copy(cube[0:N][0:M][0:P])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                cube[i][j][k] = (float)(i + j + k);
            }
        }
    }
    
    #pragma acc update self(cube[0:N][0:M][0:P])
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                assert(cube[i][j][k] == (float)(i + j + k));
            }
        }
    }
    printf("  Passed: fully partitioned 3D array\n");
}

/* Test default case simulation - using runtime variables to potentially trigger edge cases */
void test_edge_cases() {
    printf("Testing edge cases for default path coverage...\n");
    
    /* Variable loop bounds can affect partitioning decisions */
    int dyn_size = 256;
    float dyn_arr[256];
    
    for (int i = 0; i < dyn_size; i++) dyn_arr[i] = 0.0f;
    
    /* Mixed directives with runtime parameters */
    #pragma acc parallel loop gang vector copy(dyn_arr[0:dyn_size])
    for (int i = 0; i < dyn_size; i++) {
        dyn_arr[i] = (float)(i * 2);
    }
    
    #pragma acc update self(dyn_arr[0:dyn_size])
    
    for (int i = 0; i < dyn_size; i++) {
        assert(dyn_arr[i] == (float)(i * 2));
    }
    
    /* Triangular loop pattern */
    float tri_arr[100][100];
    
    #pragma acc parallel loop gang collapse(2) copy(tri_arr[0:100][0:100])
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j <= i; j++) {  /* Non-rectangular */
            tri_arr[i][j] = (float)(i * j);
        }
    }
    
    printf("  Passed: edge cases with runtime parameters\n");
}

int main() {
    printf("Starting partition mapping coverage tests...\n\n");
    
    /* Execute all test cases to trigger different partition codes */
    test_gang_redundant();           /* Should trigger case 0 */
    test_gang_partitioned();         /* Should trigger case 1 */
    test_worker_partitioned();       /* Should trigger case 2 */
    test_gang_worker_partitioned();  /* Should trigger case 3 */
    test_vector_partitioned();       /* Should trigger case 4 */
    test_gang_vector_partitioned();  /* Should trigger case 5 */
    test_worker_vector_partitioned();/* Should trigger case 6 */
    test_fully_partitioned();        /* Should trigger case 7 */
    
    test_edge_cases();               /* Additional edge cases */
    
    printf("\nAll tests completed successfully!\n");
    printf("All partition mapping cases (0-7) should have been triggered during compilation.\n");
    
    return 0;
}
