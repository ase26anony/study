#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>

#define N 1024
#define M 256
#define P 128

// Enable debug output to trigger partition string mapping
void enable_debug() {
    // Set environment variables that might trigger debug output
    // These are GCC-specific OpenACC debug variables
    setenv("ACC_DEBUG", "1", 1);
    setenv("GACC_DEBUG", "1", 1);
    setenv("LIBGOMP_DEBUG", "1", 1);
}

// Test 1: Gang redundant partitioning
void test_gang_redundant() {
    int i;
    int scalar = 42;
    int arr[N];
    
    // Initialize array
    for (i = 0; i < N; i++) {
        arr[i] = i;
    }
    
    // Simple parallel region with scalar - likely gang redundant
    #pragma acc parallel copyin(scalar) copy(arr) num_gangs(4)
    {
        int gang_id = __pgi_gangidx();
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            arr[i] += scalar + gang_id;
        }
    }
    
    // Verify
    int sum = 0;
    for (i = 0; i < N; i++) {
        sum += arr[i];
    }
    printf("Test 1 (gang redundant) sum: %d\n", sum);
}

// Test 2: Gang partitioned
void test_gang_partitioned() {
    int i, j;
    int matrix[N][M];
    
    // Initialize matrix
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            matrix[i][j] = i * M + j;
        }
    }
    
    // Gang partitioned loop
    #pragma acc parallel copy(matrix) num_gangs(8)
    {
        #pragma acc loop gang independent
        for (i = 0; i < N; i++) {
            #pragma acc loop vector
            for (j = 0; j < M; j++) {
                matrix[i][j] *= 2;
            }
        }
    }
    
    // Verify
    int sum = 0;
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            sum += matrix[i][j];
        }
    }
    printf("Test 2 (gang partitioned) sum: %d\n", sum);
}

// Test 3: Worker partitioned
void test_worker_partitioned() {
    int i, j;
    int matrix[N][M];
    
    // Initialize
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            matrix[i][j] = 1;
        }
    }
    
    // Worker partitioned with nested parallelism
    #pragma acc parallel copy(matrix) num_gangs(2) num_workers(4)
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            #pragma acc loop worker
            for (j = 0; j < M; j++) {
                matrix[i][j] += i + j;
            }
        }
    }
    
    // Verify
    int sum = 0;
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            sum += matrix[i][j];
        }
    }
    printf("Test 3 (worker partitioned) sum: %d\n", sum);
}

// Test 4: Gang+worker partitioned
void test_gang_worker_partitioned() {
    int i, j, k;
    int cube[N][M][P];
    
    // Initialize 3D array
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            for (k = 0; k < P; k++) {
                cube[i][j][k] = 1;
            }
        }
    }
    
    // Complex nested parallelism
    #pragma acc parallel copy(cube) num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            #pragma acc loop worker
            for (j = 0; j < M; j++) {
                #pragma acc loop vector
                for (k = 0; k < P; k++) {
                    cube[i][j][k] = i + j + k;
                }
            }
        }
    }
    
    // Verify
    int sum = 0;
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            for (k = 0; k < P; k++) {
                sum += cube[i][j][k];
            }
        }
    }
    printf("Test 4 (gang+worker partitioned) sum: %d\n", sum);
}

// Test 5: Vector partitioned
void test_vector_partitioned() {
    int i;
    float arr[N];
    
    // Initialize
    for (i = 0; i < N; i++) {
        arr[i] = i * 1.5f;
    }
    
    // Vector partitioned loop
    #pragma acc parallel copy(arr) vector_length(64)
    {
        #pragma acc loop vector
        for (i = 0; i < N; i++) {
            arr[i] = arr[i] * 2.0f;
        }
    }
    
    // Verify
    float sum = 0;
    for (i = 0; i < N; i++) {
        sum += arr[i];
    }
    printf("Test 5 (vector partitioned) sum: %.2f\n", sum);
}

// Test 6: Gang+vector partitioned
void test_gang_vector_partitioned() {
    int i, j;
    int matrix[N][M];
    
    // Initialize
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            matrix[i][j] = i * j;
        }
    }
    
    // Gang+vector partitioning
    #pragma acc parallel copy(matrix) num_gangs(8) vector_length(32)
    {
        #pragma acc loop gang vector
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                matrix[i][j] += 1;
            }
        }
    }
    
    // Verify
    int sum = 0;
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            sum += matrix[i][j];
        }
    }
    printf("Test 6 (gang+vector partitioned) sum: %d\n", sum);
}

// Test 7: Worker+vector partitioned
void test_worker_vector_partitioned() {
    int i, j;
    int matrix[N][M];
    
    // Initialize
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            matrix[i][j] = 0;
        }
    }
    
    // Worker+vector partitioning
    #pragma acc parallel copy(matrix) num_workers(4) vector_length(16)
    {
        #pragma acc loop worker vector
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                matrix[i][j] = i * 100 + j;
            }
        }
    }
    
    // Verify
    int sum = 0;
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            sum += matrix[i][j];
        }
    }
    printf("Test 7 (worker+vector partitioned) sum: %d\n", sum);
}

// Test 8: Fully partitioned
void test_fully_partitioned() {
    int i, j, k;
    int result = 0;
    
    // Fully partitioned with reduction
    #pragma acc parallel copyin(i, j, k) reduction(+:result) \
        num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang worker vector reduction(+:result)
        for (i = 0; i < 100; i++) {
            #pragma acc loop gang worker vector reduction(+:result)
            for (j = 0; j < 100; j++) {
                #pragma acc loop gang worker vector reduction(+:result)
                for (k = 0; k < 100; k++) {
                    result += 1;
                }
            }
        }
    }
    
    printf("Test 8 (fully partitioned) result: %d\n", result);
}

// Test 9: Try to trigger illegal/default case
void test_illegal_case() {
    // Try various edge cases that might trigger unexpected partition codes
    
    // 1. Dynamic parallelism with async
    int arr[N];
    for (int i = 0; i < N; i++) {
        arr[i] = i;
    }
    
    int async_id = 1;
    #pragma acc parallel copy(arr) async(async_id)
    {
        #pragma acc loop
        for (int i = 0; i < N; i++) {
            arr[i] *= 2;
        }
    }
    #pragma acc wait(async_id);
    
    // 2. Conditional parallelism
    int condition = 1;
    #pragma acc parallel copy(arr) if(condition)
    {
        #pragma acc loop
        for (int i = 0; i < N; i++) {
            arr[i] += 1;
        }
    }
    
    // 3. Device management - might trigger different paths
    acc_device_t dev_type = acc_get_device_type();
    printf("Device type: %d\n", (int)dev_type);
    
    // 4. Try with invalid/edge-case parameters
    #pragma acc parallel copy(arr) num_gangs(0) num_workers(0) vector_length(0)
    {
        // Empty parallel region with unusual parameters
    }
    
    // Verify
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += arr[i];
    }
    printf("Test 9 (edge cases) sum: %d\n", sum);
}

// Test with combined constructs
void test_combined_constructs() {
    int i, j;
    int matrix[N][M];
    
    // Initialize
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            matrix[i][j] = 1;
        }
    }
    
    // Combined parallel loop construct
    #pragma acc parallel loop gang worker vector collapse(2) copy(matrix) \
        num_gangs(4) num_workers(2) vector_length(16)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            matrix[i][j] = matrix[i][j] * (i + j + 1);
        }
    }
    
    // Nested parallelism
    #pragma acc parallel copy(matrix) num_gangs(2)
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            #pragma acc parallel loop worker vector
            for (j = 0; j < M; j++) {
                matrix[i][j] += 100;
            }
        }
    }
    
    // Verify
    int sum = 0;
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            sum += matrix[i][j];
        }
    }
    printf("Combined constructs sum: %d\n", sum);
}

// Test with data regions and update
void test_data_regions() {
    int host_data[N];
    int device_data[N];
    
    // Initialize
    for (int i = 0; i < N; i++) {
        host_data[i] = i;
        device_data[i] = 0;
    }
    
    // Data region with explicit data movement
    #pragma acc data copyin(host_data) copyout(device_data)
    {
        #pragma acc parallel present(host_data, device_data)
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < N; i++) {
                device_data[i] = host_data[i] * 2;
            }
        }
        
        // Update directive
        #pragma acc update host(device_data)
    }
    
    // Verify
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += device_data[i];
    }
    printf("Data regions sum: %d\n", sum);
}

int main() {
    printf("Starting OpenACC partition coverage test...\n");
    
    // Enable debug to increase chance of hitting string mapping function
    enable_debug();
    
    // Initialize device
    acc_init(acc_device_default);
    
    // Run all tests to cover different partition cases
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    
    // Test combined constructs and data regions
    test_combined_constructs();
    test_data_regions();
    
    // Try to trigger edge cases
    test_illegal_case();
    
    printf("All tests completed successfully!\n");
    
    // Shutdown device
    acc_shutdown(acc_device_default);
    
    return 0;
}
