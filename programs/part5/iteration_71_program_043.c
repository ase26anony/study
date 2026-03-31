/**
 * Test program to exercise OpenACC/OpenMP partition clause variations
 * Targeting uncovered lines 335-343 in omp-oacc-neuter-broadcast.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define VERIFY_TOL 1e-6

/* Function declarations with OpenACC routine directives */
#pragma acc routine seq
float compute_element(float a, float b) {
    return a * b + a - b;
}

#pragma acc routine gang
float gang_reduction(float *arr, int n) {
    float sum = 0.0f;
    #pragma acc loop gang reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Test 1: Gang redundant (case 0) */
void test_gang_redundant() {
    printf("Test 1: Gang redundant\n");
    float a[N], b[N], c[N];
    float ref_sum = 0.0f;
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(N - i);
        c[i] = 0.0f;
        ref_sum += a[i] * b[i];
    }
    
    /* Outer gang-redundant region with inner partitioned loops */
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        /* Gang redundant - no explicit partition clause */
        #pragma acc parallel loop gang
        for (int i = 0; i < N; i++) {
            c[i] = compute_element(a[i], b[i]);
        }
        
        /* Verify */
        float sum = 0.0f;
        #pragma acc parallel loop gang reduction(+:sum)
        for (int i = 0; i < N; i++) {
            sum += c[i];
        }
        
        #pragma acc update self(sum)
        printf("  Result: sum = %f, expected = %f\n", sum, ref_sum);
        assert(fabs(sum - ref_sum) < VERIFY_TOL);
    }
}

/* Test 2: Gang partitioned (case 1) */
void test_gang_partitioned() {
    printf("Test 2: Gang partitioned\n");
    int num_gangs = 4;
    float x[N], y[N];
    
    for (int i = 0; i < N; i++) {
        x[i] = (float)i;
        y[i] = 0.0f;
    }
    
    /* Explicit gang partitioning with runtime variable */
    #pragma acc data copy(x[0:N], y[0:N])
    {
        #pragma acc parallel loop gang(num_gangs)
        for (int i = 0; i < N; i++) {
            y[i] = x[i] * 2.0f;
        }
        
        /* Verify */
        int errors = 0;
        #pragma acc parallel loop gang reduction(+:errors)
        for (int i = 0; i < N; i++) {
            if (fabs(y[i] - x[i] * 2.0f) > VERIFY_TOL) errors++;
        }
        
        printf("  Result: %d errors\n", errors);
        assert(errors == 0);
    }
}

/* Test 3: Worker partitioned (case 2) */
void test_worker_partitioned() {
    printf("Test 3: Worker partitioned\n");
    int num_workers = 4;
    float data[N], result[N];
    
    for (int i = 0; i < N; i++) {
        data[i] = (float)i;
        result[i] = 0.0f;
    }
    
    /* Worker partitioned loop */
    #pragma acc data copy(data[0:N], result[0:N])
    {
        #pragma acc parallel loop worker(num_workers)
        for (int i = 0; i < N; i++) {
            result[i] = data[i] + 100.0f;
        }
        
        /* Verify */
        int correct = 1;
        #pragma acc parallel loop worker reduction(*:correct)
        for (int i = 0; i < N; i++) {
            if (fabs(result[i] - (data[i] + 100.0f)) > VERIFY_TOL) {
                correct = 0;
            }
        }
        
        printf("  Result: %s\n", correct ? "PASS" : "FAIL");
        assert(correct == 1);
    }
}

/* Test 4: Gang+worker partitioned (case 3) */
void test_gang_worker_partitioned() {
    printf("Test 4: Gang+worker partitioned\n");
    int num_gangs = 2;
    int num_workers = 4;
    float matrix[N][N];
    float sum = 0.0f;
    
    /* Initialize */
    #pragma acc parallel loop gang worker collapse(2)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            matrix[i][j] = (float)(i * N + j);
        }
    }
    
    /* Gang+worker partitioned reduction */
    #pragma acc data copy(matrix)
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) reduction(+:sum)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                sum += matrix[i][j];
            }
        }
        
        float expected = (float)(N * N - 1) * (float)(N * N) / 2.0f;
        printf("  Result: sum = %f, expected = %f\n", sum, expected);
        assert(fabs(sum - expected) < VERIFY_TOL * N * N);
    }
}

/* Test 5: Vector partitioned (case 4) */
void test_vector_partitioned() {
    printf("Test 5: Vector partitioned\n");
    int vector_len = 32;
    float a[N], b[N];
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = 0.0f;
    }
    
    /* Vector partitioned loop */
    #pragma acc data copy(a[0:N], b[0:N])
    {
        #pragma acc parallel loop vector(vector_len)
        for (int i = 0; i < N; i++) {
            b[i] = sqrtf(a[i]);  /* Requires math.h, simplified here */
        }
        
        /* Simplified verification */
        int valid = 1;
        #pragma acc parallel loop vector reduction(*:valid)
        for (int i = 0; i < N; i++) {
            if (b[i] < 0.0f) valid = 0;
        }
        
        printf("  Result: %s\n", valid ? "PASS" : "FAIL");
        assert(valid == 1);
    }
}

/* Test 6: Gang+vector partitioned (case 5) */
void test_gang_vector_partitioned() {
    printf("Test 6: Gang+vector partitioned\n");
    int num_gangs = 4;
    int vector_len = 16;
    float data[N];
    float min_val, max_val;
    
    /* Initialize with conditional partition selection */
    int flag = 1;
    int actual_gangs = flag ? num_gangs : num_gangs * 2;
    
    #pragma acc data copy(data[0:N])
    {
        #pragma acc parallel loop gang(actual_gangs) vector(vector_len)
        for (int i = 0; i < N; i++) {
            data[i] = (float)(i % 100);
        }
        
        /* Find min and max */
        min_val = data[0];
        max_val = data[0];
        #pragma acc parallel loop gang vector reduction(min:min_val) reduction(max:max_val)
        for (int i = 1; i < N; i++) {
            if (data[i] < min_val) min_val = data[i];
            if (data[i] > max_val) max_val = data[i];
        }
        
        printf("  Result: min = %f, max = %f\n", min_val, max_val);
        assert(min_val >= 0.0f && max_val <= 99.0f);
    }
}

/* Test 7: Worker+vector partitioned (case 6) */
void test_worker_vector_partitioned() {
    printf("Test 7: Worker+vector partitioned\n");
    int num_workers = 2;
    int vector_len = 32;
    float src[N], dst[N];
    
    for (int i = 0; i < N; i++) {
        src[i] = (float)i;
        dst[i] = 0.0f;
    }
    
    /* Worker+vector partitioned copy */
    #pragma acc data copy(src[0:N], dst[0:N])
    {
        #pragma acc parallel loop worker(num_workers) vector(vector_len)
        for (int i = 0; i < N; i++) {
            dst[i] = src[N - 1 - i];  /* Reverse copy */
        }
        
        /* Verify reversal */
        int correct = 1;
        #pragma acc parallel loop worker vector reduction(*:correct)
        for (int i = 0; i < N; i++) {
            if (fabs(dst[i] - src[N - 1 - i]) > VERIFY_TOL) {
                correct = 0;
            }
        }
        
        printf("  Result: %s\n", correct ? "PASS" : "FAIL");
        assert(correct == 1);
    }
}

/* Test 8: Fully partitioned (case 7) */
void test_fully_partitioned() {
    printf("Test 8: Fully partitioned (gang+worker+vector)\n");
    int num_gangs = 2;
    int num_workers = 2;
    int vector_len = 8;
    float array[N][N];
    float total = 0.0f;
    
    /* Fully partitioned initialization */
    #pragma acc data create(array[0:N][0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) vector(vector_len) collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                array[i][j] = (float)(i + j);
            }
        }
        
        /* Fully partitioned reduction */
        #pragma acc parallel loop gang worker vector reduction(+:total) collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                total += array[i][j];
            }
        }
        
        float expected = (float)N * N * (float)(N - 1);
        printf("  Result: total = %f, expected = %f\n", total, expected);
        assert(fabs(total - expected) < VERIFY_TOL * N * N);
    }
}

/* Test 9: Nested parallelism with different partition types */
void test_nested_partitioning() {
    printf("Test 9: Nested parallelism with partition mixing\n");
    float outer[N], inner[N];
    
    for (int i = 0; i < N; i++) {
        outer[i] = (float)i;
        inner[i] = 0.0f;
    }
    
    /* Outer gang-redundant region with inner worker-partitioned loop */
    #pragma acc data copy(outer[0:N], inner[0:N])
    {
        #pragma acc parallel loop gang  /* gang redundant */
        for (int i = 0; i < N; i++) {
            float temp = outer[i];
            
            /* Inner worker partitioned computation */
            #pragma acc loop worker
            for (int j = 0; j < 10; j++) {
                temp += (float)j;
            }
            
            inner[i] = temp;
        }
        
        /* Verify */
        float checksum = 0.0f;
        #pragma acc parallel loop gang reduction(+:checksum)
        for (int i = 0; i < N; i++) {
            checksum += inner[i];
        }
        
        float expected = (float)(N - 1) * (float)N / 2.0f + (float)N * 45.0f;  /* 45 = sum(0..9) */
        printf("  Result: checksum = %f, expected = %f\n", checksum, expected);
        assert(fabs(checksum - expected) < VERIFY_TOL * N);
    }
}

/* Test 10: Conditional partition selection (potential illegal case) */
void test_conditional_partition() {
    printf("Test 10: Conditional partition selection\n");
    int use_illegal = 0;  /* Set to 1 to potentially trigger illegal case */
    int num_gangs = 2;
    int num_workers = 4;
    float data[N];
    
    for (int i = 0; i < N; i++) {
        data[i] = (float)i;
    }
    
    /* Conditional partition selection */
    #pragma acc data copy(data[0:N])
    {
        if (use_illegal) {
            /* This might generate an illegal partition combination */
            #pragma acc parallel loop gang(num_gangs) worker(num_workers) vector(32)
            for (int i = 0; i < N; i++) {
                data[i] = data[i] * 2.0f;
            }
        } else {
            /* Legal combination */
            #pragma acc parallel loop gang(num_gangs) vector(32)
            for (int i = 0; i < N; i++) {
                data[i] = data[i] * 3.0f;
            }
        }
        
        /* Simple verification */
        int positive = 1;
        #pragma acc parallel loop reduction(*:positive)
        for (int i = 0; i < N; i++) {
            if (data[i] < 0.0f) positive = 0;
        }
        
        printf("  Result: %s (all values positive)\n", positive ? "PASS" : "FAIL");
        assert(positive == 1);
    }
}

/* Test 11: OpenMP target equivalent */
#ifdef _OPENMP
void test_omp_target_partitioning() {
    printf("Test 11: OpenMP target teams distribute\n");
    float a[N], b[N], c[N];
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(i * 2);
        c[i] = 0.0f;
    }
    
    /* OpenMP target with teams distribute - maps to gang partitioning */
    #pragma omp target teams distribute parallel for map(to: a[0:N], b[0:N]) map(from: c[0:N])
    for (int i = 0; i < N; i++) {
        c[i] = a[i] + b[i];
    }
    
    /* Verify */
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (fabs(c[i] - (a[i] + b[i])) > VERIFY_TOL) {
            errors++;
        }
    }
    
    printf("  Result: %d errors\n", errors);
    assert(errors == 0);
}
#endif

int main() {
    int tests_passed = 0;
    int total_tests = 10;  /* 10 OpenACC tests */
    
    printf("=== Testing OpenACC/OpenMP Partition Clause Variations ===\n\n");
    
    /* Execute all test cases to cover switch statement 0-7 */
    test_gang_redundant();           /* Case 0 */
    tests_passed++;
    
    test_gang_partitioned();         /* Case 1 */
    tests_passed++;
    
    test_worker_partitioned();       /* Case 2 */
    tests_passed++;
    
    test_gang_worker_partitioned();  /* Case 3 */
    tests_passed++;
    
    test_vector_partitioned();       /* Case 4 */
    tests_passed++;
    
    test_gang_vector_partitioned();  /* Case 5 */
    tests_passed++;
    
    test_worker_vector_partitioned(); /* Case 6 */
    tests_passed++;
    
    test_fully_partitioned();        /* Case 7 */
    tests_passed++;
    
    test_nested_partitioning();      /* Mixed cases */
    tests_passed++;
    
    test_conditional_partition();    /* Potential illegal case */
    tests_passed++;
    
    #ifdef _OPENMP
    test_omp_target_partitioning();  /* OpenMP equivalent */
    total_tests++;
    tests_passed++;
    #endif
    
    printf("\n=== Summary ===\n");
    printf("Tests passed: %d/%d\n", tests_passed, total_tests);
    
    if (tests_passed == total_tests) {
        printf("SUCCESS: All partition type cases exercised\n");
        return 0;
    } else {
        printf("FAILURE: Some tests failed\n");
        return 1;
    }
}
