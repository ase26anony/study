/* Test program to cover partition code mapping in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O1 -fopenacc -foffload=disable -o test_partition test_partition.c
 * Run with: ACC_DEBUG=1 ./test_partition
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>

#define N 1024
#define M 512
#define P 256

void initialize_array(float *arr, int size, float value) {
    for (int i = 0; i < size; i++) {
        arr[i] = value + i * 0.1f;
    }
}

int verify_array(float *arr, int size, float expected_base) {
    for (int i = 0; i < size; i++) {
        float expected = expected_base + i * 0.1f;
        if (abs(arr[i] - expected) > 0.001f) {
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
    
    // Initialize test data
    initialize_array(a, N, 1.0f);
    initialize_array(b, N, 2.0f);
    initialize_array(c, N, 0.0f);
    initialize_array(matrix, M * P, 1.5f);
    initialize_array(result, M * P, 0.0f);
    
    int reduction_sum = 0;
    int reduction_product = 1;
    float reduction_min = 1000.0f;
    float reduction_max = -1000.0f;
    
    printf("Starting OpenACC partition coverage test...\n");
    
    /* Test 1: Simple gang-partitioned loop - likely case 1 */
    printf("Test 1: Gang partitioned computation\n");
    #pragma acc parallel loop gang copyin(a[0:N], b[0:N]) copyout(c[0:N]) num_gangs(8)
    for (int i = 0; i < N; i++) {
        c[i] = a[i] + b[i];
    }
    
    if (!verify_array(c, N, 3.0f)) {
        printf("Test 1 failed!\n");
        return 1;
    }
    
    /* Test 2: Worker partitioned with reduction - likely case 2 */
    printf("Test 2: Worker partitioned with reduction\n");
    int local_sum = 0;
    #pragma acc parallel loop worker reduction(+:local_sum) num_workers(4) vector_length(32)
    for (int i = 0; i < N; i++) {
        local_sum += (int)a[i];
    }
    reduction_sum += local_sum;
    
    /* Test 3: Gang+worker partitioned nested parallelism - likely case 3 */
    printf("Test 3: Gang+worker partitioned nested computation\n");
    #pragma acc parallel num_gangs(4) copy(matrix[0:M*P], result[0:M*P])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker
            for (int j = 0; j < P; j++) {
                result[i * P + j] = matrix[i * P + j] * 2.0f;
            }
        }
    }
    
    if (!verify_array(result, M * P, 3.0f)) {
        printf("Test 3 failed!\n");
        return 1;
    }
    
    /* Test 4: Vector partitioned with private data - likely case 4 */
    printf("Test 4: Vector partitioned computation\n");
    #pragma acc parallel loop vector private(a[0:N]) vector_length(64)
    for (int i = 0; i < N; i++) {
        float temp = a[i];
        c[i] = temp * temp;
    }
    
    /* Test 5: Gang+vector partitioned - likely case 5 */
    printf("Test 5: Gang+vector partitioned\n");
    #pragma acc parallel loop gang vector num_gangs(8) vector_length(32) \
        copyin(a[0:N]) copy(c[0:N])
    for (int i = 0; i < N; i++) {
        c[i] = a[i] * 3.0f;
    }
    
    if (!verify_array(c, N, 3.0f)) {
        printf("Test 5 failed!\n");
        return 1;
    }
    
    /* Test 6: Worker+vector partitioned with tile - likely case 6 */
    printf("Test 6: Worker+vector partitioned with tiling\n");
    #pragma acc parallel loop tile(32, 16) worker vector \
        copy(matrix[0:M*P]) copyout(result[0:M*P]) num_workers(2) vector_length(16)
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            result[i * P + j] = matrix[i * P + j] / 2.0f;
        }
    }
    
    if (!verify_array(result, M * P, 0.75f)) {
        printf("Test 6 failed!\n");
        return 1;
    }
    
    /* Test 7: Fully partitioned with complex access pattern - likely case 7 */
    printf("Test 7: Fully partitioned with complex pattern\n");
    float complex_result = 0.0f;
    #pragma acc parallel loop gang worker vector reduction(+:complex_result) \
        num_gangs(4) num_workers(2) vector_length(16) \
        copyin(a[0:N], b[0:N])
    for (int i = 0; i < N; i++) {
        complex_result += a[i] * b[i];
    }
    
    /* Test 8: Gang redundant (shared) data - likely case 0 */
    printf("Test 8: Gang redundant computation\n");
    float shared_value = 10.0f;
    #pragma acc parallel copy(shared_value) num_gangs(4)
    {
        #pragma acc loop gang
        for (int i = 0; i < 100; i++) {
            #pragma acc atomic
            shared_value += 0.1f;
        }
    }
    
    /* Test 9: Runtime-dependent partitioning */
    printf("Test 9: Runtime-dependent partitioning\n");
    int dynamic_chunk = N / (acc_get_num_devices(acc_device_default) + 1);
    if (dynamic_chunk < 1) dynamic_chunk = 1;
    
    #pragma acc parallel loop gang worker vector \
        copy(c[0:N]) copyin(a[0:N]) \
        num_gangs(N / dynamic_chunk) vector_length(32)
    for (int i = 0; i < N; i++) {
        c[i] = a[i] + i * 0.01f;
    }
    
    /* Test 10: Multiple devices and async operations */
    printf("Test 10: Multi-device async operations\n");
    int num_devices = acc_get_num_devices(acc_device_default);
    
    for (int dev = 0; dev < num_devices && dev < 2; dev++) {
        acc_set_device_num(dev, acc_device_default);
        
        #pragma acc parallel loop async(dev) gang worker \
            copy(a[0:100], c[0:100]) wait(dev)
        for (int i = 0; i < 100; i++) {
            c[i] = a[i] * 2.0f;
        }
        
        acc_async_test(dev);
    }
    
    /* Test 11: Structured data with arrays */
    printf("Test 11: Structured data handling\n");
    struct {
        float x[N];
        float y[N];
        int id;
    } data_struct;
    
    data_struct.id = 42;
    initialize_array(data_struct.x, N, 5.0f);
    initialize_array(data_struct.y, N, 0.0f);
    
    #pragma acc parallel loop copy(data_struct) gang worker vector
    for (int i = 0; i < N; i++) {
        data_struct.y[i] = data_struct.x[i] * data_struct.id;
    }
    
    /* Test 12: Conditional partitioning based on runtime values */
    printf("Test 12: Conditional partitioning\n");
    int use_workers = 1;
    #pragma acc parallel loop gang copy(c[0:N]) copyin(a[0:N]) \
        if(use_workers) worker
    for (int i = 0; i < N; i++) {
        if (i % 2 == 0) {
            c[i] = a[i] * 2.0f;
        } else {
            c[i] = a[i] / 2.0f;
        }
    }
    
    /* Test 13: Manual device memory management */
    printf("Test 13: Manual device memory\n");
    float *d_a = acc_malloc(N * sizeof(float));
    float *d_b = acc_malloc(N * sizeof(float));
    
    if (d_a && d_b) {
        acc_memcpy_to_device(d_a, a, N * sizeof(float));
        acc_memcpy_to_device(d_b, b, N * sizeof(float));
        
        #pragma acc parallel loop deviceptr(d_a, d_b) gang worker vector
        for (int i = 0; i < N; i++) {
            d_a[i] = d_a[i] + d_b[i];
        }
        
        acc_memcpy_from_device(c, d_a, N * sizeof(float));
        acc_free(d_a);
        acc_free(d_b);
    }
    
    /* Test 14: Reduction with multiple data types */
    printf("Test 14: Multi-type reductions\n");
    float float_sum = 0.0f;
    int int_sum = 0;
    
    #pragma acc parallel loop reduction(+:float_sum, int_sum) gang worker vector
    for (int i = 0; i < N; i++) {
        float_sum += a[i];
        int_sum += (int)a[i];
    }
    
    /* Test 15: Nested parallel regions */
    printf("Test 15: Nested parallel regions\n");
    #pragma acc parallel num_gangs(2) copy(c[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < 2; i++) {
            int start = i * (N / 2);
            int end = (i + 1) * (N / 2);
            
            #pragma acc parallel loop worker vector
            for (int j = start; j < end; j++) {
                c[j] = a[j] * 4.0f;
            }
        }
    }
    
    /* Final validation */
    printf("\nFinal validation...\n");
    
    // Re-initialize and run a comprehensive test
    initialize_array(a, N, 1.0f);
    initialize_array(b, N, 2.0f);
    
    float final_sum = 0.0f;
    #pragma acc parallel loop reduction(+:final_sum) gang worker vector \
        copyin(a[0:N], b[0:N])
    for (int i = 0; i < N; i++) {
        final_sum += a[i] * b[i];
    }
    
    // Expected sum: Σ(1.0 + i*0.1) * (2.0 + i*0.1) for i=0..N-1
    float expected_sum = 0.0f;
    for (int i = 0; i < N; i++) {
        float val_a = 1.0f + i * 0.1f;
        float val_b = 2.0f + i * 0.1f;
        expected_sum += val_a * val_b;
    }
    
    if (abs(final_sum - expected_sum) > 0.01f * N) {
        printf("Final validation failed: got %f, expected %f\n", final_sum, expected_sum);
        return 1;
    }
    
    printf("\nAll tests completed successfully!\n");
    printf("The partition mapping function (cases 0-7) should have been exercised.\n");
    printf("To verify coverage, run with: ACC_DEBUG=1 LIBGOMP_DEBUG=1 ./test_partition\n");
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(matrix);
    free(result);
    
    return 0;
}
