#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>

#define N 1024
#define M 512
#define P 256

// Enable debug output to trigger partition string mapping
void enable_acc_debug() {
    // Try to set debug environment variables
    setenv("ACC_DEBUG", "1", 1);
    setenv("LIBGOMP_DEBUG", "1", 1);
    setenv("GACC_DEBUG", "1", 1);
}

// Function to verify results
int verify_results(float *arr, float expected, int size, const char *test_name) {
    for (int i = 0; i < size; i++) {
        if (arr[i] != expected) {
            printf("Test %s failed at index %d: got %f, expected %f\n", 
                   test_name, i, arr[i], expected);
            return 0;
        }
    }
    return 1;
}

int main() {
    int i, j, k;
    int passed = 1;
    
    // Enable debug to increase chance of partition mapping calls
    enable_acc_debug();
    
    // Initialize arrays
    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));
    float *c = (float*)malloc(N * sizeof(float));
    float *matrix = (float*)malloc(M * P * sizeof(float));
    float *result = (float*)malloc(M * P * sizeof(float));
    
    for (i = 0; i < N; i++) {
        a[i] = 1.0f;
        b[i] = 2.0f;
        c[i] = 0.0f;
    }
    
    for (i = 0; i < M * P; i++) {
        matrix[i] = (float)i;
        result[i] = 0.0f;
    }
    
    printf("Starting OpenACC tests to cover partition mapping...\n");
    
    // Test 1: Simple gang-level parallelism (likely "gang partitioned" or "gang redundant")
    #pragma acc parallel loop gang copyin(a[0:N], b[0:N]) copyout(c[0:N]) num_gangs(4)
    for (i = 0; i < N; i++) {
        c[i] = a[i] + b[i];
    }
    
    passed &= verify_results(c, 3.0f, N, "Test 1: Gang parallel");
    
    // Test 2: Gang+worker parallelism with reduction (may trigger "gang+worker partitioned")
    float sum = 0.0f;
    #pragma acc parallel loop gang worker reduction(+:sum) copyin(a[0:N]) num_gangs(2) num_workers(4)
    for (i = 0; i < N; i++) {
        sum += a[i];
    }
    
    if (sum != N * 1.0f) {
        printf("Test 2 failed: sum = %f, expected %f\n", sum, N * 1.0f);
        passed = 0;
    }
    
    // Test 3: Fully nested parallelism (gang+worker+vector) - may trigger "fully partitioned"
    #pragma acc parallel loop gang worker vector collapse(2) \
        copyin(matrix[0:M*P]) copyout(result[0:M*P]) \
        num_gangs(2) num_workers(2) vector_length(32)
    for (i = 0; i < M; i++) {
        for (j = 0; j < P; j++) {
            result[i * P + j] = matrix[i * P + j] * 2.0f;
        }
    }
    
    // Verify Test 3
    int correct = 1;
    #pragma acc parallel loop gang reduction(&&:correct) copyin(matrix[0:M*P], result[0:M*P])
    for (i = 0; i < M * P; i++) {
        if (result[i] != matrix[i] * 2.0f) {
            correct = 0;
        }
    }
    
    if (!correct) {
        printf("Test 3 failed: matrix multiplication incorrect\n");
        passed = 0;
    }
    
    // Test 4: Worker-only parallelism (may trigger "worker partitioned")
    // Reset array
    for (i = 0; i < N; i++) c[i] = 0.0f;
    
    #pragma acc parallel loop worker copy(c[0:N]) num_workers(8)
    for (i = 0; i < N; i++) {
        c[i] = i * 1.0f;
    }
    
    // Test 5: Vector-only parallelism (may trigger "vector partitioned")
    float *d = (float*)malloc(N * sizeof(float));
    #pragma acc parallel loop vector copyout(d[0:N]) vector_length(64)
    for (i = 0; i < N; i++) {
        d[i] = sqrtf((float)(i + 1));
    }
    
    // Test 6: Gang+vector parallelism (may trigger "gang+vector partitioned")
    float *e = (float*)malloc(N * sizeof(float));
    #pragma acc parallel loop gang vector copyout(e[0:N]) num_gangs(4) vector_length(32)
    for (i = 0; i < N; i++) {
        e[i] = sinf((float)i * 0.01f);
    }
    
    // Test 7: Worker+vector parallelism (may trigger "worker+vector partitioned")
    float *f = (float*)malloc(N * sizeof(float));
    #pragma acc parallel loop worker vector copyout(f[0:N]) num_workers(4) vector_length(16)
    for (i = 0; i < N; i++) {
        f[i] = cosf((float)i * 0.01f);
    }
    
    // Test 8: Complex nested regions with different data clauses
    // This may trigger various partition codes depending on optimization
    {
        float private_var = 0.0f;
        #pragma acc parallel copyin(a[0:N]) private(private_var) firstprivate(b[0:N]) \
            num_gangs(2) num_workers(2) vector_length(16)
        {
            #pragma acc loop gang worker
            for (i = 0; i < N/2; i++) {
                private_var = a[i] + b[i];
                #pragma acc loop vector
                for (j = 0; j < 10; j++) {
                    // Nested vector loop
                }
            }
        }
    }
    
    // Test 9: Runtime-dependent partitioning
    // Use dynamic conditions that might affect partitioning
    int use_gang = 1;
    int use_worker = 1;
    int use_vector = 1;
    
    for (int test_case = 0; test_case < 8; test_case++) {
        use_gang = (test_case & 1) != 0;
        use_worker = (test_case & 2) != 0;
        use_vector = (test_case & 4) != 0;
        
        #pragma acc parallel copy(c[0:10])
        {
            if (use_gang) {
                #pragma acc loop gang independent
                for (i = 0; i < 10; i++) {
                    if (use_worker) {
                        #pragma acc loop worker independent
                        for (j = 0; j < 5; j++) {
                            if (use_vector) {
                                #pragma acc loop vector independent
                                for (k = 0; k < 3; k++) {
                                    c[i] += 0.1f;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    // Test 10: Try to trigger potential error/default case
    // Use device management and potentially invalid operations
    acc_init(acc_device_default);
    
    // Multi-device test if available
    int num_devices = acc_get_num_devices(acc_device_default);
    if (num_devices > 0) {
        acc_set_device_num(0, acc_device_default);
        
        // Allocate device memory manually
        float *dev_ptr = (float*)acc_malloc(N * sizeof(float));
        if (dev_ptr) {
            #pragma acc parallel loop deviceptr(dev_ptr) present_or_copyin(a[0:N])
            for (i = 0; i < N; i++) {
                dev_ptr[i] = a[i] * 2.0f;
            }
            acc_free(dev_ptr);
        }
    }
    
    // Test with async operations
    int async_id = 1;
    #pragma acc parallel loop async(async_id) copy(c[0:N]) num_gangs(2)
    for (i = 0; i < N; i++) {
        c[i] = i * 2.0f;
    }
    acc_wait(async_id);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    free(e);
    free(f);
    free(matrix);
    free(result);
    
    if (passed) {
        printf("All tests passed successfully!\n");
        return 0;
    } else {
        printf("Some tests failed!\n");
        return 1;
    }
}
