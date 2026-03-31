/* Test program to cover partition code mapping in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O1 -fopenacc -foffload=disable -o test_partition test_partition.c
 * Run with: ACC_DEBUG=1 ./test_partition
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>

#define N 1024
#define M 256
#define P 128

void initialize_array(float *arr, int size, float value) {
    for (int i = 0; i < size; i++) {
        arr[i] = value + i * 0.1f;
    }
}

int verify_array(float *arr, int size, float expected_base) {
    for (int i = 0; i < size; i++) {
        float expected = expected_base + i * 0.1f;
        if (arr[i] != expected) {
            printf("Verification failed at index %d: got %f, expected %f\n",
                   i, arr[i], expected);
            return 0;
        }
    }
    return 1;
}

int main() {
    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));
    float *c = (float*)malloc(N * sizeof(float));
    float *matrix = (float*)malloc(M * P * sizeof(float));
    float *result = (float*)malloc(M * P * sizeof(float));
    
    if (!a || !b || !c || !matrix || !result) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize test data
    initialize_array(a, N, 1.0f);
    initialize_array(b, N, 2.0f);
    initialize_array(matrix, M * P, 0.5f);
    
    int success = 1;
    
    printf("Starting OpenACC partition coverage test...\n");
    
    /* Test 1: Gang redundant partitioning (likely case 0)
     * Simple parallel region with scalar reduction
     */
    printf("Test 1: Gang redundant partitioning\n");
    float sum = 0.0f;
    #pragma acc parallel loop gang reduction(+:sum) copyin(a[0:N]) copy(sum)
    for (int i = 0; i < N; i++) {
        sum += a[i];
    }
    printf("  Sum: %f\n", sum);
    
    /* Test 2: Gang partitioned (likely case 1)
     * Parallel loop with gang-level parallelism only
     */
    printf("Test 2: Gang partitioned\n");
    #pragma acc parallel loop gang copy(a[0:N], b[0:N]) copyout(c[0:N]) num_gangs(8)
    for (int i = 0; i < N; i++) {
        c[i] = a[i] + b[i];
    }
    success &= verify_array(c, 10, 3.0f); // Check first 10 elements
    
    /* Test 3: Worker partitioned (likely case 2)
     * Using worker-level parallelism with vector_length(1)
     */
    printf("Test 3: Worker partitioned\n");
    #pragma acc parallel loop worker vector_length(1) copy(a[0:N]) copyout(c[0:N])
    for (int i = 0; i < N; i++) {
        c[i] = a[i] * 2.0f;
    }
    success &= verify_array(c, 10, 2.0f);
    
    /* Test 4: Gang+worker partitioned (likely case 3)
     * Nested parallelism - outer gang, inner worker
     */
    printf("Test 4: Gang+worker partitioned\n");
    #pragma acc parallel num_gangs(4) copy(matrix[0:M*P], result[0:M*P])
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker
            for (int j = 0; j < P; j++) {
                result[i * P + j] = matrix[i * P + j] * (i + j);
            }
        }
    }
    
    /* Test 5: Vector partitioned (likely case 4)
     * Vector-level parallelism with small gang/worker counts
     */
    printf("Test 5: Vector partitioned\n");
    #pragma acc parallel loop vector num_gangs(1) num_workers(1) vector_length(32) \
        copy(a[0:N]) copyout(c[0:N])
    for (int i = 0; i < N; i++) {
        c[i] = a[i] / 2.0f;
    }
    success &= verify_array(c, 10, 0.5f);
    
    /* Test 6: Gang+vector partitioned (likely case 5)
     * Combined gang and vector parallelism
     */
    printf("Test 6: Gang+vector partitioned\n");
    #pragma acc parallel loop gang vector num_gangs(4) vector_length(16) \
        copy(a[0:N], b[0:N]) copyout(c[0:N])
    for (int i = 0; i < N; i++) {
        c[i] = a[i] - b[i];
    }
    success &= verify_array(c, 10, -1.0f);
    
    /* Test 7: Worker+vector partitioned (likely case 6)
     * Worker and vector parallelism without gang
     */
    printf("Test 7: Worker+vector partitioned\n");
    #pragma acc parallel loop worker vector num_workers(2) vector_length(8) \
        copy(a[0:N]) copyout(c[0:N])
    for (int i = 0; i < N; i++) {
        c[i] = a[i] + i * 0.01f;
    }
    
    /* Test 8: Fully partitioned (likely case 7)
     * Using all three levels: gang, worker, vector
     */
    printf("Test 8: Fully partitioned\n");
    float max_val = 0.0f;
    #pragma acc parallel loop gang worker vector reduction(max:max_val) \
        num_gangs(2) num_workers(4) vector_length(32) copyin(a[0:N]) copy(max_val)
    for (int i = 0; i < N; i++) {
        if (a[i] > max_val) max_val = a[i];
    }
    printf("  Max value: %f\n", max_val);
    
    /* Test 9: Complex data structures with runtime-dependent partitioning
     * Using conditional compilation and async operations
     */
    printf("Test 9: Runtime-dependent partitioning\n");
    int use_gang = 1;
    int use_worker = 1;
    int use_vector = 1;
    
    #pragma acc parallel copy(a[0:N], c[0:N]) \
        async(1) if(use_gang && use_worker && use_vector)
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * 3.14f;
        }
    }
    #pragma acc wait(1);
    
    /* Test 10: Multi-dimensional array with tile clause
     * Combined construct that may trigger various partition codes
     */
    printf("Test 10: Multi-dimensional with tile\n");
    #pragma acc parallel loop gang tile(32, 16) copy(matrix[0:M*P]) \
        copyout(result[0:M*P])
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            result[i * P + j] = matrix[i * P + j] * 2.0f;
        }
    }
    
    /* Test 11: Firstprivate and private clauses
     * Different data sharing patterns
     */
    printf("Test 11: Firstprivate/private data\n");
    float local_var = 10.0f;
    #pragma acc parallel loop gang firstprivate(local_var) private(b[0:N]) \
        copy(a[0:N]) copyout(c[0:N]) num_gangs(4)
    for (int i = 0; i < N; i++) {
        float temp = local_var + a[i];
        c[i] = temp;
    }
    
    /* Test 12: Device management and multi-device context
     * May trigger different internal paths
     */
    printf("Test 12: Device management\n");
    acc_device_t dev_type = acc_get_device_type();
    printf("  Device type: %d\n", dev_type);
    
    if (acc_get_num_devices(dev_type) > 0) {
        acc_set_device_num(0, dev_type);
        printf("  Set device 0\n");
    }
    
    /* Test 13: Manual data management with deviceptr
     * May exercise different broadcast patterns
     */
    printf("Test 13: Manual data management\n");
    float *d_dev = (float*)acc_malloc(N * sizeof(float));
    if (d_dev) {
        #pragma acc parallel loop present(d_dev[0:N]) copyin(a[0:N])
        for (int i = 0; i < N; i++) {
            d_dev[i] = a[i] * 2.0f;
        }
        
        // Copy back to verify
        #pragma acc update host(d_dev[0:10])
        for (int i = 0; i < 10; i++) {
            if (d_dev[i] != a[i] * 2.0f) {
                success = 0;
                break;
            }
        }
        acc_free(d_dev);
    }
    
    /* Test 14: Nested parallel regions
     * Complex hierarchy that may generate multiple partition codes
     */
    printf("Test 14: Nested parallelism\n");
    #pragma acc parallel num_gangs(2) copy(a[0:N], c[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N/2; i++) {
            #pragma acc parallel loop worker vector num_workers(2) vector_length(8)
            for (int j = 0; j < 2; j++) {
                int idx = i * 2 + j;
                if (idx < N) {
                    c[idx] = a[idx] + 100.0f;
                }
            }
        }
    }
    
    /* Test 15: Reduction with multiple variables
     * Complex reduction pattern
     */
    printf("Test 15: Multiple reductions\n");
    float sum1 = 0.0f, sum2 = 0.0f, sum3 = 0.0f;
    #pragma acc parallel loop gang worker vector \
        reduction(+:sum1, sum2, sum3) copyin(a[0:N], b[0:N]) \
        num_gangs(2) num_workers(2) vector_length(16)
    for (int i = 0; i < N; i++) {
        sum1 += a[i];
        sum2 += b[i];
        sum3 += a[i] * b[i];
    }
    printf("  Sums: %f, %f, %f\n", sum1, sum2, sum3);
    
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
