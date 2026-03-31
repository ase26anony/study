#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define VERIFY_TOLERANCE 1e-6

// Function to be called with different partition specifications
#pragma acc routine gang
void compute_vector_add(float *a, float *b, float *c, int n) {
    #pragma acc loop gang worker vector
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

// Test all partition type combinations
int main() {
    float *a, *b, *c, *d;
    int i;
    int success_count = 0;
    int total_tests = 0;
    
    // Allocate and initialize arrays
    a = (float*)malloc(N * sizeof(float));
    b = (float*)malloc(N * sizeof(float));
    c = (float*)malloc(N * sizeof(float));
    d = (float*)malloc(N * sizeof(float));
    
    for (i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(i * 2);
        d[i] = a[i] + b[i];  // Expected result
    }
    
    printf("Testing OpenACC/OpenMP partition type coverage...\n");
    
    // Runtime-determined partition counts
    int num_gangs = 2;
    int num_workers = 4;
    int vector_length = 32;
    int flag = 1;
    
    // ============================================
    // TEST 1: gang redundant (case 0)
    // ============================================
    printf("\nTest 1: gang redundant\n");
    total_tests++;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        // Outer region - gang redundant
        #pragma acc parallel loop gang(num_gangs) copy(c[0:N])
        for (i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
    
    // Verify
    int correct = 1;
    for (i = 0; i < N; i++) {
        if (fabs(c[i] - d[i]) > VERIFY_TOLERANCE) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        success_count++;
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
    
    // ============================================
    // TEST 2: gang partitioned (case 1)
    // ============================================
    printf("\nTest 2: gang partitioned\n");
    total_tests++;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        // Explicit gang partitioning
        #pragma acc parallel loop gang(num_gangs) gang
        for (i = 0; i < N; i++) {
            c[i] = a[i] * b[i];
        }
    }
    
    // Verify multiplication
    correct = 1;
    for (i = 0; i < N; i++) {
        if (fabs(c[i] - (a[i] * b[i])) > VERIFY_TOLERANCE) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        success_count++;
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
    
    // ============================================
    // TEST 3: worker partitioned (case 2)
    // ============================================
    printf("\nTest 3: worker partitioned\n");
    total_tests++;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        // Worker partitioned with conditional worker count
        #pragma acc parallel loop worker(num_workers) worker
        for (i = 0; i < N; i++) {
            c[i] = a[i] - b[i];
        }
    }
    
    correct = 1;
    for (i = 0; i < N; i++) {
        if (fabs(c[i] - (a[i] - b[i])) > VERIFY_TOLERANCE) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        success_count++;
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
    
    // ============================================
    // TEST 4: gang+worker partitioned (case 3)
    // ============================================
    printf("\nTest 4: gang+worker partitioned\n");
    total_tests++;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        // Nested parallelism: outer gang, inner worker
        #pragma acc kernels copy(c[0:N])
        {
            #pragma acc loop gang(num_gangs)
            for (int g = 0; g < num_gangs; g++) {
                #pragma acc loop worker(num_workers) worker
                for (i = g * (N/num_gangs); i < (g+1) * (N/num_gangs) && i < N; i++) {
                    c[i] = a[i] / (b[i] + 1.0f);
                }
            }
        }
    }
    
    correct = 1;
    for (i = 0; i < N; i++) {
        if (fabs(c[i] - (a[i] / (b[i] + 1.0f))) > VERIFY_TOLERANCE) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        success_count++;
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
    
    // ============================================
    // TEST 5: vector partitioned (case 4)
    // ============================================
    printf("\nTest 5: vector partitioned\n");
    total_tests++;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        // Vector partitioned with conditional length
        #pragma acc parallel loop vector(vector_length) vector
        for (i = 0; i < N; i++) {
            c[i] = sqrtf(a[i] * a[i] + b[i] * b[i]);
        }
    }
    
    correct = 1;
    for (i = 0; i < N; i++) {
        float expected = sqrtf(a[i] * a[i] + b[i] * b[i]);
        if (fabs(c[i] - expected) > VERIFY_TOLERANCE) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        success_count++;
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
    
    // ============================================
    // TEST 6: gang+vector partitioned (case 5)
    // ============================================
    printf("\nTest 6: gang+vector partitioned\n");
    total_tests++;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        // Combined gang and vector partitioning
        #pragma acc parallel loop gang(num_gangs) vector(vector_length) gang vector
        for (i = 0; i < N; i++) {
            c[i] = a[i] * 2.0f + b[i];
        }
    }
    
    correct = 1;
    for (i = 0; i < N; i++) {
        if (fabs(c[i] - (a[i] * 2.0f + b[i])) > VERIFY_TOLERANCE) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        success_count++;
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
    
    // ============================================
    // TEST 7: worker+vector partitioned (case 6)
    // ============================================
    printf("\nTest 7: worker+vector partitioned\n");
    total_tests++;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        // Worker and vector combined with runtime condition
        #pragma acc parallel loop worker(flag ? 2 : 4) vector(vector_length) worker vector
        for (i = 0; i < N; i++) {
            c[i] = sinf(a[i]) + cosf(b[i]);
        }
    }
    
    correct = 1;
    for (i = 0; i < N; i++) {
        float expected = sinf(a[i]) + cosf(b[i]);
        if (fabs(c[i] - expected) > VERIFY_TOLERANCE) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        success_count++;
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
    
    // ============================================
    // TEST 8: fully partitioned (case 7)
    // ============================================
    printf("\nTest 8: fully partitioned (gang+worker+vector)\n");
    total_tests++;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        // Fully partitioned: gang, worker, and vector
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) vector(vector_length) gang worker vector
        for (i = 0; i < N; i++) {
            c[i] = a[i] + b[i] * 0.5f;
        }
    }
    
    correct = 1;
    for (i = 0; i < N; i++) {
        if (fabs(c[i] - (a[i] + b[i] * 0.5f)) > VERIFY_TOLERANCE) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        success_count++;
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
    
    // ============================================
    // TEST 9: OpenMP target version
    // ============================================
    printf("\nTest 9: OpenMP target with partition variations\n");
    total_tests++;
    
    #pragma omp target data map(to: a[0:N], b[0:N]) map(from: c[0:N])
    {
        // OpenMP target with teams and distribute
        #pragma omp target teams distribute parallel for \
                    num_teams(num_gangs) num_threads(num_workers) \
                    simdlen(vector_length)
        for (i = 0; i < N; i++) {
            c[i] = a[i] * 3.0f - b[i];
        }
    }
    
    correct = 1;
    for (i = 0; i < N; i++) {
        if (fabs(c[i] - (a[i] * 3.0f - b[i])) > VERIFY_TOLERANCE) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        success_count++;
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
    
    // ============================================
    // TEST 10: Function call with partition specification
    // ============================================
    printf("\nTest 10: Function with gang routine\n");
    total_tests++;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) vector(vector_length)
        for (i = 0; i < 1; i++) {  // Single iteration to call function
            compute_vector_add(a, b, c, N);
        }
    }
    
    correct = 1;
    for (i = 0; i < N; i++) {
        if (fabs(c[i] - d[i]) > VERIFY_TOLERANCE) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        success_count++;
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
    
    // ============================================
    // TEST 11: Device-specific behavior
    // ============================================
    printf("\nTest 11: Device-specific partition\n");
    total_tests++;
    
    #pragma acc set device_type(nvidia)
    {
        #pragma acc data copy(a[0:N], b[0:N], c[0:N])
        {
            #pragma acc parallel loop gang(num_gangs) vector(vector_length)
            for (i = 0; i < N; i++) {
                c[i] = expf(a[i] / 100.0f);
            }
        }
    }
    
    correct = 1;
    for (i = 0; i < N; i++) {
        float expected = expf(a[i] / 100.0f);
        if (fabs(c[i] - expected) > VERIFY_TOLERANCE) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        success_count++;
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
    
    // Summary
    printf("\n========================================\n");
    printf("Test Summary: %d/%d tests passed\n", success_count, total_tests);
    printf("Coverage targets:\n");
    printf("  - gang redundant (case 0): Test 1\n");
    printf("  - gang partitioned (case 1): Test 2\n");
    printf("  - worker partitioned (case 2): Test 3\n");
    printf("  - gang+worker partitioned (case 3): Test 4\n");
    printf("  - vector partitioned (case 4): Test 5\n");
    printf("  - gang+vector partitioned (case 5): Test 6\n");
    printf("  - worker+vector partitioned (case 6): Test 7\n");
    printf("  - fully partitioned (case 7): Test 8\n");
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return (success_count == total_tests) ? 0 : 1;
}
