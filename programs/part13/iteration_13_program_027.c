#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>

#define N 1024
#define M 256
#define P 128

// Enable debug output to trigger the mapping function
void enable_acc_debug() {
    // Try to set debug environment variable
    putenv("ACC_DEBUG=1");
    putenv("LIBGOMP_DEBUG=1");
    putenv("GACC_DEBUG=1");
}

// Initialize array with test data
void init_array(double *arr, int size, double value) {
    for (int i = 0; i < size; i++) {
        arr[i] = value + i * 0.1;
    }
}

// Verify array contents
int verify_array(double *arr, int size, double expected_base) {
    for (int i = 0; i < size; i++) {
        double expected = expected_base + i * 0.1;
        if (abs(arr[i] - expected) > 1e-6) {
            printf("Verification failed at index %d: got %f, expected %f\n", 
                   i, arr[i], expected);
            return 0;
        }
    }
    return 1;
}

int main() {
    enable_acc_debug();
    
    // Allocate test arrays
    double *a = (double*)malloc(N * sizeof(double));
    double *b = (double*)malloc(N * sizeof(double));
    double *c = (double*)malloc(N * sizeof(double));
    double *matrix = (double*)malloc(M * P * sizeof(double));
    double *result = (double*)malloc(M * P * sizeof(double));
    
    // Initialize arrays
    init_array(a, N, 1.0);
    init_array(b, N, 2.0);
    init_array(c, N, 0.0);
    init_array(matrix, M * P, 1.5);
    init_array(result, M * P, 0.0);
    
    int success = 1;
    
    printf("Starting OpenACC tests to cover partition mapping...\n");
    
    // Test 1: Simple gang-partitioned loop (likely case 1)
    printf("Test 1: Gang partitioned\n");
    #pragma acc parallel loop gang copyin(a[0:N], b[0:N]) copyout(c[0:N]) num_gangs(8)
    for (int i = 0; i < N; i++) {
        c[i] = a[i] + b[i];
    }
    success &= verify_array(c, N, 3.0);
    
    // Test 2: Worker partitioned (likely case 2)
    printf("Test 2: Worker partitioned\n");
    double worker_sum = 0.0;
    #pragma acc parallel loop worker reduction(+:worker_sum) copyin(a[0:100]) num_workers(4)
    for (int i = 0; i < 100; i++) {
        worker_sum += a[i];
    }
    printf("Worker sum: %f\n", worker_sum);
    
    // Test 3: Vector partitioned (likely case 4)
    printf("Test 3: Vector partitioned\n");
    #pragma acc parallel loop vector copy(c[0:N]) vector_length(32)
    for (int i = 0; i < N; i++) {
        c[i] = c[i] * 2.0;
    }
    success &= verify_array(c, N, 6.0);
    
    // Test 4: Gang+worker partitioned (likely case 3)
    printf("Test 4: Gang+worker partitioned\n");
    double gw_sum = 0.0;
    #pragma acc parallel num_gangs(4) num_workers(2) reduction(+:gw_sum) copyin(a[0:N])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            gw_sum += a[i];
        }
    }
    printf("Gang+worker sum: %f\n", gw_sum);
    
    // Test 5: Gang+vector partitioned (likely case 5)
    printf("Test 5: Gang+vector partitioned\n");
    #pragma acc parallel loop gang vector copy(c[0:N]) num_gangs(4) vector_length(16)
    for (int i = 0; i < N; i++) {
        c[i] = c[i] / 2.0;
    }
    success &= verify_array(c, N, 3.0);
    
    // Test 6: Worker+vector partitioned (likely case 6)
    printf("Test 6: Worker+vector partitioned\n");
    #pragma acc parallel loop worker vector copy(c[0:N]) num_workers(2) vector_length(8)
    for (int i = 0; i < N; i++) {
        c[i] = c[i] + i * 0.01;
    }
    
    // Test 7: Fully partitioned - gang+worker+vector (likely case 7)
    printf("Test 7: Fully partitioned\n");
    double total_sum = 0.0;
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(16) \
                copyin(a[0:N], b[0:N]) reduction(+:total_sum)
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < N; i++) {
            total_sum += a[i] + b[i];
        }
    }
    printf("Fully partitioned sum: %f\n", total_sum);
    
    // Test 8: Gang redundant (likely case 0)
    printf("Test 8: Gang redundant\n");
    double gang_var = 0.0;
    #pragma acc parallel num_gangs(4) copy(gang_var)
    {
        #pragma acc loop gang
        for (int i = 0; i < 10; i++) {
            // All gangs do the same work
            gang_var += 1.0;
        }
    }
    printf("Gang redundant result: %f\n", gang_var);
    
    // Test 9: Nested parallelism with different data clauses
    printf("Test 9: Nested parallelism with tile\n");
    #pragma acc parallel copyin(matrix[0:M*P]) copyout(result[0:M*P]) \
                num_gangs(2) num_workers(2) vector_length(8)
    {
        #pragma acc loop gang tile(32, 32)
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker tile(8, 8)
            for (int j = 0; j < P; j++) {
                int idx = i * P + j;
                result[idx] = matrix[idx] * 2.0;
            }
        }
    }
    
    // Test 10: Runtime-dependent partitioning
    printf("Test 10: Runtime-dependent partitioning\n");
    int use_workers = 1;
    #pragma acc parallel copy(use_workers) if(use_workers)
    {
        if (use_workers) {
            #pragma acc loop worker
            for (int i = 0; i < 50; i++) {
                // Some work
            }
        }
    }
    
    // Test 11: Async operations with device management
    printf("Test 11: Async operations\n");
    int async_id = 1;
    #pragma acc parallel loop async(async_id) copy(c[0:N]) num_gangs(4)
    for (int i = 0; i < N; i++) {
        c[i] = c[i] * 1.5;
    }
    acc_wait(async_id);
    
    // Test 12: Multi-dimensional array with complex access pattern
    printf("Test 12: Complex access pattern\n");
    int *perm = (int*)malloc(N * sizeof(int));
    for (int i = 0; i < N; i++) perm[i] = (i * 7) % N;
    
    #pragma acc parallel loop copy(c[0:N]) copyin(perm[0:N]) num_gangs(8) num_workers(2)
    for (int i = 0; i < N; i++) {
        int idx = perm[i];
        c[idx] = c[idx] + 1.0;
    }
    free(perm);
    
    // Test 13: Structure with arrays
    printf("Test 13: Structure with arrays\n");
    struct {
        double x[N];
        double y[N];
    } point_data;
    
    #pragma acc parallel loop copy(point_data) num_gangs(4)
    for (int i = 0; i < N; i++) {
        point_data.x[i] = i * 0.1;
        point_data.y[i] = i * 0.2;
    }
    
    // Test 14: Reduction with multiple variables
    printf("Test 14: Multi-variable reduction\n");
    double sum1 = 0.0, sum2 = 0.0;
    #pragma acc parallel loop reduction(+:sum1, sum2) copyin(a[0:N], b[0:N]) \
                num_gangs(4) num_workers(2) vector_length(16)
    for (int i = 0; i < N; i++) {
        sum1 += a[i];
        sum2 += b[i];
    }
    printf("Reduction sums: %f, %f\n", sum1, sum2);
    
    // Test 15: Conditional compilation for different devices
    printf("Test 15: Device-specific code\n");
    acc_device_t dev_type = acc_get_device_type();
    printf("Device type: %d\n", dev_type);
    
    // Try to trigger potential error/illegal cases
    printf("Test 16: Testing edge cases\n");
    {
        // This might trigger internal error paths
        double *null_ptr = NULL;
        #pragma acc parallel present_or_copy(null_ptr[0:0])
        {
            // Empty parallel region
        }
    }
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(matrix);
    free(result);
    
    if (success) {
        printf("\nAll tests completed successfully!\n");
        return 0;
    } else {
        printf("\nSome tests failed!\n");
        return 1;
    }
}
