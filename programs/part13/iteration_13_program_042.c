#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>

#define N 1024
#define M 256
#define P 128

// Helper function to verify results
int verify_array(int *arr, int size, int expected) {
    for (int i = 0; i < size; i++) {
        if (arr[i] != expected) {
            printf("Verification failed at index %d: got %d, expected %d\n", 
                   i, arr[i], expected);
            return 0;
        }
    }
    return 1;
}

int main() {
    int *a, *b, *c, *d;
    int scalar = 0;
    int reduction_result = 0;
    int success = 1;
    
    // Allocate and initialize arrays
    a = (int*)malloc(N * sizeof(int));
    b = (int*)malloc(N * sizeof(int));
    c = (int*)malloc(N * M * sizeof(int));
    d = (int*)malloc(N * M * P * sizeof(int));
    
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = 2 * i;
    }
    
    for (int i = 0; i < N * M; i++) {
        c[i] = i % 100;
    }
    
    for (int i = 0; i < N * M * P; i++) {
        d[i] = i % 50;
    }
    
    printf("Starting OpenACC tests to cover partition code mapping...\n");
    
    // Test 1: Gang redundant partitioning (likely case 0)
    // Simple parallel region with gang-level parallelism
    #pragma acc parallel loop gang copy(a[0:N], b[0:N]) copyout(c[0:N])
    for (int i = 0; i < N; i++) {
        c[i] = a[i] + b[i];
    }
    
    // Verify Test 1
    for (int i = 0; i < N; i++) {
        if (c[i] != 3 * i) {
            printf("Test 1 failed at index %d\n", i);
            success = 0;
            break;
        }
    }
    
    // Test 2: Gang partitioned (likely case 1)
    // Multi-dimensional array with gang partitioning
    #pragma acc parallel loop gang collapse(2) copy(c[0:N*M])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            int idx = i * M + j;
            c[idx] = c[idx] * 2;
        }
    }
    
    // Test 3: Worker partitioned (likely case 2)
    // Using worker clause explicitly
    #pragma acc parallel num_gangs(4) num_workers(8) vector_length(32) \
                copyin(a[0:N]) copyout(b[0:N])
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            b[i] = a[i] * 3;
        }
    }
    
    // Test 4: Gang+worker partitioned (likely case 3)
    // Nested parallelism with gang and worker levels
    #pragma acc parallel num_gangs(8) num_workers(4) vector_length(16) \
                copy(c[0:N*M])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                int idx = i * M + j;
                c[idx] = c[idx] + i + j;
            }
        }
    }
    
    // Test 5: Vector partitioned (likely case 4)
    // Using vector operations
    #pragma acc parallel loop vector_length(64) copy(a[0:N], b[0:N])
    for (int i = 0; i < N; i++) {
        b[i] = a[i] * a[i];
    }
    
    // Test 6: Gang+vector partitioned (likely case 5)
    // Combined gang and vector parallelism
    #pragma acc parallel loop gang vector copy(a[0:N], b[0:N])
    for (int i = 0; i < N; i++) {
        b[i] = a[i] * 4;
    }
    
    // Test 7: Worker+vector partitioned (likely case 6)
    // Combined worker and vector parallelism
    #pragma acc parallel num_workers(4) vector_length(32) \
                copy(a[0:N], b[0:N])
    {
        #pragma acc loop worker vector
        for (int i = 0; i < N; i++) {
            b[i] = a[i] + 100;
        }
    }
    
    // Test 8: Fully partitioned (likely case 7)
    // Using all three levels: gang, worker, and vector
    #pragma acc parallel loop gang worker vector \
                num_gangs(8) num_workers(4) vector_length(16) \
                copy(a[0:N], b[0:N]) reduction(+:reduction_result)
    for (int i = 0; i < N; i++) {
        reduction_result += a[i];
    }
    
    // Verify reduction result
    int expected_reduction = (N - 1) * N / 2;
    if (reduction_result != expected_reduction) {
        printf("Reduction test failed: got %d, expected %d\n", 
               reduction_result, expected_reduction);
        success = 0;
    }
    
    // Test 9: Complex nested parallelism with tile clauses
    // This may trigger various partitioning strategies
    #pragma acc parallel loop gang tile(32, 16) copy(c[0:N*M])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            int idx = i * M + j;
            c[idx] = (c[idx] * 7) % 1000;
        }
    }
    
    // Test 10: Runtime-dependent partitioning
    // Using async and wait to create complex execution patterns
    int async_id = 0;
    #pragma acc parallel loop async(async_id) copy(a[0:N], b[0:N])
    for (int i = 0; i < N; i++) {
        b[i] = a[i] * 2;
    }
    
    #pragma acc parallel loop async(async_id + 1) copy(a[0:N], c[0:N])
    for (int i = 0; i < N; i++) {
        c[i] = a[i] * 3;
    }
    
    #pragma acc wait
    
    // Test 11: Multi-dimensional array with collapse
    #pragma acc parallel loop collapse(3) copy(d[0:N*M*P])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                int idx = (i * M * P) + (j * P) + k;
                d[idx] = (d[idx] + 1) % 100;
            }
        }
    }
    
    // Test 12: Firstprivate and private clauses
    int local_var = 42;
    #pragma acc parallel loop firstprivate(local_var) private(scalar) \
                copy(a[0:N])
    for (int i = 0; i < N; i++) {
        scalar = local_var + i;
        a[i] = scalar;
    }
    
    // Test 13: Conditional parallelism
    int use_gpu = 1;  // Runtime decision
    #pragma acc parallel loop if(use_gpu) copy(a[0:N], b[0:N])
    for (int i = 0; i < N; i++) {
        b[i] = a[i] * a[i];
    }
    
    // Test 14: Device-specific operations
    // Try to trigger device-specific partitioning
    acc_device_t dev_type = acc_get_device_type();
    if (dev_type != acc_device_none) {
        #pragma acc parallel loop device_type(dev_type) copy(a[0:N], b[0:N])
        for (int i = 0; i < N; i++) {
            b[i] = a[i] + 500;
        }
    }
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    if (success) {
        printf("All tests completed successfully.\n");
        printf("The partition code mapping function should have been called with various inputs (0-7).\n");
        return 0;
    } else {
        printf("Some tests failed.\n");
        return 1;
    }
}
