/* Test program to exercise all OpenACC partition mapping cases (0-7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define M 128
#define P 32

/* Helper for verification */
static int errors = 0;
#define CHECK(cond, msg) if (!(cond)) { printf("FAIL: %s\n", msg); errors++; }

/* Test case 0: gang redundant - scalar reduction, no data partitioning across gangs */
void test_gang_redundant() {
    printf("Testing case 0 (gang redundant)...\n");
    int sum = 0;
    int arr[N];
    
    for (int i = 0; i < N; i++) arr[i] = i % 10;
    
    #pragma acc parallel copyin(arr[0:N]) reduction(+:sum)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            sum += arr[i];
        }
    }
    
    int expected = 0;
    for (int i = 0; i < N; i++) expected += arr[i];
    CHECK(sum == expected, "gang redundant reduction");
}

/* Test case 1: gang partitioned - array distributed across gangs */
void test_gang_partitioned() {
    printf("Testing case 1 (gang partitioned)...\n");
    int arr[N], result[N];
    
    for (int i = 0; i < N; i++) arr[i] = i;
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] * 2;
        }
    }
    
    for (int i = 0; i < N; i++) {
        CHECK(result[i] == arr[i] * 2, "gang partitioned computation");
    }
}

/* Test case 2: worker partitioned - worker-level distribution */
void test_worker_partitioned() {
    printf("Testing case 2 (worker partitioned)...\n");
    int arr[N], result[N];
    
    for (int i = 0; i < N; i++) arr[i] = i;
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N]) num_workers(4)
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] + 1;
        }
    }
    
    for (int i = 0; i < N; i++) {
        CHECK(result[i] == arr[i] + 1, "worker partitioned computation");
    }
}

/* Test case 3: gang+worker partitioned - nested gang/worker distribution */
void test_gang_worker_partitioned() {
    printf("Testing case 3 (gang+worker partitioned)...\n");
    int arr[M][N], result[M][N];
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr[i][j] = i * N + j;
        }
    }
    
    #pragma acc parallel copyin(arr[0:M][0:N]) copyout(result[0:M][0:N])
    {
        #pragma acc loop gang worker collapse(2)
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < N; j++) {
                result[i][j] = arr[i][j] * 3;
            }
        }
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            CHECK(result[i][j] == arr[i][j] * 3, "gang+worker partitioned computation");
        }
    }
}

/* Test case 4: vector partitioned - vector-level SIMD operations */
void test_vector_partitioned() {
    printf("Testing case 4 (vector partitioned)...\n");
    float arr[N], result[N];
    
    for (int i = 0; i < N; i++) arr[i] = i * 0.5f;
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N]) vector_length(32)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] * 2.0f;
        }
    }
    
    for (int i = 0; i < N; i++) {
        CHECK(result[i] == arr[i] * 2.0f, "vector partitioned computation");
    }
}

/* Test case 5: gang+vector partitioned - gang and vector without workers */
void test_gang_vector_partitioned() {
    printf("Testing case 5 (gang+vector partitioned)...\n");
    int arr[N], result[N];
    
    for (int i = 0; i < N; i++) arr[i] = i;
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] * 4;
        }
    }
    
    for (int i = 0; i < N; i++) {
        CHECK(result[i] == arr[i] * 4, "gang+vector partitioned computation");
    }
}

/* Test case 6: worker+vector partitioned - worker and vector combination */
void test_worker_vector_partitioned() {
    printf("Testing case 6 (worker+vector partitioned)...\n");
    float arr[N], result[N];
    
    for (int i = 0; i < N; i++) arr[i] = i * 1.5f;
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N]) num_workers(4) vector_length(16)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] / 2.0f;
        }
    }
    
    for (int i = 0; i < N; i++) {
        CHECK(result[i] == arr[i] / 2.0f, "worker+vector partitioned computation");
    }
}

/* Test case 7: fully partitioned - gang, worker, and vector all active */
void test_fully_partitioned() {
    printf("Testing case 7 (fully partitioned)...\n");
    int arr[M][N][P], result[M][N][P];
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            for (int k = 0; k < P; k++) {
                arr[i][j][k] = i * N * P + j * P + k;
            }
        }
    }
    
    #pragma acc parallel copyin(arr[0:M][0:N][0:P]) copyout(result[0:M][0:N][0:P])
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < N; j++) {
                for (int k = 0; k < P; k++) {
                    result[i][j][k] = arr[i][j][k] + 100;
                }
            }
        }
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            for (int k = 0; k < P; k++) {
                CHECK(result[i][j][k] == arr[i][j][k] + 100, "fully partitioned computation");
            }
        }
    }
}

/* Additional test with runtime parameters to influence partitioning */
void test_runtime_partitioning() {
    printf("Testing runtime partitioning decisions...\n");
    int size = N;
    int arr[N], result[N];
    
    for (int i = 0; i < size; i++) arr[i] = i;
    
    /* Runtime gang count */
    int gang_count = 8;
    #pragma acc parallel copyin(arr[0:size]) copyout(result[0:size]) num_gangs(gang_count)
    {
        #pragma acc loop gang
        for (int i = 0; i < size; i++) {
            result[i] = arr[i] * 2;
        }
    }
    
    for (int i = 0; i < size; i++) {
        CHECK(result[i] == arr[i] * 2, "runtime gang partitioning");
    }
}

/* Test with different data clauses to influence partitioning */
void test_data_clause_variations() {
    printf("Testing data clause variations...\n");
    
    /* Test with present clause (simulating already resident data) */
    int arr[N], result[N];
    
    for (int i = 0; i < N; i++) arr[i] = i;
    
    #pragma acc enter data copyin(arr[0:N])
    
    #pragma acc parallel present(arr[0:N]) copyout(result[0:N])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] * 3;
        }
    }
    
    #pragma acc exit data delete(arr[0:N])
    
    for (int i = 0; i < N; i++) {
        CHECK(result[i] == arr[i] * 3, "present clause partitioning");
    }
    
    /* Test with private variables */
    int private_var = 10;
    int sum = 0;
    
    #pragma acc parallel reduction(+:sum) private(private_var)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            private_var = i % 5;
            sum += private_var;
        }
    }
    
    printf("  Private variable test sum: %d\n", sum);
}

/* Test triangular/non-rectangular iteration space */
void test_triangular_loop() {
    printf("Testing triangular loop space...\n");
    int arr[M][M], result[M][M];
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr[i][j] = i * M + j;
        }
    }
    
    #pragma acc parallel copyin(arr[0:M][0:M]) copyout(result[0:M][0:M])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < M; i++) {
            #pragma acc loop vector
            for (int j = 0; j <= i; j++) {  /* Triangular pattern */
                result[i][j] = arr[i][j] * 2;
            }
        }
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j <= i; j++) {
            CHECK(result[i][j] == arr[i][j] * 2, "triangular loop partitioning");
        }
    }
}

/* Main test driver */
int main() {
    printf("OpenACC Partition Mapping Test Program\n");
    printf("======================================\n");
    
    /* Execute all partition test cases */
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    
    /* Additional tests for coverage */
    test_runtime_partitioning();
    test_data_clause_variations();
    test_triangular_loop();
    
    printf("\n======================================\n");
    if (errors == 0) {
        printf("All tests PASSED\n");
        return 0;
    } else {
        printf("%d test(s) FAILED\n", errors);
        return 1;
    }
}
