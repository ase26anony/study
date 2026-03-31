#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>

#define N 1024
#define M 256
#define P 128

// Enable debug output to trigger partition string mapping
void enable_acc_debug() {
    // Try to set debug environment variables
    // These may cause the runtime to call the partition mapping function
    setenv("ACC_DEBUG", "1", 1);
    setenv("LIBGOMP_DEBUG", "1", 1);
    setenv("GACC_DEBUG", "3", 1);
}

// Initialize arrays with test data
void init_arrays(float *a, float *b, float *c, int size) {
    for (int i = 0; i < size; i++) {
        a[i] = (float)(i % 100);
        b[i] = (float)((i + 1) % 100);
        c[i] = 0.0f;
    }
}

// Verify results
int verify_results(float *c, float expected, int size) {
    for (int i = 0; i < size; i++) {
        if (c[i] != expected) {
            printf("Verification failed at index %d: got %f, expected %f\n", 
                   i, c[i], expected);
            return 0;
        }
    }
    return 1;
}

int main() {
    // Enable debug to increase chance of partition mapping calls
    enable_acc_debug();
    
    float *a, *b, *c, *d, *e;
    float *a_dev, *b_dev, *c_dev;
    float reduction_sum = 0.0f;
    int success = 1;
    
    // Allocate host memory
    a = (float*)malloc(N * sizeof(float));
    b = (float*)malloc(N * sizeof(float));
    c = (float*)malloc(N * sizeof(float));
    d = (float*)malloc(N * M * sizeof(float));
    e = (float*)malloc(N * M * sizeof(float));
    
    if (!a || !b || !c || !d || !e) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize arrays
    init_arrays(a, b, c, N);
    init_arrays(d, e, NULL, N * M);
    
    printf("Starting OpenACC tests to cover partition mapping...\n");
    
    // Test 1: Simple gang-partitioned parallel loop
    // Likely maps to "gang partitioned" (case 1)
    printf("Test 1: Gang-partitioned parallel loop\n");
    #pragma acc parallel loop gang copy(a[0:N], b[0:N], c[0:N]) \
        num_gangs(4) vector_length(32)
    for (int i = 0; i < N; i++) {
        c[i] = a[i] + b[i];
    }
    
    // Test 2: Fully partitioned with gang, worker, vector
    // Likely maps to "fully partitioned" (case 7)
    printf("Test 2: Fully partitioned with reduction\n");
    reduction_sum = 0.0f;
    #pragma acc parallel loop gang worker vector reduction(+:reduction_sum) \
        copy(a[0:N]) copyin(b[0:N]) num_gangs(2) num_workers(4) vector_length(16)
    for (int i = 0; i < N; i++) {
        reduction_sum += a[i] + b[i];
    }
    printf("Reduction sum: %f\n", reduction_sum);
    
    // Test 3: Worker-partitioned region
    // Likely maps to "worker partitioned" (case 2)
    printf("Test 3: Worker-partitioned region\n");
    #pragma acc parallel num_workers(8) vector_length(64) \
        copyout(c[0:N])
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * 2.0f;
        }
    }
    
    // Test 4: Vector-partitioned loop
    // Likely maps to "vector partitioned" (case 4)
    printf("Test 4: Vector-partitioned loop\n");
    #pragma acc parallel loop vector copy(a[0:N], c[0:N]) vector_length(128)
    for (int i = 0; i < N; i++) {
        c[i] = a[i] * 3.0f;
    }
    
    // Test 5: Gang+worker partitioned (nested parallelism)
    // Likely maps to "gang+worker partitioned" (case 3)
    printf("Test 5: Gang+worker partitioned (2D array)\n");
    #pragma acc parallel copy(d[0:N*M], e[0:N*M]) num_gangs(4) num_workers(2)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                int idx = i * M + j;
                e[idx] = d[idx] * 2.0f;
            }
        }
    }
    
    // Test 6: Gang+vector partitioned
    // Likely maps to "gang+vector partitioned" (case 5)
    printf("Test 6: Gang+vector partitioned\n");
    #pragma acc parallel loop gang vector copy(a[0:N], c[0:N]) \
        num_gangs(2) vector_length(64)
    for (int i = 0; i < N; i++) {
        c[i] = a[i] + i;
    }
    
    // Test 7: Worker+vector partitioned
    // Likely maps to "worker+vector partitioned" (case 6)
    printf("Test 7: Worker+vector partitioned\n");
    #pragma acc parallel loop worker vector copy(a[0:N], c[0:N]) \
        num_workers(4) vector_length(32)
    for (int i = 0; i < N; i++) {
        c[i] = a[i] - b[i];
    }
    
    // Test 8: Gang redundant (no partitioning)
    // Likely maps to "gang redundant" (case 0)
    printf("Test 8: Gang redundant (broadcast)\n");
    float broadcast_val = 42.0f;
    #pragma acc parallel copyout(c[0:N]) num_gangs(1)
    {
        // All gangs get the same value
        float local_val = broadcast_val;
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            c[i] = local_val + i;
        }
    }
    
    // Test 9: Complex combined construct with tile
    // May trigger various partition mappings
    printf("Test 9: Combined parallel loop with tile\n");
    #pragma acc parallel loop gang tile(32, 16) \
        copy(d[0:N*M], e[0:N*M]) num_gangs(8)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            int idx = i * M + j;
            e[idx] = d[idx] / 2.0f;
        }
    }
    
    // Test 10: Runtime-dependent partitioning
    // May explore different code paths
    printf("Test 10: Runtime-dependent partitioning\n");
    int use_workers = 1;  // Runtime value
    #pragma acc parallel copy(a[0:N], c[0:N]) \
        num_gangs(use_workers ? 2 : 4) num_workers(use_workers ? 4 : 1)
    {
        if (use_workers) {
            #pragma acc loop gang worker
            for (int i = 0; i < N; i++) {
                c[i] = a[i] * 1.5f;
            }
        } else {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                c[i] = a[i] * 0.5f;
            }
        }
    }
    
    // Test 11: Async operations with device memory
    // May trigger different internal paths
    printf("Test 11: Async operations with device memory\n");
    a_dev = (float*)acc_malloc(N * sizeof(float));
    b_dev = (float*)acc_malloc(N * sizeof(float));
    c_dev = (float*)acc_malloc(N * sizeof(float));
    
    if (a_dev && b_dev && c_dev) {
        acc_memcpy_to_device(a_dev, a, N * sizeof(float));
        acc_memcpy_to_device(b_dev, b, N * sizeof(float));
        
        int async_id = 1;
        #pragma acc parallel deviceptr(a_dev, b_dev, c_dev) async(async_id) \
            num_gangs(4) num_workers(2) vector_length(32)
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < N; i++) {
                c_dev[i] = a_dev[i] + b_dev[i];
            }
        }
        
        acc_wait(async_id);
        acc_memcpy_from_device(c, c_dev, N * sizeof(float));
        
        acc_free(a_dev);
        acc_free(b_dev);
        acc_free(c_dev);
    }
    
    // Test 12: Multi-device test (if available)
    // May trigger device-specific partitioning
    printf("Test 12: Multi-device context\n");
    int num_devices = acc_get_num_devices(acc_device_default);
    printf("Number of devices: %d\n", num_devices);
    
    if (num_devices > 1) {
        // Try to use different devices
        for (int dev = 0; dev < num_devices && dev < 2; dev++) {
            acc_set_device_num(dev, acc_device_default);
            
            #pragma acc parallel loop copy(a[0:N], c[0:N]) \
                num_gangs(2) vector_length(64)
            for (int i = 0; i < N; i++) {
                c[i] = a[i] + dev;  // Different computation per device
            }
        }
    }
    
    // Test 13: Invalid/edge case that might trigger default case
    // Potentially maps to "<illegal>" (default case)
    printf("Test 13: Edge case with large dimensions\n");
    {
        // Use very large loop with small gang count
        // This might trigger unusual partitioning decisions
        int small_n = 16;
        #pragma acc parallel loop gang copy(a[0:small_n], c[0:small_n]) \
            num_gangs(32)  // More gangs than iterations
        for (int i = 0; i < small_n; i++) {
            c[i] = a[i] * 10.0f;
        }
    }
    
    // Verification
    printf("\nVerifying results...\n");
    
    // Verify Test 1 results
    for (int i = 0; i < N; i++) {
        if (c[i] != a[i] + b[i]) {
            printf("Test 1 verification failed at index %d\n", i);
            success = 0;
            break;
        }
    }
    
    if (success) {
        printf("All tests completed successfully!\n");
    } else {
        printf("Some tests failed verification\n");
    }
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    free(e);
    
    return success ? 0 : 1;
}
