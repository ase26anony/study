/* Test program to exercise all OpenACC partition mapping cases
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partitions test_partitions.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define M 128
#define P 32

/* Helper to verify results */
int verify_array(float *arr, int size, float expected) {
    for (int i = 0; i < size; i++) {
        if (arr[i] != expected) return 0;
    }
    return 1;
}

/* Test case 0: gang redundant - scalar reduction */
void test_gang_redundant() {
    printf("Testing case 0 (gang redundant)...\n");
    float sum = 0.0f;
    float data[N];
    
    for (int i = 0; i < N; i++) data[i] = 1.0f;
    
    #pragma acc parallel copy(data[0:N]) reduction(+:sum)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            sum += data[i];
        }
    }
    
    printf("  Sum = %f (expected %d)\n", sum, N);
    assert(sum == (float)N);
}

/* Test case 1: gang partitioned - array distributed across gangs */
void test_gang_partitioned() {
    printf("Testing case 1 (gang partitioned)...\n");
    float arr[N];
    
    #pragma acc parallel loop gang copy(arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] = i * 2.0f;
    }
    
    #pragma acc update self(arr[0:N])
    
    for (int i = 0; i < N; i++) {
        assert(arr[i] == i * 2.0f);
    }
    printf("  Gang partitioned array verified\n");
}

/* Test case 2: worker partitioned */
void test_worker_partitioned() {
    printf("Testing case 2 (worker partitioned)...\n");
    float arr[M];
    
    #pragma acc parallel loop worker num_workers(4) copy(arr[0:M])
    for (int i = 0; i < M; i++) {
        arr[i] = (float)(i % 16);
    }
    
    #pragma acc update self(arr[0:M])
    
    for (int i = 0; i < M; i++) {
        assert(arr[i] == (float)(i % 16));
    }
    printf("  Worker partitioned array verified\n");
}

/* Test case 3: gang+worker partitioned */
void test_gang_worker_partitioned() {
    printf("Testing case 3 (gang+worker partitioned)...\n");
    float matrix[N][M];
    
    #pragma acc parallel loop gang worker collapse(2) copy(matrix[0:N][0:M])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            matrix[i][j] = (float)(i + j);
        }
    }
    
    #pragma acc update self(matrix[0:N][0:M])
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            assert(matrix[i][j] == (float)(i + j));
        }
    }
    printf("  Gang+worker partitioned matrix verified\n");
}

/* Test case 4: vector partitioned */
void test_vector_partitioned() {
    printf("Testing case 4 (vector partitioned)...\n");
    float vec[N];
    
    #pragma acc parallel loop vector vector_length(32) copy(vec[0:N])
    for (int i = 0; i < N; i++) {
        vec[i] = (float)(i * i);
    }
    
    #pragma acc update self(vec[0:N])
    
    for (int i = 0; i < N; i++) {
        assert(vec[i] == (float)(i * i));
    }
    printf("  Vector partitioned array verified\n");
}

/* Test case 5: gang+vector partitioned */
void test_gang_vector_partitioned() {
    printf("Testing case 5 (gang+vector partitioned)...\n");
    float arr[N];
    
    #pragma acc parallel loop gang vector copy(arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] = (float)(N - i);
    }
    
    #pragma acc update self(arr[0:N])
    
    for (int i = 0; i < N; i++) {
        assert(arr[i] == (float)(N - i));
    }
    printf("  Gang+vector partitioned array verified\n");
}

/* Test case 6: worker+vector partitioned */
void test_worker_vector_partitioned() {
    printf("Testing case 6 (worker+vector partitioned)...\n");
    float arr[M];
    
    #pragma acc parallel loop worker vector num_workers(4) vector_length(16) copy(arr[0:M])
    for (int i = 0; i < M; i++) {
        arr[i] = (float)(i * 3);
    }
    
    #pragma acc update self(arr[0:M])
    
    for (int i = 0; i < M; i++) {
        assert(arr[i] == (float)(i * 3));
    }
    printf("  Worker+vector partitioned array verified\n");
}

/* Test case 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned() {
    printf("Testing case 7 (fully partitioned)...\n");
    float cube[N][M][P];
    
    #pragma acc parallel loop gang worker vector collapse(3) copy(cube[0:N][0:M][0:P])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                cube[i][j][k] = (float)(i * j * k);
            }
        }
    }
    
    #pragma acc update self(cube[0:N][0:M][0:P])
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                assert(cube[i][j][k] == (float)(i * j * k));
            }
        }
    }
    printf("  Fully partitioned 3D array verified\n");
}

/* Additional tests with different data clauses to influence partitioning */
void test_varied_data_clauses() {
    printf("Testing with varied data clauses...\n");
    
    /* Test with present clause */
    float preset_arr[N];
    #pragma acc enter data copyin(preset_arr[0:N])
    
    #pragma acc parallel loop gang present(preset_arr[0:N])
    for (int i = 0; i < N; i++) {
        preset_arr[i] = 42.0f;
    }
    
    #pragma acc update self(preset_arr[0:N])
    #pragma acc exit data delete(preset_arr)
    
    assert(verify_array(preset_arr, N, 42.0f));
    
    /* Test with private variables */
    float private_test[N];
    #pragma acc parallel loop gang private(private_test[0:N]) copy(private_test[0:N])
    for (int i = 0; i < N; i++) {
        private_test[i] = (float)i;
    }
    
    /* Test array reduction */
    float arr_reduce[N];
    for (int i = 0; i < N; i++) arr_reduce[i] = 1.0f;
    
    #pragma acc parallel loop gang reduction(+:arr_reduce[0:N]) copy(arr_reduce[0:N])
    for (int i = 0; i < N; i++) {
        arr_reduce[i] += 1.0f;
    }
    
    printf("  Varied data clauses tests completed\n");
}

/* Test with runtime parameters */
void test_runtime_partitioning(int size) {
    printf("Testing runtime partitioning with size=%d...\n", size);
    
    float *dyn_arr = (float*)malloc(size * sizeof(float));
    
    #pragma acc parallel loop gang copy(dyn_arr[0:size])
    for (int i = 0; i < size; i++) {
        dyn_arr[i] = (float)(size - i);
    }
    
    #pragma acc update self(dyn_arr[0:size])
    
    for (int i = 0; i < size; i++) {
        assert(dyn_arr[i] == (float)(size - i));
    }
    
    free(dyn_arr);
    printf("  Runtime partitioning verified\n");
}

/* Test triangular loop (non-rectangular iteration space) */
void test_triangular_loop() {
    printf("Testing triangular loop pattern...\n");
    float tri_arr[N][N];
    
    #pragma acc parallel loop gang worker collapse(2) copy(tri_arr[0:N][0:N])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j <= i; j++) {
            tri_arr[i][j] = (float)(i + j);
        }
    }
    
    printf("  Triangular loop test completed\n");
}

int main() {
    printf("=== OpenACC Partition Mapping Test Program ===\n\n");
    
    /* Execute all partition test cases */
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    
    /* Additional tests to influence partitioning decisions */
    test_varied_data_clauses();
    test_runtime_partitioning(512);
    test_triangular_loop();
    
    printf("\n=== All partition tests completed successfully ===\n");
    
    /* Note: The default case (case 8+) would require compiler-internal
     * testing hooks or invalid partition codes, which cannot be
     * triggered from standard user code */
    
    return 0;
}
