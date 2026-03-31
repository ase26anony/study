/* test_partition_codes.c - Cover partition code mapping in omp-oacc-neuter-broadcast.cc */
/* Compile with: gcc -O1 -fopenacc -foffload=disable -ftree-parallelize-loops=0 -o test_partition test_partition_codes.c */
/* Run with: ACC_DEBUG=1 LIBGOMP_DEBUG=1 ./test_partition */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>

#define N 1024
#define M 128
#define P 64

/* Helper to initialize arrays */
void init_array(float *arr, int size, float val) {
    for (int i = 0; i < size; i++) {
        arr[i] = val + i * 0.1f;
    }
}

/* Validate results */
int verify_array(float *arr, int size, float expected_base) {
    for (int i = 0; i < size; i++) {
        float expected = expected_base + i * 0.1f;
        if (abs(arr[i] - expected) > 0.001f) {
            printf("Mismatch at index %d: got %f, expected %f\n", 
                   i, arr[i], expected);
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
    
    /* Test 1: Simple gang-partitioned loop (likely case 1) */
    printf("Test 1: Gang-partitioned vector addition\n");
    #pragma acc parallel loop gang copyin(a[0:N], b[0:N]) copyout(c[0:N]) num_gangs(8)
    for (i = 0; i < N; i++) {
        c[i] = a[i] + b[i];
    }
    success &= verify_array(c, N, 3.0f);
    
    /* Test 2: Fully partitioned with reduction (likely case 7) */
    printf("Test 2: Fully partitioned reduction\n");
    float sum = 0.0f;
    #pragma acc parallel loop gang worker vector reduction(+:sum) \
        copyin(a[0:N]) copy(sum) num_gangs(4) num_workers(2) vector_length(32)
    for (i = 0; i < N; i++) {
        sum += a[i];
    }
    printf("Reduction sum: %f\n", sum);
    
    /* Test 3: Worker-partitioned computation (likely case 2) */
    printf("Test 3: Worker-partitioned matrix initialization\n");
    #pragma acc parallel loop worker copyout(matrix[0:M*P]) num_workers(4)
    for (i = 0; i < M * P; i++) {
        matrix[i] = i * 0.01f;
    }
    
    /* Test 4: Gang+worker partitioned nested loops (likely case 3) */
    printf("Test 4: Gang+worker partitioned matrix operations\n");
    #pragma acc parallel copyin(matrix[0:M*P]) copyout(result[0:M*P]) \
        num_gangs(4) num_workers(2)
    {
        #pragma acc loop gang
        for (i = 0; i < M; i++) {
            #pragma acc loop worker
            for (j = 0; j < P; j++) {
                int idx = i * P + j;
                result[idx] = matrix[idx] * 2.0f;
            }
        }
    }
    
    /* Test 5: Vector-partitioned operation (likely case 4) */
    printf("Test 5: Vector-partitioned computation\n");
    #pragma acc parallel loop vector copyin(b[0:N]) copyout(c[0:N]) vector_length(64)
    for (i = 0; i < N; i++) {
        c[i] = b[i] * b[i];
    }
    
    /* Test 6: Gang+vector partitioned (likely case 5) */
    printf("Test 6: Gang+vector partitioned\n");
    #pragma acc parallel loop gang vector copy(c[0:N]) num_gangs(8) vector_length(32)
    for (i = 0; i < N; i++) {
        c[i] = sqrtf(c[i]);
    }
    
    /* Test 7: Worker+vector partitioned (likely case 6) */
    printf("Test 7: Worker+vector partitioned\n");
    float *temp = (float*)malloc(N * sizeof(float));
    #pragma acc parallel loop worker vector copyout(temp[0:N]) num_workers(4) vector_length(16)
    for (i = 0; i < N; i++) {
        temp[i] = sinf(c[i]);
    }
    
    /* Test 8: Gang redundant (likely case 0) with private/firstprivate */
    printf("Test 8: Gang redundant with private data\n");
    float gang_var = 10.0f;
    #pragma acc parallel copyin(a[0:N]) copyout(c[0:N]) \
        private(gang_var) num_gangs(1)
    {
        gang_var = 5.0f;  /* Each gang gets its own copy */
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            c[i] = a[i] + gang_var;
        }
    }
    success &= verify_array(c, N, 6.0f);
    
    /* Test 9: Combined construct with tile clause */
    printf("Test 9: Combined parallel loop with tile\n");
    #pragma acc parallel loop tile(32, 16) copy(matrix[0:M*P]) num_gangs(8)
    for (i = 0; i < M; i++) {
        for (j = 0; j < P; j++) {
            int idx = i * P + j;
            matrix[idx] += 1.0f;
        }
    }
    
    /* Test 10: Runtime-dependent partitioning */
    printf("Test 10: Runtime-dependent partitioning\n");
    int use_workers = 1;  /* Could be runtime determined */
    if (use_workers) {
        #pragma acc parallel loop gang worker copy(c[0:N]) num_gangs(2) num_workers(4)
        for (i = 0; i < N; i++) {
            c[i] += 1.0f;
        }
    } else {
        #pragma acc parallel loop gang copy(c[0:N]) num_gangs(4)
        for (i = 0; i < N; i++) {
            c[i] += 2.0f;
        }
    }
    
    /* Test 11: Async operations with wait */
    printf("Test 11: Async operations\n");
    int async_id = 1;
    #pragma acc parallel loop async(async_id) copy(a[0:N], b[0:N]) num_gangs(4)
    for (i = 0; i < N; i++) {
        a[i] = b[i] * 0.5f;
    }
    #pragma acc wait(async_id)
    
    /* Test 12: Multi-device test (if supported) */
    printf("Test 12: Device management test\n");
    acc_device_t dev_type = acc_get_device_type();
    printf("Current device type: %d\n", dev_type);
    
    /* Force host-fallback execution to ensure runtime paths are taken */
    acc_set_device_type(acc_device_host);
    
    /* Test 13: Complex data structures */
    printf("Test 13: Complex data structure operations\n");
    struct {
        float x[N];
        float y[N];
    } point;
    
    #pragma acc parallel loop copyout(point.x[0:N], point.y[0:N]) num_gangs(8)
    for (i = 0; i < N; i++) {
        point.x[i] = i * 0.1f;
        point.y[i] = i * 0.2f;
    }
    
    /* Test 14: Out-of-bounds/error condition simulation */
    printf("Test 14: Testing edge cases\n");
    /* This might trigger error paths in the runtime */
    float *null_ptr = NULL;
    #pragma acc parallel present_or_copy(null_ptr[0:0]) num_gangs(1)
    {
        /* Empty - testing error handling */
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(matrix);
    free(result);
    free(temp);
    
    if (success) {
        printf("\nAll tests completed successfully!\n");
        return 0;
    } else {
        printf("\nSome tests failed!\n");
        return 1;
    }
}
