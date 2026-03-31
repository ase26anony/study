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
    float *a, *b, *c, *d, *e;
    float *dev_a, *dev_b;
    float reduction_sum = 0.0f;
    int i, j, k;
    
    /* Allocate and initialize host arrays */
    a = (float*)malloc(N * sizeof(float));
    b = (float*)malloc(N * sizeof(float));
    c = (float*)malloc(N * M * sizeof(float));
    d = (float*)malloc(N * M * P * sizeof(float));
    e = (float*)malloc(N * sizeof(float));
    
    if (!a || !b || !c || !d || !e) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    initialize_array(a, N, 1.0f);
    initialize_array(b, N, 2.0f);
    initialize_array(e, N, 0.0f);
    
    /* Region 1: Simple gang-partitioned loop - likely case 1 */
    printf("Region 1: Gang partitioned\n");
    #pragma acc parallel loop gang copy(a[0:N], b[0:N]) copyout(c[0:N*M]) \
        num_gangs(8) vector_length(32)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            c[i * M + j] = a[i] * b[i] + j * 0.5f;
        }
    }
    
    /* Region 2: Fully partitioned with reduction - likely case 7 */
    printf("Region 2: Fully partitioned with reduction\n");
    reduction_sum = 0.0f;
    #pragma acc parallel loop gang worker vector reduction(+:reduction_sum) \
        copyin(a[0:N]) copy(reduction_sum) \
        num_gangs(4) num_workers(2) vector_length(64)
    for (i = 0; i < N; i++) {
        reduction_sum += a[i];
    }
    printf("Reduction sum: %f\n", reduction_sum);
    
    /* Region 3: Worker partitioned - likely case 2 */
    printf("Region 3: Worker partitioned\n");
    #pragma acc parallel loop worker copy(a[0:N]) copyout(b[0:N]) \
        num_workers(4)
    for (i = 0; i < N; i++) {
        b[i] = a[i] * 2.0f;
    }
    
    /* Region 4: Vector partitioned - likely case 4 */
    printf("Region 4: Vector partitioned\n");
    #pragma acc parallel loop vector copy(a[0:N], b[0:N]) copyout(e[0:N]) \
        vector_length(128)
    for (i = 0; i < N; i++) {
        e[i] = a[i] + b[i];
    }
    
    /* Region 5: Gang+worker partitioned - likely case 3 */
    printf("Region 5: Gang+worker partitioned\n");
    #pragma acc parallel loop gang worker copy(c[0:N*M]) \
        num_gangs(2) num_workers(4)
    for (i = 0; i < N; i++) {
        float row_sum = 0.0f;
        for (j = 0; j < M; j++) {
            row_sum += c[i * M + j];
        }
        b[i] = row_sum / M;
    }
    
    /* Region 6: Gang+vector partitioned - likely case 5 */
    printf("Region 6: Gang+vector partitioned\n");
    #pragma acc parallel loop gang vector copy(a[0:N], b[0:N]) copyout(e[0:N]) \
        num_gangs(8) vector_length(32)
    for (i = 0; i < N; i++) {
        e[i] = sinf(a[i]) * cosf(b[i]);
    }
    
    /* Region 7: Worker+vector partitioned - likely case 6 */
    printf("Region 7: Worker+vector partitioned\n");
    #pragma acc parallel loop worker vector copy(a[0:N]) copyout(b[0:N]) \
        num_workers(2) vector_length(64)
    for (i = 0; i < N; i++) {
        b[i] = sqrtf(fabsf(a[i]));
    }
    
    /* Region 8: Nested parallelism - may trigger multiple partition types */
    printf("Region 8: Nested parallelism\n");
    #pragma acc parallel copyin(a[0:N]) copyout(d[0:N*M*P]) \
        num_gangs(2)
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            #pragma acc loop worker
            for (j = 0; j < M; j++) {
                #pragma acc loop vector
                for (k = 0; k < P; k++) {
                    d[(i * M + j) * P + k] = a[i] + j * 0.01f + k * 0.001f;
                }
            }
        }
    }
    
    /* Region 9: Combined construct with tile clause */
    printf("Region 9: Combined construct with tile\n");
    #pragma acc parallel loop gang tile(32, 16) copy(c[0:N*M]) \
        num_gangs(16)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            c[i * M + j] = c[i * M + j] * 1.1f;
        }
    }
    
    /* Region 10: Runtime-dependent partitioning */
    printf("Region 10: Runtime-dependent partitioning\n");
    int use_workers = 1;  /* Runtime value */
    if (use_workers) {
        #pragma acc parallel loop gang worker copy(a[0:N], b[0:N]) copyout(e[0:N]) \
            num_gangs(4) num_workers(2)
        for (i = 0; i < N; i++) {
            e[i] = a[i] - b[i];
        }
    } else {
        #pragma acc parallel loop gang copy(a[0:N], b[0:N]) copyout(e[0:N]) \
            num_gangs(8)
        for (i = 0; i < N; i++) {
            e[i] = a[i] + b[i];
        }
    }
    
    /* Region 11: Async operations with wait */
    printf("Region 11: Async operations\n");
    int async_id = 1;
    #pragma acc parallel loop gang async(async_id) copy(a[0:N]) copyout(b[0:N]) \
        num_gangs(4)
    for (i = 0; i < N; i++) {
        b[i] = a[i] * 3.0f;
    }
    #pragma acc wait(async_id)
    
    /* Region 12: Firstprivate and private clauses */
    printf("Region 12: Firstprivate and private\n");
    float base_value = 5.0f;
    #pragma acc parallel loop gang vector firstprivate(base_value) \
        private(i) copyout(e[0:N]) \
        num_gangs(4) vector_length(32)
    for (i = 0; i < N; i++) {
        float private_val = base_value + i * 0.01f;
        e[i] = private_val;
    }
    
    /* Region 13: Multi-dimensional array with complex access pattern */
    printf("Region 13: Complex access pattern\n");
    int *perm = (int*)malloc(N * sizeof(int));
    for (i = 0; i < N; i++) {
        perm[i] = (i * 37) % N;  /* Create a permutation */
    }
    
    #pragma acc parallel loop gang copy(perm[0:N], a[0:N]) copyout(b[0:N]) \
        num_gangs(8)
    for (i = 0; i < N; i++) {
        b[i] = a[perm[i]];
    }
    
    free(perm);
    
    /* Region 14: Device memory operations */
    printf("Region 14: Device memory operations\n");
    dev_a = (float*)acc_malloc(N * sizeof(float));
    dev_b = (float*)acc_malloc(N * sizeof(float));
    
    if (dev_a && dev_b) {
        acc_memcpy_to_device(dev_a, a, N * sizeof(float));
        
        #pragma acc parallel loop deviceptr(dev_a, dev_b) \
            num_gangs(4) num_workers(2) vector_length(32)
        for (i = 0; i < N; i++) {
            dev_b[i] = dev_a[i] * 2.0f;
        }
        
        acc_memcpy_from_device(b, dev_b, N * sizeof(float));
        acc_free(dev_a);
        acc_free(dev_b);
    }
    
    /* Region 15: Structure with arrays */
    printf("Region 15: Structure with arrays\n");
    typedef struct {
        float x[N];
        float y[N];
    } PointArray;
    
    PointArray points;
    initialize_array(points.x, N, 1.5f);
    initialize_array(points.y, N, 2.5f);
    
    #pragma acc parallel loop gang copy(points) \
        num_gangs(4)
    for (i = 0; i < N; i++) {
        points.x[i] = points.x[i] + points.y[i];
        points.y[i] = points.x[i] * points.y[i];
    }
    
    /* Verification */
    printf("\nVerifying results...\n");
    
    /* Verify Region 3 results */
    if (!verify_array(b, N, 4.0f)) {
        printf("Region 3 verification failed\n");
    }
    
    /* Verify Region 4 results */
    if (!verify_array(e, N, 3.0f)) {
        printf("Region 4 verification failed\n");
    }
    
    /* Verify Region 12 results */
    initialize_array(e, N, 0.0f);
    #pragma acc parallel loop gang vector firstprivate(base_value) \
        private(i) copyout(e[0:N]) \
        num_gangs(4) vector_length(32)
    for (i = 0; i < N; i++) {
        float private_val = base_value + i * 0.01f;
        e[i] = private_val;
    }
    
    int success = 1;
    for (i = 0; i < N; i++) {
        float expected = 5.0f + i * 0.01f;
        if (fabs(e[i] - expected) > 0.0001f) {
            printf("Region 12 verification failed at %d: %f != %f\n",
                   i, e[i], expected);
            success = 0;
            break;
        }
    }
    
    if (success) {
        printf("All verifications passed!\n");
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    free(e);
    
    /* Try to trigger potential error/illegal case */
    printf("\nAttempting to trigger error conditions...\n");
    
    /* This might trigger internal error paths */
    acc_set_device_num(999, acc_device_default);  /* Invalid device number */
    
    /* Try with null pointers (should fail gracefully) */
    float *null_ptr = NULL;
    #pragma acc parallel loop gang copy(null_ptr[0:10]) \
        num_gangs(1)
    for (i = 0; i < 10; i++) {
        /* This should not execute */
    }
    
    return 0;
}
