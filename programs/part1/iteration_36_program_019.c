/* test_partition_cases.c - Exercise all partition mapping cases */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <openacc.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Force compiler to generate partition mapping logic */
#define FORCE_USED(x) asm volatile("" : : "r"(x))
#define PREVENT_OPTIMIZATION __attribute__((noinline,used))

/* Test functions for each partition type */
PREVENT_OPTIMIZATION
void test_gang_redundant(int *data, int n) {
    volatile int force_runtime = n;
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) copy(data[0:n])
    {
        int idx = 0;
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            if (force_runtime > 0) {
                data[i] += 1;  /* Simple computation */
            }
        }
    }
    
    /* Force partition logic generation */
    if (force_runtime < 0) {
        /* This path never taken but forces compiler to consider all cases */
        const char *desc = "<illegal>";
        FORCE_USED(desc);
    }
}

PREVENT_OPTIMIZATION
void test_gang_partitioned(int *data, int n) {
    volatile int gangs = 4;
    #pragma acc parallel num_gangs(gangs) num_workers(1) vector_length(1) copy(data[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            data[i] = acc_gang_id(1) + 1;
        }
    }
}

PREVENT_OPTIMIZATION
void test_worker_partitioned(int *data, int n) {
    volatile int workers = 4;
    #pragma acc parallel num_gangs(1) num_workers(workers) vector_length(1) copy(data[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            data[i] = acc_worker_id(1) + 1;
        }
    }
}

PREVENT_OPTIMIZATION
void test_gang_worker_partitioned(int *data, int n) {
    volatile int gangs = 2, workers = 4;
    #pragma acc parallel num_gangs(gangs) num_workers(workers) vector_length(1) copy(data[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            data[i] = acc_gang_id(1) * 100 + acc_worker_id(1);
        }
    }
}

PREVENT_OPTIMIZATION
void test_vector_partitioned(int *data, int n) {
    volatile int vec_len = 32;
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(vec_len) copy(data[0:n])
    {
        #pragma acc loop vector
        for (int i = 0; i < n; i++) {
            data[i] = acc_vector_id(1) + 1;
        }
    }
}

PREVENT_OPTIMIZATION
void test_gang_vector_partitioned(int *data, int n) {
    volatile int gangs = 4, vec_len = 16;
    #pragma acc parallel num_gangs(gangs) num_workers(1) vector_length(vec_len) copy(data[0:n])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < n; i++) {
            data[i] = acc_gang_id(1) * 1000 + acc_vector_id(1);
        }
    }
}

PREVENT_OPTIMIZATION
void test_worker_vector_partitioned(int *data, int n) {
    volatile int workers = 2, vec_len = 8;
    #pragma acc parallel num_gangs(1) num_workers(workers) vector_length(vec_len) copy(data[0:n])
    {
        #pragma acc loop worker vector
        for (int i = 0; i < n; i++) {
            data[i] = acc_worker_id(1) * 100 + acc_vector_id(1);
        }
    }
}

PREVENT_OPTIMIZATION
void test_fully_partitioned(int *data, int n) {
    volatile int gangs = 2, workers = 2, vec_len = 4;
    #pragma acc parallel num_gangs(gangs) num_workers(workers) vector_length(vec_len) copy(data[0:n])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            data[i] = acc_gang_id(1) * 10000 + acc_worker_id(1) * 100 + acc_vector_id(1);
        }
    }
}

/* OpenMP equivalents to trigger different partition mappings */
PREVENT_OPTIMIZATION
void test_omp_gang_redundant(int *data, int n) {
    volatile int force = n;
    #pragma omp target teams num_teams(1) thread_limit(1) map(tofrom: data[0:n])
    {
        #pragma omp distribute parallel for simd
        for (int i = 0; i < n; i++) {
            if (force > 0) {
                data[i] += 1000;
            }
        }
    }
}

PREVENT_OPTIMIZATION
void test_omp_gang_partitioned(int *data, int n) {
    volatile int teams = 4;
    #pragma omp target teams num_teams(teams) thread_limit(1) map(tofrom: data[0:n])
    {
        #pragma omp distribute
        for (int i = 0; i < n; i++) {
            data[i] = omp_get_team_num() + 2000;
        }
    }
}

/* Combined test with runtime selection */
PREVENT_OPTIMIZATION
void test_partition_by_code(int *data, int n, int partition_code) {
    /* This mimics the switch logic in the compiler */
    switch (partition_code) {
        case 0:
            test_gang_redundant(data, n);
            break;
        case 1:
            test_gang_partitioned(data, n);
            break;
        case 2:
            test_worker_partitioned(data, n);
            break;
        case 3:
            test_gang_worker_partitioned(data, n);
            break;
        case 4:
            test_vector_partitioned(data, n);
            break;
        case 5:
            test_gang_vector_partitioned(data, n);
            break;
        case 6:
            test_worker_vector_partitioned(data, n);
            break;
        case 7:
            test_fully_partitioned(data, n);
            break;
        default:
            /* This should trigger the default case "<illegal>" */
            if (partition_code < 0 || partition_code > 7) {
                /* Force error path that might use the illegal string */
                volatile int *p = NULL;
                if (partition_code == 999) {  /* Never true */
                    *p = 42;  /* Potential crash - forces compiler to consider */
                }
            }
            break;
    }
}

/* Template/macro approach for C++ */
#ifdef __cplusplus
template<int PartitionCode>
void test_template_partition(int *data, int n) {
    volatile int code = PartitionCode;
    if (code == 0) {
        #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) copy(data[0:n])
        { for (int i = 0; i < n; i++) data[i] += 1; }
    } else if (code == 1) {
        #pragma acc parallel num_gangs(4) num_workers(1) vector_length(1) copy(data[0:n])
        { for (int i = 0; i < n; i++) data[i] += 2; }
    } else if (code == 2) {
        #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) copy(data[0:n])
        { for (int i = 0; i < n; i++) data[i] += 3; }
    } else if (code == 3) {
        #pragma acc parallel num_gangs(2) num_workers(2) vector_length(1) copy(data[0:n])
        { for (int i = 0; i < n; i++) data[i] += 4; }
    } else if (code == 4) {
        #pragma acc parallel num_gangs(1) num_workers(1) vector_length(32) copy(data[0:n])
        { for (int i = 0; i < n; i++) data[i] += 5; }
    } else if (code == 5) {
        #pragma acc parallel num_gangs(2) num_workers(1) vector_length(16) copy(data[0:n])
        { for (int i = 0; i < n; i++) data[i] += 6; }
    } else if (code == 6) {
        #pragma acc parallel num_gangs(1) num_workers(2) vector_length(8) copy(data[0:n])
        { for (int i = 0; i < n; i++) data[i] += 7; }
    } else if (code == 7) {
        #pragma acc parallel num_gangs(2) num_workers(2) vector_length(4) copy(data[0:n])
        { for (int i = 0; i < n; i++) data[i] += 8; }
    }
}
#endif

int main() {
    const int N = 1024;
    int *data = (int*)malloc(N * sizeof(int));
    int *verify = (int*)malloc(N * sizeof(int));
    
    if (!data || !verify) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    printf("Testing all partition mapping cases...\n");
    
    /* Test each partition case systematically */
    for (int case_id = 0; case_id <= 8; case_id++) {  /* Include illegal case */
        /* Initialize data */
        for (int i = 0; i < N; i++) {
            data[i] = i;
            verify[i] = i;
        }
        
        printf("Testing partition case %d: ", case_id);
        
        /* Execute test with this partition code */
        test_partition_by_code(data, N, case_id);
        
        /* Simple verification */
        int errors = 0;
        for (int i = 0; i < N; i++) {
            if (data[i] != verify[i] && case_id <= 7) {
                errors++;
                if (errors < 5) {
                    printf("Mismatch at %d: %d != %d\n", i, data[i], verify[i]);
                }
            }
        }
        
        if (errors > 0 && case_id <= 7) {
            printf("Found %d errors in case %d\n", errors, case_id);
        } else {
            printf("OK\n");
        }
        
        /* Also test OpenMP variants for some cases */
        if (case_id == 0 || case_id == 1) {
            for (int i = 0; i < N; i++) data[i] = i;
            if (case_id == 0) test_omp_gang_redundant(data, N);
            if (case_id == 1) test_omp_gang_partitioned(data, N);
        }
    }
    
    #ifdef __cplusplus
    /* Test template instantiations */
    printf("\nTesting template instantiations...\n");
    for (int i = 0; i < N; i++) data[i] = i;
    test_template_partition<0>(data, N);
    
    for (int i = 0; i < N; i++) data[i] = i;
    test_template_partition<1>(data, N);
    
    for (int i = 0; i < N; i++) data[i] = i;
    test_template_partition<7>(data, N);
    #endif
    
    /* Test with runtime-dependent partitioning */
    printf("\nTesting runtime-dependent partitioning...\n");
    volatile int runtime_choice = 3;  /* Could be changed at runtime */
    for (int iter = 0; iter < 3; iter++) {
        for (int i = 0; i < N; i++) data[i] = i;
        
        if (runtime_choice == 0) {
            test_gang_redundant(data, N);
        } else if (runtime_choice == 1) {
            test_gang_partitioned(data, N);
        } else if (runtime_choice == 2) {
            test_worker_partitioned(data, N);
        } else {
            test_gang_worker_partitioned(data, N);
        }
        
        runtime_choice = (runtime_choice + 1) % 4;
    }
    
    free(data);
    free(verify);
    
    printf("All tests completed.\n");
    return 0;
}

#ifdef __cplusplus
}
#endif
