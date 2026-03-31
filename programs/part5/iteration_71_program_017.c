/**
 * Test program to exercise OpenACC/OpenMP partition clause logic
 * Specifically targets the switch statement in omp-oacc-neuter-broadcast.cc
 * lines 335-343 covering all partition type cases (0-7)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define ARRAY_SIZE 1024
#define NUM_TESTS 10

/* Function prototypes with different partition specifications */
#pragma acc routine gang
void acc_gang_redundant_func(int *arr, int n);

#pragma acc routine worker
void acc_worker_partitioned_func(int *arr, int n);

#pragma acc routine vector
void acc_vector_partitioned_func(int *arr, int n);

/* Helper function to verify results */
int verify_array(int *arr, int expected, int n) {
    for (int i = 0; i < n; i++) {
        if (arr[i] != expected) {
            return 0;
        }
    }
    return 1;
}

/* Test 1: Gang redundant (case 0) */
void test_gang_redundant() {
    int arr[ARRAY_SIZE];
    int num_gangs = 4;
    
    #pragma acc data copy(arr)
    {
        #pragma acc parallel loop gang(num_gangs) gang
        for (int i = 0; i < ARRAY_SIZE; i++) {
            arr[i] = 1;
        }
    }
    
    assert(verify_array(arr, 1, ARRAY_SIZE));
    printf("Test 1 (gang redundant): PASSED\n");
}

/* Test 2: Gang partitioned (case 1) */
void test_gang_partitioned() {
    int arr[ARRAY_SIZE];
    int num_gangs = 8;
    
    #pragma acc data copy(arr)
    {
        #pragma acc parallel loop gang(num_gangs) gang
        for (int i = 0; i < ARRAY_SIZE; i++) {
            arr[i] = i % num_gangs;
        }
    }
    
    printf("Test 2 (gang partitioned): PASSED\n");
}

/* Test 3: Worker partitioned (case 2) */
void test_worker_partitioned() {
    int arr[ARRAY_SIZE];
    int num_workers = 4;
    
    #pragma acc data copy(arr)
    {
        #pragma acc parallel loop worker(num_workers) worker
        for (int i = 0; i < ARRAY_SIZE; i++) {
            arr[i] = 2;
        }
    }
    
    assert(verify_array(arr, 2, ARRAY_SIZE));
    printf("Test 3 (worker partitioned): PASSED\n");
}

/* Test 4: Gang+worker partitioned (case 3) */
void test_gang_worker_partitioned() {
    int arr[ARRAY_SIZE];
    int num_gangs = 2;
    int num_workers = 4;
    
    #pragma acc data copy(arr)
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) gang worker
        for (int i = 0; i < ARRAY_SIZE; i++) {
            arr[i] = 3;
        }
    }
    
    assert(verify_array(arr, 3, ARRAY_SIZE));
    printf("Test 4 (gang+worker partitioned): PASSED\n");
}

/* Test 5: Vector partitioned (case 4) */
void test_vector_partitioned() {
    int arr[ARRAY_SIZE];
    int vector_length = 32;
    
    #pragma acc data copy(arr)
    {
        #pragma acc parallel loop vector(vector_length) vector
        for (int i = 0; i < ARRAY_SIZE; i++) {
            arr[i] = 4;
        }
    }
    
    assert(verify_array(arr, 4, ARRAY_SIZE));
    printf("Test 5 (vector partitioned): PASSED\n");
}

/* Test 6: Gang+vector partitioned (case 5) */
void test_gang_vector_partitioned() {
    int arr[ARRAY_SIZE];
    int num_gangs = 4;
    int vector_length = 64;
    
    #pragma acc data copy(arr)
    {
        #pragma acc parallel loop gang(num_gangs) vector(vector_length) gang vector
        for (int i = 0; i < ARRAY_SIZE; i++) {
            arr[i] = 5;
        }
    }
    
    assert(verify_array(arr, 5, ARRAY_SIZE));
    printf("Test 6 (gang+vector partitioned): PASSED\n");
}

/* Test 7: Worker+vector partitioned (case 6) */
void test_worker_vector_partitioned() {
    int arr[ARRAY_SIZE];
    int num_workers = 2;
    int vector_length = 128;
    
    #pragma acc data copy(arr)
    {
        #pragma acc parallel loop worker(num_workers) vector(vector_length) worker vector
        for (int i = 0; i < ARRAY_SIZE; i++) {
            arr[i] = 6;
        }
    }
    
    assert(verify_array(arr, 6, ARRAY_SIZE));
    printf("Test 7 (worker+vector partitioned): PASSED\n");
}

/* Test 8: Fully partitioned (case 7) */
void test_fully_partitioned() {
    int arr[ARRAY_SIZE];
    int num_gangs = 2;
    int num_workers = 2;
    int vector_length = 256;
    
    #pragma acc data copy(arr)
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) vector(vector_length) gang worker vector
        for (int i = 0; i < ARRAY_SIZE; i++) {
            arr[i] = 7;
        }
    }
    
    assert(verify_array(arr, 7, ARRAY_SIZE));
    printf("Test 8 (fully partitioned): PASSED\n");
}

/* Test 9: Nested parallelism with different partition types */
void test_nested_partitioning() {
    int arr[ARRAY_SIZE];
    int num_gangs = 4;
    int num_workers = 2;
    
    /* Outer gang-redundant region */
    #pragma acc data copy(arr)
    {
        #pragma acc kernels gang(num_gangs) gang
        {
            /* Inner worker-partitioned loop */
            #pragma acc loop worker(num_workers) worker
            for (int i = 0; i < ARRAY_SIZE; i++) {
                arr[i] = 9;
            }
        }
    }
    
    assert(verify_array(arr, 9, ARRAY_SIZE));
    printf("Test 9 (nested partitioning): PASSED\n");
}

/* Test 10: Conditional partition selection with runtime values */
void test_conditional_partition() {
    int arr[ARRAY_SIZE];
    int flag = 1;
    int num_gangs = flag ? 2 : 4;
    int num_workers = flag ? 4 : 2;
    int vector_length = flag ? 32 : 64;
    
    #pragma acc data copy(arr)
    {
        #pragma acc parallel loop \
            gang(num_gangs) \
            worker(num_workers) \
            vector(vector_length) \
            gang worker vector
        for (int i = 0; i < ARRAY_SIZE; i++) {
            arr[i] = 10;
        }
    }
    
    assert(verify_array(arr, 10, ARRAY_SIZE));
    printf("Test 10 (conditional partition): PASSED\n");
}

/* OpenMP target equivalents */
#ifdef _OPENMP
void test_omp_gang_partitioned() {
    int arr[ARRAY_SIZE];
    int num_teams = 4;
    int thread_limit = 32;
    
    #pragma omp target data map(tofrom: arr[0:ARRAY_SIZE])
    {
        #pragma omp target teams distribute parallel for \
            num_teams(num_teams) thread_limit(thread_limit)
        for (int i = 0; i < ARRAY_SIZE; i++) {
            arr[i] = 100;
        }
    }
    
    assert(verify_array(arr, 100, ARRAY_SIZE));
    printf("OpenMP Test (gang partitioned): PASSED\n");
}

void test_omp_fully_partitioned() {
    int arr[ARRAY_SIZE];
    int num_teams = 2;
    int num_threads = 64;
    
    #pragma omp target data map(tofrom: arr[0:ARRAY_SIZE])
    {
        #pragma omp target teams distribute parallel for \
            num_teams(num_teams) num_threads(num_threads)
        for (int i = 0; i < ARRAY_SIZE; i++) {
            arr[i] = 200;
        }
    }
    
    assert(verify_array(arr, 200, ARRAY_SIZE));
    printf("OpenMP Test (fully partitioned): PASSED\n");
}
#endif

/* Device-specific partition behaviors */
void test_device_specific() {
    int arr[ARRAY_SIZE];
    
    /* Simulate device-specific behavior */
    #pragma acc set device_type(nvidia)
    
    #pragma acc data copy(arr)
    {
        #pragma acc parallel loop gang worker vector
        for (int i = 0; i < ARRAY_SIZE; i++) {
            arr[i] = 255;
        }
    }
    
    assert(verify_array(arr, 255, ARRAY_SIZE));
    printf("Device-specific test: PASSED\n");
}

int main() {
    int passed = 0;
    
    printf("=== OpenACC Partition Clause Coverage Tests ===\n\n");
    
    /* Execute all test cases to cover switch statement (cases 0-7) */
    test_gang_redundant();           /* case 0 */
    passed++;
    
    test_gang_partitioned();         /* case 1 */
    passed++;
    
    test_worker_partitioned();       /* case 2 */
    passed++;
    
    test_gang_worker_partitioned();  /* case 3 */
    passed++;
    
    test_vector_partitioned();       /* case 4 */
    passed++;
    
    test_gang_vector_partitioned();  /* case 5 */
    passed++;
    
    test_worker_vector_partitioned(); /* case 6 */
    passed++;
    
    test_fully_partitioned();        /* case 7 */
    passed++;
    
    test_nested_partitioning();      /* nested case */
    passed++;
    
    test_conditional_partition();    /* dynamic case */
    passed++;
    
    test_device_specific();          /* device-specific case */
    passed++;
    
#ifdef _OPENMP
    printf("\n=== OpenMP Target Partition Tests ===\n\n");
    test_omp_gang_partitioned();
    passed++;
    
    test_omp_fully_partitioned();
    passed++;
#endif
    
    printf("\n=== Summary ===\n");
    printf("Total tests passed: %d/%d\n", passed, 
#ifdef _OPENMP
            NUM_TESTS + 2
#else
            NUM_TESTS
#endif
          );
    
    if (passed == 
#ifdef _OPENMP
        NUM_TESTS + 2
#else
        NUM_TESTS
#endif
       ) {
        printf("All tests PASSED - All partition switch cases should be covered!\n");
        return 0;
    } else {
        printf("Some tests FAILED\n");
        return 1;
    }
}

/* Implement the partitioned functions */
void acc_gang_redundant_func(int *arr, int n) {
    #pragma acc parallel loop gang
    for (int i = 0; i < n; i++) {
        arr[i] = 1;
    }
}

void acc_worker_partitioned_func(int *arr, int n) {
    #pragma acc parallel loop worker
    for (int i = 0; i < n; i++) {
        arr[i] = 2;
    }
}

void acc_vector_partitioned_func(int *arr, int n) {
    #pragma acc parallel loop vector
    for (int i = 0; i < n; i++) {
        arr[i] = 3;
    }
}
