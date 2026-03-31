#include <stdio.h>
#include <stdlib.h>
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
    int i, j, k;
    
    // Allocate and initialize arrays
    a = (int*)malloc(N * sizeof(int));
    b = (int*)malloc(N * sizeof(int));
    c = (int*)malloc(N * M * sizeof(int));
    d = (int*)malloc(N * M * P * sizeof(int));
    
    for (i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N - i;
    }
    
    for (i = 0; i < N * M; i++) {
        c[i] = i % 100;
    }
    
    for (i = 0; i < N * M * P; i++) {
        d[i] = i % 50;
    }
    
    printf("Starting OpenACC tests to cover partition code mapping...\n");
    
    // Test 1: Gang redundant partitioning (likely case 0)
    // Simple parallel region with gang-level parallelism
    #pragma acc parallel loop gang copy(a[0:N], b[0:N]) copyout(c[0:N])
    for (i = 0; i < N; i++) {
        c[i] = a[i] + b[i];
    }
    
    if (!verify_array(c, N, N)) {
        printf("Test 1 failed\n");
        return 1;
    }
    
    // Test 2: Gang partitioned (likely case 1)
    // Using gang clause explicitly
    #pragma acc parallel num_gangs(4) gang copy(a[0:N])
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            a[i] *= 2;
        }
    }
    
    // Test 3: Worker partitioned (likely case 2)
    // Using worker-level parallelism
    #pragma acc parallel num_workers(8) vector_length(32) \
        copyin(a[0:N]) copyout(b[0:N])
    {
        #pragma acc loop worker
        for (i = 0; i < N; i++) {
            b[i] = a[i] * 3;
        }
    }
    
    if (!verify_array(b, N, 2 * i * 3)) {
        printf("Test 3 failed\n");
        return 1;
    }
    
    // Test 4: Gang+worker partitioned (likely case 3)
    // Nested parallelism with gang and worker
    #pragma acc parallel num_gangs(2) num_workers(4) vector_length(16) \
        copy(c[0:N*M])
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            #pragma acc loop worker
            for (j = 0; j < M; j++) {
                c[i * M + j] = i + j;
            }
        }
    }
    
    // Test 5: Vector partitioned (likely case 4)
    // Using vector-level parallelism
    #pragma acc parallel vector_length(64) copy(a[0:N])
    {
        #pragma acc loop vector
        for (i = 0; i < N; i++) {
            a[i] = a[i] + 1;
        }
    }
    
    // Test 6: Gang+vector partitioned (likely case 5)
    // Combined gang and vector parallelism
    #pragma acc parallel loop gang vector \
        num_gangs(8) vector_length(32) \
        copy(a[0:N]) copyout(b[0:N])
    for (i = 0; i < N; i++) {
        b[i] = a[i] * a[i];
    }
    
    // Test 7: Worker+vector partitioned (likely case 6)
    // Combined worker and vector parallelism
    #pragma acc parallel loop worker vector \
        num_workers(4) vector_length(64) \
        copy(c[0:N*M])
    for (i = 0; i < N * M; i++) {
        c[i] = c[i] % 17;
    }
    
    // Test 8: Fully partitioned (likely case 7)
    // Using all three levels of parallelism with reduction
    reduction_result = 0;
    #pragma acc parallel loop gang worker vector reduction(+:reduction_result) \
        num_gangs(4) num_workers(2) vector_length(32) \
        copyin(a[0:N])
    for (i = 0; i < N; i++) {
        reduction_result += a[i];
    }
    
    printf("Reduction result: %d\n", reduction_result);
    
    // Test 9: Complex nested structure with tile clauses
    // This may trigger various partitioning strategies
    #pragma acc parallel loop gang tile(32, 16) \
        copy(d[0:N*M*P])
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            for (k = 0; k < P; k++) {
                int idx = (i * M * P) + (j * P) + k;
                d[idx] = (d[idx] + i + j + k) % 23;
            }
        }
    }
    
    // Test 10: Runtime-dependent partitioning
    // Using async and wait to create complex execution patterns
    int async_id = 0;
    #pragma acc parallel loop async(async_id) \
        copy(a[0:N]) copyout(b[0:N])
    for (i = 0; i < N; i++) {
        b[i] = a[i] * 2;
    }
    
    #pragma acc wait(async_id)
    
    // Test 11: Firstprivate and private clauses
    // These create different data distribution patterns
    int private_var = 42;
    #pragma acc parallel loop firstprivate(private_var) \
        private(scalar) copyout(c[0:N])
    for (i = 0; i < N; i++) {
        c[i] = private_var + i;
        scalar = i;  // private copy
    }
    
    // Test 12: Multi-device test if supported
    // Try to trigger different device-specific partitioning
    acc_device_t dev_type = acc_get_device_type();
    printf("Current device type: %d\n", dev_type);
    
    // Test 13: Manual device data management
    // Using acc_create and acc_copyin
    int *device_a = (int*)acc_create(a, N * sizeof(int));
    int *device_b = (int*)acc_create(b, N * sizeof(int));
    
    #pragma acc parallel present(device_a[0:N], device_b[0:N])
    {
        #pragma acc loop
        for (i = 0; i < N; i++) {
            device_b[i] = device_a[i] * 3;
        }
    }
    
    acc_copyout(b, N * sizeof(int));
    acc_delete(a, N * sizeof(int));
    
    // Test 14: Conditional parallelism
    // May trigger different code paths
    int use_gang = 1;
    #pragma acc parallel if(use_gang) num_gangs(4) \
        copy(a[0:N])
    {
        if (use_gang) {
            #pragma acc loop gang
            for (i = 0; i < N; i++) {
                a[i] += 100;
            }
        } else {
            #pragma acc loop
            for (i = 0; i < N; i++) {
                a[i] += 50;
            }
        }
    }
    
    // Final verification
    printf("Running final verification...\n");
    
    // Verify Test 8 reduction makes sense
    int expected_sum = 0;
    for (i = 0; i < N; i++) {
        expected_sum += (2 * i + 101);  // Account for modifications
    }
    
    if (abs(reduction_result - expected_sum) > expected_sum * 0.01) {
        printf("Reduction verification failed: got %d, expected ~%d\n", 
               reduction_result, expected_sum);
        return 1;
    }
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    printf("All tests completed successfully!\n");
    printf("Note: To see partition code mapping in action, run with:\n");
    printf("  ACC_DEBUG=1 ./test_program\n");
    printf("  or\n");
    printf("  LIBGOMP_DEBUG=1 ./test_program\n");
    
    return 0;
}
