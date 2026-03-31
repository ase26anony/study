/* Test program to cover partition code mapping in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O1 -fopenacc -foffload=disable -o test_partition test_partition.c
 * Run with: ACC_DEBUG=1 ./test_partition
 */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define N 1024
#define M 128
#define P 64

void test_gang_redundant() {
    printf("Testing gang redundant pattern...\n");
    int data[N];
    
    #pragma acc parallel loop gang copyout(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = i * 2;
    }
    
    // Verify
    for (int i = 0; i < N; i++) {
        if (data[i] != i * 2) {
            printf("Error in gang redundant test at index %d\n", i);
            exit(1);
        }
    }
}

void test_gang_partitioned() {
    printf("Testing gang partitioned pattern...\n");
    int data[N];
    
    #pragma acc parallel loop gang copy(data[0:N]) num_gangs(4)
    for (int i = 0; i < N; i++) {
        data[i] += i;
    }
    
    // Simple verification
    int sum = 0;
    for (int i = 0; i < N; i++) sum += data[i];
    printf("  Sum: %d\n", sum);
}

void test_worker_partitioned() {
    printf("Testing worker partitioned pattern...\n");
    float data[M][P];
    
    #pragma acc parallel loop gang worker copy(data[0:M][0:P]) num_workers(2)
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            data[i][j] = i * 100.0f + j;
        }
    }
    
    // Verify a sample
    if (data[10][20] != 10 * 100.0f + 20) {
        printf("Error in worker partitioned test\n");
        exit(1);
    }
}

void test_gang_worker_partitioned() {
    printf("Testing gang+worker partitioned pattern...\n");
    int data[N];
    int sum = 0;
    
    #pragma acc parallel loop gang worker reduction(+:sum) copyin(data[0:N]) copyout(sum)
    for (int i = 0; i < N; i++) {
        sum += data[i];
    }
    
    printf("  Reduction sum: %d\n", sum);
}

void test_vector_partitioned() {
    printf("Testing vector partitioned pattern...\n");
    float vec[N];
    
    #pragma acc parallel loop vector copy(vec[0:N]) vector_length(32)
    for (int i = 0; i < N; i++) {
        vec[i] = i * 3.14f;
    }
    
    // Verify
    if (vec[100] != 100 * 3.14f) {
        printf("Error in vector partitioned test\n");
        exit(1);
    }
}

void test_gang_vector_partitioned() {
    printf("Testing gang+vector partitioned pattern...\n");
    double matrix[M][P];
    
    #pragma acc parallel loop gang vector collapse(2) copy(matrix[0:M][0:P]) \
        num_gangs(2) vector_length(16)
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            matrix[i][j] = (i + j) * 2.5;
        }
    }
    
    // Verify
    if (matrix[50][25] != (50 + 25) * 2.5) {
        printf("Error in gang+vector partitioned test\n");
        exit(1);
    }
}

void test_worker_vector_partitioned() {
    printf("Testing worker+vector partitioned pattern...\n");
    int data[M][P];
    
    #pragma acc parallel loop worker vector collapse(2) copy(data[0:M][0:P]) \
        num_workers(2) vector_length(8)
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            data[i][j] = i * P + j;
        }
    }
    
    // Verify
    if (data[30][40] != 30 * P + 40) {
        printf("Error in worker+vector partitioned test\n");
        exit(1);
    }
}

void test_fully_partitioned() {
    printf("Testing fully partitioned pattern...\n");
    int data[N];
    int max_val = 0;
    
    #pragma acc parallel loop gang worker vector reduction(max:max_val) \
        copy(data[0:N]) copyout(max_val) \
        num_gangs(2) num_workers(2) vector_length(16)
    for (int i = 0; i < N; i++) {
        if (data[i] > max_val) {
            max_val = data[i];
        }
    }
    
    printf("  Max value: %d\n", max_val);
}

void test_nested_parallelism() {
    printf("Testing nested parallelism...\n");
    int outer_data[M];
    int inner_data[M][P];
    
    #pragma acc parallel copy(outer_data[0:M]) copy(inner_data[0:M][0:P])
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            outer_data[i] = i * 10;
            
            #pragma acc loop worker vector
            for (int j = 0; j < P; j++) {
                inner_data[i][j] = i * 100 + j;
            }
        }
    }
    
    // Verify
    if (inner_data[25][35] != 25 * 100 + 35) {
        printf("Error in nested parallelism test\n");
        exit(1);
    }
}

void test_combined_directives() {
    printf("Testing combined directives...\n");
    int tile_data[M][P];
    
    #pragma acc parallel loop tile(32,16) copy(tile_data[0:M][0:P])
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            tile_data[i][j] = (i << 8) | j;
        }
    }
    
    // Verify
    if (tile_data[40][15] != ((40 << 8) | 15)) {
        printf("Error in combined directives test\n");
        exit(1);
    }
}

void test_runtime_partitioning() {
    printf("Testing runtime-dependent partitioning...\n");
    int data[N];
    int chunk_size = N / 4;
    
    // Initialize data
    for (int i = 0; i < N; i++) {
        data[i] = i;
    }
    
    #pragma acc parallel loop gang copy(data[0:N])
    for (int i = 0; i < N; i += chunk_size) {
        int end = i + chunk_size;
        if (end > N) end = N;
        
        #pragma acc loop worker vector
        for (int j = i; j < end; j++) {
            data[j] *= 2;
        }
    }
    
    // Verify
    if (data[250] != 500) {
        printf("Error in runtime partitioning test\n");
        exit(1);
    }
}

void test_multi_device() {
    printf("Testing multi-device scenarios...\n");
    
    // Get device type
    acc_device_t dev_type = acc_get_device_type();
    printf("  Device type: %d\n", dev_type);
    
    // Try to set device (may trigger different paths)
    int num_devices = acc_get_num_devices(dev_type);
    if (num_devices > 0) {
        acc_set_device_num(0, dev_type);
    }
    
    // Simple parallel region
    int data[100];
    #pragma acc parallel loop copy(data[0:100])
    for (int i = 0; i < 100; i++) {
        data[i] = i * 3;
    }
    
    // Verify
    if (data[50] != 150) {
        printf("Error in multi-device test\n");
        exit(1);
    }
}

void test_async_operations() {
    printf("Testing async operations...\n");
    int data[N];
    int async_id = 1;
    
    #pragma acc parallel loop async(async_id) copy(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = i * 5;
    }
    
    #pragma acc wait(async_id)
    
    // Verify
    if (data[200] != 1000) {
        printf("Error in async operations test\n");
        exit(1);
    }
}

void test_complex_data_structures() {
    printf("Testing complex data structures...\n");
    
    typedef struct {
        int id;
        float values[16];
        int *dynamic;
    } ComplexStruct;
    
    ComplexStruct structs[M];
    int dynamic_data[M * 16];
    
    // Initialize
    for (int i = 0; i < M; i++) {
        structs[i].id = i;
        structs[i].dynamic = &dynamic_data[i * 16];
        for (int j = 0; j < 16; j++) {
            structs[i].values[j] = i * 1.5f + j;
            structs[i].dynamic[j] = i * 100 + j;
        }
    }
    
    #pragma acc parallel loop gang copy(structs[0:M]) copy(dynamic_data[0:M*16])
    for (int i = 0; i < M; i++) {
        structs[i].id *= 2;
        for (int j = 0; j < 16; j++) {
            structs[i].values[j] += 1.0f;
            structs[i].dynamic[j] += 10;
        }
    }
    
    // Verify
    if (structs[10].id != 20 || structs[10].values[5] != 10 * 1.5f + 5 + 1.0f) {
        printf("Error in complex data structures test\n");
        exit(1);
    }
}

int main() {
    printf("Starting partition code coverage test...\n");
    
    // Enable debug output to trigger logging paths
    char *debug_env = getenv("ACC_DEBUG");
    if (!debug_env) {
        printf("Set ACC_DEBUG=1 for maximum coverage\n");
    }
    
    // Initialize test data
    printf("Initializing test data...\n");
    int init_data[N];
    for (int i = 0; i < N; i++) {
        init_data[i] = i;
    }
    #pragma acc enter data copyin(init_data[0:N])
    
    // Run tests for different partition patterns
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    
    // Additional tests to explore more code paths
    test_nested_parallelism();
    test_combined_directives();
    test_runtime_partitioning();
    test_multi_device();
    test_async_operations();
    test_complex_data_structures();
    
    // Cleanup
    #pragma acc exit data delete(init_data[0:N])
    
    printf("\nAll tests completed successfully!\n");
    printf("The partition code mapping function should have been called with:\n");
    printf("  - Codes 0-7 for various partitioning levels\n");
    printf("  - Potentially out-of-range codes for error conditions\n");
    
    return 0;
}
