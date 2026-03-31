/* test_partition_codes.c - Exercise OpenACC partition code mapping */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>

#define N 1024
#define M 512
#define P 256

/* Helper to check errors */
void check_acc_error(const char *msg) {
    acc_device_t dev_type = acc_get_device_type();
    if (acc_async_test_all() != 0) {
        fprintf(stderr, "%s: Async operations pending\n", msg);
    }
}

int main() {
    int i, j, k;
    int *a, *b, *c, *d;
    int **matrix;
    int scalar = 42;
    int reduction_sum = 0;
    int gang_size = 2, worker_size = 4, vector_len = 32;
    
    /* Enable debug output if compiled with debug flag */
    #ifdef _DEBUG
    acc_init(acc_device_default);
    acc_set_device_num(0, acc_device_default);
    #endif
    
    /* Allocate and initialize arrays */
    a = (int*)malloc(N * sizeof(int));
    b = (int*)malloc(N * sizeof(int));
    c = (int*)malloc(N * sizeof(int));
    d = (int*)malloc(N * sizeof(int));
    matrix = (int**)malloc(M * sizeof(int*));
    
    for (i = 0; i < N; i++) {
        a[i] = i % 100;
        b[i] = (i + 1) % 100;
        c[i] = 0;
        d[i] = 0;
    }
    
    for (i = 0; i < M; i++) {
        matrix[i] = (int*)malloc(P * sizeof(int));
        for (j = 0; j < P; j++) {
            matrix[i][j] = i * j;
        }
    }
    
    printf("Testing various OpenACC partition patterns...\n");
    
    /* ===== REGION 1: Gang redundant (likely case 0) ===== */
    /* Simple parallel region with scalar - should be gang redundant */
    #pragma acc parallel copy(scalar) num_gangs(gang_size)
    {
        scalar = 100;
    }
    printf("Region 1 (gang redundant) completed\n");
    
    /* ===== REGION 2: Gang partitioned (likely case 1) ===== */
    /* Parallel loop with gang partitioning */
    #pragma acc parallel loop gang copy(a[0:N], c[0:N]) num_gangs(gang_size)
    for (i = 0; i < N; i++) {
        c[i] = a[i] * 2;
    }
    printf("Region 2 (gang partitioned) completed\n");
    
    /* ===== REGION 3: Worker partitioned (likely case 2) ===== */
    /* Worker-level parallelism */
    #pragma acc parallel loop worker copy(b[0:N], d[0:N]) \
            num_workers(worker_size) vector_length(vector_len)
    for (i = 0; i < N; i++) {
        d[i] = b[i] + i;
    }
    printf("Region 3 (worker partitioned) completed\n");
    
    /* ===== REGION 4: Gang+worker partitioned (likely case 3) ===== */
    /* Nested parallelism with gang and worker levels */
    #pragma acc parallel copy(a[0:N]) copyout(c[0:N]) \
            num_gangs(gang_size) num_workers(worker_size)
    {
        #pragma acc loop gang
        for (i = 0; i < N/64; i++) {
            #pragma acc loop worker
            for (j = 0; j < 64; j++) {
                int idx = i * 64 + j;
                if (idx < N) {
                    c[idx] = a[idx] * 3;
                }
            }
        }
    }
    printf("Region 4 (gang+worker partitioned) completed\n");
    
    /* ===== REGION 5: Vector partitioned (likely case 4) ===== */
    /* Vector-level operations */
    #pragma acc parallel loop vector copy(a[0:N], b[0:N]) \
            vector_length(vector_len)
    for (i = 0; i < N; i++) {
        b[i] = a[i] / 2;
    }
    printf("Region 5 (vector partitioned) completed\n");
    
    /* ===== REGION 6: Gang+vector partitioned (likely case 5) ===== */
    /* Combined gang and vector parallelism */
    #pragma acc parallel loop gang vector copy(a[0:N], c[0:N]) \
            num_gangs(gang_size) vector_length(vector_len)
    for (i = 0; i < N; i++) {
        c[i] = a[i] * a[i];
    }
    printf("Region 6 (gang+vector partitioned) completed\n");
    
    /* ===== REGION 7: Worker+vector partitioned (likely case 6) ===== */
    /* Worker and vector combined */
    #pragma acc parallel loop worker vector copy(b[0:N], d[0:N]) \
            num_workers(worker_size) vector_length(vector_len)
    for (i = 0; i < N; i++) {
        d[i] = b[i] - i;
    }
    printf("Region 7 (worker+vector partitioned) completed\n");
    
    /* ===== REGION 8: Fully partitioned (likely case 7) ===== */
    /* All three levels of parallelism with reduction */
    reduction_sum = 0;
    #pragma acc parallel loop gang worker vector reduction(+:reduction_sum) \
            copy(a[0:N]) copyin(b[0:N]) \
            num_gangs(gang_size) num_workers(worker_size) vector_length(vector_len)
    for (i = 0; i < N; i++) {
        reduction_sum += a[i] + b[i];
    }
    printf("Region 8 (fully partitioned) completed, reduction sum = %d\n", reduction_sum);
    
    /* ===== REGION 9: Complex nested structure ===== */
    /* Multi-dimensional array with complex access pattern */
    int temp_sum = 0;
    #pragma acc parallel copyin(matrix[0:M][0:P]) copyout(c[0:M]) \
            num_gangs(4) num_workers(2) vector_length(16)
    {
        #pragma acc loop gang
        for (i = 0; i < M; i++) {
            int row_sum = 0;
            #pragma acc loop worker vector reduction(+:row_sum)
            for (j = 0; j < P; j++) {
                row_sum += matrix[i][j];
            }
            #pragma acc atomic update
            temp_sum += row_sum;
            c[i] = row_sum;
        }
    }
    printf("Region 9 (complex nested) completed, matrix sum = %d\n", temp_sum);
    
    /* ===== REGION 10: Runtime-dependent partitioning ===== */
    /* Conditional parallelism based on runtime value */
    int use_workers = 1;
    #pragma acc parallel copy(a[0:N], b[0:N]) if(use_workers) \
            num_gangs(gang_size) num_workers(worker_size)
    {
        if (use_workers) {
            #pragma acc loop gang worker
            for (i = 0; i < N; i++) {
                b[i] = a[i] * 4;
            }
        } else {
            #pragma acc loop gang
            for (i = 0; i < N; i++) {
                b[i] = a[i] * 2;
            }
        }
    }
    printf("Region 10 (runtime-dependent) completed\n");
    
    /* ===== REGION 11: Async operations with device management ===== */
    /* Multiple devices and async queues */
    int async_id = 1;
    #pragma acc parallel loop async(async_id) copy(a[0:N], c[0:N]) \
            num_gangs(gang_size) vector_length(vector_len)
    for (i = 0; i < N; i++) {
        c[i] = a[i] * 5;
    }
    
    #pragma acc wait(async_id)
    printf("Region 11 (async operations) completed\n");
    
    /* ===== REGION 12: Tile clauses for multi-level partitioning ===== */
    /* Combined construct with tile */
    #pragma acc parallel loop tile(32, 16) copy(matrix[0:M][0:P]) \
            num_gangs(8) num_workers(2) vector_length(8)
    for (i = 0; i < M; i++) {
        for (j = 0; j < P; j++) {
            matrix[i][j] += 1;
        }
    }
    printf("Region 12 (tiled) completed\n");
    
    /* ===== REGION 13: Firstprivate and private clauses ===== */
    /* Different data sharing attributes */
    int local_var = 10;
    #pragma acc parallel loop firstprivate(local_var) private(scalar) \
            copy(a[0:N], b[0:N]) num_gangs(gang_size)
    for (i = 0; i < N; i++) {
        scalar = local_var + i;
        b[i] = a[i] + scalar;
    }
    printf("Region 13 (firstprivate/private) completed\n");
    
    /* ===== REGION 14: Multi-device code path ===== */
    /* Try to trigger device-specific partitioning */
    acc_device_t dev_type = acc_get_device_type();
    if (dev_type == acc_device_nvidia || dev_type == acc_device_radeon) {
        #pragma acc parallel loop gang worker copy(a[0:N], d[0:N]) \
                num_gangs(4) num_workers(8)
        for (i = 0; i < N; i++) {
            d[i] = a[i] * 6;
        }
        printf("Region 14 (device-specific) completed\n");
    }
    
    /* ===== REGION 15: Potential error/illegal case ===== */
    /* Try to create a situation that might trigger default case */
    /* Using deviceptr with NULL to potentially cause internal error path */
    int *null_ptr = NULL;
    #pragma acc parallel present_or_copy(a[0:10]) /* deviceptr(null_ptr) would be invalid */
    {
        #pragma acc loop
        for (i = 0; i < 10; i++) {
            a[i] = i;
        }
    }
    printf("Region 15 (potential error path) completed\n");
    
    /* Validation */
    int errors = 0;
    for (i = 0; i < 10; i++) {
        if (a[i] != i) {
            errors++;
            printf("Validation error at a[%d] = %d, expected %d\n", i, a[i], i);
        }
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    for (i = 0; i < M; i++) {
        free(matrix[i]);
    }
    free(matrix);
    
    if (errors == 0) {
        printf("\nAll tests completed successfully!\n");
        return 0;
    } else {
        printf("\nFound %d validation errors\n", errors);
        return 1;
    }
}
