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
    int i, j, k;
    int success = 1;
    
    // Enable debug output to trigger string mapping calls
    setenv("ACC_DEBUG", "1", 1);
    setenv("LIBGOMP_DEBUG", "1", 1);
    
    // Initialize arrays
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    int *c = (int*)malloc(N * sizeof(int));
    int *d = (int*)malloc(N * sizeof(int));
    int *e = (int*)malloc(N * sizeof(int));
    
    int **matrix = (int**)malloc(M * sizeof(int*));
    for (i = 0; i < M; i++) {
        matrix[i] = (int*)malloc(P * sizeof(int));
    }
    
    // Initialize data
    for (i = 0; i < N; i++) {
        a[i] = i;
        b[i] = 2 * i;
        c[i] = 0;
        d[i] = 0;
        e[i] = 0;
    }
    
    for (i = 0; i < M; i++) {
        for (j = 0; j < P; j++) {
            matrix[i][j] = i * P + j;
        }
    }
    
    printf("Starting OpenACC tests to cover partition codes...\n");
    
    // Test 1: Gang redundant (likely case 0)
    // Simple parallel region with gang-level parallelism
    #pragma acc parallel loop gang copyin(a[0:N]) copyout(c[0:N]) num_gangs(4)
    for (i = 0; i < N; i++) {
        c[i] = a[i] * 2;
    }
    success &= verify_array(c, N, 0);  // Will be overwritten
    
    // Test 2: Gang partitioned (likely case 1)
    // Explicit gang partitioning with worker and vector clauses
    #pragma acc parallel loop gang worker vector \
        copyin(a[0:N], b[0:N]) copyout(d[0:N]) \
        num_gangs(8) num_workers(2) vector_length(32)
    for (i = 0; i < N; i++) {
        d[i] = a[i] + b[i];
    }
    success &= verify_array(d, N, 3 * N / 2);  // Approximate check
    
    // Test 3: Worker partitioned (likely case 2)
    // Worker-level parallelism without gang
    #pragma acc parallel loop worker \
        copy(a[0:N]) copyout(e[0:N]) \
        num_workers(4) vector_length(16)
    for (i = 0; i < N; i++) {
        e[i] = a[i] * 3;
    }
    
    // Test 4: Gang+worker partitioned (likely case 3)
    // Nested parallelism to trigger gang+worker partitioning
    #pragma acc parallel copyin(a[0:N]) copyout(c[0:N]) num_gangs(4)
    {
        #pragma acc loop gang
        for (i = 0; i < N/4; i++) {
            #pragma acc loop worker
            for (j = 0; j < 4; j++) {
                int idx = i * 4 + j;
                if (idx < N) {
                    c[idx] = a[idx] * 4;
                }
            }
        }
    }
    
    // Test 5: Vector partitioned (likely case 4)
    // Vector-level operations
    #pragma acc parallel loop vector \
        copy(a[0:N]) copyout(d[0:N]) \
        vector_length(64)
    for (i = 0; i < N; i++) {
        d[i] = a[i] * 5;
    }
    
    // Test 6: Gang+vector partitioned (likely case 5)
    // Combined gang and vector parallelism
    #pragma acc parallel loop gang vector \
        copy(a[0:N], b[0:N]) copyout(e[0:N]) \
        num_gangs(8) vector_length(32)
    for (i = 0; i < N; i++) {
        e[i] = a[i] * b[i];
    }
    
    // Test 7: Worker+vector partitioned (likely case 6)
    // Worker and vector combined
    #pragma acc parallel loop worker vector \
        copy(a[0:N]) copyout(c[0:N]) \
        num_workers(4) vector_length(32)
    for (i = 0; i < N; i++) {
        c[i] = a[i] * 6;
    }
    
    // Test 8: Fully partitioned (likely case 7)
    // Using all levels of parallelism
    int reduction_sum = 0;
    #pragma acc parallel loop gang worker vector reduction(+:reduction_sum) \
        copy(a[0:N]) \
        num_gangs(4) num_workers(2) vector_length(16)
    for (i = 0; i < N; i++) {
        reduction_sum += a[i];
    }
    printf("Reduction sum: %d (expected: %d)\n", reduction_sum, N*(N-1)/2);
    
    // Test 9: Multi-dimensional array with tile clause
    // This may trigger various partition codes
    int matrix_sum = 0;
    #pragma acc parallel loop collapse(2) gang worker vector reduction(+:matrix_sum) \
        copy(matrix[0:M][0:P]) \
        num_gangs(4) num_workers(2) vector_length(16)
    for (i = 0; i < M; i++) {
        for (j = 0; j < P; j++) {
            matrix_sum += matrix[i][j];
        }
    }
    
    // Test 10: Runtime-dependent partitioning
    // Using async and wait to create complex execution patterns
    int async_id = 0;
    #pragma acc parallel loop gang async(async_id) \
        copy(a[0:N]) copyout(b[0:N]) \
        num_gangs(4)
    for (i = 0; i < N; i++) {
        b[i] = a[i] * 7;
    }
    
    #pragma acc parallel loop worker vector async(async_id + 1) \
        copy(b[0:N]) copyout(c[0:N]) \
        num_workers(2) vector_length(32)
    for (i = 0; i < N; i++) {
        c[i] = b[i] + 1;
    }
    
    #pragma acc wait
    
    // Test 11: Firstprivate and private clauses
    // These create different data distribution patterns
    int private_var = 42;
    #pragma acc parallel loop gang worker vector firstprivate(private_var) \
        copy(a[0:N]) copyout(d[0:N]) \
        num_gangs(2) num_workers(2) vector_length(16)
    for (i = 0; i < N; i++) {
        d[i] = a[i] + private_var;
    }
    
    // Test 12: Device-specific operations
    // Try to trigger device-specific partitioning logic
    acc_device_t dev_type = acc_get_device_type();
    printf("Device type: %d\n", dev_type);
    
    if (acc_get_num_devices(dev_type) > 0) {
        acc_set_device_num(0, dev_type);
        
        #pragma acc enter data copyin(a[0:N])
        
        #pragma acc parallel loop present(a[0:N]) copyout(e[0:N]) \
            num_gangs(4) num_workers(2) vector_length(32)
        for (i = 0; i < N; i++) {
            e[i] = a[i] * 8;
        }
        
        #pragma acc exit data delete(a[0:N])
    }
    
    // Test 13: Conditional parallelism
    // May trigger different partition paths
    int use_gang = 1;
    #pragma acc parallel loop copy(a[0:N]) copyout(b[0:N]) \
        num_gangs(use_gang ? 4 : 1) \
        num_workers(use_gang ? 2 : 4) \
        vector_length(use_gang ? 32 : 64)
    for (i = 0; i < N; i++) {
        b[i] = a[i] * (use_gang ? 9 : 10);
    }
    
    // Test 14: Complex nested regions
    // Deep nesting to stress partition logic
    #pragma acc parallel num_gangs(2)
    {
        #pragma acc loop gang
        for (i = 0; i < 2; i++) {
            #pragma acc parallel loop worker num_workers(2)
            for (j = 0; j < N/2; j++) {
                int idx = i * (N/2) + j;
                #pragma acc loop vector vector_length(16)
                for (k = 0; k < 2; k++) {
                    if (idx < N) {
                        c[idx] = idx * 11;
                    }
                }
            }
        }
    }
    
    // Test 15: Manual device memory management
    // May trigger different broadcast patterns
    int *dev_ptr = (int*)acc_malloc(N * sizeof(int));
    if (dev_ptr) {
        #pragma acc parallel loop deviceptr(dev_ptr) \
            num_gangs(4) num_workers(2) vector_length(16)
        for (i = 0; i < N; i++) {
            dev_ptr[i] = i * 12;
        }
        
        acc_memcpy_from_device(c, dev_ptr, N * sizeof(int));
        acc_free(dev_ptr);
    }
    
    // Cleanup
    for (i = 0; i < M; i++) {
        free(matrix[i]);
    }
    free(matrix);
    free(a);
    free(b);
    free(c);
    free(d);
    free(e);
    
    if (success) {
        printf("All tests completed successfully\n");
        return 0;
    } else {
        printf("Some tests failed\n");
        return 1;
    }
}
