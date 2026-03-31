#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define N 1024
#define M 512
#define P 256

// Function to verify results
int verify_results(int *arr, int size, int expected) {
    for (int i = 0; i < size; i++) {
        if (arr[i] != expected) {
            return 0;
        }
    }
    return 1;
}

int main() {
    int *a, *b, *c, *d;
    int scalar = 42;
    int reduction_sum = 0;
    
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
    
    printf("Starting OpenACC tests to cover partition mapping...\n");
    
    // Test 1: Gang redundant (likely case 0)
    // Simple parallel region with scalar
    #pragma acc parallel copy(scalar) num_gangs(4)
    {
        scalar = 100;
    }
    printf("Test 1 (gang redundant) completed\n");
    
    // Test 2: Gang partitioned (likely case 1)
    #pragma acc parallel loop gang copy(a[0:N]) copyout(b[0:N]) num_gangs(8)
    for (int i = 0; i < N; i++) {
        b[i] = a[i] * 2;
    }
    printf("Test 2 (gang partitioned) completed\n");
    
    // Test 3: Worker partitioned (likely case 2)
    #pragma acc parallel loop worker copy(a[0:N]) num_workers(4) vector_length(32)
    for (int i = 0; i < N; i++) {
        a[i] += 1;
    }
    printf("Test 3 (worker partitioned) completed\n");
    
    // Test 4: Gang+worker partitioned (likely case 3)
    #pragma acc parallel loop gang worker copy(c[0:N*M]) \
        num_gangs(4) num_workers(2) vector_length(16)
    for (int i = 0; i < N * M; i++) {
        c[i] = c[i] * 3;
    }
    printf("Test 4 (gang+worker partitioned) completed\n");
    
    // Test 5: Vector partitioned (likely case 4)
    #pragma acc parallel loop vector copy(a[0:N]) vector_length(64)
    for (int i = 0; i < N; i++) {
        a[i] = a[i] / 2;
    }
    printf("Test 5 (vector partitioned) completed\n");
    
    // Test 6: Gang+vector partitioned (likely case 5)
    #pragma acc parallel loop gang vector copy(b[0:N]) \
        num_gangs(2) vector_length(128)
    for (int i = 0; i < N; i++) {
        b[i] = b[i] + scalar;
    }
    printf("Test 6 (gang+vector partitioned) completed\n");
    
    // Test 7: Worker+vector partitioned (likely case 6)
    #pragma acc parallel loop worker vector copy(a[0:N]) \
        num_workers(2) vector_length(32)
    for (int i = 0; i < N; i++) {
        a[i] = a[i] - 10;
    }
    printf("Test 7 (worker+vector partitioned) completed\n");
    
    // Test 8: Fully partitioned (likely case 7)
    // Complex reduction with multi-level parallelism
    reduction_sum = 0;
    #pragma acc parallel loop gang worker vector reduction(+:reduction_sum) \
        copyin(d[0:N*M*P]) num_gangs(2) num_workers(2) vector_length(32)
    for (int i = 0; i < N * M * P; i++) {
        reduction_sum += d[i];
    }
    printf("Test 8 (fully partitioned) completed, reduction sum = %d\n", reduction_sum);
    
    // Test 9: Nested parallelism to trigger different partitioning
    #pragma acc parallel copy(a[0:N]) num_gangs(2)
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < 10; j++) {
                a[i] += j;
            }
        }
    }
    printf("Test 9 (nested parallelism) completed\n");
    
    // Test 10: Runtime-dependent partitioning
    int use_gang = 1;
    #pragma acc parallel copy(a[0:N]) if(use_gang) num_gangs(4)
    {
        if (use_gang) {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                a[i] = 999;
            }
        } else {
            #pragma acc loop vector
            for (int i = 0; i < N; i++) {
                a[i] = 888;
            }
        }
    }
    printf("Test 10 (runtime-dependent) completed\n");
    
    // Test 11: Multi-dimensional array with tile clause
    int matrix[100][100];
    #pragma acc parallel loop gang tile(10,10) copyout(matrix)
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            matrix[i][j] = i * 100 + j;
        }
    }
    printf("Test 11 (tiled 2D) completed\n");
    
    // Test 12: Async operations with wait
    int async_id = 1;
    #pragma acc parallel loop async(async_id) copy(a[0:N]) num_gangs(4)
    for (int i = 0; i < N; i++) {
        a[i] = a[i] * 2;
    }
    #pragma acc wait(async_id)
    printf("Test 12 (async) completed\n");
    
    // Test 13: Structured data with arrays
    struct {
        int x[N];
        int y[N];
    } data;
    
    #pragma acc parallel loop copy(data) num_gangs(2) num_workers(2) vector_length(32)
    for (int i = 0; i < N; i++) {
        data.x[i] = i;
        data.y[i] = N - i;
    }
    printf("Test 13 (structured data) completed\n");
    
    // Test 14: Private and firstprivate clauses
    int private_var = 100;
    #pragma acc parallel loop private(private_var) copy(a[0:N]) num_gangs(4)
    for (int i = 0; i < N; i++) {
        private_var = i;
        a[i] += private_var;
    }
    printf("Test 14 (private) completed\n");
    
    // Test 15: Device management (might trigger different paths)
    acc_device_t dev_type = acc_get_device_type();
    printf("Current device type: %d\n", dev_type);
    
    // Try to set device (might trigger error paths)
    int num_devices = acc_get_num_devices(dev_type);
    if (num_devices > 0) {
        acc_set_device_num(0, dev_type);
        printf("Set device 0 for type %d\n", dev_type);
    }
    
    // Validation
    printf("\nValidation:\n");
    
    // Verify some computations were done
    int all_ok = 1;
    
    // Check array b was computed in test 2
    #pragma acc update host(b[0:N])
    for (int i = 0; i < 10; i++) {
        if (b[i] != i * 2 + scalar - 10) { // Modified by multiple tests
            printf("Mismatch at b[%d]: expected %d, got %d\n", 
                   i, i * 2 + scalar - 10, b[i]);
            all_ok = 0;
            break;
        }
    }
    
    if (all_ok) {
        printf("All validations passed!\n");
    } else {
        printf("Some validations failed\n");
    }
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return all_ok ? 0 : 1;
}
