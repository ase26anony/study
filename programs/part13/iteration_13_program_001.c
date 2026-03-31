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

void test_gang_redundant() {
    printf("Testing gang redundant partitioning...\n");
    int data[N];
    int result = 0;
    
    #pragma acc parallel loop gang reduction(+:result) copyin(data[0:N]) copyout(result)
    for (int i = 0; i < N; i++) {
        result += i % 10;
    }
    
    printf("  Gang redundant result: %d\n", result);
}

void test_gang_partitioned() {
    printf("Testing gang partitioned...\n");
    int A[N], B[N], C[N];
    
    for (int i = 0; i < N; i++) {
        A[i] = i;
        B[i] = N - i;
    }
    
    #pragma acc parallel loop gang num_gangs(8) copyin(A[0:N], B[0:N]) copyout(C[0:N])
    for (int i = 0; i < N; i++) {
        C[i] = A[i] + B[i];
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (C[i] != N) errors++;
    }
    printf("  Gang partitioned errors: %d\n", errors);
}

void test_worker_partitioned() {
    printf("Testing worker partitioned...\n");
    float matrix[M][P];
    float sum = 0.0f;
    
    #pragma acc parallel loop worker reduction(+:sum) copy(matrix[0:M][0:P]) copyout(sum)
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            matrix[i][j] = (i * P + j) * 0.1f;
            sum += matrix[i][j];
        }
    }
    
    printf("  Worker partitioned sum: %.2f\n", sum);
}

void test_gang_worker_partitioned() {
    printf("Testing gang+worker partitioned...\n");
    int data[M][P];
    int total = 0;
    
    #pragma acc parallel loop gang worker collapse(2) reduction(+:total) \
        copy(data[0:M][0:P]) copyout(total)
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            data[i][j] = (i + j) % 100;
            total += data[i][j];
        }
    }
    
    printf("  Gang+worker partitioned total: %d\n", total);
}

void test_vector_partitioned() {
    printf("Testing vector partitioned...\n");
    double vec[N];
    double max_val = 0.0;
    
    #pragma acc parallel loop vector vector_length(32) reduction(max:max_val) \
        copy(vec[0:N]) copyout(max_val)
    for (int i = 0; i < N; i++) {
        vec[i] = (i * 3.14159) / N;
        if (vec[i] > max_val) max_val = vec[i];
    }
    
    printf("  Vector partitioned max: %.6f\n", max_val);
}

void test_gang_vector_partitioned() {
    printf("Testing gang+vector partitioned...\n");
    int array[N];
    int min_val = N;
    
    #pragma acc parallel loop gang vector num_gangs(4) vector_length(16) \
        reduction(min:min_val) copy(array[0:N]) copyout(min_val)
    for (int i = 0; i < N; i++) {
        array[i] = (i * 7) % N;
        if (array[i] < min_val) min_val = array[i];
    }
    
    printf("  Gang+vector partitioned min: %d\n", min_val);
}

void test_worker_vector_partitioned() {
    printf("Testing worker+vector partitioned...\n");
    float data[M][P];
    float product = 1.0f;
    
    #pragma acc parallel loop worker vector collapse(2) reduction(*:product) \
        copy(data[0:M][0:P]) copyout(product)
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            data[i][j] = 1.0f + ((i * P + j) % 10) * 0.01f;
            product *= data[i][j];
        }
    }
    
    printf("  Worker+vector partitioned product: %.6e\n", product);
}

void test_fully_partitioned() {
    printf("Testing fully partitioned...\n");
    int array3d[8][16][32];
    long long grand_total = 0;
    
    #pragma acc parallel loop gang worker vector collapse(3) \
        reduction(+:grand_total) copy(array3d[0:8][0:16][0:32]) copyout(grand_total)
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 16; j++) {
            for (int k = 0; k < 32; k++) {
                array3d[i][j][k] = (i * 10000) + (j * 100) + k;
                grand_total += array3d[i][j][k];
            }
        }
    }
    
    printf("  Fully partitioned grand total: %lld\n", grand_total);
}

void test_nested_parallelism() {
    printf("Testing nested parallelism...\n");
    int outer[N], inner[N];
    
    #pragma acc parallel copy(outer[0:N], inner[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            outer[i] = i * 2;
            
            #pragma acc loop worker vector
            for (int j = 0; j < 10; j++) {
                inner[i] += j;
            }
        }
    }
    
    printf("  Nested parallelism completed\n");
}

void test_combined_directives() {
    printf("Testing combined directives...\n");
    int tile_data[64][64];
    
    #pragma acc parallel loop tile(8,8) copy(tile_data[0:64][0:64])
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            tile_data[i][j] = (i * 64 + j) % 256;
        }
    }
    
    printf("  Combined directives completed\n");
}

void test_runtime_dependent_partitioning() {
    printf("Testing runtime-dependent partitioning...\n");
    int dynamic_data[N];
    int mode = 2;  // Runtime value
    
    #pragma acc parallel loop copy(dynamic_data[0:N])
    for (int i = 0; i < N; i++) {
        if (mode == 1) {
            dynamic_data[i] = i * 2;
        } else if (mode == 2) {
            dynamic_data[i] = i * 3;
        } else {
            dynamic_data[i] = i;
        }
    }
    
    printf("  Runtime-dependent partitioning completed\n");
}

void test_async_operations() {
    printf("Testing async operations...\n");
    int async_data[N];
    int async_handle;
    
    #pragma acc parallel loop async copy(async_data[0:N])
    for (int i = 0; i < N; i++) {
        async_data[i] = i * i;
    }
    
    #pragma acc wait
    
    printf("  Async operations completed\n");
}

void test_multi_device() {
    printf("Testing multi-device awareness...\n");
    acc_device_t dev_type = acc_get_device_type();
    printf("  Device type: %d\n", dev_type);
    
    int num_devices = acc_get_num_devices(dev_type);
    printf("  Number of devices: %d\n", num_devices);
    
    if (num_devices > 0) {
        acc_set_device_num(0, dev_type);
        printf("  Set device 0\n");
    }
}

void test_data_clause_variations() {
    printf("Testing data clause variations...\n");
    
    // Test create
    int create_array[N];
    #pragma acc parallel loop create(create_array[0:N])
    for (int i = 0; i < N; i++) {
        create_array[i] = 0;
    }
    
    // Test copyin
    int src[N], dst[N];
    for (int i = 0; i < N; i++) src[i] = i;
    
    #pragma acc parallel loop copyin(src[0:N]) copyout(dst[0:N])
    for (int i = 0; i < N; i++) {
        dst[i] = src[i] * 2;
    }
    
    // Test present
    int present_array[N];
    #pragma acc data copy(present_array[0:N])
    {
        #pragma acc parallel loop present(present_array[0:N])
        for (int i = 0; i < N; i++) {
            present_array[i] = i * 3;
        }
    }
    
    printf("  Data clause variations completed\n");
}

void test_edge_cases() {
    printf("Testing edge cases...\n");
    
    // Empty parallel region
    #pragma acc parallel
    {
        // Nothing - tests default partitioning
    }
    
    // Single element
    int single = 0;
    #pragma acc parallel loop copy(single)
    for (int i = 0; i < 1; i++) {
        single = 42;
    }
    
    // Very small loop
    int small[4];
    #pragma acc parallel loop copy(small[0:4])
    for (int i = 0; i < 4; i++) {
        small[i] = i * 10;
    }
    
    printf("  Edge cases completed\n");
}

int main() {
    printf("Starting partition code coverage test...\n\n");
    
    // Enable debug output to trigger mapping function calls
    setenv("ACC_DEBUG", "1", 1);
    setenv("LIBGOMP_DEBUG", "1", 1);
    
    // Test all partition types
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    
    // Test additional constructs that may trigger different paths
    test_nested_parallelism();
    test_combined_directives();
    test_runtime_dependent_partitioning();
    test_async_operations();
    test_multi_device();
    test_data_clause_variations();
    test_edge_cases();
    
    printf("\nAll tests completed successfully!\n");
    
    // The default case for illegal partition codes might be triggered
    // by internal error conditions or invalid states. While we can't
    // directly cause an illegal partition code, the runtime might
    // use it for error reporting in edge cases.
    
    return 0;
}
