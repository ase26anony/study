/* test_partition_cases.c - Exercise all partition mapping cases in omp-oacc-neuter-broadcast.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>
#ifdef _OPENMP
#include <omp.h>
#endif

/* Prevent optimization */
volatile int force_runtime = 1;
#define NO_OPTIMIZE(var) asm volatile("" : : "r"(var))

/* Helper to generate debug output that might use partition strings */
void debug_partition_info(int partition_code) {
    /* This function's implementation would be in the compiler runtime */
    /* We simulate it to show where partition strings would be used */
    if (force_runtime > 1000) { /* Always false at compile time */
        printf("Partition: %d\n", partition_code);
    }
}

/* Test case 0: Gang redundant (single gang) */
void test_gang_redundant(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    #pragma acc parallel copy(data[0:n]) num_gangs(1) num_workers(1) vector_length(1)
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            data[i] += 1;
        }
    }
    
    /* Verify */
    int errors = 0;
    for (int i = 0; i < n; i++) {
        if (data[i] != i + 1) errors++;
    }
    NO_OPTIMIZE(errors);
    free(data);
}

/* Test case 1: Gang partitioned */
void test_gang_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    #pragma acc parallel copy(data[0:n]) num_gangs(4) num_workers(1) vector_length(1)
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            data[i] *= 2;
        }
    }
    
    NO_OPTIMIZE(data[n/2]);
    free(data);
}

/* Test case 2: Worker partitioned */
void test_worker_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    #pragma acc parallel copy(data[0:n]) num_gangs(1) num_workers(4) vector_length(1)
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            data[i] += i % 10;
        }
    }
    
    NO_OPTIMIZE(data[n/3]);
    free(data);
}

/* Test case 3: Gang+worker partitioned */
void test_gang_worker_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    #pragma acc parallel copy(data[0:n]) num_gangs(2) num_workers(2) vector_length(1)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            data[i] = data[i] * 3 + 1;
        }
    }
    
    NO_OPTIMIZE(data[n/4]);
    free(data);
}

/* Test case 4: Vector partitioned */
void test_vector_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    #pragma acc parallel copy(data[0:n]) num_gangs(1) num_workers(1) vector_length(128)
    {
        #pragma acc loop vector
        for (int i = 0; i < n; i++) {
            data[i] -= 5;
        }
    }
    
    NO_OPTIMIZE(data[n-1]);
    free(data);
}

/* Test case 5: Gang+vector partitioned */
void test_gang_vector_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    #pragma acc parallel copy(data[0:n]) num_gangs(4) num_workers(1) vector_length(64)
    {
        #pragma acc loop gang vector
        for (int i = 0; i < n; i++) {
            data[i] = (data[i] << 1) | 0x1;
        }
    }
    
    NO_OPTIMIZE(data[0]);
    free(data);
}

/* Test case 6: Worker+vector partitioned */
void test_worker_vector_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    #pragma acc parallel copy(data[0:n]) num_gangs(1) num_workers(2) vector_length(32)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < n; i++) {
            data[i] ^= 0xFF;
        }
    }
    
    NO_OPTIMIZE(data[n/2]);
    free(data);
}

/* Test case 7: Fully partitioned */
void test_fully_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    #pragma acc parallel copy(data[0:n]) num_gangs(8) num_workers(4) vector_length(16)
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            data[i] = ~data[i];
        }
    }
    
    NO_OPTIMIZE(data[n-1]);
    free(data);
}

/* OpenMP equivalents to trigger similar partitioning logic */
#ifdef _OPENMP
void test_omp_partitioning(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* This should generate various partition codes through OMP mapping */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: data[0:n]) num_teams(4) thread_limit(32)
    for (int i = 0; i < n; i++) {
        data[i] += omp_get_team_num() + omp_get_thread_num();
    }
    
    /* Nested with different clauses */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: data[0:n]) num_teams(2) num_threads(16)
    for (int i = 0; i < n; i++) {
        data[i] *= 2;
    }
    
    NO_OPTIMIZE(data[0]);
    free(data);
}
#endif

/* Generate invalid partition codes for default case */
void test_invalid_partitions(int n) {
    /* Force generation of partition codes through macro expansion */
    #define GEN_PARTITION_CODE(x) \
        if (force_runtime > 1000) { \
            debug_partition_info(x); \
        }
    
    /* Generate codes outside valid range */
    for (int i = 8; i < 12; i++) {
        GEN_PARTITION_CODE(i)
    }
    
    /* Variable partition code that could be out of range */
    int dynamic_code = n * 2;
    if (dynamic_code > 7 && dynamic_code < 20) {
        GEN_PARTITION_CODE(dynamic_code)
    }
}

/* Template-like macro to generate different partition schemes */
#define GENERATE_PARTITION_TEST(NAME, GANGS, WORKERS, VECTORS) \
void NAME(int n) { \
    int *data = (int*)malloc(n * sizeof(int)); \
    for (int i = 0; i < n; i++) data[i] = i; \
    \
    #pragma acc parallel copy(data[0:n]) \
        num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTORS) \
    { \
        if (GANGS > 1 && WORKERS > 1 && VECTORS > 1) { \
            #pragma acc loop gang worker vector \
            for (int i = 0; i < n; i++) { \
                data[i] = data[i] * 7 - 3; \
            } \
        } else if (GANGS > 1 && VECTORS > 1) { \
            #pragma acc loop gang vector \
            for (int i = 0; i < n; i++) { \
                data[i] ^= 0xAA; \
            } \
        } else if (WORKERS > 1 && VECTORS > 1) { \
            #pragma acc loop worker vector \
            for (int i = 0; i < n; i++) { \
                data[i] |= 0x55; \
            } \
        } else if (GANGS > 1) { \
            #pragma acc loop gang \
            for (int i = 0; i < n; i++) { \
                data[i] += GANGS; \
            } \
        } \
    } \
    NO_OPTIMIZE(data[n/2]); \
    free(data); \
}

/* Instantiate template tests */
GENERATE_PARTITION_TEST(test_template_1, 1, 1, 1)    /* Case 0 */
GENERATE_PARTITION_TEST(test_template_2, 4, 1, 1)    /* Case 1 */
GENERATE_PARTITION_TEST(test_template_3, 1, 4, 1)    /* Case 2 */
GENERATE_PARTITION_TEST(test_template_4, 2, 2, 1)    /* Case 3 */
GENERATE_PARTITION_TEST(test_template_5, 1, 1, 128)  /* Case 4 */
GENERATE_PARTITION_TEST(test_template_6, 4, 1, 64)   /* Case 5 */
GENERATE_PARTITION_TEST(test_template_7, 1, 2, 32)   /* Case 6 */
GENERATE_PARTITION_TEST(test_template_8, 8, 4, 16)   /* Case 7 */

int main(int argc, char **argv) {
    int n = 1000;
    if (argc > 1) n = atoi(argv[1]);
    if (n < 100) n = 100;
    
    printf("Testing partition mapping cases...\n");
    
    /* Test all OpenACC partition cases */
    test_gang_redundant(n);
    test_gang_partitioned(n);
    test_worker_partitioned(n);
    test_gang_worker_partitioned(n);
    test_vector_partitioned(n);
    test_gang_vector_partitioned(n);
    test_worker_vector_partitioned(n);
    test_fully_partitioned(n);
    
    /* Test template-generated cases */
    test_template_1(n);
    test_template_2(n);
    test_template_3(n);
    test_template_4(n);
    test_template_5(n);
    test_template_6(n);
    test_template_7(n);
    test_template_8(n);
    
#ifdef _OPENMP
    test_omp_partitioning(n);
#endif
    
    /* Test invalid codes for default case */
    test_invalid_partitions(n);
    
    /* Complex nested computation with runtime-dependent partitioning */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = n - i;
    }
    
    /* Runtime condition forces different partition schemes */
    for (int mode = 0; mode < 8; mode++) {
        if (force_runtime & (1 << mode)) {
            #pragma acc parallel copy(a[0:n], b[0:n])
            {
                if (mode == 0) {
                    #pragma acc loop gang
                    for (int i = 0; i < n; i++) a[i] += b[i];
                } else if (mode == 1) {
                    #pragma acc loop gang
                    for (int i = 0; i < n; i++) a[i] -= b[i];
                } else if (mode == 2) {
                    #pragma acc loop worker
                    for (int i = 0; i < n; i++) a[i] *= b[i];
                } else if (mode == 3) {
                    #pragma acc loop gang worker
                    for (int i = 0; i < n; i++) a[i] &= b[i];
                } else if (mode == 4) {
                    #pragma acc loop vector
                    for (int i = 0; i < n; i++) a[i] |= b[i];
                } else if (mode == 5) {
                    #pragma acc loop gang vector
                    for (int i = 0; i < n; i++) a[i] ^= b[i];
                } else if (mode == 6) {
                    #pragma acc loop worker vector
                    for (int i = 0; i < n; i++) a[i] = (a[i] << 1) + b[i];
                } else if (mode == 7) {
                    #pragma acc loop gang worker vector
                    for (int i = 0; i < n; i++) a[i] = ~(a[i] + b[i]);
                }
            }
        }
    }
    
    NO_OPTIMIZE(a[0]);
    NO_OPTIMIZE(b[0]);
    
    free(a);
    free(b);
    
    printf("Partition testing complete.\n");
    return 0;
}
