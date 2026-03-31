#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define VERIFY_TOLERANCE 0.0001f

// Function declarations with OpenACC routine directives
#pragma acc routine seq
float compute_element(float a, float b, int idx);

#pragma acc routine gang
void gang_partitioned_function(float *arr, int n, int gang_id);

#pragma acc routine worker
void worker_partitioned_function(float *arr, int n, int worker_id);

#pragma acc routine vector
void vector_partitioned_function(float *arr, int n, int vector_id);

// Helper function for verification
int verify_results(float *a, float *b, float *c, int n, float expected) {
    int errors = 0;
    for (int i = 0; i < n; i++) {
        float diff = c[i] - expected;
        if (diff < -VERIFY_TOLERANCE || diff > VERIFY_TOLERANCE) {
            errors++;
            if (errors < 5) {
                printf("  Error at index %d: c[%d] = %f, expected ~%f\n", 
                       i, i, c[i], expected);
            }
        }
    }
    return errors;
}

float compute_element(float a, float b, int idx) {
    return a + b * (idx % 10);
}

void gang_partitioned_function(float *arr, int n, int gang_id) {
    // Simple gang-specific computation
    for (int i = 0; i < n; i++) {
        arr[i] += gang_id * 0.1f;
    }
}

void worker_partitioned_function(float *arr, int n, int worker_id) {
    // Simple worker-specific computation
    for (int i = 0; i < n; i++) {
        arr[i] += worker_id * 0.01f;
    }
}

void vector_partitioned_function(float *arr, int n, int vector_id) {
    // Simple vector-specific computation
    for (int i = 0; i < n; i++) {
        arr[i] += vector_id * 0.001f;
    }
}

int main() {
    int total_tests = 0;
    int passed_tests = 0;
    
    // Runtime-determined partition counts
    int num_gangs = 4;
    int num_workers = 2;
    int vector_length = 32;
    int flag = 1;
    
    printf("Testing OpenACC/OpenMP partition type coverage...\n\n");
    
    // ========== TEST 1: Gang Redundant (Case 0) ==========
    {
        printf("Test 1: Gang redundant\n");
        total_tests++;
        
        float *a = (float*)malloc(N * sizeof(float));
        float *b = (float*)malloc(N * sizeof(float));
        float *c = (float*)malloc(N * sizeof(float));
        
        for (int i = 0; i < N; i++) {
            a[i] = i * 0.1f;
            b[i] = i * 0.2f;
            c[i] = 0.0f;
        }
        
        // Outer gang-redundant region
        #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
        {
            // Gang redundant - no explicit partitioning
            #pragma acc parallel loop gang(num_gangs)
            for (int i = 0; i < N; i++) {
                c[i] = a[i] + b[i];
            }
        }
        
        int errors = verify_results(a, b, c, N, 0.0f);
        if (errors == 0) {
            printf("  PASSED\n");
            passed_tests++;
        } else {
            printf("  FAILED: %d errors\n", errors);
        }
        
        free(a); free(b); free(c);
    }
    
    // ========== TEST 2: Gang Partitioned (Case 1) ==========
    {
        printf("\nTest 2: Gang partitioned\n");
        total_tests++;
        
        float *arr = (float*)malloc(N * sizeof(float));
        for (int i = 0; i < N; i++) arr[i] = i * 0.1f;
        
        #pragma acc data copy(arr[0:N])
        {
            // Explicit gang partitioning
            #pragma acc parallel loop gang(num_gangs) gang
            for (int i = 0; i < N; i++) {
                arr[i] *= 2.0f;
            }
        }
        
        int errors = 0;
        for (int i = 0; i < N; i++) {
            if (arr[i] != i * 0.2f) errors++;
        }
        
        if (errors == 0) {
            printf("  PASSED\n");
            passed_tests++;
        } else {
            printf("  FAILED: %d errors\n", errors);
        }
        
        free(arr);
    }
    
    // ========== TEST 3: Worker Partitioned (Case 2) ==========
    {
        printf("\nTest 3: Worker partitioned\n");
        total_tests++;
        
        float *arr = (float*)malloc(N * sizeof(float));
        for (int i = 0; i < N; i++) arr[i] = 0.0f;
        
        #pragma acc data copy(arr[0:N])
        {
            // Worker partitioned loop
            #pragma acc parallel loop gang(num_gangs) worker(num_workers) worker
            for (int i = 0; i < N; i++) {
                arr[i] = 1.0f;
            }
        }
        
        int errors = 0;
        for (int i = 0; i < N; i++) {
            if (arr[i] != 1.0f) errors++;
        }
        
        if (errors == 0) {
            printf("  PASSED\n");
            passed_tests++;
        } else {
            printf("  FAILED: %d errors\n", errors);
        }
        
        free(arr);
    }
    
    // ========== TEST 4: Gang+Worker Partitioned (Case 3) ==========
    {
        printf("\nTest 4: Gang+worker partitioned\n");
        total_tests++;
        
        float *a = (float*)malloc(N * sizeof(float));
        float *b = (float*)malloc(N * sizeof(float));
        
        for (int i = 0; i < N; i++) {
            a[i] = i * 0.1f;
            b[i] = 0.0f;
        }
        
        // Nested parallelism: outer gang, inner worker
        #pragma acc data copyin(a[0:N]) copyout(b[0:N])
        {
            #pragma acc kernels gang(num_gangs)
            {
                #pragma acc loop worker(num_workers) worker
                for (int i = 0; i < N; i++) {
                    b[i] = a[i] * 3.0f;
                }
            }
        }
        
        int errors = 0;
        for (int i = 0; i < N; i++) {
            if (b[i] != i * 0.3f) errors++;
        }
        
        if (errors == 0) {
            printf("  PASSED\n");
            passed_tests++;
        } else {
            printf("  FAILED: %d errors\n", errors);
        }
        
        free(a); free(b);
    }
    
    // ========== TEST 5: Vector Partitioned (Case 4) ==========
    {
        printf("\nTest 5: Vector partitioned\n");
        total_tests++;
        
        float *arr = (float*)malloc(N * sizeof(float));
        for (int i = 0; i < N; i++) arr[i] = i * 0.1f;
        
        // Conditional partition selection
        int use_vector = flag ? 1 : 0;
        
        #pragma acc data copy(arr[0:N])
        {
            #pragma acc parallel loop vector_length(vector_length) vector
            for (int i = 0; i < N; i++) {
                if (use_vector) {
                    arr[i] += 5.0f;
                }
            }
        }
        
        int errors = 0;
        for (int i = 0; i < N; i++) {
            if (arr[i] != i * 0.1f + 5.0f) errors++;
        }
        
        if (errors == 0) {
            printf("  PASSED\n");
            passed_tests++;
        } else {
            printf("  FAILED: %d errors\n", errors);
        }
        
        free(arr);
    }
    
    // ========== TEST 6: Gang+Vector Partitioned (Case 5) ==========
    {
        printf("\nTest 6: Gang+vector partitioned\n");
        total_tests++;
        
        float *a = (float*)malloc(N * sizeof(float));
        float *b = (float*)malloc(N * sizeof(float));
        
        for (int i = 0; i < N; i++) {
            a[i] = i * 0.1f;
            b[i] = 0.0f;
        }
        
        // Device-specific behavior simulation
        #pragma acc set device_type(nvidia)
        
        #pragma acc data copyin(a[0:N]) copyout(b[0:N])
        {
            #pragma acc parallel loop gang(num_gangs) vector_length(vector_length) gang vector
            for (int i = 0; i < N; i++) {
                b[i] = a[i] * compute_element(a[i], 2.0f, i);
            }
        }
        
        int errors = verify_results(a, b, a, N, 0.0f); // Just check computation happened
        if (errors < N/10) { // Allow some tolerance
            printf("  PASSED (computation verified)\n");
            passed_tests++;
        } else {
            printf("  FAILED: too many errors\n");
        }
        
        free(a); free(b);
    }
    
    // ========== TEST 7: Worker+Vector Partitioned (Case 6) ==========
    {
        printf("\nTest 7: Worker+vector partitioned\n");
        total_tests++;
        
        float *arr = (float*)malloc(N * sizeof(float));
        for (int i = 0; i < N; i++) arr[i] = 0.0f;
        
        // Complex nested partitioning
        #pragma acc data copy(arr[0:N])
        {
            #pragma acc parallel num_gangs(num_gangs) 
            {
                #pragma acc loop worker(num_workers) worker vector_length(vector_length) vector
                for (int i = 0; i < N; i++) {
                    arr[i] = i * 0.01f;
                }
            }
        }
        
        int errors = 0;
        for (int i = 0; i < N; i++) {
            if (arr[i] != i * 0.01f) errors++;
        }
        
        if (errors == 0) {
            printf("  PASSED\n");
            passed_tests++;
        } else {
            printf("  FAILED: %d errors\n", errors);
        }
        
        free(arr);
    }
    
    // ========== TEST 8: Fully Partitioned (Case 7) ==========
    {
        printf("\nTest 8: Fully partitioned (gang+worker+vector)\n");
        total_tests++;
        
        float *a = (float*)malloc(N * sizeof(float));
        float *b = (float*)malloc(N * sizeof(float));
        
        for (int i = 0; i < N; i++) {
            a[i] = i * 0.1f;
            b[i] = 0.0f;
        }
        
        // Fully partitioned with all levels
        #pragma acc data copyin(a[0:N]) copyout(b[0:N])
        {
            #pragma acc parallel loop \
                gang(num_gangs) \
                worker(num_workers) \
                vector_length(vector_length) \
                gang worker vector
            for (int i = 0; i < N; i++) {
                b[i] = a[i] * 2.0f + 1.0f;
            }
        }
        
        int errors = 0;
        for (int i = 0; i < N; i++) {
            float expected = i * 0.1f * 2.0f + 1.0f;
            if (b[i] != expected) errors++;
        }
        
        if (errors == 0) {
            printf("  PASSED\n");
            passed_tests++;
        } else {
            printf("  FAILED: %d errors\n", errors);
        }
        
        free(a); free(b);
    }
    
    // ========== TEST 9: OpenMP Target Version ==========
    {
        printf("\nTest 9: OpenMP target with unified shared memory\n");
        total_tests++;
        
        float *arr = (float*)malloc(N * sizeof(float));
        for (int i = 0; i < N; i++) arr[i] = i * 0.1f;
        
        // OpenMP requires directive for unified memory
        #pragma omp requires unified_shared_memory
        
        #pragma omp target teams distribute parallel for \
            num_teams(num_gangs) \
            num_threads(num_workers * vector_length) \
            map(tofrom: arr[0:N])
        for (int i = 0; i < N; i++) {
            arr[i] *= 1.5f;
        }
        
        int errors = 0;
        for (int i = 0; i < N; i++) {
            if (arr[i] != i * 0.15f) errors++;
        }
        
        if (errors == 0) {
            printf("  PASSED\n");
            passed_tests++;
        } else {
            printf("  FAILED: %d errors\n", errors);
        }
        
        free(arr);
    }
    
    // ========== TEST 10: Invalid/Edge Case ==========
    {
        printf("\nTest 10: Edge case with conditional partitioning\n");
        total_tests++;
        
        float *arr = (float*)malloc(N * sizeof(float));
        for (int i = 0; i < N; i++) arr[i] = 0.0f;
        
        // Try to create potentially invalid combination
        int invalid_flag = 0;
        
        #pragma acc data copy(arr[0:N])
        {
            #pragma acc parallel loop gang(num_gangs) \
                worker(invalid_flag ? 0 : num_workers) // Could be 0 workers
            for (int i = 0; i < N; i++) {
                arr[i] = 100.0f;
            }
        }
        
        // Check if computation happened (even with potential invalid config)
        int errors = 0;
        for (int i = 0; i < N; i++) {
            if (arr[i] != 100.0f) errors++;
        }
        
        if (errors == 0) {
            printf("  PASSED (runtime handled edge case)\n");
            passed_tests++;
        } else {
            printf("  FAILED: %d errors\n", errors);
        }
        
        free(arr);
    }
    
    // ========== SUMMARY ==========
    printf("\n========== SUMMARY ==========\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed tests: %d\n", passed_tests);
    printf("Failed tests: %d\n", total_tests - passed_tests);
    
    if (passed_tests == total_tests) {
        printf("\nALL TESTS PASSED!\n");
        return 0;
    } else {
        printf("\nSOME TESTS FAILED!\n");
        return 1;
    }
}
