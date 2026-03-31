/* test_partition_cases.c - Exercise all partition mapping cases in omp-oacc-neuter-broadcast.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <openacc.h>

/* Prevent optimization elimination */
#define KEEP(var) asm volatile("" : : "r"(var))
#define VOLATILE(var) volatile var

/* Debug output that could trigger partition string mapping */
void debug_partition_info(int partition_code) {
    /* This mimics internal compiler logic that maps codes to strings */
    const char* desc;
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
    printf("Partition debug: code=%d, desc=%s\n", partition_code, desc);
}

/* Case 0: Gang redundant (single gang) */
void test_gang_redundant() {
    const int N = 1024;
    VOLATILE(int) sum = 0;
    int* array = (int*)malloc(N * sizeof(int));
    
    /* Initialize with runtime values */
    for(int i = 0; i < N; i++) array[i] = i % 100;
    
    #pragma acc parallel copy(array[0:N]) copy(sum) num_gangs(1) num_workers(1) vector_length(1)
    {
        #pragma acc loop gang
        for(int i = 0; i < N; i++) {
            array[i] += 1;
            if(i == 0) sum = array[i];
        }
    }
    
    /* Force compiler to materialize partition logic */
    KEEP(sum);
    debug_partition_info(0);  /* Could trigger internal mapping */
    free(array);
}

/* Case 1: Gang partitioned (multiple gangs) */
void test_gang_partitioned() {
    const int N = 2048;
    int* array = (int*)malloc(N * sizeof(int));
    VOLATILE(int) check = 0;
    
    for(int i = 0; i < N; i++) array[i] = i;
    
    #pragma acc parallel copy(array[0:N]) num_gangs(8) num_workers(1) vector_length(1)
    {
        #pragma acc loop gang independent
        for(int i = 0; i < N; i++) {
            /* Gang-dependent computation */
            array[i] = array[i] * 2 + (i % 8);
        }
    }
    
    /* Verify and trigger debug */
    for(int i = 0; i < N; i++) check += array[i];
    KEEP(check);
    debug_partition_info(1);
    free(array);
}

/* Case 2: Worker partitioned (single gang, multiple workers) */
void test_worker_partitioned() {
    const int N = 1024;
    float* data = (float*)malloc(N * sizeof(float));
    VOLATILE(float) result = 0.0f;
    
    for(int i = 0; i < N; i++) data[i] = (float)i;
    
    #pragma acc parallel copy(data[0:N]) num_gangs(1) num_workers(4) vector_length(1)
    {
        #pragma acc loop worker independent
        for(int i = 0; i < N; i++) {
            data[i] = data[i] * 3.14f;
        }
    }
    
    for(int i = 0; i < N; i++) result += data[i];
    KEEP(result);
    debug_partition_info(2);
    free(data);
}

/* Case 3: Gang+Worker partitioned */
void test_gang_worker_partitioned() {
    const int N = 4096;
    double* matrix = (double*)malloc(N * sizeof(double));
    VOLATILE(double) total = 0.0;
    
    for(int i = 0; i < N; i++) matrix[i] = (double)i / 100.0;
    
    #pragma acc parallel copy(matrix[0:N]) num_gangs(4) num_workers(8) vector_length(1)
    {
        #pragma acc loop gang worker independent collapse(2)
        for(int i = 0; i < 64; i++) {
            for(int j = 0; j < 64; j++) {
                int idx = i * 64 + j;
                if(idx < N) {
                    matrix[idx] = matrix[idx] * (i + 1) + (j + 1);
                }
            }
        }
    }
    
    for(int i = 0; i < N; i++) total += matrix[i];
    KEEP(total);
    debug_partition_info(3);
    free(matrix);
}

/* Case 4: Vector partitioned */
void test_vector_partitioned() {
    const int N = 512;
    int* vec = (int*)malloc(N * sizeof(int));
    VOLATILE(int) sum = 0;
    
    for(int i = 0; i < N; i++) vec[i] = i * 2;
    
    #pragma acc parallel copy(vec[0:N]) num_gangs(1) num_workers(1) vector_length(32)
    {
        #pragma acc loop vector independent
        for(int i = 0; i < N; i++) {
            vec[i] = vec[i] + (i % 32);
        }
    }
    
    for(int i = 0; i < N; i++) sum += vec[i];
    KEEP(sum);
    debug_partition_info(4);
    free(vec);
}

/* Case 5: Gang+Vector partitioned */
void test_gang_vector_partitioned() {
    const int N = 2048;
    float* data = (float*)malloc(N * sizeof(float));
    VOLATILE(float) checksum = 0.0f;
    
    for(int i = 0; i < N; i++) data[i] = (float)(i % 256);
    
    #pragma acc parallel copy(data[0:N]) num_gangs(8) num_workers(1) vector_length(16)
    {
        #pragma acc loop gang vector independent
        for(int i = 0; i < N; i++) {
            data[i] = data[i] * 2.0f + (float)(i % 16);
        }
    }
    
    for(int i = 0; i < N; i++) checksum += data[i];
    KEEP(checksum);
    debug_partition_info(5);
    free(data);
}

/* Case 6: Worker+Vector partitioned */
void test_worker_vector_partitioned() {
    const int N = 1024;
    double* array = (double*)malloc(N * sizeof(double));
    VOLATILE(double) result = 0.0;
    
    for(int i = 0; i < N; i++) array[i] = (double)i / 50.0;
    
    #pragma acc parallel copy(array[0:N]) num_gangs(1) num_workers(4) vector_length(8)
    {
        #pragma acc loop worker vector independent
        for(int i = 0; i < N; i++) {
            array[i] = array[i] + (double)((i % 8) * 0.1);
        }
    }
    
    for(int i = 0; i < N; i++) result += array[i];
    KEEP(result);
    debug_partition_info(6);
    free(array);
}

/* Case 7: Fully partitioned (Gang+Worker+Vector) */
void test_fully_partitioned() {
    const int N = 4096;
    int* grid = (int*)malloc(N * sizeof(int));
    VOLATILE(int) total = 0;
    
    for(int i = 0; i < N; i++) grid[i] = i;
    
    #pragma acc parallel copy(grid[0:N]) num_gangs(8) num_workers(4) vector_length(16)
    {
        #pragma acc loop gang worker vector independent collapse(2)
        for(int i = 0; i < 64; i++) {
            for(int j = 0; j < 64; j++) {
                int idx = i * 64 + j;
                if(idx < N) {
                    grid[idx] = grid[idx] * 3 + (i % 8) + (j % 16);
                }
            }
        }
    }
    
    for(int i = 0; i < N; i++) total += grid[i];
    KEEP(total);
    debug_partition_info(7);
    free(grid);
}

/* OpenMP equivalents to trigger similar partitioning logic */
void test_omp_partitioning() {
    const int N = 1024;
    int* data = (int*)malloc(N * sizeof(int));
    VOLATILE(int) sum = 0;
    
    for(int i = 0; i < N; i++) data[i] = i;
    
    /* OpenMP target with teams and distribute - triggers partition mapping */
    #pragma omp target teams distribute parallel for map(tofrom: data[0:N]) \
        num_teams(4) thread_limit(32)
    for(int i = 0; i < N; i++) {
        data[i] = data[i] * 2 + omp_get_team_num();
    }
    
    /* SIMD vector partitioning */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: data[0:N]) num_teams(2) simdlen(8)
    for(int i = 0; i < N; i++) {
        data[i] += omp_get_thread_num() % 8;
    }
    
    for(int i = 0; i < N; i++) sum += data[i];
    KEEP(sum);
    free(data);
}

/* Test invalid partition codes through boundary conditions */
void test_invalid_partitions() {
    /* Force compiler to generate default case handling */
    VOLATILE(int) invalid_codes[] = {-1, 8, 255, 1024};
    
    for(int i = 0; i < 4; i++) {
        debug_partition_info(invalid_codes[i]);
    }
    
    /* Template-like macro expansion for different partition schemes */
    #define TEST_PARTITION_SCHEME(code, gangs, workers, vectors) \
        do { \
            int N = 256; \
            int* arr = (int*)malloc(N * sizeof(int)); \
            for(int j = 0; j < N; j++) arr[j] = j; \
            _Pragma("acc parallel copy(arr[0:N]) num_gangs(gangs) num_workers(workers) vector_length(vectors)") \
            { \
                _Pragma("acc loop gang worker vector independent") \
                for(int j = 0; j < N; j++) { \
                    arr[j] += (j % 32); \
                } \
            } \
            VOLATILE(int) s = 0; \
            for(int j = 0; j < N; j++) s += arr[j]; \
            KEEP(s); \
            free(arr); \
        } while(0)
    
    /* Generate various partition combinations */
    TEST_PARTITION_SCHEME(0, 1, 1, 1);
    TEST_PARTITION_SCHEME(1, 4, 1, 1);
    TEST_PARTITION_SCHEME(2, 1, 4, 1);
    TEST_PARTITION_SCHEME(3, 4, 4, 1);
    TEST_PARTITION_SCHEME(4, 1, 1, 32);
    TEST_PARTITION_SCHEME(5, 4, 1, 16);
    TEST_PARTITION_SCHEME(6, 1, 4, 8);
    TEST_PARTITION_SCHEME(7, 4, 4, 8);
}

int main() {
    printf("Testing all partition mapping cases...\n");
    
    /* Execute all partition test cases */
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    
    /* Test OpenMP partitioning */
    test_omp_partitioning();
    
    /* Test invalid/boundary cases */
    test_invalid_partitions();
    
    printf("All partition tests completed.\n");
    
    /* Final check that could trigger partition string usage */
    VOLATILE(int) final_check = 0;
    for(int i = 0; i <= 8; i++) {  /* Includes invalid case 8 */
        if(i == 7) final_check = 1;
        KEEP(final_check);
    }
    
    return 0;
}
