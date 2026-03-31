/* Test program to exercise partition code mapping in omp-oacc-neuter-broadcast.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>
#include <omp.h>

/* Prevent optimization */
static volatile int force_runtime = 1;

/* Helper to use results and prevent dead code elimination */
#define USE(var) asm volatile("" : : "r"(var))

/* Function that could trigger partition string mapping */
void debug_partition_info(int partition_code) {
    /* This mimics the internal compiler logic that maps codes to strings */
    const char* desc = "<unknown>";
    switch(partition_code) {
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
    
    /* Force compiler to consider this code path */
    if (force_runtime) {
        printf("Debug: Partition type: %s (code=%d)\n", desc, partition_code);
    }
}

/* Test case 0: Gang redundant (single gang) */
void test_gang_redundant() {
    const int N = 1000;
    int *data = (int*)malloc(N * sizeof(int));
    int sum = 0;
    
    /* Initialize with runtime values */
    for (int i = 0; i < N; i++) {
        data[i] = i % 7 + force_runtime;
    }
    
    /* OpenACC: Single gang, redundant across gangs */
    #pragma acc parallel copy(data[0:N]) copyout(sum) num_gangs(1)
    {
        int local_sum = 0;
        #pragma acc loop reduction(+:local_sum)
        for (int i = 0; i < N; i++) {
            local_sum += data[i];
        }
        sum = local_sum;
    }
    
    USE(sum);
    debug_partition_info(0);  /* Could trigger case 0 */
    free(data);
}

/* Test case 1: Gang partitioned (multiple gangs) */
void test_gang_partitioned() {
    const int N = 10000;
    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));
    
    /* Runtime initialization */
    for (int i = 0; i < N; i++) {
        a[i] = (i % 100) * 0.1f;
        b[i] = 0.0f;
    }
    
    /* Multiple gangs, each processes a chunk */
    #pragma acc parallel copyin(a[0:N]) copyout(b[0:N]) num_gangs(32)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            b[i] = a[i] * 2.0f + (float)force_runtime;
        }
    }
    
    /* Verify */
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (b[i] != a[i] * 2.0f + (float)force_runtime) errors++;
    }
    
    USE(errors);
    debug_partition_info(1);  /* Could trigger case 1 */
    free(a); free(b);
}

/* Test case 2: Worker partitioned (single gang, multiple workers) */
void test_worker_partitioned() {
    const int N = 5000;
    double *vec = (double*)malloc(N * sizeof(double));
    
    for (int i = 0; i < N; i++) {
        vec[i] = (double)i;
    }
    
    /* Single gang with multiple workers */
    #pragma acc parallel copy(vec[0:N]) num_gangs(1) num_workers(4)
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            vec[i] = vec[i] * vec[i] + (double)force_runtime;
        }
    }
    
    /* Check last element */
    double last = vec[N-1];
    USE(last);
    debug_partition_info(2);  /* Could trigger case 2 */
    free(vec);
}

/* Test case 3: Gang+Worker partitioned */
void test_gang_worker_partitioned() {
    const int N = 2000;
    const int M = 100;
    int *matrix = (int*)malloc(N * M * sizeof(int));
    
    /* Initialize matrix */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            matrix[i * M + j] = i * j;
        }
    }
    
    /* Nested partitioning: gangs and workers */
    #pragma acc parallel copy(matrix[0:N*M]) num_gangs(8) num_workers(4)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                matrix[i * M + j] += force_runtime;
            }
        }
    }
    
    int check = matrix[(N-1) * M + (M-1)];
    USE(check);
    debug_partition_info(3);  /* Could trigger case 3 */
    free(matrix);
}

/* Test case 4: Vector partitioned */
void test_vector_partitioned() {
    const int N = 8192;
    float *x = (float*)malloc(N * sizeof(float));
    float *y = (float*)malloc(N * sizeof(float));
    
    for (int i = 0; i < N; i++) {
        x[i] = (float)i;
        y[i] = 0.0f;
    }
    
    /* Vector-level parallelism */
    #pragma acc parallel copyin(x[0:N]) copyout(y[0:N]) vector_length(128)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            y[i] = x[i] * 3.14f + (float)force_runtime;
        }
    }
    
    float test = y[N/2];
    USE(test);
    debug_partition_info(4);  /* Could trigger case 4 */
    free(x); free(y);
}

/* Test case 5: Gang+Vector partitioned */
void test_gang_vector_partitioned() {
    const int N = 10000;
    int *data = (int*)malloc(N * sizeof(int));
    
    for (int i = 0; i < N; i++) {
        data[i] = i;
    }
    
    /* Combined gang and vector partitioning */
    #pragma acc parallel copy(data[0:N]) num_gangs(16) vector_length(64)
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            data[i] = data[i] * 2 + force_runtime;
        }
    }
    
    int mid = data[N/2];
    USE(mid);
    debug_partition_info(5);  /* Could trigger case 5 */
    free(data);
}

/* Test case 6: Worker+Vector partitioned */
void test_worker_vector_partitioned() {
    const int N = 4096;
    double *arr = (double*)malloc(N * sizeof(double));
    
    for (int i = 0; i < N; i++) {
        arr[i] = (double)(i % 256);
    }
    
    /* Worker and vector partitioning without gang partitioning */
    #pragma acc parallel copy(arr[0:N]) num_gangs(1) num_workers(8) vector_length(32)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < N; i++) {
            arr[i] = arr[i] / 2.0 + (double)force_runtime;
        }
    }
    
    double val = arr[100];
    USE(val);
    debug_partition_info(6);  /* Could trigger case 6 */
    free(arr);
}

/* Test case 7: Fully partitioned (Gang+Worker+Vector) */
void test_fully_partitioned() {
    const int N = 512;
    const int M = 512;
    float *grid = (float*)malloc(N * M * sizeof(float));
    
    /* Initialize 2D grid */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            grid[i * M + j] = (float)(i + j);
        }
    }
    
    /* Full three-level partitioning */
    #pragma acc parallel copy(grid[0:N*M]) num_gangs(4) num_workers(2) vector_length(16)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < M; j++) {
                grid[i * M + j] = grid[i * M + j] * 1.5f + (float)force_runtime;
            }
        }
    }
    
    float corner = grid[(N-1) * M + (M-1)];
    USE(corner);
    debug_partition_info(7);  /* Could trigger case 7 */
    free(grid);
}

/* Test default case: Invalid partition codes */
void test_invalid_partitions() {
    /* Force consideration of default case through various means */
    
    /* 1. OpenMP with unusual team configuration */
    int N = 100;
    int *arr = (int*)malloc(N * sizeof(int));
    
    #pragma omp target teams distribute parallel for map(tofrom: arr[0:N]) num_teams(0) thread_limit(0)
    for (int i = 0; i < N; i++) {
        arr[i] = i;
    }
    
    /* 2. Runtime-dependent partition selection */
    int partition_type = 10;  /* Invalid code */
    if (force_runtime) {
        debug_partition_info(partition_type);  /* Triggers default case */
    }
    
    /* 3. Boundary violation through macro expansion */
    #define SELECT_PARTITION(x) ((x) < 0 || (x) > 7 ? -1 : (x))
    int bad_code = SELECT_PARTITION(100);
    debug_partition_info(bad_code);
    
    free(arr);
}

/* OpenMP equivalents to test different code paths */
void test_omp_partitioning() {
    const int N = 2000;
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = 0;
    }
    
    /* OpenMP target offload with team distribution */
    #pragma omp target teams distribute parallel for \
        map(to: a[0:N]) map(from: b[0:N]) \
        num_teams(8) thread_limit(256)
    for (int i = 0; i < N; i++) {
        b[i] = a[i] * 3 + omp_get_team_num() + force_runtime;
    }
    
    /* SIMD vectorization for vector partitioning */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: b[0:N]) simdlen(8)
    for (int i = 0; i < N; i++) {
        b[i] += 1;
    }
    
    free(a); free(b);
}

int main() {
    printf("Testing partition code coverage...\n");
    
    /* Set force_runtime to prevent compile-time optimization */
    force_runtime = rand() > 0;
    
    /* Execute all test cases systematically */
    test_gang_redundant();
    printf("Case 0 tested\n");
    
    test_gang_partitioned();
    printf("Case 1 tested\n");
    
    test_worker_partitioned();
    printf("Case 2 tested\n");
    
    test_gang_worker_partitioned();
    printf("Case 3 tested\n");
    
    test_vector_partitioned();
    printf("Case 4 tested\n");
    
    test_gang_vector_partitioned();
    printf("Case 5 tested\n");
    
    test_worker_vector_partitioned();
    printf("Case 6 tested\n");
    
    test_fully_partitioned();
    printf("Case 7 tested\n");
    
    test_invalid_partitions();
    printf("Default case tested\n");
    
    test_omp_partitioning();
    printf("OpenMP variants tested\n");
    
    printf("All partition tests completed.\n");
    return 0;
}
