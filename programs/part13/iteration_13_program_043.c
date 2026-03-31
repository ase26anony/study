#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>

#define N 1024
#define M 256
#define P 128

// Helper function to initialize arrays
void init_array(float *arr, int size, float value) {
    for (int i = 0; i < size; i++) {
        arr[i] = value + i;
    }
}

// Helper function to verify results
int verify_array(float *arr, int size, float expected_base) {
    for (int i = 0; i < size; i++) {
        if (arr[i] != expected_base + i) {
            printf("Verification failed at index %d: expected %f, got %f\n", 
                   i, expected_base + i, arr[i]);
            return 0;
        }
    }
    return 1;
}

int main() {
    // Enable debug output to trigger string mapping function calls
    // This environment variable may cause the runtime to log partition info
    setenv("ACC_DEBUG", "1", 1);
    setenv("LIBGOMP_DEBUG", "1", 1);
    
    // Test 1: Gang redundant partitioning (case 0)
    printf("Test 1: Gang redundant\n");
    {
        float scalar = 10.0f;
        float result = 0.0f;
        
        #pragma acc parallel copyin(scalar) copyout(result) num_gangs(4)
        {
            #pragma acc loop gang
            for (int i = 0; i < 4; i++) {
                // Each gang operates on the same scalar value
                result = scalar * 2.0f;
            }
        }
        
        if (result != 20.0f) {
            printf("Test 1 failed: expected 20.0, got %f\n", result);
            return 1;
        }
    }
    
    // Test 2: Gang partitioned (case 1)
    printf("Test 2: Gang partitioned\n");
    {
        float arr[N];
        init_array(arr, N, 1.0f);
        
        #pragma acc parallel copy(arr[0:N]) num_gangs(8)
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                arr[i] = arr[i] * 2.0f;
            }
        }
        
        if (!verify_array(arr, N, 2.0f)) {
            return 1;
        }
    }
    
    // Test 3: Worker partitioned (case 2)
    printf("Test 3: Worker partitioned\n");
    {
        float arr[M];
        init_array(arr, M, 2.0f);
        
        #pragma acc parallel copy(arr[0:M]) num_gangs(2) num_workers(4)
        {
            #pragma acc loop worker
            for (int i = 0; i < M; i++) {
                arr[i] = arr[i] + 1.0f;
            }
        }
        
        if (!verify_array(arr, M, 3.0f)) {
            return 1;
        }
    }
    
    // Test 4: Gang+worker partitioned (case 3)
    printf("Test 4: Gang+worker partitioned\n");
    {
        float arr[N][M];
        float sum = 0.0f;
        
        // Initialize 2D array
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr[i][j] = (float)(i * M + j);
            }
        }
        
        #pragma acc parallel copy(arr[0:N][0:M]) copy(sum) \
                num_gangs(4) num_workers(4)
        {
            #pragma acc loop gang worker collapse(2)
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    arr[i][j] = arr[i][j] * 2.0f;
                }
            }
            
            // Reduction across gangs and workers
            #pragma acc loop gang worker reduction(+:sum)
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    sum += arr[i][j];
                }
            }
        }
        
        printf("Test 4 sum: %f\n", sum);
    }
    
    // Test 5: Vector partitioned (case 4)
    printf("Test 5: Vector partitioned\n");
    {
        float arr[P];
        init_array(arr, P, 5.0f);
        
        #pragma acc parallel copy(arr[0:P]) vector_length(32)
        {
            #pragma acc loop vector
            for (int i = 0; i < P; i++) {
                arr[i] = arr[i] * 3.0f;
            }
        }
        
        if (!verify_array(arr, P, 15.0f)) {
            return 1;
        }
    }
    
    // Test 6: Gang+vector partitioned (case 5)
    printf("Test 6: Gang+vector partitioned\n");
    {
        float arr[N];
        init_array(arr, N, 3.0f);
        
        #pragma acc parallel copy(arr[0:N]) num_gangs(4) vector_length(64)
        {
            #pragma acc loop gang vector
            for (int i = 0; i < N; i++) {
                arr[i] = arr[i] + 2.0f;
            }
        }
        
        if (!verify_array(arr, N, 5.0f)) {
            return 1;
        }
    }
    
    // Test 7: Worker+vector partitioned (case 6)
    printf("Test 7: Worker+vector partitioned\n");
    {
        float arr[M];
        init_array(arr, M, 4.0f);
        
        #pragma acc parallel copy(arr[0:M]) num_workers(4) vector_length(32)
        {
            #pragma acc loop worker vector
            for (int i = 0; i < M; i++) {
                arr[i] = arr[i] * 2.0f;
            }
        }
        
        if (!verify_array(arr, M, 8.0f)) {
            return 1;
        }
    }
    
    // Test 8: Fully partitioned (case 7)
    printf("Test 8: Fully partitioned\n");
    {
        float arr[N][M];
        float total = 0.0f;
        
        // Initialize
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr[i][j] = 1.0f;
            }
        }
        
        #pragma acc parallel copy(arr[0:N][0:M]) copy(total) \
                num_gangs(8) num_workers(4) vector_length(32)
        {
            #pragma acc loop gang worker vector collapse(2) reduction(+:total)
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    arr[i][j] = arr[i][j] * 2.0f;
                    total += arr[i][j];
                }
            }
        }
        
        float expected_total = 2.0f * N * M;
        if (total != expected_total) {
            printf("Test 8 failed: expected %f, got %f\n", expected_total, total);
            return 1;
        }
    }
    
    // Test 9: Nested parallelism for complex partitioning
    printf("Test 9: Nested parallelism\n");
    {
        float arr[N];
        init_array(arr, N, 10.0f);
        
        #pragma acc parallel copy(arr[0:N]) num_gangs(4)
        {
            // Outer gang level
            #pragma acc loop gang
            for (int g = 0; g < 4; g++) {
                // Inner worker level within each gang
                #pragma acc loop worker
                for (int i = g * (N/4); i < (g+1) * (N/4); i++) {
                    arr[i] = arr[i] + g;
                }
            }
        }
        
        // Verify
        int success = 1;
        for (int g = 0; g < 4; g++) {
            for (int i = g * (N/4); i < (g+1) * (N/4); i++) {
                if (arr[i] != 10.0f + i + g) {
                    printf("Test 9 failed at index %d\n", i);
                    success = 0;
                    break;
                }
            }
        }
        if (!success) return 1;
    }
    
    // Test 10: Runtime-dependent partitioning
    printf("Test 10: Runtime-dependent partitioning\n");
    {
        int dynamic_size = 512;
        float *dyn_arr = (float*)malloc(dynamic_size * sizeof(float));
        init_array(dyn_arr, dynamic_size, 7.0f);
        
        // Use runtime-computed bounds
        #pragma acc parallel copy(dyn_arr[0:dynamic_size]) 
        {
            int chunk = dynamic_size / acc_num_gangs(0);
            #pragma acc loop gang
            for (int g = 0; g < acc_num_gangs(0); g++) {
                int start = g * chunk;
                int end = (g == acc_num_gangs(0)-1) ? dynamic_size : (g+1) * chunk;
                #pragma acc loop worker vector
                for (int i = start; i < end; i++) {
                    dyn_arr[i] = dyn_arr[i] * 2.0f;
                }
            }
        }
        
        if (!verify_array(dyn_arr, dynamic_size, 14.0f)) {
            free(dyn_arr);
            return 1;
        }
        free(dyn_arr);
    }
    
    // Test 11: Async operations with device management
    printf("Test 11: Async operations\n");
    {
        int device_num = acc_get_device_num(acc_get_device_type());
        printf("Using device %d\n", device_num);
        
        float arr1[N], arr2[N];
        init_array(arr1, N, 1.0f);
        init_array(arr2, N, 2.0f);
        
        int async_id = 1;
        
        #pragma acc parallel copy(arr1[0:N]) async(async_id)
        {
            #pragma acc loop gang worker
            for (int i = 0; i < N; i++) {
                arr1[i] = arr1[i] * 3.0f;
            }
        }
        
        #pragma acc parallel copy(arr2[0:N]) async(async_id+1)
        {
            #pragma acc loop gang vector
            for (int i = 0; i < N; i++) {
                arr2[i] = arr2[i] * 4.0f;
            }
        }
        
        // Wait for both async operations
        #pragma acc wait(async_id)
        #pragma acc wait(async_id+1)
        
        if (!verify_array(arr1, N, 3.0f) || !verify_array(arr2, N, 8.0f)) {
            return 1;
        }
    }
    
    // Test 12: Multi-device test (if supported)
    printf("Test 12: Multi-device test\n");
    {
        int num_devices = acc_get_num_devices(acc_get_device_type());
        printf("Number of devices: %d\n", num_devices);
        
        if (num_devices > 1) {
            // Try to switch devices
            acc_set_device_num(0, acc_get_device_type());
            
            float arr[N];
            init_array(arr, N, 20.0f);
            
            #pragma acc parallel copy(arr[0:N])
            {
                #pragma acc loop gang worker vector
                for (int i = 0; i < N; i++) {
                    arr[i] = arr[i] / 2.0f;
                }
            }
            
            if (!verify_array(arr, N, 10.0f)) {
                return 1;
            }
        }
    }
    
    // Test 13: Complex data structures
    printf("Test 13: Complex data structures\n");
    {
        typedef struct {
            float x[N];
            float y[N];
            int id;
        } PointCloud;
        
        PointCloud cloud;
        cloud.id = 100;
        
        init_array(cloud.x, N, 1.0f);
        init_array(cloud.y, N, 2.0f);
        
        #pragma acc parallel copy(cloud)
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < N; i++) {
                cloud.x[i] = cloud.x[i] * cloud.id;
                cloud.y[i] = cloud.y[i] + cloud.id;
            }
        }
        
        // Verify
        for (int i = 0; i < N; i++) {
            if (cloud.x[i] != 100.0f + i * 100.0f || 
                cloud.y[i] != 102.0f + i) {
                printf("Test 13 failed at index %d\n", i);
                return 1;
            }
        }
    }
    
    // Test 14: Reduction with different partitioning
    printf("Test 14: Complex reductions\n");
    {
        float matrix[N][M];
        float row_sums[N] = {0};
        float total_sum = 0.0f;
        
        // Initialize matrix
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                matrix[i][j] = (float)(i * M + j);
            }
        }
        
        // Row-wise reduction (gang partitioned)
        #pragma acc parallel copy(matrix) copyout(row_sums) num_gangs(N/32)
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                float row_sum = 0.0f;
                #pragma acc loop worker vector reduction(+:row_sum)
                for (int j = 0; j < M; j++) {
                    row_sum += matrix[i][j];
                }
                row_sums[i] = row_sum;
            }
        }
        
        // Total reduction (fully partitioned)
        #pragma acc parallel copy(row_sums) copy(total_sum) \
                num_gangs(8) num_workers(4) vector_length(32)
        {
            #pragma acc loop gang worker vector reduction(+:total_sum)
            for (int i = 0; i < N; i++) {
                total_sum += row_sums[i];
            }
        }
        
        printf("Test 14 total sum: %f\n", total_sum);
    }
    
    printf("All tests passed!\n");
    return 0;
}
