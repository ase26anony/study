#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>

#define N 1024
#define M 128
#define P 64

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
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    int *c = (int*)malloc(N * sizeof(int));
    int *d = (int*)malloc(N * sizeof(int));
    int *e = (int*)malloc(N * sizeof(int));
    int *f = (int*)malloc(N * sizeof(int));
    int *g = (int*)malloc(N * sizeof(int));
    int *h = (int*)malloc(N * sizeof(int));
    
    int *matrix = (int*)malloc(M * P * sizeof(int));
    int *result = (int*)malloc(M * P * sizeof(int));
    
    // Initialize arrays
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = 0;
        d[i] = 0;
        e[i] = 0;
        f[i] = 0;
        g[i] = 0;
        h[i] = 0;
    }
    
    for (int i = 0; i < M * P; i++) {
        matrix[i] = i % 100;
        result[i] = 0;
    }
    
    int sum = 0;
    int reduction_result = 0;
    
    printf("Starting OpenACC tests...\n");
    
    // Test 1: Simple gang-partitioned loop (likely case 1)
    #pragma acc parallel loop gang copyin(a[0:N], b[0:N]) copyout(c[0:N]) num_gangs(4)
    for (int i = 0; i < N; i++) {
        c[i] = a[i] + b[i];
    }
    
    if (!verify_array(c, N, a[0] + b[0])) {
        printf("Test 1 failed\n");
        return 1;
    }
    
    // Test 2: Worker-partitioned loop (likely case 2)
    #pragma acc parallel loop worker copyin(a[0:N]) copyout(d[0:N]) num_workers(8)
    for (int i = 0; i < N; i++) {
        d[i] = a[i] * 2;
    }
    
    if (!verify_array(d, N, a[0] * 2)) {
        printf("Test 2 failed\n");
        return 1;
    }
    
    // Test 3: Vector-partitioned loop (likely case 4)
    #pragma acc parallel loop vector copyin(a[0:N]) copyout(e[0:N]) vector_length(32)
    for (int i = 0; i < N; i++) {
        e[i] = a[i] + 1;
    }
    
    if (!verify_array(e, N, a[0] + 1)) {
        printf("Test 3 failed\n");
        return 1;
    }
    
    // Test 4: Gang+worker partitioned (likely case 3)
    #pragma acc parallel loop gang worker copyin(a[0:N], b[0:N]) copyout(f[0:N]) \
        num_gangs(2) num_workers(4)
    for (int i = 0; i < N; i++) {
        f[i] = a[i] - b[i];
    }
    
    if (!verify_array(f, N, a[0] - b[0])) {
        printf("Test 4 failed\n");
        return 1;
    }
    
    // Test 5: Gang+vector partitioned (likely case 5)
    #pragma acc parallel loop gang vector copyin(a[0:N]) copyout(g[0:N]) \
        num_gangs(4) vector_length(16)
    for (int i = 0; i < N; i++) {
        g[i] = a[i] * 3;
    }
    
    if (!verify_array(g, N, a[0] * 3)) {
        printf("Test 5 failed\n");
        return 1;
    }
    
    // Test 6: Worker+vector partitioned (likely case 6)
    #pragma acc parallel loop worker vector copyin(a[0:N]) copyout(h[0:N]) \
        num_workers(8) vector_length(32)
    for (int i = 0; i < N; i++) {
        h[i] = a[i] / 2;
    }
    
    if (!verify_array(h, N, a[0] / 2)) {
        printf("Test 6 failed\n");
        return 1;
    }
    
    // Test 7: Fully partitioned - gang+worker+vector (likely case 7)
    #pragma acc parallel loop gang worker vector copyin(a[0:N], b[0:N]) copy(c[0:N]) \
        num_gangs(2) num_workers(4) vector_length(16)
    for (int i = 0; i < N; i++) {
        c[i] = a[i] + b[i] + 1;
    }
    
    if (!verify_array(c, N, a[0] + b[0] + 1)) {
        printf("Test 7 failed\n");
        return 1;
    }
    
    // Test 8: Reduction with gang partitioning (may trigger gang redundant - case 0)
    sum = 0;
    #pragma acc parallel loop gang reduction(+:sum) copyin(a[0:N]) num_gangs(4)
    for (int i = 0; i < N; i++) {
        sum += a[i];
    }
    
    int expected_sum = 0;
    for (int i = 0; i < N; i++) expected_sum += a[i];
    if (sum != expected_sum) {
        printf("Test 8 failed: sum = %d, expected = %d\n", sum, expected_sum);
        return 1;
    }
    
    // Test 9: Nested parallelism - outer gang, inner worker
    #pragma acc parallel copyin(matrix[0:M*P]) copyout(result[0:M*P]) num_gangs(2)
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker
            for (int j = 0; j < P; j++) {
                result[i * P + j] = matrix[i * P + j] * 2;
            }
        }
    }
    
    // Test 10: Multi-dimensional array with tile clause
    #pragma acc parallel loop gang tile(16, 8) copyin(matrix[0:M*P]) copy(result[0:M*P]) \
        num_gangs(4)
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            result[i * P + j] = matrix[i * P + j] + 10;
        }
    }
    
    // Test 11: Async operations with wait
    int async_id = 1;
    #pragma acc parallel loop async(async_id) copyin(a[0:N]) copyout(d[0:N]) \
        num_gangs(4) num_workers(2) vector_length(32)
    for (int i = 0; i < N; i++) {
        d[i] = a[i] * 5;
    }
    #pragma acc wait(async_id)
    
    if (!verify_array(d, N, a[0] * 5)) {
        printf("Test 11 failed\n");
        return 1;
    }
    
    // Test 12: Conditional parallelism
    int use_parallel = 1;
    #pragma acc parallel loop if(use_parallel) copyin(a[0:N]) copyout(e[0:N]) \
        num_gangs(2) num_workers(4) vector_length(16)
    for (int i = 0; i < N; i++) {
        e[i] = a[i] - 10;
    }
    
    if (!verify_array(e, N, a[0] - 10)) {
        printf("Test 12 failed\n");
        return 1;
    }
    
    // Test 13: Private variables
    int private_var = 100;
    #pragma acc parallel loop private(private_var) copyin(a[0:N]) copyout(f[0:N]) \
        num_gangs(4)
    for (int i = 0; i < N; i++) {
        private_var = a[i];
        f[i] = private_var + 20;
    }
    
    if (!verify_array(f, N, a[0] + 20)) {
        printf("Test 13 failed\n");
        return 1;
    }
    
    // Test 14: Firstprivate variables
    int firstprivate_var = 50;
    #pragma acc parallel loop firstprivate(firstprivate_var) copyin(a[0:N]) copyout(g[0:N]) \
        num_gangs(2) num_workers(2)
    for (int i = 0; i < N; i++) {
        g[i] = a[i] + firstprivate_var;
    }
    
    if (!verify_array(g, N, a[0] + 50)) {
        printf("Test 14 failed\n");
        return 1;
    }
    
    // Test 15: Complex reduction with multiple levels
    reduction_result = 0;
    #pragma acc parallel loop gang worker vector reduction(+:reduction_result) \
        copyin(a[0:N], b[0:N]) num_gangs(2) num_workers(4) vector_length(8)
    for (int i = 0; i < N; i++) {
        reduction_result += a[i] * b[i];
    }
    
    // Test 16: Device management
    acc_device_t dev_type = acc_get_device_type();
    printf("Device type: %d\n", (int)dev_type);
    
    // Test 17: Multi-device scenario simulation
    #pragma acc set device_num(0)
    
    // Test 18: Manual data management with deviceptr
    int *device_a = (int*)acc_malloc(N * sizeof(int));
    if (device_a) {
        #pragma acc parallel loop present(device_a[0:N]) num_gangs(4)
        for (int i = 0; i < N; i++) {
            device_a[i] = i * 10;
        }
        acc_free(device_a);
    }
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    free(e);
    free(f);
    free(g);
    free(h);
    free(matrix);
    free(result);
    
    printf("All tests completed successfully!\n");
    
    // To potentially trigger the default case (<illegal>), we could try:
    // 1. Invalid device operations (commented out to avoid crashes)
    // 2. Corrupted data pointers (not safe to include in production test)
    // 3. Boundary conditions that might cause internal errors
    
    return 0;
}
