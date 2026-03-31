#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>

#define N 1024
#define M 256
#define P 128

// Helper function to initialize arrays
void init_array(float *arr, int size, float value) {
    for (int i = 0; i < size; i++) {
        arr[i] = value + (i % 100) * 0.1f;
    }
}

// Helper function to verify results
int verify_array(float *arr, int size, float expected) {
    for (int i = 0; i < size; i++) {
        if (arr[i] != expected) {
            printf("Verification failed at index %d: got %f, expected %f\n", 
                   i, arr[i], expected);
            return 0;
        }
    }
    return 1;
}

int main() {
    int i, j, k;
    int success = 1;
    
    // Enable debug output to trigger partition string mapping
    // This environment variable may cause the runtime to call the mapping function
    setenv("ACC_DEBUG", "1", 1);
    setenv("LIBGOMP_DEBUG", "1", 1);
    
    // Allocate and initialize test arrays
    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));
    float *c = (float*)malloc(N * sizeof(float));
    float *matrix = (float*)malloc(M * P * sizeof(float));
    float *result = (float*)malloc(M * P * sizeof(float));
    
    init_array(a, N, 1.0f);
    init_array(b, N, 2.0f);
    init_array(c, N, 0.0f);
    init_array(matrix, M * P, 1.5f);
    init_array(result, M * P, 0.0f);
    
    printf("Starting OpenACC tests to cover partition mapping cases...\n");
    
    // Test 1: Simple gang-redundant computation (likely case 0)
    printf("\nTest 1: Gang redundant computation\n");
    #pragma acc parallel copyin(a[0:N], b[0:N]) copyout(c[0:N]) num_gangs(4)
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
    
    // Verify Test 1
    for (i = 0; i < N; i++) {
        if (c[i] != a[i] + b[i]) {
            printf("Test 1 failed at index %d\n", i);
            success = 0;
            break;
        }
    }
    
    // Test 2: Gang partitioned with reduction (likely case 1)
    printf("\nTest 2: Gang partitioned with reduction\n");
    float sum = 0.0f;
    #pragma acc parallel copyin(a[0:N]) copy(sum) reduction(+:sum) num_gangs(8)
    {
        #pragma acc loop gang reduction(+:sum)
        for (i = 0; i < N; i++) {
            sum += a[i];
        }
    }
    printf("Reduction sum: %f\n", sum);
    
    // Test 3: Worker partitioned computation (likely case 2)
    printf("\nTest 3: Worker partitioned computation\n");
    #pragma acc parallel copy(c[0:N]) num_workers(4) vector_length(32)
    {
        #pragma acc loop worker
        for (i = 0; i < N; i++) {
            c[i] = i * 0.5f;
        }
    }
    
    // Test 4: Gang+worker partitioned (likely case 3)
    printf("\nTest 4: Gang+worker partitioned\n");
    #pragma acc parallel copyin(matrix[0:M*P]) copyout(result[0:M*P]) \
                num_gangs(4) num_workers(2) vector_length(16)
    {
        #pragma acc loop gang worker
        for (i = 0; i < M; i++) {
            for (j = 0; j < P; j++) {
                result[i * P + j] = matrix[i * P + j] * 2.0f;
            }
        }
    }
    
    // Verify Test 4
    for (i = 0; i < M * P; i++) {
        if (result[i] != matrix[i] * 2.0f) {
            printf("Test 4 failed at index %d\n", i);
            success = 0;
            break;
        }
    }
    
    // Test 5: Vector partitioned (likely case 4)
    printf("\nTest 5: Vector partitioned\n");
    #pragma acc parallel copy(c[0:N]) vector_length(64)
    {
        #pragma acc loop vector
        for (i = 0; i < N; i++) {
            c[i] = c[i] * 3.0f;
        }
    }
    
    // Test 6: Gang+vector partitioned (likely case 5)
    printf("\nTest 6: Gang+vector partitioned\n");
    #pragma acc parallel copy(c[0:N]) num_gangs(4) vector_length(32)
    {
        #pragma acc loop gang vector
        for (i = 0; i < N; i++) {
            c[i] = c[i] + 1.0f;
        }
    }
    
    // Test 7: Worker+vector partitioned (likely case 6)
    printf("\nTest 7: Worker+vector partitioned\n");
    #pragma acc parallel copy(c[0:N]) num_workers(2) vector_length(64)
    {
        #pragma acc loop worker vector
        for (i = 0; i < N; i++) {
            c[i] = c[i] * 0.5f;
        }
    }
    
    // Test 8: Fully partitioned - gang+worker+vector (likely case 7)
    printf("\nTest 8: Fully partitioned (gang+worker+vector)\n");
    float total_sum = 0.0f;
    #pragma acc parallel copyin(a[0:N], b[0:N]) copy(total_sum) \
                num_gangs(4) num_workers(2) vector_length(32) \
                reduction(+:total_sum)
    {
        #pragma acc loop gang worker vector reduction(+:total_sum)
        for (i = 0; i < N; i++) {
            total_sum += a[i] * b[i];
        }
    }
    printf("Fully partitioned reduction result: %f\n", total_sum);
    
    // Test 9: Nested parallelism to explore different partition combinations
    printf("\nTest 9: Nested parallelism\n");
    #pragma acc parallel copy(c[0:N]) num_gangs(2)
    {
        #pragma acc loop gang
        for (i = 0; i < 2; i++) {
            int start = i * (N/2);
            int end = (i + 1) * (N/2);
            
            #pragma acc parallel loop worker vector copy(c[start:end-start]) \
                        num_workers(2) vector_length(16)
            for (j = start; j < end; j++) {
                c[j] = c[j] * 2.0f + j;
            }
        }
    }
    
    // Test 10: Dynamic partitioning based on runtime conditions
    printf("\nTest 10: Dynamic partitioning\n");
    int use_gang = 1;
    int use_worker = 1;
    int use_vector = 1;
    
    #pragma acc parallel copy(c[0:N]) copyin(use_gang, use_worker, use_vector) \
                num_gangs(use_gang ? 4 : 1) \
                num_workers(use_worker ? 2 : 1) \
                vector_length(use_vector ? 32 : 1)
    {
        if (use_gang && use_worker && use_vector) {
            #pragma acc loop gang worker vector
            for (i = 0; i < N; i++) {
                c[i] = c[i] / 2.0f;
            }
        } else if (use_gang && use_worker) {
            #pragma acc loop gang worker
            for (i = 0; i < N; i++) {
                c[i] = c[i] + 10.0f;
            }
        } else if (use_gang) {
            #pragma acc loop gang
            for (i = 0; i < N; i++) {
                c[i] = c[i] - 5.0f;
            }
        }
    }
    
    // Test 11: Multi-dimensional array with tile clause
    printf("\nTest 11: Multi-dimensional with tile clause\n");
    float mat1[M][P], mat2[M][P], mat3[M][P];
    
    // Initialize matrices
    for (i = 0; i < M; i++) {
        for (j = 0; j < P; j++) {
            mat1[i][j] = 1.0f;
            mat2[i][j] = 2.0f;
            mat3[i][j] = 0.0f;
        }
    }
    
    #pragma acc parallel copyin(mat1, mat2) copyout(mat3) \
                num_gangs(4) num_workers(2) vector_length(16)
    {
        #pragma acc loop gang worker tile(32, 16)
        for (i = 0; i < M; i++) {
            for (j = 0; j < P; j++) {
                mat3[i][j] = mat1[i][j] + mat2[i][j];
            }
        }
    }
    
    // Verify Test 11
    for (i = 0; i < M; i++) {
        for (j = 0; j < P; j++) {
            if (mat3[i][j] != 3.0f) {
                printf("Test 11 failed at [%d][%d]\n", i, j);
                success = 0;
                break;
            }
        }
        if (!success) break;
    }
    
    // Test 12: Async operations with wait
    printf("\nTest 12: Async operations\n");
    int async_id = 1;
    
    #pragma acc parallel copy(c[0:N]) async(async_id) num_gangs(4)
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            c[i] = 100.0f;
        }
    }
    
    #pragma acc wait(async_id)
    
    // Test 13: Firstprivate and private clauses
    printf("\nTest 13: Firstprivate and private clauses\n");
    float base = 10.0f;
    #pragma acc parallel copyout(c[0:N]) firstprivate(base) num_gangs(4)
    {
        float local;  // private by default
        #pragma acc loop gang private(local)
        for (i = 0; i < N; i++) {
            local = base + i;
            c[i] = local;
        }
    }
    
    // Test 14: Structured data with arrays
    printf("\nTest 14: Structured data\n");
    struct {
        float x[N];
        float y[N];
        float z[N];
    } point_data;
    
    #pragma acc parallel copy(point_data) num_gangs(4) num_workers(2)
    {
        #pragma acc loop gang worker
        for (i = 0; i < N; i++) {
            point_data.x[i] = i * 1.0f;
            point_data.y[i] = i * 2.0f;
            point_data.z[i] = i * 3.0f;
        }
    }
    
    // Test 15: Device management and multi-device context
    printf("\nTest 15: Device management\n");
    acc_device_t dev_type = acc_get_device_type();
    printf("Current device type: %d\n", dev_type);
    
    int num_devices = acc_get_num_devices(dev_type);
    printf("Number of devices: %d\n", num_devices);
    
    if (num_devices > 0) {
        acc_set_device_num(0, dev_type);
        
        // Run a computation on the selected device
        #pragma acc parallel copy(c[0:N]) device_type(dev_type) num_gangs(4)
        {
            #pragma acc loop gang
            for (i = 0; i < N; i++) {
                c[i] = c[i] * 0.1f;
            }
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
