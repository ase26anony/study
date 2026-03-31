#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define VERIFY_TOL 1e-6

/* Function with explicit partition specification */
#pragma acc routine gang
void acc_gang_function(int *arr, int start, int end, int multiplier) {
    #pragma acc loop gang
    for (int i = start; i < end; i++) {
        arr[i] *= multiplier;
    }
}

/* Worker-level function */
#pragma acc routine worker
void acc_worker_function(int *arr, int idx, int offset) {
    #pragma acc loop worker
    for (int i = 0; i < 16; i++) {
        arr[idx + i] += offset;
    }
}

int main() {
    /* Runtime-determined partition counts */
    int num_gangs = 2;
    int num_workers = 4;
    int vector_len = 32;
    int flag = 1;
    
    /* Test arrays */
    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));
    float *c = (float*)malloc(N * sizeof(float));
    int *int_arr = (int*)malloc(N * sizeof(int));
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(2 * i);
        c[i] = 0.0f;
        int_arr[i] = i;
    }
    
    int success_count = 0;
    int total_tests = 0;
    
    printf("=== Testing Partition Type Identification ===\n\n");
    
    /* Test 1: Gang Redundant (case 0) */
    printf("Test 1: Gang Redundant\n");
    total_tests++;
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) gang_redundant
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
    
    /* Verify results */
    int correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != a[i] + b[i]) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        printf("  ✓ PASS\n");
        success_count++;
    } else {
        printf("  ✗ FAIL\n");
    }
    
    /* Test 2: Gang Partitioned (case 1) */
    printf("\nTest 2: Gang Partitioned\n");
    total_tests++;
    #pragma acc data copy(int_arr[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) gang
        for (int i = 0; i < N; i++) {
            int_arr[i] *= 2;
        }
    }
    
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (int_arr[i] != i * 2) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        printf("  ✓ PASS\n");
        success_count++;
    } else {
        printf("  ✗ FAIL\n");
    }
    
    /* Test 3: Worker Partitioned (case 2) */
    printf("\nTest 3: Worker Partitioned\n");
    total_tests++;
    #pragma acc data copy(int_arr[0:N])
    {
        #pragma acc parallel num_gangs(1) num_workers(num_workers)
        {
            #pragma acc loop worker
            for (int i = 0; i < N; i++) {
                int_arr[i] += 1;
            }
        }
    }
    
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (int_arr[i] != i * 2 + 1) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        printf("  ✓ PASS\n");
        success_count++;
    } else {
        printf("  ✗ FAIL\n");
    }
    
    /* Test 4: Gang+Worker Partitioned (case 3) */
    printf("\nTest 4: Gang+Worker Partitioned\n");
    total_tests++;
    #pragma acc data copy(a[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) gang worker
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * 3.0f;
        }
    }
    
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != a[i] * 3.0f) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        printf("  ✓ PASS\n");
        success_count++;
    } else {
        printf("  ✗ FAIL\n");
    }
    
    /* Test 5: Vector Partitioned (case 4) */
    printf("\nTest 5: Vector Partitioned\n");
    total_tests++;
    #pragma acc data copy(a[0:N], b[0:N])
    {
        #pragma acc parallel loop vector_length(vector_len) vector
        for (int i = 0; i < N; i++) {
            b[i] = a[i] / 2.0f;
        }
    }
    
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (b[i] != a[i] / 2.0f) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        printf("  ✓ PASS\n");
        success_count++;
    } else {
        printf("  ✗ FAIL\n");
    }
    
    /* Test 6: Gang+Vector Partitioned (case 5) */
    printf("\nTest 6: Gang+Vector Partitioned\n");
    total_tests++;
    #pragma acc data copy(a[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) vector_length(vector_len) gang vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * a[i];
        }
    }
    
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != a[i] * a[i]) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        printf("  ✓ PASS\n");
        success_count++;
    } else {
        printf("  ✗ FAIL\n");
    }
    
    /* Test 7: Worker+Vector Partitioned (case 6) */
    printf("\nTest 7: Worker+Vector Partitioned\n");
    total_tests++;
    #pragma acc data copy(b[0:N], c[0:N])
    {
        #pragma acc parallel loop worker(num_workers) vector_length(vector_len) worker vector
        for (int i = 0; i < N; i++) {
            c[i] = b[i] + 100.0f;
        }
    }
    
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != b[i] + 100.0f) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        printf("  ✓ PASS\n");
        success_count++;
    } else {
        printf("  ✗ FAIL\n");
    }
    
    /* Test 8: Fully Partitioned (case 7) */
    printf("\nTest 8: Fully Partitioned\n");
    total_tests++;
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) vector_length(vector_len) gang worker vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i] + 50.0f;
        }
    }
    
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != a[i] + b[i] + 50.0f) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        printf("  ✓ PASS\n");
        success_count++;
    } else {
        printf("  ✗ FAIL\n");
    }
    
    /* Test 9: Nested parallelism with different partitions */
    printf("\nTest 9: Nested Parallelism\n");
    total_tests++;
    #pragma acc data copy(int_arr[0:N])
    {
        /* Outer gang-redundant region */
        #pragma acc kernels gang_redundant
        {
            /* Inner worker-partitioned loop */
            #pragma acc loop worker(num_workers) worker
            for (int i = 0; i < N; i++) {
                int_arr[i] -= 5;
            }
        }
    }
    
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (int_arr[i] != i * 2 + 1 - 5) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        printf("  ✓ PASS\n");
        success_count++;
    } else {
        printf("  ✗ FAIL\n");
    }
    
    /* Test 10: Conditional partition selection */
    printf("\nTest 10: Conditional Partition Selection\n");
    total_tests++;
    #pragma acc data copy(a[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(flag ? 2 : 4) vector_length(flag ? 16 : 32) gang worker vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * 0.5f;
        }
    }
    
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != a[i] * 0.5f) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        printf("  ✓ PASS\n");
        success_count++;
    } else {
        printf("  ✗ FAIL\n");
    }
    
    /* Test 11: OpenMP Target with explicit partitioning */
    printf("\nTest 11: OpenMP Target Partitioning\n");
    total_tests++;
    
    #pragma omp target data map(to: a[0:N], b[0:N]) map(from: c[0:N])
    {
        #pragma omp target teams distribute parallel for \
                num_teams(num_gangs) num_threads(num_workers) \
                dist_schedule(static, N/num_gangs)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] - b[i];
        }
    }
    
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != a[i] - b[i]) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        printf("  ✓ PASS\n");
        success_count++;
    } else {
        printf("  ✗ FAIL\n");
    }
    
    /* Test 12: Device-specific partition behavior */
    printf("\nTest 12: Device-Specific Partitioning\n");
    total_tests++;
    
    #pragma acc set device_type(nvidia)
    #pragma acc data copy(a[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) vector_length(64) gang vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * 2.0f;
        }
    }
    
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != a[i] * 2.0f) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        printf("  ✓ PASS\n");
        success_count++;
    } else {
        printf("  ✗ FAIL\n");
    }
    
    /* Test 13: Function call with explicit partition */
    printf("\nTest 13: Function with Partition Specification\n");
    total_tests++;
    
    /* Reset int_arr */
    for (int i = 0; i < N; i++) {
        int_arr[i] = i;
    }
    
    #pragma acc data copy(int_arr[0:N])
    {
        #pragma acc parallel num_gangs(num_gangs)
        {
            #pragma acc loop gang
            for (int g = 0; g < num_gangs; g++) {
                int start = g * (N / num_gangs);
                int end = (g + 1) * (N / num_gangs);
                if (g == num_gangs - 1) end = N;
                acc_gang_function(int_arr, start, end, 3);
            }
        }
    }
    
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (int_arr[i] != i * 3) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        printf("  ✓ PASS\n");
        success_count++;
    } else {
        printf("  ✗ FAIL\n");
    }
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Tests passed: %d/%d\n", success_count, total_tests);
    printf("Coverage target: All partition type switch cases (0-7)\n");
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(int_arr);
    
    return (success_count == total_tests) ? 0 : 1;
}
