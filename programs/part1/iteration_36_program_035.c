/* test_partition_cases.c - Exercise GCC's OpenMP/OpenACC partition mapping logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <openacc.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Prevent optimization */
volatile int force_runtime = 1;

/* Helper to use results and prevent elimination */
static void use(void *p) {
    asm volatile("" : : "r"(p) : "memory");
}

/* Test functions for each partition type */
void test_gang_redundant(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 0: Single gang, redundant across gangs */
    #pragma acc parallel copy(data[0:n]) num_gangs(1) num_workers(1) vector_length(1)
    {
        int idx = 0;
        #pragma acc loop gang
        for (int g = 0; g < 1; g++) {
            #pragma acc loop worker
            for (int w = 0; w < 1; w++) {
                #pragma acc loop vector
                for (int v = 0; v < n; v++) {
                    if (v < n) data[v] += 1;
                }
            }
        }
    }
    
    /* Verify and potentially trigger diagnostic */
    int errors = 0;
    for (int i = 0; i < n; i++) {
        if (data[i] != i + 1) errors++;
    }
    if (errors > 0) {
        /* Could trigger partition string usage in diagnostics */
        fprintf(stderr, "Gang redundant test: %d errors\n", errors);
    }
    
    use(data);
    free(data);
}

void test_gang_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    int gangs = (force_runtime) ? 4 : 2;
    
    for (int i = 0; i < n; i++) data[i] = 0;
    
    /* Case 1: Multiple gangs, partitioned across gangs */
    #pragma acc parallel copy(data[0:n]) num_gangs(gangs) num_workers(1) vector_length(1)
    {
        #pragma acc loop gang independent
        for (int g = 0; g < gangs; g++) {
            int start = (n / gangs) * g;
            int end = (g == gangs - 1) ? n : (n / gangs) * (g + 1);
            #pragma acc loop worker
            for (int i = start; i < end; i++) {
                data[i] = g + 1;
            }
        }
    }
    
    /* Runtime check that might use partition info */
    int valid = 1;
    for (int i = 0; i < n; i++) {
        int expected_gang = (i / (n / gangs)) + 1;
        if (expected_gang > gangs) expected_gang = gangs;
        if (data[i] != expected_gang) valid = 0;
    }
    
    if (!valid) {
        /* Potential diagnostic output */
        fprintf(stderr, "Gang partitioned validation failed\n");
    }
    
    use(data);
    free(data);
}

void test_worker_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    int workers = (force_runtime) ? 8 : 4;
    
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 2: Single gang, multiple workers */
    #pragma acc parallel copy(data[0:n]) num_gangs(1) num_workers(workers) vector_length(1)
    {
        #pragma acc loop gang
        for (int g = 0; g < 1; g++) {
            #pragma acc loop worker independent
            for (int w = 0; w < workers; w++) {
                int start = (n / workers) * w;
                int end = (w == workers - 1) ? n : (n / workers) * (w + 1);
                for (int i = start; i < end; i++) {
                    data[i] *= 2;
                }
            }
        }
    }
    
    use(data);
    free(data);
}

void test_gang_worker_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    int gangs = (force_runtime) ? 2 : 1;
    int workers = (force_runtime) ? 4 : 2;
    
    for (int i = 0; i < n; i++) data[i] = 0;
    
    /* Case 3: Both gang and worker partitioned */
    #pragma acc parallel copy(data[0:n]) num_gangs(gangs) num_workers(workers) vector_length(1)
    {
        #pragma acc loop gang independent
        for (int g = 0; g < gangs; g++) {
            #pragma acc loop worker independent
            for (int w = 0; w < workers; w++) {
                int idx = g * workers + w;
                if (idx < n) {
                    data[idx] = (g << 16) | w;
                }
            }
        }
    }
    
    use(data);
    free(data);
}

void test_vector_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    int vector_len = (force_runtime) ? 32 : 16;
    
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 4: Vector partitioned only */
    #pragma acc parallel copy(data[0:n]) num_gangs(1) num_workers(1) vector_length(vector_len)
    {
        #pragma acc loop gang
        for (int g = 0; g < 1; g++) {
            #pragma acc loop worker
            for (int w = 0; w < 1; w++) {
                #pragma acc loop vector independent
                for (int v = 0; v < n; v++) {
                    data[v] += v;
                }
            }
        }
    }
    
    use(data);
    free(data);
}

void test_gang_vector_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    int gangs = (force_runtime) ? 4 : 2;
    int vector_len = (force_runtime) ? 16 : 8;
    
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 5: Gang + vector partitioned */
    #pragma acc parallel copy(data[0:n]) num_gangs(gangs) num_workers(1) vector_length(vector_len)
    {
        #pragma acc loop gang independent
        for (int g = 0; g < gangs; g++) {
            #pragma acc loop worker
            for (int w = 0; w < 1; w++) {
                #pragma acc loop vector independent
                for (int v = 0; v < vector_len; v++) {
                    int idx = g * vector_len + v;
                    if (idx < n) {
                        data[idx] += gangs;
                    }
                }
            }
        }
    }
    
    use(data);
    free(data);
}

void test_worker_vector_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    int workers = (force_runtime) ? 4 : 2;
    int vector_len = (force_runtime) ? 8 : 4;
    
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 6: Worker + vector partitioned */
    #pragma acc parallel copy(data[0:n]) num_gangs(1) num_workers(workers) vector_length(vector_len)
    {
        #pragma acc loop gang
        for (int g = 0; g < 1; g++) {
            #pragma acc loop worker independent
            for (int w = 0; w < workers; w++) {
                #pragma acc loop vector independent
                for (int v = 0; v < vector_len; v++) {
                    int idx = w * vector_len + v;
                    if (idx < n) {
                        data[idx] *= 3;
                    }
                }
            }
        }
    }
    
    use(data);
    free(data);
}

void test_fully_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    int gangs = (force_runtime) ? 2 : 1;
    int workers = (force_runtime) ? 2 : 1;
    int vector_len = (force_runtime) ? 4 : 2;
    
    for (int i = 0; i < n; i++) data[i] = 0;
    
    /* Case 7: Fully partitioned (gang + worker + vector) */
    #pragma acc parallel copy(data[0:n]) \
        num_gangs(gangs) num_workers(workers) vector_length(vector_len)
    {
        #pragma acc loop gang independent
        for (int g = 0; g < gangs; g++) {
            #pragma acc loop worker independent
            for (int w = 0; w < workers; w++) {
                #pragma acc loop vector independent
                for (int v = 0; v < vector_len; v++) {
                    int idx = (g * workers * vector_len) + (w * vector_len) + v;
                    if (idx < n) {
                        data[idx] = (g << 24) | (w << 16) | v;
                    }
                }
            }
        }
    }
    
    /* Complex runtime check that could trigger diagnostics */
    int correct = 1;
    for (int g = 0; g < gangs && correct; g++) {
        for (int w = 0; w < workers && correct; w++) {
            for (int v = 0; v < vector_len && correct; v++) {
                int idx = (g * workers * vector_len) + (w * vector_len) + v;
                if (idx < n) {
                    int expected = (g << 24) | (w << 16) | v;
                    if (data[idx] != expected) {
                        correct = 0;
                        /* This error path might use partition strings */
                        fprintf(stderr, "Fully partitioned mismatch at idx %d\n", idx);
                    }
                }
            }
        }
    }
    
    use(data);
    free(data);
}

/* OpenMP equivalents for additional coverage */
void test_omp_partitioning(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    
    /* OpenMP target offload with various partitioning */
    #pragma omp target teams distribute parallel for \
        num_teams(4) thread_limit(32) map(tofrom: data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = omp_get_team_num() * 1000 + omp_get_thread_num();
    }
    
    /* SIMD for vector partitioning */
    #pragma omp target teams distribute parallel for simd \
        num_teams(2) thread_limit(16) simdlen(8) map(tofrom: data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] += i;
    }
    
    use(data);
    free(data);
}

/* Force invalid partition codes through template/macro tricks */
#ifdef __cplusplus
template<int N>
void test_invalid_partition() {
    // This might generate invalid partition codes through constant propagation
    volatile int x = N;
    if (x < 0 || x > 7) {
        // Could trigger default case
        fprintf(stderr, "Invalid partition code: %d\n", x);
    }
}
#endif

/* Boundary case testing */
void test_boundary_cases() {
    /* Force generation of various partition codes through conditional compilation */
    #ifdef TEST_ALL_CASES
    for (int p = 0; p <= 8; p++) {  // Includes invalid case 8
        switch (p) {
            case 0: case 1: case 2: case 3:
            case 4: case 5: case 6: case 7:
                /* Valid cases handled above */
                break;
            default:
                /* Case 8+ triggers default "<illegal>" */
                fprintf(stderr, "Partition code %d\n", p);
                break;
        }
    }
    #endif
}

int main(int argc, char **argv) {
    int n = 1024;
    if (argc > 1) n = atoi(argv[1]);
    if (n < 64) n = 64;
    
    printf("Testing partition mapping cases with n=%d\n", n);
    
    /* Execute all test cases */
    test_gang_redundant(n);
    printf("Case 0 tested\n");
    
    test_gang_partitioned(n);
    printf("Case 1 tested\n");
    
    test_worker_partitioned(n);
    printf("Case 2 tested\n");
    
    test_gang_worker_partitioned(n);
    printf("Case 3 tested\n");
    
    test_vector_partitioned(n);
    printf("Case 4 tested\n");
    
    test_gang_vector_partitioned(n);
    printf("Case 5 tested\n");
    
    test_worker_vector_partitioned(n);
    printf("Case 6 tested\n");
    
    test_fully_partitioned(n);
    printf("Case 7 tested\n");
    
    /* Additional OpenMP coverage */
    test_omp_partitioning(n);
    printf("OpenMP partitioning tested\n");
    
    /* Boundary/invalid cases */
    test_boundary_cases();
    
    printf("All partition tests completed\n");
    
    return 0;
}

#ifdef __cplusplus
}
#endif
