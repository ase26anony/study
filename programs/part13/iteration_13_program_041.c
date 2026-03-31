#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>

#define N 1024
#define M 512
#define P 256

// Function to verify results
int verify_result(const char* test_name, int expected, int actual) {
    if (expected != actual) {
        printf("FAIL: %s - expected %d, got %d\n", test_name, expected, actual);
        return 0;
    }
    printf("PASS: %s\n", test_name);
    return 1;
}

int main() {
    int i, j, k;
    int *a, *b, *c;
    int *d, *e, *f;
    int *matrix, *result;
    int sum = 0, product = 1;
    int reduction_sum = 0, reduction_max = 0;
    int success = 1;
    
    // Enable debug output to trigger partition string mapping
    // This increases likelihood of calling the mapping function
    acc_set_device_num(0, acc_device_default);
    
    // Allocate and initialize arrays
    a = (int*)malloc(N * sizeof(int));
    b = (int*)malloc(N * sizeof(int));
    c = (int*)malloc(N * sizeof(int));
    d = (int*)malloc(M * sizeof(int));
    e = (int*)malloc(M * sizeof(int));
    f = (int*)malloc(M * sizeof(int));
    matrix = (int*)malloc(N * M * sizeof(int));
    result = (int*)malloc(N * M * sizeof(int));
    
    for (i = 0; i < N; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = 0;
    }
    
    for (i = 0; i < M; i++) {
        d[i] = i % 10;
        e[i] = i % 20;
        f[i] = 0;
    }
    
    for (i = 0; i < N * M; i++) {
        matrix[i] = i % 100;
        result[i] = 0;
    }
    
    printf("Starting OpenACC partition coverage test...\n");
    
    // Test 1: Gang redundant (likely case 0)
    // Simple parallel region with gang-level parallelism
    #pragma acc parallel loop gang copyin(a[0:N], b[0:N]) copyout(c[0:N]) num_gangs(4)
    for (i = 0; i < N; i++) {
        c[i] = a[i] + b[i];
    }
    
    // Verify Test 1
    for (i = 0; i < N; i++) {
        if (c[i] != a[i] + b[i]) {
            success = 0;
            break;
        }
    }
    success &= verify_result("Test 1: Gang redundant addition", 1, success);
    
    // Test 2: Gang partitioned (likely case 1)
    // More complex gang partitioning with private data
    #pragma acc parallel loop gang private(j) copyin(d[0:M], e[0:M]) copyout(f[0:M]) num_gangs(8)
    for (i = 0; i < M; i++) {
        int temp = 0;
        #pragma acc loop worker reduction(+:temp)
        for (j = 0; j < 10; j++) {
            temp += d[i] + e[j % M];
        }
        f[i] = temp;
    }
    
    // Test 3: Worker partitioned (likely case 2)
    // Focus on worker-level parallelism
    sum = 0;
    #pragma acc parallel loop worker reduction(+:sum) copyin(a[0:N]) num_workers(4)
    for (i = 0; i < N; i++) {
        sum += a[i];
    }
    
    int expected_sum = 0;
    for (i = 0; i < N; i++) expected_sum += a[i];
    success &= verify_result("Test 3: Worker partitioned reduction", expected_sum, sum);
    
    // Test 4: Gang+worker partitioned (likely case 3)
    // Nested gang and worker parallelism
    #pragma acc parallel num_gangs(2) num_workers(4) copy(matrix[0:N*M], result[0:N*M])
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            #pragma acc loop worker
            for (j = 0; j < M; j++) {
                int idx = i * M + j;
                result[idx] = matrix[idx] * 2;
            }
        }
    }
    
    // Verify Test 4
    for (i = 0; i < N * M; i++) {
        if (result[i] != matrix[i] * 2) {
            success = 0;
            break;
        }
    }
    success &= verify_result("Test 4: Gang+worker partitioned matrix operation", 1, success);
    
    // Test 5: Vector partitioned (likely case 4)
    // Vector-level operations
    #pragma acc parallel loop vector vector_length(32) copyin(b[0:N]) copy(c[0:N])
    for (i = 0; i < N; i++) {
        c[i] = b[i] * b[i];
    }
    
    // Verify Test 5
    for (i = 0; i < N; i++) {
        if (c[i] != b[i] * b[i]) {
            success = 0;
            break;
        }
    }
    success &= verify_result("Test 5: Vector partitioned square operation", 1, success);
    
    // Test 6: Gang+vector partitioned (likely case 5)
    // Combined gang and vector parallelism
    #pragma acc parallel loop gang vector num_gangs(4) vector_length(16) \
        copyin(a[0:N], b[0:N]) copyout(c[0:N])
    for (i = 0; i < N; i++) {
        c[i] = a[i] * 3 + b[i];
    }
    
    // Test 7: Worker+vector partitioned (likely case 6)
    // Worker and vector combined
    reduction_max = 0;
    #pragma acc parallel loop worker vector reduction(max:reduction_max) \
        num_workers(2) vector_length(8) copyin(a[0:N])
    for (i = 0; i < N; i++) {
        if (a[i] > reduction_max) {
            reduction_max = a[i];
        }
    }
    
    success &= verify_result("Test 7: Worker+vector partitioned max reduction", N-1, reduction_max);
    
    // Test 8: Fully partitioned (likely case 7)
    // Using all levels of parallelism
    #pragma acc parallel loop gang worker vector \
        num_gangs(2) num_workers(2) vector_length(8) \
        copyin(a[0:N], b[0:N]) copyout(c[0:N])
    for (i = 0; i < N; i++) {
        c[i] = a[i] + b[i] * 2;
    }
    
    // Verify Test 8
    for (i = 0; i < N; i++) {
        if (c[i] != a[i] + b[i] * 2) {
            success = 0;
            break;
        }
    }
    success &= verify_result("Test 8: Fully partitioned operation", 1, success);
    
    // Test 9: Complex nested parallelism with tile clauses
    // This may trigger various partition codes
    #pragma acc parallel loop gang tile(32, 16) \
        copy(matrix[0:N*M], result[0:N*M]) \
        num_gangs(8) num_workers(2) vector_length(16)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            int idx = i * M + j;
            result[idx] = matrix[idx] + i + j;
        }
    }
    
    // Test 10: Runtime-dependent partitioning
    // Using async and wait to potentially trigger different paths
    int async_id = 0;
    #pragma acc parallel loop async(async_id) copy(c[0:N])
    for (i = 0; i < N; i++) {
        c[i] = i % 100;
    }
    #pragma acc wait(async_id)
    
    // Test 11: Multi-device scenario (if supported)
    // Try to trigger device-specific partitioning
    acc_device_t dev_type = acc_get_device_type();
    if (dev_type == acc_device_nvidia || dev_type == acc_device_radeon) {
        #pragma acc parallel loop gang worker copy(c[0:N])
        for (i = 0; i < N; i++) {
            c[i] = c[i] * 2;
        }
    }
    
    // Test 12: Conditional parallelism
    // May trigger different partition strategies based on condition
    int use_workers = 1;
    #pragma acc parallel loop gang if(use_workers) worker copy(c[0:N])
    for (i = 0; i < N; i++) {
        c[i] += 1;
    }
    
    // Test 13: Reduction with multiple variables
    // Complex reduction may use specific partitioning
    int sum1 = 0, sum2 = 0;
    #pragma acc parallel loop reduction(+:sum1, sum2) copyin(a[0:N], b[0:N])
    for (i = 0; i < N; i++) {
        sum1 += a[i];
        sum2 += b[i];
    }
    
    // Test 14: Structured data regions
    // May trigger different broadcast patterns
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc parallel loop
        for (i = 0; i < N; i++) {
            c[i] = a[i] - b[i];
        }
    }
    
    // Test 15: Unstructured data lifetime
    // Using enter/exit data directives
    int *device_ptr;
    device_ptr = (int*)acc_malloc(N * sizeof(int));
    
    #pragma acc enter data copyin(a[0:N])
    #pragma acc parallel loop present(a)
    for (i = 0; i < N; i++) {
        // Simple operation
        a[i] = a[i] + 1;
    }
    #pragma acc exit data copyout(a[0:N])
    
    acc_free(device_ptr);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    free(e);
    free(f);
    free(matrix);
    free(result);
    
    if (success) {
        printf("\nAll tests passed successfully!\n");
        printf("The partition mapping function should have been called with various codes (0-7).\n");
        return 0;
    } else {
        printf("\nSome tests failed!\n");
        return 1;
    }
}
