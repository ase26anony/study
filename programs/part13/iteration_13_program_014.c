/* Test program to cover partition code mapping in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O1 -fopenacc -foffload=disable -o test_partition test_partition.c
 * Run with: ACC_DEBUG=1 ./test_partition
 */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define N 1024
#define M 256
#define P 128

void test_gang_redundant() {
    printf("Testing gang redundant (case 0)...\n");
    int scalar = 42;
    int array[N];
    
    #pragma acc parallel copyout(array[0:N]) copy(scalar) num_gangs(4)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            array[i] = scalar + i;  // scalar is gang-redundant
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (array[i] != 42 + i) errors++;
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_gang_partitioned() {
    printf("Testing gang partitioned (case 1)...\n");
    int array[N];
    
    #pragma acc parallel copyout(array[0:N]) num_gangs(8)
    {
        #pragma acc loop gang independent
        for (int i = 0; i < N; i++) {
            array[i] = i * 2;
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (array[i] != i * 2) errors++;
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_worker_partitioned() {
    printf("Testing worker partitioned (case 2)...\n");
    int array[N];
    
    #pragma acc parallel copyout(array[0:N]) num_gangs(2) num_workers(4)
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            array[i] = i + 1000;
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (array[i] != i + 1000) errors++;
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_gang_worker_partitioned() {
    printf("Testing gang+worker partitioned (case 3)...\n");
    int matrix[M][P];
    
    #pragma acc parallel copyout(matrix[0:M][0:P]) num_gangs(4) num_workers(2)
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker
            for (int j = 0; j < P; j++) {
                matrix[i][j] = i * P + j;
            }
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            if (matrix[i][j] != i * P + j) errors++;
        }
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_vector_partitioned() {
    printf("Testing vector partitioned (case 4)...\n");
    int array[N];
    
    #pragma acc parallel copyout(array[0:N]) vector_length(32)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            array[i] = i * 3;
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (array[i] != i * 3) errors++;
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_gang_vector_partitioned() {
    printf("Testing gang+vector partitioned (case 5)...\n");
    int array[N];
    
    #pragma acc parallel copyout(array[0:N]) num_gangs(4) vector_length(16)
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            array[i] = i * 4;
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (array[i] != i * 4) errors++;
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_worker_vector_partitioned() {
    printf("Testing worker+vector partitioned (case 6)...\n");
    int array[N];
    
    #pragma acc parallel copyout(array[0:N]) num_workers(2) vector_length(8)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < N; i++) {
            array[i] = i * 5;
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (array[i] != i * 5) errors++;
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_fully_partitioned() {
    printf("Testing fully partitioned (case 7)...\n");
    int array[N];
    int sum = 0;
    
    #pragma acc parallel copyout(array[0:N]) reduction(+:sum) \
        num_gangs(4) num_workers(2) vector_length(16)
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < N; i++) {
            array[i] = i;
            sum += i;
        }
    }
    
    // Verify
    int errors = 0;
    int expected_sum = 0;
    for (int i = 0; i < N; i++) {
        if (array[i] != i) errors++;
        expected_sum += i;
    }
    if (sum != expected_sum) {
        printf("  Reduction error: got %d, expected %d\n", sum, expected_sum);
        errors++;
    }
    if (errors > 0) printf("  Total errors: %d\n", errors);
}

void test_nested_parallelism() {
    printf("Testing nested parallelism for varied partitioning...\n");
    int matrix[M][P];
    
    #pragma acc parallel copyout(matrix[0:M][0:P]) num_gangs(2)
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc parallel loop worker vector num_workers(2) vector_length(8)
            for (int j = 0; j < P; j++) {
                matrix[i][j] = (i + 1) * (j + 1);
            }
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            if (matrix[i][j] != (i + 1) * (j + 1)) errors++;
        }
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_combined_directives() {
    printf("Testing combined directives...\n");
    int array[N];
    
    // Combined parallel loop with tile
    #pragma acc parallel loop copyout(array[0:N]) gang worker vector \
        tile(32, 16)
    for (int i = 0; i < N; i++) {
        array[i] = i * i;
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (array[i] != i * i) errors++;
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_runtime_determined_partitioning() {
    printf("Testing runtime-determined partitioning...\n");
    int array[N];
    int use_workers = 1;  // Runtime value
    
    if (use_workers) {
        #pragma acc parallel copyout(array[0:N]) num_workers(4)
        {
            #pragma acc loop worker
            for (int i = 0; i < N; i++) {
                array[i] = i + 500;
            }
        }
    } else {
        #pragma acc parallel copyout(array[0:N]) num_gangs(4)
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                array[i] = i + 1000;
            }
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (array[i] != i + 500) errors++;
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_multi_device() {
    printf("Testing multi-device scenarios...\n");
    
    // Try to set different device types
    acc_device_t dev_type = acc_get_device_type();
    printf("  Current device type: %d\n", dev_type);
    
    // Test with async
    int array[N];
    int async_id = 1;
    
    #pragma acc parallel copyout(array[0:N]) async(async_id) num_gangs(4)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            array[i] = i * 10;
        }
    }
    
    #pragma acc wait(async_id)
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (array[i] != i * 10) errors++;
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_complex_data_structures() {
    printf("Testing complex data structures...\n");
    
    typedef struct {
        int x[N];
        int y[M];
        int scalar;
    } Data;
    
    Data d;
    d.scalar = 99;
    
    #pragma acc parallel copyout(d) num_gangs(2) num_workers(2) vector_length(8)
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < N; i++) {
            d.x[i] = i + d.scalar;
        }
        
        #pragma acc loop gang worker
        for (int i = 0; i < M; i++) {
            d.y[i] = i * 2;
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (d.x[i] != i + 99) errors++;
    }
    for (int i = 0; i < M; i++) {
        if (d.y[i] != i * 2) errors++;
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

int main() {
    printf("Starting OpenACC partition coverage test...\n");
    
    // Enable debug output to trigger mapping function calls
    char* debug_env = getenv("ACC_DEBUG");
    if (!debug_env) {
        printf("Set ACC_DEBUG=1 for verbose output from runtime\n");
    }
    
    // Test all partition cases
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    
    // Additional tests for varied execution paths
    test_nested_parallelism();
    test_combined_directives();
    test_runtime_determined_partitioning();
    test_multi_device();
    test_complex_data_structures();
    
    // Test potential error conditions that might trigger default case
    printf("\nTesting potential error conditions...\n");
    
    // Try with invalid device (might trigger error paths)
    int original_device = acc_get_device_num(acc_get_device_type());
    printf("  Original device: %d\n", original_device);
    
    // Test with zero-length array (edge case)
    int empty_array[1];
    #pragma acc parallel copyout(empty_array[0:0]) num_gangs(1)
    {
        // Empty parallel region
    }
    
    printf("\nAll tests completed.\n");
    printf("If compiled with coverage and run with ACC_DEBUG=1,\n");
    printf("the runtime should have called the partition mapping function\n");
    printf("with codes 0-7 and potentially the default case.\n");
    
    return 0;
}
