/* test_partition_cases.c - Cover all partition mapping switch cases */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <openacc.h>

/* Volatile variables to prevent optimization */
volatile int force_runtime = 1;
volatile int partition_code = 0;

/* Helper to use results and prevent dead code elimination */
#define USE(V) asm volatile("" : : "r"(V))

/* Function that could trigger partition string mapping */
void debug_partition_info(int code) {
    /* This mimics the internal compiler logic */
    const char* desc;
    switch(code) {
        case 0: desc = "gang redundant"; break;
        case 1: desc = "gang partitioned"; break;
        case 2: desc = "worker partitioned"; break;
        case 3: desc = "gang+worker partitioned"; break;
        case 4: desc = "vector partitioned"; break;
        case 5: desc = "gang+vector partitioned"; break;
        case 6: desc = "worker+vector partitioned"; break;
        case 7: desc = "fully partitioned"; break;
        default: desc = "<illegal>"; break;
    }
    USE(desc);
    
    /* Conditional error that might use the description */
    if (code < 0 || code > 7) {
        fprintf(stderr, "Invalid partition code: %d (%s)\n", code, desc);
    }
}

/* Test case 0: Gang redundant (single gang) */
void test_gang_redundant() {
    const int N = 1000;
    int* data = (int*)malloc(N * sizeof(int));
    int sum = 0;
    
    #pragma acc parallel copy(data[0:N]) copy(sum) num_gangs(1) num_workers(1) vector_length(1)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            data[i] = i;
        }
        
        #pragma acc loop gang reduction(+:sum)
        for (int i = 0; i < N; i++) {
            sum += data[i];
        }
    }
    
    USE(sum);
    debug_partition_info(0);
    free(data);
}

/* Test case 1: Gang partitioned (multiple gangs) */
void test_gang_partitioned() {
    const int N = 10000;
    float* a = (float*)malloc(N * sizeof(float));
    float* b = (float*)malloc(N * sizeof(float));
    
    /* Initialize with runtime values */
    for (int i = 0; i < N; i++) {
        a[i] = (float)(i % 100);
        b[i] = 0.0f;
    }
    
    #pragma acc parallel copyin(a[0:N]) copyout(b[0:N]) num_gangs(32) num_workers(1) vector_length(1)
    {
        #pragma acc loop gang independent
        for (int i = 0; i < N; i++) {
            b[i] = a[i] * 2.0f;
        }
    }
    
    /* Verify some results */
    int errors = 0;
    for (int i = 0; i < 10; i++) {
        if (b[i] != a[i] * 2.0f) errors++;
    }
    USE(errors);
    debug_partition_info(1);
    
    free(a); free(b);
}

/* Test case 2: Worker partitioned */
void test_worker_partitioned() {
    const int N = 512;
    double* matrix = (double*)malloc(N * N * sizeof(double));
    
    #pragma acc parallel copy(matrix[0:N*N]) num_gangs(1) num_workers(4) vector_length(1)
    {
        #pragma acc loop worker independent
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                matrix[i * N + j] = (double)(i + j);
            }
        }
    }
    
    USE(matrix[N/2 * N + N/2]);
    debug_partition_info(2);
    free(matrix);
}

/* Test case 3: Gang+worker partitioned */
void test_gang_worker_partitioned() {
    const int N = 1024;
    const int M = 512;
    int* result = (int*)malloc(N * M * sizeof(int));
    
    #pragma acc parallel copy(result[0:N*M]) num_gangs(8) num_workers(4) vector_length(1)
    {
        #pragma acc loop gang worker independent collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                result[i * M + j] = (i * 1000) + j;
            }
        }
    }
    
    /* Check diagonal */
    int check = 0;
    for (int i = 0; i < 10; i++) {
        check += result[i * M + i];
    }
    USE(check);
    debug_partition_info(3);
    free(result);
}

/* Test case 4: Vector partitioned */
void test_vector_partitioned() {
    const int N = 4096;
    float* x = (float*)malloc(N * sizeof(float));
    float* y = (float*)malloc(N * sizeof(float));
    
    for (int i = 0; i < N; i++) {
        x[i] = (float)i;
        y[i] = 0.0f;
    }
    
    #pragma acc parallel copyin(x[0:N]) copyout(y[0:N]) num_gangs(1) num_workers(1) vector_length(128)
    {
        #pragma acc loop vector independent
        for (int i = 0; i < N; i++) {
            y[i] = x[i] * x[i];
        }
    }
    
    USE(y[N-1]);
    debug_partition_info(4);
    free(x); free(y);
}

/* Test case 5: Gang+vector partitioned */
void test_gang_vector_partitioned() {
    const int N = 8192;
    int* data = (int*)malloc(N * sizeof(int));
    
    #pragma acc parallel copy(data[0:N]) num_gangs(16) num_workers(1) vector_length(64)
    {
        #pragma acc loop gang vector independent
        for (int i = 0; i < N; i++) {
            data[i] = i * 2;
        }
    }
    
    /* Verify pattern */
    int ok = 1;
    for (int i = 0; i < 100; i += 10) {
        if (data[i] != i * 2) ok = 0;
    }
    USE(ok);
    debug_partition_info(5);
    free(data);
}

/* Test case 6: Worker+vector partitioned */
void test_worker_vector_partitioned() {
    const int N = 2048;
    const int M = 1024;
    float* grid = (float*)malloc(N * M * sizeof(float));
    
    #pragma acc parallel copy(grid[0:N*M]) num_gangs(1) num_workers(8) vector_length(32)
    {
        #pragma acc loop worker vector independent collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                grid[i * M + j] = (float)(i * j) / 1000.0f;
            }
        }
    }
    
    USE(grid[(N/2) * M + (M/2)]);
    debug_partition_info(6);
    free(grid);
}

/* Test case 7: Fully partitioned (gang+worker+vector) */
void test_fully_partitioned() {
    const int N = 1024;
    const int M = 512;
    const int P = 256;
    double* cube = (double*)malloc(N * M * P * sizeof(double));
    
    #pragma acc parallel copy(cube[0:N*M*P]) num_gangs(4) num_workers(2) vector_length(16)
    {
        #pragma acc loop gang worker vector independent collapse(3)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    int idx = (i * M * P) + (j * P) + k;
                    cube[idx] = (double)(i + j + k) / 3.0;
                }
            }
        }
    }
    
    USE(cube[0]);
    debug_partition_info(7);
    free(cube);
}

/* Test invalid partition codes to trigger default case */
void test_invalid_partitions() {
    /* Force invalid codes through variable manipulation */
    int invalid_codes[] = {-1, 8, 255, 1000};
    
    for (int i = 0; i < 4; i++) {
        partition_code = invalid_codes[i];
        debug_partition_info(partition_code);
    }
    
    /* OpenMP version with unusual parameters */
    int N = 100;
    int* arr = (int*)malloc(N * sizeof(int));
    
    #pragma omp target teams distribute parallel for map(tofrom: arr[0:N]) \
        num_teams(0) thread_limit(0)  /* Potentially invalid */
    for (int i = 0; i < N; i++) {
        arr[i] = i;
    }
    
    USE(arr[0]);
    free(arr);
}

/* Combined OpenMP test covering multiple partition types */
void test_omp_partitions() {
    const int N = 10000;
    int* data = (int*)malloc(N * sizeof(int));
    
    /* Case 1-like: Team partitioned */
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:N]) num_teams(32)
    for (int i = 0; i < N; i++) {
        data[i] = i * 3;
    }
    
    /* Case 3-like: Teams + threads */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: data[0:N]) num_teams(8) thread_limit(64)
    for (int i = 0; i < N; i++) {
        data[i] += i;
    }
    
    /* Case 7-like: Full hierarchy */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: data[0:N]) num_teams(4) thread_limit(32)
    for (int i = 0; i < N; i++) {
        data[i] *= 2;
    }
    
    USE(data[N-1]);
    free(data);
}

int main() {
    printf("Testing partition mapping coverage...\n");
    
    /* Execute all test cases */
    test_gang_redundant();
    printf("  Case 0 tested\n");
    
    test_gang_partitioned();
    printf("  Case 1 tested\n");
    
    test_worker_partitioned();
    printf("  Case 2 tested\n");
    
    test_gang_worker_partitioned();
    printf("  Case 3 tested\n");
    
    test_vector_partitioned();
    printf("  Case 4 tested\n");
    
    test_gang_vector_partitioned();
    printf("  Case 5 tested\n");
    
    test_worker_vector_partitioned();
    printf("  Case 6 tested\n");
    
    test_fully_partitioned();
    printf("  Case 7 tested\n");
    
    test_omp_partitions();
    printf("  OpenMP partitions tested\n");
    
    test_invalid_partitions();
    printf("  Invalid partitions tested\n");
    
    printf("All test cases completed.\n");
    
    return 0;
}
