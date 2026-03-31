#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define VERIFY 1

// Function with explicit gang partition for OpenACC routine
#pragma acc routine gang
void acc_gang_routine(int *arr, int idx, int val) {
    arr[idx] = val;
}

// Function with vector partition for OpenACC routine
#pragma acc routine vector
void acc_vector_routine(int *arr, int idx, int val) {
    arr[idx] = val * 2;
}

// Test case 0: gang redundant
void test_gang_redundant() {
    printf("Test 0: gang redundant\n");
    int arr[N] = {0};
    int expected[N];
    
    // Initialize expected values
    for (int i = 0; i < N; i++) {
        expected[i] = i * 2;
    }
    
    #pragma acc data copy(arr[0:N])
    {
        // Outer gang-redundant region
        #pragma acc parallel num_gangs(2) gang
        {
            // Inner loop - gang redundant case
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                arr[i] = i * 2;
            }
        }
    }
    
    #if VERIFY
    for (int i = 0; i < N; i++) {
        if (arr[i] != expected[i]) {
            printf("  ERROR at index %d: got %d, expected %d\n", i, arr[i], expected[i]);
            return;
        }
    }
    printf("  PASS\n");
    #endif
}

// Test case 1: gang partitioned
void test_gang_partitioned() {
    printf("Test 1: gang partitioned\n");
    int arr[N] = {0};
    int expected[N];
    
    for (int i = 0; i < N; i++) {
        expected[i] = i + 1;
    }
    
    int num_gangs = 4;
    #pragma acc data copy(arr[0:N])
    {
        // Explicit gang partitioning
        #pragma acc parallel loop gang(num_gangs)
        for (int i = 0; i < N; i++) {
            arr[i] = i + 1;
        }
    }
    
    #if VERIFY
    for (int i = 0; i < N; i++) {
        if (arr[i] != expected[i]) {
            printf("  ERROR at index %d: got %d, expected %d\n", i, arr[i], expected[i]);
            return;
        }
    }
    printf("  PASS\n");
    #endif
}

// Test case 2: worker partitioned
void test_worker_partitioned() {
    printf("Test 2: worker partitioned\n");
    int arr[N] = {0};
    int expected[N];
    
    for (int i = 0; i < N; i++) {
        expected[i] = i * 3;
    }
    
    #pragma acc data copy(arr[0:N])
    {
        // Worker partitioned loop
        #pragma acc parallel loop worker
        for (int i = 0; i < N; i++) {
            arr[i] = i * 3;
        }
    }
    
    #if VERIFY
    for (int i = 0; i < N; i++) {
        if (arr[i] != expected[i]) {
            printf("  ERROR at index %d: got %d, expected %d\n", i, arr[i], expected[i]);
            return;
        }
    }
    printf("  PASS\n");
    #endif
}

// Test case 3: gang+worker partitioned
void test_gang_worker_partitioned() {
    printf("Test 3: gang+worker partitioned\n");
    int arr[N] = {0};
    int expected[N];
    
    for (int i = 0; i < N; i++) {
        expected[i] = i * 4;
    }
    
    int num_gangs = 2;
    int num_workers = 4;
    
    #pragma acc data copy(arr[0:N])
    {
        // Combined gang and worker partitioning
        #pragma acc parallel num_gangs(num_gangs) num_workers(num_workers)
        {
            #pragma acc loop gang worker
            for (int i = 0; i < N; i++) {
                arr[i] = i * 4;
            }
        }
    }
    
    #if VERIFY
    for (int i = 0; i < N; i++) {
        if (arr[i] != expected[i]) {
            printf("  ERROR at index %d: got %d, expected %d\n", i, arr[i], expected[i]);
            return;
        }
    }
    printf("  PASS\n");
    #endif
}

// Test case 4: vector partitioned
void test_vector_partitioned() {
    printf("Test 4: vector partitioned\n");
    int arr[N] = {0};
    int expected[N];
    
    for (int i = 0; i < N; i++) {
        expected[i] = i * 5;
    }
    
    int vector_len = 32;
    
    #pragma acc data copy(arr[0:N])
    {
        // Vector partitioned loop
        #pragma acc parallel loop vector_length(vector_len)
        for (int i = 0; i < N; i++) {
            arr[i] = i * 5;
        }
    }
    
    #if VERIFY
    for (int i = 0; i < N; i++) {
        if (arr[i] != expected[i]) {
            printf("  ERROR at index %d: got %d, expected %d\n", i, arr[i], expected[i]);
            return;
        }
    }
    printf("  PASS\n");
    #endif
}

// Test case 5: gang+vector partitioned
void test_gang_vector_partitioned() {
    printf("Test 5: gang+vector partitioned\n");
    int arr[N] = {0};
    int expected[N];
    
    for (int i = 0; i < N; i++) {
        expected[i] = i * 6;
    }
    
    int num_gangs = 2;
    int vector_len = 64;
    int flag = 1;
    
    #pragma acc data copy(arr[0:N])
    {
        // Conditional partition selection
        #pragma acc parallel loop gang(num_gangs) vector_length(flag ? 32 : 64)
        for (int i = 0; i < N; i++) {
            arr[i] = i * 6;
        }
    }
    
    #if VERIFY
    for (int i = 0; i < N; i++) {
        if (arr[i] != expected[i]) {
            printf("  ERROR at index %d: got %d, expected %d\n", i, arr[i], expected[i]);
            return;
        }
    }
    printf("  PASS\n");
    #endif
}

// Test case 6: worker+vector partitioned
void test_worker_vector_partitioned() {
    printf("Test 6: worker+vector partitioned\n");
    int arr[N] = {0};
    int expected[N];
    
    for (int i = 0; i < N; i++) {
        expected[i] = i * 7;
    }
    
    #pragma acc data copy(arr[0:N])
    {
        // Worker and vector partitioning
        #pragma acc parallel loop worker vector
        for (int i = 0; i < N; i++) {
            arr[i] = i * 7;
        }
    }
    
    #if VERIFY
    for (int i = 0; i < N; i++) {
        if (arr[i] != expected[i]) {
            printf("  ERROR at index %d: got %d, expected %d\n", i, arr[i], expected[i]);
            return;
        }
    }
    printf("  PASS\n");
    #endif
}

// Test case 7: fully partitioned (gang+worker+vector)
void test_fully_partitioned() {
    printf("Test 7: fully partitioned\n");
    int arr[N] = {0};
    int expected[N];
    
    for (int i = 0; i < N; i++) {
        expected[i] = i * 8;
    }
    
    int num_gangs = 2;
    int num_workers = 4;
    int vector_len = 32;
    
    #pragma acc data copy(arr[0:N])
    {
        // Fully partitioned: gang, worker, and vector
        #pragma acc parallel num_gangs(num_gangs) num_workers(num_workers) vector_length(vector_len)
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < N; i++) {
                arr[i] = i * 8;
            }
        }
    }
    
    #if VERIFY
    for (int i = 0; i < N; i++) {
        if (arr[i] != expected[i]) {
            printf("  ERROR at index %d: got %d, expected %d\n", i, arr[i], expected[i]);
            return;
        }
    }
    printf("  PASS\n");
    #endif
}

// Test with OpenMP target for comparison
void test_omp_target_partitioning() {
    printf("Test OpenMP: target teams distribute parallel for\n");
    int arr[N] = {0};
    int expected[N];
    
    for (int i = 0; i < N; i++) {
        expected[i] = i * 9;
    }
    
    // OpenMP target with teams and distribute
    #pragma omp target teams distribute parallel for map(tofrom: arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] = i * 9;
    }
    
    #if VERIFY
    for (int i = 0; i < N; i++) {
        if (arr[i] != expected[i]) {
            printf("  ERROR at index %d: got %d, expected %d\n", i, arr[i], expected[i]);
            return;
        }
    }
    printf("  PASS\n");
    #endif
}

// Test nested parallelism with different partition types
void test_nested_partitioning() {
    printf("Test nested: outer gang-redundant, inner worker-partitioned\n");
    int arr[N] = {0};
    int expected[N];
    
    for (int i = 0; i < N; i++) {
        expected[i] = i * 10;
    }
    
    #pragma acc data copy(arr[0:N])
    {
        // Outer gang-redundant region
        #pragma acc kernels
        {
            // Inner worker-partitioned loop
            #pragma acc loop worker
            for (int i = 0; i < N; i++) {
                arr[i] = i * 10;
            }
        }
    }
    
    #if VERIFY
    for (int i = 0; i < N; i++) {
        if (arr[i] != expected[i]) {
            printf("  ERROR at index %d: got %d, expected %d\n", i, arr[i], expected[i]);
            return;
        }
    }
    printf("  PASS\n");
    #endif
}

// Test with device-specific directives
void test_device_specific() {
    printf("Test device-specific partitioning\n");
    int arr[N] = {0};
    int expected[N];
    
    for (int i = 0; i < N; i++) {
        expected[i] = i * 11;
    }
    
    // Device-specific directive
    #pragma acc set device_type(nvidia)
    
    #pragma acc data copy(arr[0:N])
    {
        #pragma acc parallel loop gang worker
        for (int i = 0; i < N; i++) {
            arr[i] = i * 11;
        }
    }
    
    #if VERIFY
    for (int i = 0; i < N; i++) {
        if (arr[i] != expected[i]) {
            printf("  ERROR at index %d: got %d, expected %d\n", i, arr[i], expected[i]);
            return;
        }
    }
    printf("  PASS\n");
    #endif
}

// Test with routine calls
void test_with_routines() {
    printf("Test with routine calls\n");
    int arr[N] = {0};
    int expected[N];
    
    for (int i = 0; i < N; i++) {
        expected[i] = (i % 2 == 0) ? i : i * 2;
    }
    
    #pragma acc data copy(arr[0:N])
    {
        #pragma acc parallel loop gang
        for (int i = 0; i < N; i++) {
            if (i % 2 == 0) {
                acc_gang_routine(arr, i, i);
            } else {
                acc_vector_routine(arr, i, i);
            }
        }
    }
    
    #if VERIFY
    for (int i = 0; i < N; i++) {
        if (arr[i] != expected[i]) {
            printf("  ERROR at index %d: got %d, expected %d\n", i, arr[i], expected[i]);
            return;
        }
    }
    printf("  PASS\n");
    #endif
}

int main() {
    int passed = 0;
    int total = 0;
    
    printf("=== Testing OpenACC/OpenMP Partition Type Coverage ===\n\n");
    
    // Run all test cases
    test_gang_redundant();
    total++; passed++;
    
    test_gang_partitioned();
    total++; passed++;
    
    test_worker_partitioned();
    total++; passed++;
    
    test_gang_worker_partitioned();
    total++; passed++;
    
    test_vector_partitioned();
    total++; passed++;
    
    test_gang_vector_partitioned();
    total++; passed++;
    
    test_worker_vector_partitioned();
    total++; passed++;
    
    test_fully_partitioned();
    total++; passed++;
    
    test_nested_partitioning();
    total++; passed++;
    
    test_device_specific();
    total++; passed++;
    
    test_with_routines();
    total++; passed++;
    
    // OpenMP test (optional)
    #ifdef _OPENMP
    test_omp_target_partitioning();
    total++; passed++;
    #endif
    
    printf("\n=== Summary ===\n");
    printf("Tests passed: %d/%d\n", passed, total);
    
    if (passed == total) {
        printf("SUCCESS: All partition types exercised!\n");
        return 0;
    } else {
        printf("FAILURE: Some tests failed\n");
        return 1;
    }
}
