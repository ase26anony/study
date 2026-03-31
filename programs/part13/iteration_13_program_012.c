#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define N 1024
#define M 512
#define P 256

// Function to verify results
int verify_results(int *array, int size, int expected) {
    for (int i = 0; i < size; i++) {
        if (array[i] != expected) {
            printf("Verification failed at index %d: got %d, expected %d\n", 
                   i, array[i], expected);
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
    
    printf("Starting OpenACC tests to cover partition mapping...\n");
    
    // Test 1: Gang redundant partitioning (case 0)
    // Simple scalar operation that should be replicated across gangs
    printf("\nTest 1: Gang redundant partitioning\n");
    scalar = 0;
    #pragma acc parallel copy(scalar) num_gangs(4)
    {
        #pragma acc loop gang
        for (i = 0; i < 4; i++) {
            // All gangs see the same scalar value
            scalar += 1;
        }
    }
    printf("Scalar after gang redundant: %d\n", scalar);
    
    // Test 2: Gang partitioned (case 1)
    printf("\nTest 2: Gang partitioned\n");
    #pragma acc parallel copyout(a[0:N]) num_gangs(8)
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            a[i] = i * 2;
        }
    }
    
    if (!verify_results(a, N, -1)) {  // Will fail, but executes the region
        // Reset for next test
        for (i = 0; i < N; i++) a[i] = 0;
    }
    
    // Test 3: Worker partitioned (case 2)
    printf("\nTest 3: Worker partitioned\n");
    #pragma acc parallel copyout(b[0:N]) num_gangs(2) num_workers(4)
    {
        #pragma acc loop worker
        for (i = 0; i < N; i++) {
            b[i] = i * 3;
        }
    }
    
    // Test 4: Gang+worker partitioned (case 3)
    printf("\nTest 4: Gang+worker partitioned\n");
    #pragma acc parallel copyout(c[0:N*M]) num_gangs(4) num_workers(2)
    {
        #pragma acc loop gang worker collapse(2)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                c[i * M + j] = i + j;
            }
        }
    }
    
    // Test 5: Vector partitioned (case 4)
    printf("\nTest 5: Vector partitioned\n");
    #pragma acc parallel copyout(a[0:N]) vector_length(32)
    {
        #pragma acc loop vector
        for (i = 0; i < N; i++) {
            a[i] = i * 4;
        }
    }
    
    // Test 6: Gang+vector partitioned (case 5)
    printf("\nTest 6: Gang+vector partitioned\n");
    #pragma acc parallel copyout(b[0:N]) num_gangs(4) vector_length(16)
    {
        #pragma acc loop gang vector
        for (i = 0; i < N; i++) {
            b[i] = i * 5;
        }
    }
    
    // Test 7: Worker+vector partitioned (case 6)
    printf("\nTest 7: Worker+vector partitioned\n");
    #pragma acc parallel copyout(c[0:N*M]) num_workers(4) vector_length(8)
    {
        #pragma acc loop worker vector collapse(2)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                c[i * M + j] = (i * j) % 100;
            }
        }
    }
    
    // Test 8: Fully partitioned (case 7)
    printf("\nTest 8: Fully partitioned\n");
    reduction_result = 0;
    #pragma acc parallel copyin(d[0:N*M*P]) copy(reduction_result) \
        num_gangs(8) num_workers(2) vector_length(32) reduction(+:reduction_result)
    {
        #pragma acc loop gang worker vector collapse(3) reduction(+:reduction_result)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                for (k = 0; k < P; k++) {
                    reduction_result += d[i * M * P + j * P + k];
                }
            }
        }
    }
    printf("Reduction result: %d\n", reduction_result);
    
    // Test 9: Nested parallelism for complex partitioning
    printf("\nTest 9: Nested parallelism\n");
    #pragma acc parallel copy(a[0:N]) num_gangs(4)
    {
        #pragma acc loop gang independent
        for (i = 0; i < 4; i++) {
            #pragma acc loop worker vector
            for (j = i * (N/4); j < (i+1) * (N/4); j++) {
                a[j] = a[j] * 2 + 1;
            }
        }
    }
    
    // Test 10: Runtime-dependent partitioning
    printf("\nTest 10: Runtime-dependent partitioning\n");
    int dynamic_chunks = 16;
    #pragma acc parallel copyout(b[0:N]) num_gangs(dynamic_chunks)
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            b[i] = i % dynamic_chunks;
        }
    }
    
    // Test 11: Async operations with different partitioning
    printf("\nTest 11: Async operations\n");
    int async_id = 1;
    #pragma acc parallel copy(a[0:N]) async(async_id) num_gangs(4) num_workers(2)
    {
        #pragma acc loop gang worker
        for (i = 0; i < N; i++) {
            a[i] = a[i] * 3;
        }
    }
    acc_wait(async_id);
    
    // Test 12: Data regions with present clauses
    printf("\nTest 12: Data regions with explicit management\n");
    #pragma acc data copyin(a[0:N]) create(b[0:N]) copyout(c[0:N])
    {
        #pragma acc parallel present(a, b, c) num_gangs(8)
        {
            #pragma acc loop gang
            for (i = 0; i < N; i++) {
                b[i] = a[i];
                c[i] = b[i] * 2;
            }
        }
    }
    
    // Test 13: Multi-dimensional array with tile clause
    printf("\nTest 13: Tiled operations\n");
    #pragma acc parallel copy(c[0:N*M]) num_gangs(4) num_workers(2) vector_length(16)
    {
        #pragma acc loop gang worker vector tile(32, 16)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                c[i * M + j] = (i + j) * (i - j);
            }
        }
    }
    
    // Test 14: Private and firstprivate variables
    printf("\nTest 14: Private/firstprivate variables\n");
    int private_var = 100;
    #pragma acc parallel copyout(a[0:N]) firstprivate(private_var) num_gangs(4)
    {
        int local_private = private_var;
        #pragma acc loop gang private(local_private)
        for (i = 0; i < N; i++) {
            local_private = i;
            a[i] = local_private + private_var;
        }
    }
    
    // Test 15: Device-specific operations
    printf("\nTest 15: Device management\n");
    acc_init(acc_device_default);
    
    int num_devices = acc_get_num_devices(acc_device_default);
    printf("Number of devices: %d\n", num_devices);
    
    if (num_devices > 0) {
        acc_set_device_num(0, acc_device_default);
        
        #pragma acc parallel copy(a[0:N]) deviceptr(a) num_gangs(4)
        {
            #pragma acc loop gang
            for (i = 0; i < N; i++) {
                a[i] = a[i] + 1000;
            }
        }
    }
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    printf("\nAll OpenACC tests completed.\n");
    printf("Note: To see partition mapping calls, run with:\n");
    printf("  ACC_DEBUG=1 ./test_program\n");
    printf("  or\n");
    printf("  LIBGOMP_DEBUG=1 ./test_program\n");
    
    return 0;
}
