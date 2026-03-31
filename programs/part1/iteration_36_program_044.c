/* Test program to cover partition mapping switch cases in omp-oacc-neuter-broadcast.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _OPENACC
#include <openacc.h>
#endif
#ifdef _OPENMP
#include <omp.h>
#endif

#define N 1024
#define M 512
#define L 256

/* Prevent optimization */
volatile int force_runtime = 1;
volatile int dummy_result = 0;

/* Function to use results and prevent dead code elimination */
void use_result(int val) {
    asm volatile("" : : "r"(val));
    dummy_result += val;
}

/* Test case 0: gang redundant (single gang) */
void test_gang_redundant() {
    int data[N];
    int i;
    
    for (i = 0; i < N; i++) data[i] = i;
    
    #pragma acc parallel loop gang(num:1) copy(data[0:N])
    for (i = 0; i < N; i++) {
        data[i] += 1;
    }
    
    /* Force runtime evaluation */
    if (force_runtime) {
        int sum = 0;
        for (i = 0; i < N; i++) sum += data[i];
        use_result(sum);
    }
}

/* Test case 1: gang partitioned (multiple gangs) */
void test_gang_partitioned() {
    int data[N];
    int i;
    
    for (i = 0; i < N; i++) data[i] = i;
    
    #pragma acc parallel loop gang(num:4) copy(data[0:N])
    for (i = 0; i < N; i++) {
        data[i] *= 2;
    }
    
    if (force_runtime) {
        int max_val = 0;
        for (i = 0; i < N; i++) {
            if (data[i] > max_val) max_val = data[i];
        }
        use_result(max_val);
    }
}

/* Test case 2: worker partitioned (single gang, multiple workers) */
void test_worker_partitioned() {
    int data[M];
    int i;
    
    for (i = 0; i < M; i++) data[i] = i;
    
    #pragma acc parallel loop gang(num:1) worker(num:4) copy(data[0:M])
    for (i = 0; i < M; i++) {
        data[i] += i % 10;
    }
    
    if (force_runtime) {
        int check = 0;
        for (i = 0; i < M; i++) check ^= data[i];
        use_result(check);
    }
}

/* Test case 3: gang+worker partitioned */
void test_gang_worker_partitioned() {
    int data[N];
    int i;
    
    for (i = 0; i < N; i++) data[i] = i;
    
    #pragma acc parallel loop gang(num:2) worker(num:4) copy(data[0:N])
    for (i = 0; i < N; i++) {
        data[i] = data[i] * 3 + 1;
    }
    
    if (force_runtime) {
        int total = 0;
        #pragma acc parallel loop reduction(+:total) copy(data[0:N])
        for (i = 0; i < N; i++) {
            total += data[i];
        }
        use_result(total);
    }
}

/* Test case 4: vector partitioned */
void test_vector_partitioned() {
    float data[L];
    int i;
    
    for (i = 0; i < L; i++) data[i] = i * 0.5f;
    
    #pragma acc parallel loop vector_length(32) copy(data[0:L])
    for (i = 0; i < L; i++) {
        data[i] = data[i] * data[i];
    }
    
    if (force_runtime) {
        float sum = 0.0f;
        for (i = 0; i < L; i++) sum += data[i];
        use_result((int)sum);
    }
}

/* Test case 5: gang+vector partitioned */
void test_gang_vector_partitioned() {
    double data[N];
    int i;
    
    for (i = 0; i < N; i++) data[i] = i * 0.25;
    
    #pragma acc parallel loop gang(num:2) vector_length(64) copy(data[0:N])
    for (i = 0; i < N; i++) {
        data[i] = 1.0 / (1.0 + data[i]);
    }
    
    if (force_runtime) {
        double prod = 1.0;
        for (i = 0; i < 10; i++) prod *= data[i];
        use_result((int)(prod * 1000));
    }
}

/* Test case 6: worker+vector partitioned */
void test_worker_vector_partitioned() {
    int data[M];
    int i;
    
    for (i = 0; i < M; i++) data[i] = i;
    
    #pragma acc parallel loop gang(num:1) worker(num:2) vector_length(16) copy(data[0:M])
    for (i = 0; i < M; i++) {
        data[i] = (data[i] << 1) | 1;
    }
    
    if (force_runtime) {
        int pattern = 0;
        for (i = 0; i < M; i++) pattern |= data[i];
        use_result(pattern);
    }
}

/* Test case 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned() {
    int data[N];
    int i;
    
    for (i = 0; i < N; i++) data[i] = i;
    
    #pragma acc parallel loop gang(num:2) worker(num:2) vector_length(32) copy(data[0:N])
    for (i = 0; i < N; i++) {
        data[i] = data[i] * data[i] % 1000;
    }
    
    if (force_runtime) {
        /* Nested parallel region with different partitioning */
        #pragma acc parallel loop gang(num:4) worker(num:2) vector_length(16) copy(data[0:N])
        for (i = 0; i < N; i++) {
            data[i] += 1000;
        }
        
        int final_check = 0;
        for (i = 0; i < N; i++) final_check += data[i];
        use_result(final_check);
    }
}

/* OpenMP equivalents to trigger similar partitioning logic */
#ifdef _OPENMP
void test_omp_partitioning() {
    int data[N];
    int i;
    
    for (i = 0; i < N; i++) data[i] = i;
    
    /* Various OpenMP target offload patterns */
    #pragma omp target teams distribute parallel for num_teams(2) thread_limit(64) map(tofrom:data[0:N])
    for (i = 0; i < N; i++) {
        data[i] += omp_get_team_num() * 1000 + omp_get_thread_num();
    }
    
    /* SIMD vector partitioning */
    #pragma omp target teams distribute parallel for simd num_teams(4) map(tofrom:data[0:N])
    for (i = 0; i < N; i++) {
        data[i] *= 2;
    }
    
    if (force_runtime) {
        int verify = 0;
        for (i = 0; i < N; i++) verify ^= data[i];
        use_result(verify);
    }
}
#endif

/* Helper to potentially trigger default case with invalid partition codes */
void test_boundary_cases() {
    /* This might trigger the default case through compiler internals */
    int dynamic_size = force_runtime ? N : M;
    int data[dynamic_size];
    int i;
    
    /* Variable partitioning that might generate edge cases */
    int gangs = force_runtime ? 2 : 1;
    int workers = force_runtime ? 4 : 2;
    int vectors = force_runtime ? 32 : 16;
    
    for (i = 0; i < dynamic_size; i++) data[i] = i;
    
    /* Complex nested directives */
    #pragma acc parallel copy(data[0:dynamic_size])
    {
        #pragma acc loop gang
        for (i = 0; i < dynamic_size; i += 64) {
            #pragma acc loop worker
            for (int j = 0; j < 64 && i + j < dynamic_size; j++) {
                #pragma acc loop vector
                for (int k = 0; k < 8; k++) {
                    int idx = i + j;
                    if (idx < dynamic_size) {
                        data[idx] += k;
                    }
                }
            }
        }
    }
    
    if (force_runtime) {
        int sum = 0;
        for (i = 0; i < dynamic_size; i++) sum += data[i];
        use_result(sum);
    }
}

/* Main test driver */
int main(int argc, char **argv) {
    printf("Testing partition mapping switch cases...\n");
    
    /* Force runtime evaluation */
    if (argc > 1) {
        force_runtime = atoi(argv[1]);
    }
    
    /* Execute all test cases */
    test_gang_redundant();
    printf("  Case 0 (gang redundant) executed\n");
    
    test_gang_partitioned();
    printf("  Case 1 (gang partitioned) executed\n");
    
    test_worker_partitioned();
    printf("  Case 2 (worker partitioned) executed\n");
    
    test_gang_worker_partitioned();
    printf("  Case 3 (gang+worker partitioned) executed\n");
    
    test_vector_partitioned();
    printf("  Case 4 (vector partitioned) executed\n");
    
    test_gang_vector_partitioned();
    printf("  Case 5 (gang+vector partitioned) executed\n");
    
    test_worker_vector_partitioned();
    printf("  Case 6 (worker+vector partitioned) executed\n");
    
    test_fully_partitioned();
    printf("  Case 7 (fully partitioned) executed\n");
    
    #ifdef _OPENMP
    test_omp_partitioning();
    printf("  OpenMP partitioning tests executed\n");
    #endif
    
    test_boundary_cases();
    printf("  Boundary cases executed\n");
    
    printf("All tests completed. dummy_result = %d\n", dummy_result);
    
    return 0;
}
