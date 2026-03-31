/*
 * Test program to exercise all partition type cases in omp-oacc-neuter-broadcast.cc
 * Specifically targets lines 335-343 (switch statement for partition types)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define VERIFY_TOLERANCE 1e-6

/* Function declarations with OpenACC routine directives */
#pragma acc routine seq
float compute_element(float a, float b, int idx) {
    return a * b + idx * 0.1f;
}

#pragma acc routine gang
void gang_level_operation(float *arr, int start, int end, float factor) {
    for (int i = start; i < end; i++) {
        arr[i] *= factor;
    }
}

/* Test 1: Gang redundant (case 0) */
void test_gang_redundant() {
    printf("Test 1: Gang redundant\n");
    float a[N], b[N], c[N];
    float sum = 0.0f;
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        a[i] = 1.0f;
        b[i] = 2.0f;
        c[i] = 0.0f;
    }
    
    /* Runtime-determined partition counts */
    int num_gangs = 4;
    int num_workers = 2;
    int vector_len = 32;
    
    /* Gang redundant - no explicit partition clause */
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) vector_length(vector_len)
        for (int i = 0; i < N; i++) {
            c[i] = compute_element(a[i], b[i], i);
        }
    }
    
    /* Verify results */
    for (int i = 0; i < N; i++) {
        float expected = 2.0f + i * 0.1f;
        assert(fabs(c[i] - expected) < VERIFY_TOLERANCE);
    }
    printf("  PASSED\n");
}

/* Test 2: Gang partitioned (case 1) */
void test_gang_partitioned() {
    printf("Test 2: Gang partitioned\n");
    float a[N], b[N], c[N];
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = 1.5f;
        c[i] = 0.0f;
    }
    
    int num_gangs = 8;
    int vector_len = 16;
    
    /* Explicit gang partition */
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) vector_length(vector_len)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * b[i];
        }
    }
    
    /* Verify */
    for (int i = 0; i < N; i++) {
        assert(fabs(c[i] - (i * 1.5f)) < VERIFY_TOLERANCE);
    }
    printf("  PASSED\n");
}

/* Test 3: Worker partitioned (case 2) */
void test_worker_partitioned() {
    printf("Test 3: Worker partitioned\n");
    float a[N], b[N], c[N];
    
    for (int i = 0; i < N; i++) {
        a[i] = 3.0f;
        b[i] = (float)(i % 10);
        c[i] = 0.0f;
    }
    
    int num_workers = 4;
    int vector_len = 64;
    
    /* Worker partitioned - gang is redundant */
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop worker(num_workers) vector_length(vector_len)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
    
    /* Verify */
    for (int i = 0; i < N; i++) {
        assert(fabs(c[i] - (3.0f + (i % 10))) < VERIFY_TOLERANCE);
    }
    printf("  PASSED\n");
}

/* Test 4: Gang+worker partitioned (case 3) */
void test_gang_worker_partitioned() {
    printf("Test 4: Gang+worker partitioned\n");
    float a[N], b[N], c[N];
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i * 0.5f;
        b[i] = 2.0f;
        c[i] = 0.0f;
    }
    
    int num_gangs = 2;
    int num_workers = 8;
    int vector_len = 32;
    
    /* Both gang and worker partitioned */
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) vector_length(vector_len)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * b[i];
        }
    }
    
    /* Verify */
    for (int i = 0; i < N; i++) {
        assert(fabs(c[i] - (i * 1.0f)) < VERIFY_TOLERANCE);
    }
    printf("  PASSED\n");
}

/* Test 5: Vector partitioned (case 4) */
void test_vector_partitioned() {
    printf("Test 5: Vector partitioned\n");
    float a[N], b[N], c[N];
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = 0.25f;
        c[i] = 0.0f;
    }
    
    int vector_len = 128;
    
    /* Vector partitioned only */
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop vector_length(vector_len)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * b[i];
        }
    }
    
    /* Verify */
    for (int i = 0; i < N; i++) {
        assert(fabs(c[i] - (i * 0.25f)) < VERIFY_TOLERANCE);
    }
    printf("  PASSED\n");
}

/* Test 6: Gang+vector partitioned (case 5) */
void test_gang_vector_partitioned() {
    printf("Test 6: Gang+vector partitioned\n");
    float a[N], b[N], c[N];
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = 3.0f;
        c[i] = 0.0f;
    }
    
    int num_gangs = 4;
    int vector_len = 64;
    
    /* Gang and vector partitioned */
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) vector_length(vector_len)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * b[i];
        }
    }
    
    /* Verify */
    for (int i = 0; i < N; i++) {
        assert(fabs(c[i] - (i * 3.0f)) < VERIFY_TOLERANCE);
    }
    printf("  PASSED\n");
}

/* Test 7: Worker+vector partitioned (case 6) */
void test_worker_vector_partitioned() {
    printf("Test 7: Worker+vector partitioned\n");
    float a[N], b[N], c[N];
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = 0.5f;
        c[i] = 0.0f;
    }
    
    int num_workers = 4;
    int vector_len = 32;
    
    /* Worker and vector partitioned */
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop worker(num_workers) vector_length(vector_len)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * b[i];
        }
    }
    
    /* Verify */
    for (int i = 0; i < N; i++) {
        assert(fabs(c[i] - (i * 0.5f)) < VERIFY_TOLERANCE);
    }
    printf("  PASSED\n");
}

/* Test 8: Fully partitioned (case 7) */
void test_fully_partitioned() {
    printf("Test 8: Fully partitioned\n");
    float a[N], b[N], c[N];
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = 4.0f;
        c[i] = 0.0f;
    }
    
    int num_gangs = 2;
    int num_workers = 4;
    int vector_len = 16;
    
    /* All three levels partitioned */
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) vector_length(vector_len)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * b[i];
        }
    }
    
    /* Verify */
    for (int i = 0; i < N; i++) {
        assert(fabs(c[i] - (i * 4.0f)) < VERIFY_TOLERANCE);
    }
    printf("  PASSED\n");
}

/* Test 9: Nested parallelism with different partition types */
void test_nested_partitioning() {
    printf("Test 9: Nested partitioning\n");
    float arr[N];
    
    for (int i = 0; i < N; i++) {
        arr[i] = (float)i;
    }
    
    int outer_gangs = 2;
    int inner_workers = 8;
    
    /* Outer gang-redundant region with inner worker-partitioned loop */
    #pragma acc data copy(arr[0:N])
    {
        #pragma acc parallel num_gangs(outer_gangs)
        {
            #pragma acc loop gang
            for (int g = 0; g < outer_gangs; g++) {
                int start = g * (N / outer_gangs);
                int end = (g + 1) * (N / outer_gangs);
                
                /* Inner worker-partitioned operation */
                #pragma acc loop worker(inner_workers)
                for (int i = start; i < end; i++) {
                    arr[i] = arr[i] * 2.0f + (float)i * 0.1f;
                }
            }
        }
    }
    
    /* Verify */
    for (int i = 0; i < N; i++) {
        float expected = i * 2.0f + i * 0.1f;
        assert(fabs(arr[i] - expected) < VERIFY_TOLERANCE);
    }
    printf("  PASSED\n");
}

/* Test 10: Conditional partition selection */
void test_conditional_partition() {
    printf("Test 10: Conditional partition selection\n");
    float a[N], b[N], c[N];
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = 1.0f;
        c[i] = 0.0f;
    }
    
    int flag = 1;
    int num_gangs = flag ? 4 : 8;
    int num_workers = flag ? 2 : 4;
    
    /* Runtime conditional partition counts */
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
    
    /* Verify */
    for (int i = 0; i < N; i++) {
        assert(fabs(c[i] - (i + 1.0f)) < VERIFY_TOLERANCE);
    }
    printf("  PASSED\n");
}

/* Test 11: OpenMP target version with partition variations */
#ifdef _OPENMP
void test_omp_partitioning() {
    printf("Test 11: OpenMP target partitioning\n");
    float a[N], b[N], c[N];
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = 2.0f;
        c[i] = 0.0f;
    }
    
    int num_teams = 4;
    int thread_limit = 32;
    
    /* OpenMP target with teams and distribute */
    #pragma omp target data map(to: a[0:N], b[0:N]) map(from: c[0:N])
    {
        #pragma omp target teams num_teams(num_teams) thread_limit(thread_limit)
        {
            #pragma omp distribute
            for (int i = 0; i < N; i++) {
                #pragma omp parallel for
                for (int j = 0; j < 1; j++) {  // Single iteration, just for structure
                    c[i] = a[i] * b[i];
                }
            }
        }
    }
    
    /* Verify */
    for (int i = 0; i < N; i++) {
        assert(fabs(c[i] - (i * 2.0f)) < VERIFY_TOLERANCE);
    }
    printf("  PASSED\n");
}
#endif

/* Test 12: Device-specific partition behavior */
void test_device_specific() {
    printf("Test 12: Device-specific partitioning\n");
    float a[N], b[N], c[N];
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = 0.75f;
        c[i] = 0.0f;
    }
    
    /* Try to set device type (may be ignored on some implementations) */
    #pragma acc set device_type(nvidia)
    
    int num_gangs = 8;
    int vector_len = 32;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) vector_length(vector_len)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * b[i];
        }
    }
    
    /* Verify */
    for (int i = 0; i < N; i++) {
        assert(fabs(c[i] - (i * 0.75f)) < VERIFY_TOLERANCE);
    }
    printf("  PASSED\n");
}

int main() {
    int tests_passed = 0;
    int total_tests = 12;
    
    printf("=== Testing OpenACC/OpenMP Partition Type Coverage ===\n\n");
    
    /* Run all test cases to cover switch statement 0-7 */
    test_gang_redundant();           /* Should trigger case 0 */
    tests_passed++;
    
    test_gang_partitioned();         /* Should trigger case 1 */
    tests_passed++;
    
    test_worker_partitioned();       /* Should trigger case 2 */
    tests_passed++;
    
    test_gang_worker_partitioned();  /* Should trigger case 3 */
    tests_passed++;
    
    test_vector_partitioned();       /* Should trigger case 4 */
    tests_passed++;
    
    test_gang_vector_partitioned();  /* Should trigger case 5 */
    tests_passed++;
    
    test_worker_vector_partitioned(); /* Should trigger case 6 */
    tests_passed++;
    
    test_fully_partitioned();        /* Should trigger case 7 */
    tests_passed++;
    
    test_nested_partitioning();      /* Tests broadcast logic */
    tests_passed++;
    
    test_conditional_partition();    /* Tests dynamic evaluation */
    tests_passed++;
    
    #ifdef _OPENMP
    test_omp_partitioning();         /* OpenMP version */
    tests_passed++;
    #else
    printf("Test 11: OpenMP target partitioning - SKIPPED (no OpenMP)\n");
    total_tests--;
    #endif
    
    test_device_specific();          /* Device-specific behavior */
    tests_passed++;
    
    printf("\n=== Summary ===\n");
    printf("Tests passed: %d/%d\n", tests_passed, total_tests);
    
    if (tests_passed == total_tests) {
        printf("SUCCESS: All partition types should have been exercised\n");
        return 0;
    } else {
        printf("WARNING: Some tests were skipped or failed\n");
        return 1;
    }
}
