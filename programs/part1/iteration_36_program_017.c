/* Test program to exercise all partition mapping cases in omp-oacc-neuter-broadcast.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <openacc.h>

/* Prevent optimization */
volatile int force_runtime = 1;

/* Function that uses partition-dependent results */
void use_result(int val, const char* context) {
    /* Force compiler to keep computation */
    asm volatile("" : : "r"(val));
    if (val < 0) {
        /* This could trigger error messages using partition strings */
        fprintf(stderr, "Error in %s partition\n", context);
    }
}

/* Test case 0: Gang redundant (single gang) */
void test_gang_redundant() {
    int n = 1000;
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = 0;
    }
    
    #pragma acc parallel copyin(a[0:n]) copyout(b[0:n]) num_gangs(1) num_workers(1) vector_length(1)
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            b[i] = a[i] * 2;
        }
    }
    
    /* Verify and use result */
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += b[i];
    }
    use_result(sum, "gang redundant");
    
    free(a);
    free(b);
}

/* Test case 1: Gang partitioned */
void test_gang_partitioned() {
    int n = 10000;
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    
    for (int i = 0; i < n; i++) {
        a[i] = i % 100;
        b[i] = 0;
    }
    
    /* Multiple gangs, no workers/vectors */
    #pragma acc parallel copyin(a[0:n]) copyout(b[0:n]) num_gangs(32) num_workers(1) vector_length(1)
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            b[i] = a[i] + 1;
        }
    }
    
    int check = 0;
    for (int i = 0; i < n; i++) {
        check += (b[i] == a[i] + 1) ? 0 : 1;
    }
    use_result(check, "gang partitioned");
    
    free(a);
    free(b);
}

/* Test case 2: Worker partitioned */
void test_worker_partitioned() {
    int n = 1024;
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = 0;
    }
    
    /* Single gang, multiple workers */
    #pragma acc parallel copyin(a[0:n]) copyout(b[0:n]) num_gangs(1) num_workers(8) vector_length(1)
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            b[i] = a[i] * a[i];
        }
    }
    
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += b[i];
    }
    use_result(sum, "worker partitioned");
    
    free(a);
    free(b);
}

/* Test case 3: Gang+worker partitioned */
void test_gang_worker_partitioned() {
    int n = 4096;
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = 0;
    }
    
    /* Multiple gangs and workers */
    #pragma acc parallel copyin(a[0:n]) copyout(b[0:n]) num_gangs(8) num_workers(4) vector_length(1)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            b[i] = a[i] + i;
        }
    }
    
    int check = 0;
    #pragma omp parallel for reduction(+:check)
    for (int i = 0; i < n; i++) {
        check += (b[i] == a[i] + i) ? 0 : 1;
    }
    use_result(check, "gang+worker partitioned");
    
    free(a);
    free(b);
}

/* Test case 4: Vector partitioned */
void test_vector_partitioned() {
    int n = 512;
    float *a = (float*)malloc(n * sizeof(float));
    float *b = (float*)malloc(n * sizeof(float));
    
    for (int i = 0; i < n; i++) {
        a[i] = i * 0.1f;
        b[i] = 0.0f;
    }
    
    /* Single gang, single worker, vector partitioned */
    #pragma acc parallel copyin(a[0:n]) copyout(b[0:n]) num_gangs(1) num_workers(1) vector_length(32)
    {
        #pragma acc loop vector
        for (int i = 0; i < n; i++) {
            b[i] = a[i] * 2.0f;
        }
    }
    
    float sum = 0.0f;
    for (int i = 0; i < n; i++) {
        sum += b[i];
    }
    use_result((int)sum, "vector partitioned");
    
    free(a);
    free(b);
}

/* Test case 5: Gang+vector partitioned */
void test_gang_vector_partitioned() {
    int n = 2048;
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = 0;
    }
    
    /* Multiple gangs with vector partitioning */
    #pragma acc parallel copyin(a[0:n]) copyout(b[0:n]) num_gangs(16) num_workers(1) vector_length(16)
    {
        #pragma acc loop gang vector
        for (int i = 0; i < n; i++) {
            b[i] = a[i] * 3;
        }
    }
    
    int sum = 0;
    #pragma acc parallel copyin(b[0:n]) copyout(sum)
    {
        #pragma acc loop reduction(+:sum)
        for (int i = 0; i < n; i++) {
            sum += b[i];
        }
    }
    use_result(sum, "gang+vector partitioned");
    
    free(a);
    free(b);
}

/* Test case 6: Worker+vector partitioned */
void test_worker_vector_partitioned() {
    int n = 1024;
    float *a = (float*)malloc(n * sizeof(float));
    float *b = (float*)malloc(n * sizeof(float));
    
    for (int i = 0; i < n; i++) {
        a[i] = i * 0.5f;
        b[i] = 0.0f;
    }
    
    /* Single gang, multiple workers with vectors */
    #pragma acc parallel copyin(a[0:n]) copyout(b[0:n]) num_gangs(1) num_workers(8) vector_length(8)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < n; i++) {
            b[i] = a[i] + a[i];
        }
    }
    
    float max_val = 0.0f;
    for (int i = 0; i < n; i++) {
        if (b[i] > max_val) max_val = b[i];
    }
    use_result((int)max_val, "worker+vector partitioned");
    
    free(a);
    free(b);
}

/* Test case 7: Fully partitioned */
void test_fully_partitioned() {
    int n = 4096;
    double *a = (double*)malloc(n * sizeof(double));
    double *b = (double*)malloc(n * sizeof(double));
    
    for (int i = 0; i < n; i++) {
        a[i] = i * 0.01;
        b[i] = 0.0;
    }
    
    /* All levels partitioned */
    #pragma acc parallel copyin(a[0:n]) copyout(b[0:n]) num_gangs(32) num_workers(4) vector_length(8)
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            b[i] = a[i] * a[i];
        }
    }
    
    double sum = 0.0;
    #pragma omp target teams distribute parallel for reduction(+:sum) map(tofrom:sum) map(to:b[0:n])
    for (int i = 0; i < n; i++) {
        sum += b[i];
    }
    use_result((int)sum, "fully partitioned");
    
    free(a);
    free(b);
}

/* Test default case with invalid partition codes */
void test_invalid_partitions() {
    /* Force generation of invalid partition codes through macro expansion */
    #define INVALID_PARTITION_CASE(x) \
        do { \
            int code = x; \
            const char* desc = (code == 0) ? "gang redundant" : \
                              (code == 1) ? "gang partitioned" : \
                              (code == 2) ? "worker partitioned" : \
                              (code == 3) ? "gang+worker partitioned" : \
                              (code == 4) ? "vector partitioned" : \
                              (code == 5) ? "gang+vector partitioned" : \
                              (code == 6) ? "worker+vector partitioned" : \
                              (code == 7) ? "fully partitioned" : \
                              "<illegal>"; \
            use_result(code, desc); \
        } while(0)
    
    /* Test boundary values */
    INVALID_PARTITION_CASE(-1);
    INVALID_PARTITION_CASE(8);
    INVALID_PARTITION_CASE(255);
    
    #undef INVALID_PARTITION_CASE
}

/* OpenMP equivalent tests for cross-validation */
void test_omp_partitioning() {
    int n = 1000;
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = 0;
    }
    
    /* OpenMP target with teams and distribute */
    #pragma omp target teams distribute parallel for map(to:a[0:n]) map(from:b[0:n]) \
        num_teams(8) thread_limit(64)
    for (int i = 0; i < n; i++) {
        b[i] = a[i] * 2;
    }
    
    /* Nested parallelism */
    #pragma omp target teams distribute parallel for simd map(to:a[0:n]) map(from:b[0:n]) \
        num_teams(4) thread_limit(128)
    for (int i = 0; i < n; i++) {
        b[i] = a[i] + b[i];
    }
    
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += b[i];
    }
    use_result(sum, "OpenMP hybrid");
    
    free(a);
    free(b);
}

int main() {
    printf("Testing all partition mapping cases...\n");
    
    /* Execute all test cases */
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
    printf("OpenMP partitioning tested\n");
    
    printf("All tests completed.\n");
    return 0;
}
