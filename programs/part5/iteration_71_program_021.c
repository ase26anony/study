#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define VERIFY_TOL 1e-6

// Function to be called with different partition specifications
#pragma acc routine gang
void compute_element(float *a, float *b, float *c, int i, float scale) {
    c[i] = a[i] * scale + b[i];
}

// Test different partition configurations
int test_gang_redundant() {
    printf("Testing gang redundant (case 0)...\n");
    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));
    float *c = (float*)malloc(N * sizeof(float));
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(N - i);
    }
    
    int num_gangs = 4;
    int use_workers = 0;  // Runtime condition
    
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        // Gang redundant - no explicit worker/vector partitioning
        #pragma acc parallel loop gang(num_gangs)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        float expected = a[i] + b[i];
        if (c[i] != expected) errors++;
    }
    
    free(a); free(b); free(c);
    return errors;
}

int test_gang_partitioned() {
    printf("Testing gang partitioned (case 1)...\n");
    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));
    float *c = (float*)malloc(N * sizeof(float));
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)(i * 2);
        b[i] = (float)(i * 3);
    }
    
    int num_gangs = 8;
    int vector_len = 1;  // No vector partitioning
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        // Explicit gang partitioning only
        #pragma acc parallel loop gang(num_gangs)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * b[i];
        }
    }
    
    int errors = 0;
    for (int i = 0; i < N; i++) {
        float expected = a[i] * b[i];
        if (c[i] != expected) errors++;
    }
    
    free(a); free(b); free(c);
    return errors;
}

int test_worker_partitioned() {
    printf("Testing worker partitioned (case 2)...\n");
    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));
    float *c = (float*)malloc(N * sizeof(float));
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = 2.0f;
    }
    
    int num_workers = 4;
    int use_vector = 0;  // Runtime condition
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        // Worker partitioned only
        #pragma acc parallel loop worker(num_workers)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] / b[i];
        }
    }
    
    int errors = 0;
    for (int i = 0; i < N; i++) {
        float expected = a[i] / b[i];
        if (c[i] != expected) errors++;
    }
    
    free(a); free(b); free(c);
    return errors;
}

int test_gang_worker_partitioned() {
    printf("Testing gang+worker partitioned (case 3)...\n");
    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));
    float *c = (float*)malloc(N * sizeof(float));
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)(i + 1);
        b[i] = 0.5f;
    }
    
    int num_gangs = 2;
    int num_workers = 4;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        // Both gang and worker partitioned
        #pragma acc parallel loop gang(num_gangs) worker(num_workers)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] - b[i];
        }
    }
    
    int errors = 0;
    for (int i = 0; i < N; i++) {
        float expected = a[i] - b[i];
        if (c[i] != expected) errors++;
    }
    
    free(a); free(b); free(c);
    return errors;
}

int test_vector_partitioned() {
    printf("Testing vector partitioned (case 4)...\n");
    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));
    float *c = (float*)malloc(N * sizeof(float));
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = 3.0f;
    }
    
    int vector_length = 32;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        // Vector partitioned only
        #pragma acc parallel loop vector(vector_length)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * b[i];
        }
    }
    
    int errors = 0;
    for (int i = 0; i < N; i++) {
        float expected = a[i] * b[i];
        if (c[i] != expected) errors++;
    }
    
    free(a); free(b); free(c);
    return errors;
}

int test_gang_vector_partitioned() {
    printf("Testing gang+vector partitioned (case 5)...\n");
    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));
    float *c = (float*)malloc(N * sizeof(float));
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)(i * i);
        b[i] = 0.1f;
    }
    
    int num_gangs = 4;
    int vector_length = 16;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        // Gang and vector partitioned
        #pragma acc parallel loop gang(num_gangs) vector(vector_length)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
    
    int errors = 0;
    for (int i = 0; i < N; i++) {
        float expected = a[i] + b[i];
        if (c[i] != expected) errors++;
    }
    
    free(a); free(b); free(c);
    return errors;
}

int test_worker_vector_partitioned() {
    printf("Testing worker+vector partitioned (case 6)...\n");
    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));
    float *c = (float*)malloc(N * sizeof(float));
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = 4.0f;
    }
    
    int num_workers = 2;
    int vector_length = 8;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        // Worker and vector partitioned
        #pragma acc parallel loop worker(num_workers) vector(vector_length)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] / b[i];
        }
    }
    
    int errors = 0;
    for (int i = 0; i < N; i++) {
        float expected = a[i] / b[i];
        if (c[i] != expected) errors++;
    }
    
    free(a); free(b); free(c);
    return errors;
}

int test_fully_partitioned() {
    printf("Testing fully partitioned (case 7)...\n");
    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));
    float *c = (float*)malloc(N * sizeof(float));
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)(i % 10);
        b[i] = (float)(i % 5);
    }
    
    int num_gangs = 2;
    int num_workers = 2;
    int vector_length = 4;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        // Fully partitioned: gang, worker, and vector
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) vector(vector_length)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * b[i];
        }
    }
    
    int errors = 0;
    for (int i = 0; i < N; i++) {
        float expected = a[i] * b[i];
        if (c[i] != expected) errors++;
    }
    
    free(a); free(b); free(c);
    return errors;
}

// Test with nested parallelism
int test_nested_partitioning() {
    printf("Testing nested partitioning...\n");
    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));
    float *c = (float*)malloc(N * sizeof(float));
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = 2.0f;
    }
    
    int flag = 1;  // Runtime condition
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        // Outer gang-redundant region
        #pragma acc kernels
        {
            // Inner worker-partitioned loop
            #pragma acc loop worker(flag ? 2 : 4)
            for (int i = 0; i < N; i++) {
                c[i] = a[i] + b[i];
            }
        }
    }
    
    int errors = 0;
    for (int i = 0; i < N; i++) {
        float expected = a[i] + b[i];
        if (c[i] != expected) errors++;
    }
    
    free(a); free(b); free(c);
    return errors;
}

// Test with OpenMP target for comparison
#ifdef _OPENMP
int test_omp_partitioning() {
    printf("Testing OpenMP target partitioning...\n");
    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));
    float *c = (float*)malloc(N * sizeof(float));
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = 3.0f;
    }
    
    int num_teams = 4;
    int thread_limit = 32;
    
    #pragma omp target data map(to: a[0:N], b[0:N]) map(from: c[0:N])
    {
        // OpenMP equivalent with teams and distribute
        #pragma omp target teams distribute parallel for \
                    num_teams(num_teams) thread_limit(thread_limit)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * b[i];
        }
    }
    
    int errors = 0;
    for (int i = 0; i < N; i++) {
        float expected = a[i] * b[i];
        if (c[i] != expected) errors++;
    }
    
    free(a); free(b); free(c);
    return errors;
}
#endif

// Test with device-specific behavior
int test_device_specific() {
    printf("Testing device-specific partitioning...\n");
    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));
    float *c = (float*)malloc(N * sizeof(float));
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)(i % 20);
        b[i] = 0.5f;
    }
    
    // Device-specific pragma
    #pragma acc set device_type(nvidia)
    
    int num_gangs = 8;
    int vector_length = 64;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) vector(vector_length)
        for (int i = 0; i < N; i++) {
            compute_element(a, b, c, i, 2.0f);
        }
    }
    
    int errors = 0;
    for (int i = 0; i < N; i++) {
        float expected = a[i] * 2.0f + b[i];
        if (c[i] != expected) errors++;
    }
    
    free(a); free(b); free(c);
    return errors;
}

// Test with conditional partition selection
int test_conditional_partition() {
    printf("Testing conditional partition selection...\n");
    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));
    float *c = (float*)malloc(N * sizeof(float));
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(N - i);
    }
    
    int use_advanced = 1;  // Runtime flag
    int num_gangs = use_advanced ? 4 : 2;
    int num_workers = use_advanced ? 2 : 1;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        // Conditional partition specification
        #pragma acc parallel loop \
                    gang(num_gangs) \
                    worker(use_advanced ? num_workers : 1)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] - b[i];
        }
    }
    
    int errors = 0;
    for (int i = 0; i < N; i++) {
        float expected = a[i] - b[i];
        if (c[i] != expected) errors++;
    }
    
    free(a); free(b); free(c);
    return errors;
}

int main() {
    int total_errors = 0;
    int test_count = 0;
    
    printf("=== Testing OpenACC/OpenMP Partition Coverage ===\n\n");
    
    // Test all partition type cases
    total_errors += test_gang_redundant();
    test_count++;
    
    total_errors += test_gang_partitioned();
    test_count++;
    
    total_errors += test_worker_partitioned();
    test_count++;
    
    total_errors += test_gang_worker_partitioned();
    test_count++;
    
    total_errors += test_vector_partitioned();
    test_count++;
    
    total_errors += test_gang_vector_partitioned();
    test_count++;
    
    total_errors += test_worker_vector_partitioned();
    test_count++;
    
    total_errors += test_fully_partitioned();
    test_count++;
    
    // Additional tests for coverage
    total_errors += test_nested_partitioning();
    test_count++;
    
    total_errors += test_device_specific();
    test_count++;
    
    total_errors += test_conditional_partition();
    test_count++;
    
    #ifdef _OPENMP
    total_errors += test_omp_partitioning();
    test_count++;
    #endif
    
    printf("\n=== Test Summary ===\n");
    printf("Tests run: %d\n", test_count);
    printf("Total errors: %d\n", total_errors);
    
    if (total_errors == 0) {
        printf("All tests passed successfully!\n");
    } else {
        printf("Some tests failed.\n");
    }
    
    return total_errors > 0 ? 1 : 0;
}
