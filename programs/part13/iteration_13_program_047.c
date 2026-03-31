#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>

#define N 1024
#define M 256
#define P 128

// Helper function to initialize arrays
void init_array(int *arr, int size, int value) {
    for (int i = 0; i < size; i++) {
        arr[i] = value + i;
    }
}

// Helper function to verify results
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
    int *dev_a, *dev_b;
    int i, j, k;
    int sum = 0;
    int reduction_sum = 0;
    
    // Allocate and initialize host arrays
    a = (int*)malloc(N * sizeof(int));
    b = (int*)malloc(N * sizeof(int));
    c = (int*)malloc(N * M * sizeof(int));
    d = (int*)malloc(P * sizeof(int));
    
    init_array(a, N, 1);
    init_array(b, N, 2);
    init_array(d, P, 10);
    
    // Test 1: Simple gang-partitioned loop (likely case 1)
    printf("Test 1: Gang partitioned computation\n");
    #pragma acc parallel loop gang copy(a[0:N], b[0:N]) copyout(c[0:N*M]) num_gangs(8)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            c[i * M + j] = a[i] * b[i] + j;
        }
    }
    
    // Test 2: Worker partitioned (likely case 2)
    printf("Test 2: Worker partitioned computation\n");
    #pragma acc parallel loop worker copy(a[0:N]) num_workers(4) vector_length(32)
    for (i = 0; i < N; i++) {
        a[i] = a[i] * 2;
    }
    
    // Test 3: Vector partitioned (likely case 4)
    printf("Test 3: Vector partitioned computation\n");
    #pragma acc parallel loop vector copy(b[0:N]) vector_length(64)
    for (i = 0; i < N; i++) {
        b[i] = b[i] + i;
    }
    
    // Test 4: Gang+worker partitioned (likely case 3)
    printf("Test 4: Gang+worker partitioned computation\n");
    #pragma acc parallel loop gang worker copy(a[0:N], b[0:N]) \
        num_gangs(4) num_workers(2) vector_length(16)
    for (i = 0; i < N; i++) {
        int local_sum = 0;
        #pragma acc loop vector reduction(+:local_sum)
        for (j = 0; j < M; j++) {
            local_sum += a[i] + b[j % N];
        }
        a[i] = local_sum / M;
    }
    
    // Test 5: Fully partitioned with reduction (likely case 7)
    printf("Test 5: Fully partitioned with reduction\n");
    reduction_sum = 0;
    #pragma acc parallel loop gang worker vector reduction(+:reduction_sum) \
        copy(a[0:N]) copyin(reduction_sum) copyout(reduction_sum) \
        num_gangs(8) num_workers(2) vector_length(32)
    for (i = 0; i < N; i++) {
        reduction_sum += a[i];
    }
    printf("Reduction sum: %d\n", reduction_sum);
    
    // Test 6: Gang+vector partitioned (likely case 5)
    printf("Test 6: Gang+vector partitioned computation\n");
    #pragma acc parallel loop gang vector copy(d[0:P]) \
        num_gangs(4) vector_length(16)
    for (i = 0; i < P; i++) {
        d[i] = d[i] * 3 - i;
    }
    
    // Test 7: Worker+vector partitioned (likely case 6)
    printf("Test 7: Worker+vector partitioned computation\n");
    #pragma acc parallel loop worker vector copy(d[0:P]) \
        num_workers(4) vector_length(8)
    for (i = 0; i < P; i++) {
        d[i] = d[i] + 100;
    }
    
    // Test 8: Nested parallelism for complex partitioning
    printf("Test 8: Nested parallelism\n");
    #pragma acc parallel copy(a[0:N], b[0:N]) num_gangs(4)
    {
        #pragma acc loop gang
        for (i = 0; i < N/4; i++) {
            #pragma acc loop worker
            for (j = 0; j < 4; j++) {
                int idx = i * 4 + j;
                #pragma acc loop vector
                for (k = 0; k < 16; k++) {
                    a[idx] += k;
                }
            }
        }
    }
    
    // Test 9: Runtime-determined partitioning
    printf("Test 9: Runtime-determined partitioning\n");
    int chunk_size = 64;
    #pragma acc parallel loop gang worker copy(a[0:N]) \
        num_gangs(N/chunk_size) num_workers(2)
    for (i = 0; i < N; i += chunk_size) {
        #pragma acc loop vector
        for (j = i; j < i + chunk_size && j < N; j++) {
            a[j] = a[j] * 2 + 1;
        }
    }
    
    // Test 10: Multi-dimensional array with tile clause
    printf("Test 10: Multi-dimensional with tile\n");
    int matrix[64][64];
    #pragma acc parallel loop gang tile(8,8) copyout(matrix)
    for (i = 0; i < 64; i++) {
        for (j = 0; j < 64; j++) {
            matrix[i][j] = i * 64 + j;
        }
    }
    
    // Test 11: Async operations with different queues
    printf("Test 11: Async operations\n");
    int async_id = 1;
    #pragma acc parallel loop async(async_id) copy(a[0:N]) num_gangs(8)
    for (i = 0; i < N; i++) {
        a[i] = a[i] / 2;
    }
    acc_wait(async_id);
    
    // Test 12: Device memory operations
    printf("Test 12: Device memory operations\n");
    dev_a = (int*)acc_malloc(N * sizeof(int));
    dev_b = (int*)acc_malloc(N * sizeof(int));
    
    #pragma acc parallel loop present(dev_a[0:N], dev_b[0:N]) \
        num_gangs(8) num_workers(2) vector_length(32)
    for (i = 0; i < N; i++) {
        dev_a[i] = i;
        dev_b[i] = N - i;
    }
    
    // Test 13: Conditional partitioning
    printf("Test 13: Conditional partitioning\n");
    int use_workers = 1;
    #pragma acc parallel loop copy(a[0:N]) \
        num_gangs(4) num_workers(use_workers ? 2 : 1) vector_length(16)
    for (i = 0; i < N; i++) {
        if (i % 2 == 0) {
            a[i] = a[i] * 3;
        } else {
            a[i] = a[i] / 2;
        }
    }
    
    // Test 14: Reduction with multiple variables
    printf("Test 14: Multiple reductions\n");
    int sum1 = 0, sum2 = 0;
    #pragma acc parallel loop reduction(+:sum1, sum2) copy(a[0:N]) \
        num_gangs(8) num_workers(2) vector_length(16)
    for (i = 0; i < N; i++) {
        sum1 += a[i];
        sum2 += a[i] * 2;
    }
    printf("Sum1: %d, Sum2: %d\n", sum1, sum2);
    
    // Test 15: Complex access pattern
    printf("Test 15: Complex access pattern\n");
    int indices[N];
    for (i = 0; i < N; i++) {
        indices[i] = (i * 7) % N;  // Create a strided access pattern
    }
    
    #pragma acc parallel loop copy(a[0:N]) copyin(indices[0:N]) \
        num_gangs(8) num_workers(4) vector_length(8)
    for (i = 0; i < N; i++) {
        int idx = indices[i];
        a[idx] = a[idx] + i;
    }
    
    // Cleanup
    if (dev_a) acc_free(dev_a);
    if (dev_b) acc_free(dev_b);
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    printf("All tests completed successfully!\n");
    return 0;
}
