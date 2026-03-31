#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define VERIFY 1

// Function with explicit gang partitioning for OpenACC routine
#pragma acc routine gang
void acc_gang_routine(int *arr, int idx, int val) {
    arr[idx] = val * 2;
}

// Function with vector partitioning for OpenACC routine  
#pragma acc routine vector
void acc_vector_routine(int *arr, int idx, int val) {
    arr[idx] = val + 1;
}

int main() {
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    int *c = (int*)malloc(N * sizeof(int));
    int *d = (int*)malloc(N * sizeof(int));
    
    // Initialize arrays
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = 2 * i;
        c[i] = 0;
        d[i] = 0;
    }
    
    int success_count = 0;
    int total_tests = 0;
    
    // Runtime partition configuration variables
    int num_gangs = 4;
    int num_workers = 2;
    int vector_len = 32;
    int flag = 1;
    
    printf("Testing OpenACC/OpenMP partition type identification...\n\n");
    
    // ========== TEST 1: Gang Redundant (case 0) ==========
    printf("Test 1: Gang redundant\n");
    #pragma acc data copy(a[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) copy(c[0:N])
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + 1;
        }
    }
    
    #if VERIFY
    int correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != a[i] + 1) correct = 0;
    }
    if (correct) {
        printf("  ✓ PASS\n");
        success_count++;
    } else {
        printf("  ✗ FAIL\n");
    }
    #endif
    total_tests++;
    
    // ========== TEST 2: Gang Partitioned (case 1) ==========
    printf("\nTest 2: Gang partitioned\n");
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) gang
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * b[i];
        }
    }
    
    #if VERIFY
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != a[i] * b[i]) correct = 0;
    }
    if (correct) {
        printf("  ✓ PASS\n");
        success_count++;
    } else {
        printf("  ✗ FAIL\n");
    }
    #endif
    total_tests++;
    
    // ========== TEST 3: Worker Partitioned (case 2) ==========
    printf("\nTest 3: Worker partitioned\n");
    #pragma acc data copy(a[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) worker
        for (int i = 0; i < N; i++) {
            c[i] = a[i] - 5;
        }
    }
    
    #if VERIFY
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != a[i] - 5) correct = 0;
    }
    if (correct) {
        printf("  ✓ PASS\n");
        success_count++;
    } else {
        printf("  ✗ FAIL\n");
    }
    #endif
    total_tests++;
    
    // ========== TEST 4: Gang+Worker Partitioned (case 3) ==========
    printf("\nTest 4: Gang+worker partitioned\n");
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) gang worker
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
    
    #if VERIFY
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != a[i] + b[i]) correct = 0;
    }
    if (correct) {
        printf("  ✓ PASS\n");
        success_count++;
    } else {
        printf("  ✗ FAIL\n");
    }
    #endif
    total_tests++;
    
    // ========== TEST 5: Vector Partitioned (case 4) ==========
    printf("\nTest 5: Vector partitioned\n");
    #pragma acc data copy(a[0:N], c[0:N])
    {
        #pragma acc parallel loop vector_length(vector_len) vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * 3;
        }
    }
    
    #if VERIFY
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != a[i] * 3) correct = 0;
    }
    if (correct) {
        printf("  ✓ PASS\n");
        success_count++;
    } else {
        printf("  ✗ FAIL\n");
    }
    #endif
    total_tests++;
    
    // ========== TEST 6: Gang+Vector Partitioned (case 5) ==========
    printf("\nTest 6: Gang+vector partitioned\n");
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) vector_length(vector_len) gang vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] - b[i];
        }
    }
    
    #if VERIFY
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != a[i] - b[i]) correct = 0;
    }
    if (correct) {
        printf("  ✓ PASS\n");
        success_count++;
    } else {
        printf("  ✗ FAIL\n");
    }
    #endif
    total_tests++;
    
    // ========== TEST 7: Worker+Vector Partitioned (case 6) ==========
    printf("\nTest 7: Worker+vector partitioned\n");
    #pragma acc data copy(a[0:N], c[0:N])
    {
        #pragma acc parallel loop worker(num_workers) vector_length(vector_len) worker vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] / 2;
        }
    }
    
    #if VERIFY
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != a[i] / 2) correct = 0;
    }
    if (correct) {
        printf("  ✓ PASS\n");
        success_count++;
    } else {
        printf("  ✗ FAIL\n");
    }
    #endif
    total_tests++;
    
    // ========== TEST 8: Fully Partitioned (case 7) ==========
    printf("\nTest 8: Fully partitioned\n");
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) vector_length(vector_len) gang worker vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * b[i] + 1;
        }
    }
    
    #if VERIFY
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != a[i] * b[i] + 1) correct = 0;
    }
    if (correct) {
        printf("  ✓ PASS\n");
        success_count++;
    } else {
        printf("  ✗ FAIL\n");
    }
    #endif
    total_tests++;
    
    // ========== TEST 9: Nested Parallelism ==========
    printf("\nTest 9: Nested parallelism with different partitions\n");
    #pragma acc data copy(a[0:N], d[0:N])
    {
        // Outer gang-redundant region
        #pragma acc parallel loop gang(num_gangs)
        for (int i = 0; i < N/2; i++) {
            // Inner worker-partitioned loop
            #pragma acc loop worker
            for (int j = 0; j < 2; j++) {
                int idx = i*2 + j;
                d[idx] = a[idx] * 2;
            }
        }
    }
    
    #if VERIFY
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (d[i] != a[i] * 2) correct = 0;
    }
    if (correct) {
        printf("  ✓ PASS\n");
        success_count++;
    } else {
        printf("  ✗ FAIL\n");
    }
    #endif
    total_tests++;
    
    // ========== TEST 10: Conditional Partition Selection ==========
    printf("\nTest 10: Conditional partition selection\n");
    #pragma acc data copy(a[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(flag ? 2 : 4) vector_length(flag ? 16 : 32)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + 100;
        }
    }
    
    #if VERIFY
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != a[i] + 100) correct = 0;
    }
    if (correct) {
        printf("  ✓ PASS\n");
        success_count++;
    } else {
        printf("  ✗ FAIL\n");
    }
    #endif
    total_tests++;
    
    // ========== TEST 11: OpenMP Target Equivalent ==========
    printf("\nTest 11: OpenMP target with teams distribute\n");
    #pragma omp target data map(to: a[0:N], b[0:N]) map(from: c[0:N])
    {
        #pragma omp target teams distribute parallel for num_teams(num_gangs) thread_limit(num_workers*vector_len)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i] * 2;
        }
    }
    
    #if VERIFY
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != a[i] + b[i] * 2) correct = 0;
    }
    if (correct) {
        printf("  ✓ PASS\n");
        success_count++;
    } else {
        printf("  ✗ FAIL\n");
    }
    #endif
    total_tests++;
    
    // ========== TEST 12: Device-specific behavior ==========
    printf("\nTest 12: Device-specific partition behavior\n");
    #pragma acc set device_type(nvidia)
    {
        #pragma acc data copy(a[0:N], c[0:N])
        {
            #pragma acc parallel loop gang(num_gangs) vector_length(64)
            for (int i = 0; i < N; i++) {
                c[i] = a[i] * 10;
            }
        }
    }
    
    #if VERIFY
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != a[i] * 10) correct = 0;
    }
    if (correct) {
        printf("  ✓ PASS\n");
        success_count++;
    } else {
        printf("  ✗ FAIL\n");
    }
    #endif
    total_tests++;
    
    // ========== TEST 13: Routine with explicit partitioning ==========
    printf("\nTest 13: Routine with explicit gang partitioning\n");
    #pragma acc data copy(a[0:N], d[0:N])
    {
        #pragma acc parallel loop gang(num_gangs)
        for (int i = 0; i < N; i++) {
            acc_gang_routine(d, i, a[i]);
        }
    }
    
    #if VERIFY
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (d[i] != a[i] * 2) correct = 0;
    }
    if (correct) {
        printf("  ✓ PASS\n");
        success_count++;
    } else {
        printf("  ✗ FAIL\n");
    }
    #endif
    total_tests++;
    
    // ========== SUMMARY ==========
    printf("\n" "=" * 50 "\n");
    printf("SUMMARY: %d/%d tests passed\n", success_count, total_tests);
    printf("Coverage targets:\n");
    printf("  - Gang redundant (case 0): Test 1\n");
    printf("  - Gang partitioned (case 1): Test 2\n");
    printf("  - Worker partitioned (case 2): Test 3\n");
    printf("  - Gang+worker partitioned (case 3): Test 4\n");
    printf("  - Vector partitioned (case 4): Test 5\n");
    printf("  - Gang+vector partitioned (case 5): Test 6\n");
    printf("  - Worker+vector partitioned (case 6): Test 7\n");
    printf("  - Fully partitioned (case 7): Test 8\n");
    printf("  - Nested partition propagation: Test 9\n");
    printf("  - Runtime partition selection: Test 10\n");
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return (success_count == total_tests) ? 0 : 1;
}
