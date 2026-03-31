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
    int success = 1;
    
    // Allocate and initialize arrays
    a = (int*)malloc(N * sizeof(int));
    b = (int*)malloc(N * sizeof(int));
    c = (int*)malloc(N * M * sizeof(int));
    d = (int*)malloc(N * M * P * sizeof(int));
    
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = 0;
    }
    
    for (int i = 0; i < N * M; i++) {
        c[i] = i % 100;
    }
    
    for (int i = 0; i < N * M * P; i++) {
        d[i] = i % 50;
    }
    
    printf("Starting OpenACC tests to cover partition mapping cases...\n");
    
    // Test 1: Gang redundant (likely case 0)
    // Simple scalar broadcast - gang redundant
    #pragma acc parallel copyin(scalar) copyout(b[0:N]) num_gangs(4)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            b[i] = scalar + i;
        }
    }
    success &= verify_array(b, N, 0);
    
    // Test 2: Gang partitioned (likely case 1)
    #pragma acc parallel copy(a[0:N], b[0:N]) num_gangs(8)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            b[i] = a[i] * 2;
        }
    }
    success &= verify_array(b, N, 0);
    
    // Test 3: Worker partitioned (likely case 2)
    #pragma acc parallel copy(a[0:N], b[0:N]) num_workers(4) vector_length(32)
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            b[i] = a[i] + 1;
        }
    }
    success &= verify_array(b, N, 1);
    
    // Test 4: Gang+worker partitioned (likely case 3)
    // Using 2D array with nested loops
    #pragma acc parallel copy(c[0:N*M]) num_gangs(4) num_workers(2) vector_length(16)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                c[i * M + j] = c[i * M + j] * 2;
            }
        }
    }
    
    // Test 5: Vector partitioned (likely case 4)
    #pragma acc parallel copy(a[0:N], b[0:N]) vector_length(64)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            b[i] = a[i] * 3;
        }
    }
    success &= verify_array(b, N, 0);
    
    // Test 6: Gang+vector partitioned (likely case 5)
    #pragma acc parallel copy(a[0:N], b[0:N]) num_gangs(4) vector_length(32)
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            b[i] = a[i] + 10;
        }
    }
    success &= verify_array(b, N, 10);
    
    // Test 7: Worker+vector partitioned (likely case 6)
    #pragma acc parallel copy(a[0:N], b[0:N]) num_workers(2) vector_length(64)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < N; i++) {
            b[i] = a[i] * 4;
        }
    }
    success &= verify_array(b, N, 0);
    
    // Test 8: Fully partitioned (likely case 7)
    // Using reduction and all levels of parallelism
    reduction_result = 0;
    #pragma acc parallel copyin(a[0:N]) reduction(+:reduction_result) \
        num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang worker vector reduction(+:reduction_result)
        for (int i = 0; i < N; i++) {
            reduction_result += a[i];
        }
    }
    printf("Reduction result: %d (expected: %d)\n", reduction_result, N*(N-1)/2);
    
    // Test 9: Nested parallelism - may trigger different partition codes
    #pragma acc parallel copy(d[0:N*M*P]) num_gangs(2)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc parallel loop worker vector
            for (int j = 0; j < M * P; j++) {
                d[i * M * P + j] = d[i * M * P + j] + 5;
            }
        }
    }
    
    // Test 10: Conditional parallelism with runtime values
    int dynamic_size = N / 2;
    #pragma acc parallel copy(a[0:dynamic_size], b[0:dynamic_size]) if(dynamic_size > 100)
    {
        #pragma acc loop
        for (int i = 0; i < dynamic_size; i++) {
            b[i] = a[i] * 5;
        }
    }
    
    // Test 11: Async operations with data movement
    int async_id = 1;
    #pragma acc parallel copy(a[0:N], b[0:N]) async(async_id)
    {
        #pragma acc loop
        for (int i = 0; i < N; i++) {
            b[i] = a[i] * 6;
        }
    }
    acc_wait(async_id);
    
    // Test 12: Multi-dimensional array with tile clause
    #pragma acc parallel copy(c[0:N*M])
    {
        #pragma acc loop tile(32, 16)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                c[i * M + j] = c[i * M + j] / 2;
            }
        }
    }
    
    // Test 13: Firstprivate and private clauses
    int local_var = 42;
    #pragma acc parallel copyout(b[0:N]) firstprivate(local_var)
    {
        #pragma acc loop
        for (int i = 0; i < N; i++) {
            b[i] = local_var + i;
        }
    }
    
    // Test 14: Device-specific operations
    acc_device_t dev_type = acc_get_device_type();
    printf("Device type: %d\n", dev_type);
    
    // Test 15: Try to trigger potential error/default case
    // by using unusual configurations
    #pragma acc parallel copy(a[0:1]) num_gangs(1) num_workers(1) vector_length(1)
    {
        a[0] = 999;
    }
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    if (success) {
        printf("All tests completed successfully!\n");
        return 0;
    } else {
        printf("Some tests failed!\n");
        return 1;
    }
}
