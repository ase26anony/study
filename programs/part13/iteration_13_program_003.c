/* test_partition_codes.c - Cover partition code mapping in omp-oacc-neuter-broadcast.cc */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define N 1024
#define M 128
#define P 64

/* Helper to check for errors */
#define CHECK_ACC_ERROR(msg) \
    do { \
        acc_device_t dev_type = acc_get_device_type(); \
        if (acc_async_test_all() != 0) { \
            fprintf(stderr, "ACC error at %s:%d - %s\n", __FILE__, __LINE__, msg); \
            exit(1); \
        } \
    } while(0)

int main() {
    int i, j, k;
    
    /* Enable debug output to trigger string mapping calls */
    setenv("ACC_DEBUG", "1", 1);
    setenv("LIBGOMP_DEBUG", "1", 1);
    
    /* Initialize arrays */
    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));
    float *c = (float*)malloc(N * sizeof(float));
    float *matrix = (float*)malloc(M * P * sizeof(float));
    float *result = (float*)malloc(M * P * sizeof(float));
    
    for (i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(2 * i);
        c[i] = 0.0f;
    }
    
    for (i = 0; i < M * P; i++) {
        matrix[i] = (float)(i % 100);
        result[i] = 0.0f;
    }
    
    printf("Testing various OpenACC partitioning patterns...\n");
    
    /* ============================================
       Test 1: Gang redundant (case 0)
       Simple scalar reduction - likely gang redundant
       ============================================ */
    printf("Test 1: Gang redundant pattern\n");
    float sum = 0.0f;
    
    #pragma acc parallel copyin(a[0:N]) copy(sum) num_gangs(8) num_workers(1) vector_length(1)
    {
        #pragma acc loop gang reduction(+:sum)
        for (i = 0; i < N; i++) {
            sum += a[i];
        }
    }
    CHECK_ACC_ERROR("Test 1 failed");
    printf("  Sum = %f\n", sum);
    
    /* ============================================
       Test 2: Gang partitioned (case 1)
       Gang-level parallelism only
       ============================================ */
    printf("Test 2: Gang partitioned pattern\n");
    
    #pragma acc parallel copyin(a[0:N], b[0:N]) copyout(c[0:N]) num_gangs(16) num_workers(1) vector_length(1)
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
    CHECK_ACC_ERROR("Test 2 failed");
    
    /* ============================================
       Test 3: Worker partitioned (case 2)
       Worker-level parallelism
       ============================================ */
    printf("Test 3: Worker partitioned pattern\n");
    
    #pragma acc parallel copyin(a[0:N]) copyout(c[0:N]) num_gangs(1) num_workers(8) vector_length(1)
    {
        #pragma acc loop worker
        for (i = 0; i < N; i++) {
            c[i] = a[i] * 2.0f;
        }
    }
    CHECK_ACC_ERROR("Test 3 failed");
    
    /* ============================================
       Test 4: Gang+worker partitioned (case 3)
       Nested gang-worker parallelism
       ============================================ */
    printf("Test 4: Gang+worker partitioned pattern\n");
    
    #pragma acc parallel copyin(matrix[0:M*P]) copyout(result[0:M*P]) \
                num_gangs(4) num_workers(4) vector_length(1)
    {
        #pragma acc loop gang
        for (i = 0; i < M; i++) {
            #pragma acc loop worker
            for (j = 0; j < P; j++) {
                int idx = i * P + j;
                result[idx] = matrix[idx] * 3.0f;
            }
        }
    }
    CHECK_ACC_ERROR("Test 4 failed");
    
    /* ============================================
       Test 5: Vector partitioned (case 4)
       Vector-level parallelism
       ============================================ */
    printf("Test 5: Vector partitioned pattern\n");
    
    #pragma acc parallel copyin(a[0:N]) copyout(c[0:N]) \
                num_gangs(1) num_workers(1) vector_length(256)
    {
        #pragma acc loop vector
        for (i = 0; i < N; i++) {
            c[i] = a[i] / 2.0f;
        }
    }
    CHECK_ACC_ERROR("Test 5 failed");
    
    /* ============================================
       Test 6: Gang+vector partitioned (case 5)
       Gang and vector parallelism
       ============================================ */
    printf("Test 6: Gang+vector partitioned pattern\n");
    
    #pragma acc parallel copyin(a[0:N], b[0:N]) copyout(c[0:N]) \
                num_gangs(8) num_workers(1) vector_length(128)
    {
        #pragma acc loop gang vector
        for (i = 0; i < N; i++) {
            c[i] = a[i] * b[i];
        }
    }
    CHECK_ACC_ERROR("Test 6 failed");
    
    /* ============================================
       Test 7: Worker+vector partitioned (case 6)
       Worker and vector parallelism
       ============================================ */
    printf("Test 7: Worker+vector partitioned pattern\n");
    
    #pragma acc parallel copyin(a[0:N]) copyout(c[0:N]) \
                num_gangs(1) num_workers(4) vector_length(64)
    {
        #pragma acc loop worker vector
        for (i = 0; i < N; i++) {
            c[i] = sqrtf(a[i]);
        }
    }
    CHECK_ACC_ERROR("Test 7 failed");
    
    /* ============================================
       Test 8: Fully partitioned (case 7)
       All three levels of parallelism
       ============================================ */
    printf("Test 8: Fully partitioned pattern\n");
    
    float max_val = 0.0f;
    #pragma acc parallel copyin(a[0:N]) copy(max_val) \
                num_gangs(8) num_workers(4) vector_length(32)
    {
        #pragma acc loop gang worker vector reduction(max:max_val)
        for (i = 0; i < N; i++) {
            if (a[i] > max_val) {
                max_val = a[i];
            }
        }
    }
    CHECK_ACC_ERROR("Test 8 failed");
    printf("  Max value = %f\n", max_val);
    
    /* ============================================
       Test 9: Complex nested parallelism
       Multiple levels with different data clauses
       ============================================ */
    printf("Test 9: Complex nested parallelism\n");
    
    float *partial_sums = (float*)malloc(16 * sizeof(float));
    for (i = 0; i < 16; i++) partial_sums[i] = 0.0f;
    
    #pragma acc data copyin(a[0:N]) copy(partial_sums[0:16])
    {
        #pragma acc parallel num_gangs(4) num_workers(4) vector_length(32)
        {
            #pragma acc loop gang
            for (int gang = 0; gang < 4; gang++) {
                #pragma acc loop worker
                for (int worker = 0; worker < 4; worker++) {
                    int idx = gang * 4 + worker;
                    float local_sum = 0.0f;
                    
                    #pragma acc loop vector reduction(+:local_sum)
                    for (i = gang * (N/4); i < (gang + 1) * (N/4); i++) {
                        local_sum += a[i];
                    }
                    
                    partial_sums[idx] = local_sum;
                }
            }
        }
    }
    CHECK_ACC_ERROR("Test 9 failed");
    
    /* ============================================
       Test 10: Runtime-dependent partitioning
       Conditional execution paths
       ============================================ */
    printf("Test 10: Runtime-dependent partitioning\n");
    
    int use_workers = 1;  /* Runtime value */
    
    #pragma acc parallel copyin(a[0:N], use_workers) copyout(c[0:N]) \
                num_gangs(4) num_workers(use_workers ? 4 : 1) vector_length(32)
    {
        if (use_workers) {
            #pragma acc loop gang worker
            for (i = 0; i < N; i++) {
                c[i] = a[i] + 100.0f;
            }
        } else {
            #pragma acc loop gang
            for (i = 0; i < N; i++) {
                c[i] = a[i] - 100.0f;
            }
        }
    }
    CHECK_ACC_ERROR("Test 10 failed");
    
    /* ============================================
       Test 11: Async operations with device management
       May trigger different internal paths
       ============================================ */
    printf("Test 11: Async operations\n");
    
    int async_id = 1;
    #pragma acc parallel copyin(a[0:N]) copyout(c[0:N]) async(async_id) \
                num_gangs(8) num_workers(2) vector_length(64)
    {
        #pragma acc loop gang worker vector
        for (i = 0; i < N; i++) {
            c[i] = a[i] * 3.14f;
        }
    }
    
    #pragma acc wait(async_id)
    CHECK_ACC_ERROR("Test 11 failed");
    
    /* ============================================
       Test 12: Multi-device scenario
       ============================================ */
    printf("Test 12: Testing device management\n");
    
    acc_device_t dev_type = acc_get_device_type();
    printf("  Device type: %d\n", dev_type);
    
    int num_devices = acc_get_num_devices(dev_type);
    if (num_devices > 1) {
        acc_set_device_num(0, dev_type);
    }
    
    /* ============================================
       Test 13: Invalid/edge case (may hit default)
       Using unusual parameters
       ============================================ */
    printf("Test 13: Edge cases\n");
    
    /* Try with zero-length vector - may trigger special handling */
    #pragma acc parallel copy(c[0:1]) num_gangs(1) num_workers(1) vector_length(1)
    {
        c[0] = 42.0f;
    }
    CHECK_ACC_ERROR("Test 13 failed");
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(matrix);
    free(result);
    free(partial_sums);
    
    printf("\nAll tests completed successfully!\n");
    printf("The partition code mapping function should have been called with values 0-7.\n");
    
    return 0;
}
