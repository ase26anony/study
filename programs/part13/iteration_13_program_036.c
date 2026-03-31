/* Test program to cover partition code mapping in omp-oacc-neuter-broadcast.cc */
#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define N 1024
#define M 256
#define P 128

/* Helper to initialize arrays */
void init_array(float *arr, int size, float value) {
    for (int i = 0; i < size; i++) {
        arr[i] = value + i;
    }
}

/* Helper to verify results */
int verify_array(float *arr, int size, float expected_base) {
    for (int i = 0; i < size; i++) {
        if (arr[i] != expected_base + i) {
            printf("Verification failed at index %d: got %f, expected %f\n",
                   i, arr[i], expected_base + i);
            return 0;
        }
    }
    return 1;
}

int main() {
    int i, j, k;
    int success = 1;
    
    /* Allocate test arrays */
    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));
    float *c = (float*)malloc(N * sizeof(float));
    float *matrix = (float*)malloc(M * P * sizeof(float));
    float *result = (float*)malloc(M * P * sizeof(float));
    
    /* Initialize data */
    init_array(a, N, 1.0f);
    init_array(b, N, 2.0f);
    init_array(c, N, 0.0f);
    init_array(matrix, M * P, 1.5f);
    init_array(result, M * P, 0.0f);
    
    printf("Starting OpenACC tests to cover partition codes...\n");
    
    /* Test 1: Simple gang-redundant operation (likely partition code 0) */
    printf("Test 1: Gang redundant operation\n");
    #pragma acc parallel copyin(a[0:N], b[0:N]) copyout(c[0:N]) num_gangs(4)
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
    success &= verify_array(c, N, 3.0f);
    
    /* Test 2: Gang partitioned (likely partition code 1) */
    printf("Test 2: Gang partitioned with reduction\n");
    float sum = 0.0f;
    #pragma acc parallel copyin(a[0:N]) copy(sum) reduction(+:sum) num_gangs(8)
    {
        #pragma acc loop gang reduction(+:sum)
        for (i = 0; i < N; i++) {
            sum += a[i];
        }
    }
    printf("Reduction sum: %f\n", sum);
    
    /* Test 3: Worker partitioned (likely partition code 2) */
    printf("Test 3: Worker partitioned operation\n");
    #pragma acc parallel copyin(a[0:N], b[0:N]) copyout(c[0:N]) \
                num_gangs(2) num_workers(4)
    {
        #pragma acc loop worker
        for (i = 0; i < N; i++) {
            c[i] = a[i] * b[i];
        }
    }
    success &= verify_array(c, N, 2.0f);
    
    /* Test 4: Gang+worker partitioned (likely partition code 3) */
    printf("Test 4: Gang+worker partitioned (2D operation)\n");
    #pragma acc parallel copyin(matrix[0:M*P]) copyout(result[0:M*P]) \
                num_gangs(4) num_workers(4)
    {
        #pragma acc loop gang
        for (i = 0; i < M; i++) {
            #pragma acc loop worker
            for (j = 0; j < P; j++) {
                result[i * P + j] = matrix[i * P + j] * 2.0f;
            }
        }
    }
    success &= verify_array(result, M * P, 3.0f);
    
    /* Test 5: Vector partitioned (likely partition code 4) */
    printf("Test 5: Vector partitioned operation\n");
    #pragma acc parallel copyin(a[0:N], b[0:N]) copyout(c[0:N]) \
                vector_length(32)
    {
        #pragma acc loop vector
        for (i = 0; i < N; i++) {
            c[i] = a[i] - b[i];
        }
    }
    success &= verify_array(c, N, -1.0f);
    
    /* Test 6: Gang+vector partitioned (likely partition code 5) */
    printf("Test 6: Gang+vector partitioned\n");
    #pragma acc parallel copyin(a[0:N], b[0:N]) copyout(c[0:N]) \
                num_gangs(4) vector_length(64)
    {
        #pragma acc loop gang vector
        for (i = 0; i < N; i++) {
            c[i] = a[i] / (b[i] + 0.001f);
        }
    }
    
    /* Test 7: Worker+vector partitioned (likely partition code 6) */
    printf("Test 7: Worker+vector partitioned\n");
    #pragma acc parallel copyin(a[0:N], b[0:N]) copyout(c[0:N]) \
                num_workers(4) vector_length(32)
    {
        #pragma acc loop worker vector
        for (i = 0; i < N; i++) {
            c[i] = b[i] - a[i];
        }
    }
    success &= verify_array(c, N, 1.0f);
    
    /* Test 8: Fully partitioned - gang+worker+vector (likely partition code 7) */
    printf("Test 8: Fully partitioned (gang+worker+vector)\n");
    float total_sum = 0.0f;
    #pragma acc parallel copyin(matrix[0:M*P]) copy(total_sum) \
                num_gangs(2) num_workers(2) vector_length(32) \
                reduction(+:total_sum)
    {
        #pragma acc loop gang worker vector reduction(+:total_sum)
        for (i = 0; i < M * P; i++) {
            total_sum += matrix[i];
        }
    }
    printf("Fully partitioned reduction: %f\n", total_sum);
    
    /* Test 9: Nested parallelism to trigger different partitioning */
    printf("Test 9: Nested parallel regions\n");
    #pragma acc parallel copy(c[0:N]) num_gangs(2)
    {
        #pragma acc loop gang
        for (i = 0; i < 2; i++) {
            int start = i * (N/2);
            int end = (i + 1) * (N/2);
            
            #pragma acc parallel loop worker vector copy(c[start:end-start]) \
                        num_workers(2) vector_length(16)
            for (j = start; j < end; j++) {
                c[j] = c[j] * 2.0f;
            }
        }
    }
    
    /* Test 10: Runtime-dependent partitioning */
    printf("Test 10: Runtime-dependent partitioning\n");
    int use_gang = 1;
    int use_worker = 1;
    int use_vector = 1;
    
    #pragma acc parallel copyin(a[0:N], b[0:N]) copyout(c[0:N]) \
                copyin(use_gang, use_worker, use_vector)
    {
        if (use_gang && use_worker && use_vector) {
            #pragma acc loop gang worker vector
            for (i = 0; i < N; i++) {
                c[i] = a[i] + b[i] * 2.0f;
            }
        } else if (use_gang && use_worker) {
            #pragma acc loop gang worker
            for (i = 0; i < N; i++) {
                c[i] = a[i] + b[i];
            }
        } else if (use_gang) {
            #pragma acc loop gang
            for (i = 0; i < N; i++) {
                c[i] = a[i];
            }
        }
    }
    success &= verify_array(c, N, 5.0f);
    
    /* Test 11: Async operations with different queues */
    printf("Test 11: Async operations\n");
    int async_id = 1;
    #pragma acc parallel async(async_id) copy(c[0:N]) num_gangs(4)
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            c[i] = i * 1.5f;
        }
    }
    acc_wait(async_id);
    
    /* Test 12: Tile clause for multi-level partitioning */
    printf("Test 12: Tiled operations\n");
    #pragma acc parallel copy(matrix[0:M*P]) num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop tile(32, 16) gang worker vector
        for (i = 0; i < M; i++) {
            for (j = 0; j < P; j++) {
                matrix[i * P + j] += 1.0f;
            }
        }
    }
    
    /* Test 13: Private and firstprivate clauses */
    printf("Test 13: Private/firstprivate variables\n");
    float private_var = 10.0f;
    #pragma acc parallel copyout(c[0:N]) firstprivate(private_var) \
                num_gangs(4) num_workers(2)
    {
        #pragma acc loop gang worker private(private_var)
        for (i = 0; i < N; i++) {
            float local_var = private_var + i;
            #pragma acc loop vector
            for (j = 0; j < 4; j++) {
                c[i] = local_var + j;
            }
        }
    }
    
    /* Test 14: Device management and multi-device context */
    printf("Test 14: Device management\n");
    acc_device_t dev_type = acc_get_device_type();
    printf("Current device type: %d\n", dev_type);
    
    int num_devices = acc_get_num_devices(dev_type);
    printf("Number of devices: %d\n", num_devices);
    
    if (num_devices > 0) {
        acc_set_device_num(0, dev_type);
        
        /* Allocate device memory directly */
        float *d_dev = (float*)acc_malloc(N * sizeof(float));
        if (d_dev) {
            #pragma acc parallel present(d_dev[0:N]) num_gangs(4)
            {
                #pragma acc loop gang
                for (i = 0; i < N; i++) {
                    d_dev[i] = i * 2.0f;
                }
            }
            
            /* Copy back to host */
            #pragma acc update host(d_dev[0:N])
            
            acc_free(d_dev);
        }
    }
    
    /* Test 15: Conditional compilation for debug output */
    printf("Test 15: Final validation\n");
    #ifdef _DEBUG
    printf("Debug mode enabled - may trigger partition logging\n");
    #endif
    
    /* Final validation */
    init_array(a, N, 0.0f);
    init_array(b, N, 1.0f);
    
    #pragma acc parallel copyin(a[0:N], b[0:N]) copyout(c[0:N]) \
                num_gangs(8) num_workers(4) vector_length(64)
    {
        #pragma acc loop gang worker vector
        for (i = 0; i < N; i++) {
            c[i] = a[i] + b[i] * 3.0f;
        }
    }
    success &= verify_array(c, N, 3.0f);
    
    /* Cleanup */
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
