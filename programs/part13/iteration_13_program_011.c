/* Test program to cover partition code mapping in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O0 -fopenacc -foffload=disable -o test_partition test_partition.c
 * Run with: ACC_DEBUG=1 ./test_partition
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>

#define N 1024
#define M 512
#define P 256

void initialize_array(int *arr, int size, int base) {
    for (int i = 0; i < size; i++) {
        arr[i] = base + i;
    }
}

int verify_array(int *arr, int size, int expected_base) {
    for (int i = 0; i < size; i++) {
        if (arr[i] != expected_base + i) {
            printf("Verification failed at index %d: got %d, expected %d\n",
                   i, arr[i], expected_base + i);
            return 0;
        }
    }
    return 1;
}

int main() {
    int *a, *b, *c, *d;
    int *host_a, *host_b, *host_c, *host_d;
    int scalar = 42;
    int reduction_sum = 0;
    int success = 1;
    
    /* Allocate and initialize host arrays */
    host_a = (int*)malloc(N * sizeof(int));
    host_b = (int*)malloc(N * sizeof(int));
    host_c = (int*)malloc(N * M * sizeof(int));
    host_d = (int*)malloc(N * M * P * sizeof(int));
    
    initialize_array(host_a, N, 0);
    initialize_array(host_b, N, 1000);
    
    /* Test 1: Simple gang-partitioned parallel region */
    printf("Test 1: Gang partitioned (likely case 1)\n");
    #pragma acc parallel loop gang copy(host_a[0:N]) copyin(host_b[0:N]) \
        num_gangs(4) vector_length(32)
    for (int i = 0; i < N; i++) {
        host_a[i] = host_a[i] + host_b[i];
    }
    
    if (!verify_array(host_a, N, 1000)) success = 0;
    
    /* Test 2: Worker partitioned with reduction */
    printf("Test 2: Worker partitioned with reduction (likely case 2)\n");
    reduction_sum = 0;
    #pragma acc parallel loop worker reduction(+:reduction_sum) \
        copy(reduction_sum) num_workers(8)
    for (int i = 0; i < N; i++) {
        reduction_sum += host_a[i] % 10;
    }
    printf("Reduction sum: %d\n", reduction_sum);
    
    /* Test 3: Vector partitioned with private variables */
    printf("Test 3: Vector partitioned (likely case 4)\n");
    #pragma acc parallel loop vector private(scalar) \
        copy(host_a[0:N]) vector_length(64)
    for (int i = 0; i < N; i++) {
        int local_scalar = scalar;
        host_a[i] = host_a[i] * local_scalar;
    }
    
    /* Test 4: Gang+worker partitioned with nested parallelism */
    printf("Test 4: Gang+worker partitioned (likely case 3)\n");
    #pragma acc parallel copy(host_c[0:N*M]) num_gangs(2) num_workers(4)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                host_c[i * M + j] = i * 1000 + j;
            }
        }
    }
    
    /* Test 5: Fully partitioned with all levels */
    printf("Test 5: Fully partitioned (likely case 7)\n");
    #pragma acc parallel loop gang worker vector \
        copy(host_a[0:N]) copyout(host_b[0:N]) \
        num_gangs(2) num_workers(2) vector_length(16)
    for (int i = 0; i < N; i++) {
        host_b[i] = host_a[i] / 2;
    }
    
    /* Test 6: Gang+vector partitioned with async operations */
    printf("Test 6: Gang+vector partitioned (likely case 5)\n");
    int async_id = 0;
    #pragma acc parallel loop gang vector async(async_id) \
        copy(host_a[0:N]) num_gangs(4) vector_length(32)
    for (int i = 0; i < N; i++) {
        host_a[i] = host_a[i] + 1;
    }
    acc_wait(async_id);
    
    /* Test 7: Worker+vector partitioned with tile clause */
    printf("Test 7: Worker+vector partitioned (likely case 6)\n");
    #pragma acc parallel loop tile(32, 16) worker vector \
        copy(host_c[0:N*M]) num_workers(4) vector_length(8)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            host_c[i * M + j] += i + j;
        }
    }
    
    /* Test 8: Gang redundant (likely case 0) */
    printf("Test 8: Gang redundant\n");
    int gang_redundant_var = 0;
    #pragma acc parallel copy(gang_redundant_var) num_gangs(1)
    {
        gang_redundant_var = 999;
    }
    printf("Gang redundant var: %d\n", gang_redundant_var);
    
    /* Test 9: Multi-dimensional array with complex access pattern */
    printf("Test 9: Complex 3D array access\n");
    #pragma acc parallel loop collapse(3) gang worker vector \
        copy(host_d[0:N*M*P]) \
        num_gangs(2) num_workers(2) vector_length(8)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                int idx = (i * M * P) + (j * P) + k;
                host_d[idx] = i * 1000000 + j * 1000 + k;
            }
        }
    }
    
    /* Test 10: Runtime-dependent partitioning */
    printf("Test 10: Runtime-dependent partitioning\n");
    int dynamic_chunks = 8;
    #pragma acc parallel loop gang worker \
        copy(host_a[0:N]) copyin(dynamic_chunks)
    for (int i = 0; i < N; i++) {
        if (dynamic_chunks > 4) {
            host_a[i] = host_a[i] * 2;
        } else {
            host_a[i] = host_a[i] / 2;
        }
    }
    
    /* Test 11: Firstprivate and private clauses */
    printf("Test 11: Firstprivate and private\n");
    int firstprivate_var = 777;
    #pragma acc parallel loop gang worker firstprivate(firstprivate_var) \
        copy(host_a[0:N])
    for (int i = 0; i < N; i++) {
        host_a[i] += firstprivate_var;
    }
    
    /* Test 12: Device management and multi-device context */
    printf("Test 12: Device management\n");
    acc_device_t dev_type = acc_get_device_type();
    printf("Current device type: %d\n", (int)dev_type);
    
    int num_devices = acc_get_num_devices(dev_type);
    printf("Number of devices: %d\n", num_devices);
    
    if (num_devices > 0) {
        acc_set_device_num(0, dev_type);
        printf("Set device to 0\n");
    }
    
    /* Test 13: Manual data management with deviceptr */
    printf("Test 13: Manual data management\n");
    a = (int*)acc_malloc(N * sizeof(int));
    b = (int*)acc_malloc(N * sizeof(int));
    
    if (a && b) {
        #pragma acc parallel loop present(a[0:N], b[0:N]) \
            gang worker vector
        for (int i = 0; i < N; i++) {
            a[i] = i;
            b[i] = a[i] * 2;
        }
        
        acc_free(a);
        acc_free(b);
    }
    
    /* Test 14: Combined directives with multiple clauses */
    printf("Test 14: Combined directives\n");
    #pragma acc parallel loop gang worker vector collapse(2) \
        copy(host_c[0:N*M]) copyin(scalar) \
        num_gangs(4) num_workers(2) vector_length(16) \
        private(firstprivate_var)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            host_c[i * M + j] = (i + j) * scalar;
        }
    }
    
    /* Test 15: Try to trigger potential error/illegal case */
    printf("Test 15: Attempt to trigger edge cases\n");
    /* This might trigger internal error paths */
    int *null_ptr = NULL;
    #pragma acc parallel loop gang
    for (int i = 0; i < 10; i++) {
        /* Access through pointer - might trigger different paths */
        if (null_ptr) {
            null_ptr[i] = i;  /* Won't execute */
        }
    }
    
    /* Cleanup */
    free(host_a);
    free(host_b);
    free(host_c);
    free(host_d);
    
    if (success) {
        printf("\nAll tests completed successfully!\n");
        return 0;
    } else {
        printf("\nSome tests failed!\n");
        return 1;
    }
}
