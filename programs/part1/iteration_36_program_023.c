#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>
#include <omp.h>

// Prevent optimization
volatile int force_runtime = 1;

// Function to use partition-dependent results
void use_result(int val) {
    asm volatile("" : : "r"(val));
}

// Test case 0: gang redundant (single gang)
void test_gang_redundant() {
    const int N = 1000;
    int *data = (int*)malloc(N * sizeof(int));
    
    // Initialize with runtime values
    for (int i = 0; i < N; i++) {
        data[i] = i % 7 + (force_runtime ? 1 : 0);
    }
    
    // Single gang, redundant across gangs
    #pragma acc parallel num_gangs(1) copy(data[0:N])
    {
        int idx = acc_gang_id * 100 + acc_worker_id * 10 + acc_vector_id;
        if (idx < N) {
            data[idx] += 1;
        }
    }
    
    // Use result to prevent elimination
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += data[i];
    }
    use_result(sum);
    
    free(data);
}

// Test case 1: gang partitioned
void test_gang_partitioned() {
    const int N = 1024;
    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));
    
    // Runtime initialization
    for (int i = 0; i < N; i++) {
        a[i] = (float)i / N;
        b[i] = (float)(N - i) / N;
    }
    
    // Multiple gangs, partitioned by gang
    #pragma acc parallel num_gangs(8) copy(a[0:N], b[0:N])
    {
        int gang_start = acc_gang_id * (N / 8);
        int gang_end = (acc_gang_id + 1) * (N / 8);
        
        for (int i = gang_start; i < gang_end; i++) {
            a[i] = a[i] + b[i];
        }
    }
    
    // Verification that could use partition strings
    float max_val = 0.0f;
    for (int i = 0; i < N; i++) {
        if (a[i] > max_val) max_val = a[i];
    }
    if (max_val > 2.0f) {
        // This might trigger diagnostic output
        fprintf(stderr, "Gang partitioned computation check\n");
    }
    
    free(a);
    free(b);
}

// Test case 2: worker partitioned
void test_worker_partitioned() {
    const int N = 512;
    double *arr = (double*)malloc(N * sizeof(double));
    
    for (int i = 0; i < N; i++) {
        arr[i] = (double)(i * force_runtime);
    }
    
    // Single gang, multiple workers
    #pragma acc parallel num_gangs(1) num_workers(4) copy(arr[0:N])
    {
        int worker_id = acc_worker_id;
        int workers = acc_num_workers(acc_async_noval);
        
        for (int i = worker_id; i < N; i += workers) {
            arr[i] = arr[i] * 2.0;
        }
    }
    
    // Force use of result
    double check = 0.0;
    for (int i = 0; i < N; i++) {
        check += arr[i];
    }
    asm volatile("" : : "r"(check));
    
    free(arr);
}

// Test case 3: gang+worker partitioned
void test_gang_worker_partitioned() {
    const int ROWS = 64;
    const int COLS = 64;
    int *matrix = (int*)malloc(ROWS * COLS * sizeof(int));
    
    // Initialize matrix
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            matrix[i * COLS + j] = i * COLS + j;
        }
    }
    
    // Combined gang and worker partitioning
    #pragma acc parallel num_gangs(4) num_workers(4) copy(matrix[0:ROWS*COLS])
    {
        int gang_id = acc_gang_id;
        int worker_id = acc_worker_id;
        
        int rows_per_gang = ROWS / 4;
        int start_row = gang_id * rows_per_gang;
        int end_row = (gang_id + 1) * rows_per_gang;
        
        int cols_per_worker = COLS / 4;
        int start_col = worker_id * cols_per_worker;
        int end_col = (worker_id + 1) * cols_per_worker;
        
        for (int i = start_row; i < end_row; i++) {
            for (int j = start_col; j < end_col; j++) {
                matrix[i * COLS + j] += 1;
            }
        }
    }
    
    free(matrix);
}

// Test case 4: vector partitioned
void test_vector_partitioned() {
    const int N = 256;
    short *data = (short*)malloc(N * sizeof(short));
    
    for (int i = 0; i < N; i++) {
        data[i] = (short)(i & 0xFF);
    }
    
    // Vector partitioning only
    #pragma acc parallel vector_length(32) copy(data[0:N])
    {
        int idx = acc_vector_id;
        if (idx < N) {
            data[idx] = data[idx] << 1;
        }
    }
    
    free(data);
}

// Test case 5: gang+vector partitioned
void test_gang_vector_partitioned() {
    const int N = 1024;
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N - i;
    }
    
    // Gang and vector partitioning
    #pragma acc parallel num_gangs(8) vector_length(16) copy(a[0:N], b[0:N])
    {
        int gang_id = acc_gang_id;
        int vector_id = acc_vector_id;
        
        int chunk_size = N / 8;
        int base = gang_id * chunk_size;
        int idx = base + vector_id;
        
        if (idx < N && idx < base + chunk_size) {
            a[idx] = a[idx] + b[idx];
        }
    }
    
    free(a);
    free(b);
}

// Test case 6: worker+vector partitioned
void test_worker_vector_partitioned() {
    const int N = 768;
    float *arr = (float*)malloc(N * sizeof(float));
    
    for (int i = 0; i < N; i++) {
        arr[i] = (float)i / 100.0f;
    }
    
    // Worker and vector partitioning
    #pragma acc parallel num_gangs(1) num_workers(6) vector_length(8) copy(arr[0:N])
    {
        int worker_id = acc_worker_id;
        int vector_id = acc_vector_id;
        
        int workers = acc_num_workers(acc_async_noval);
        int vectors = acc_vector_length(acc_async_noval);
        
        int idx = worker_id * vectors + vector_id;
        while (idx < N) {
            arr[idx] = arr[idx] * 1.5f;
            idx += workers * vectors;
        }
    }
    
    free(arr);
}

// Test case 7: fully partitioned
void test_fully_partitioned() {
    const int DIM1 = 16;
    const int DIM2 = 16;
    const int DIM3 = 16;
    int *cube = (int*)malloc(DIM1 * DIM2 * DIM3 * sizeof(int));
    
    // Initialize 3D array
    for (int i = 0; i < DIM1; i++) {
        for (int j = 0; j < DIM2; j++) {
            for (int k = 0; k < DIM3; k++) {
                cube[(i * DIM2 + j) * DIM3 + k] = i + j + k;
            }
        }
    }
    
    // Fully partitioned: gang, worker, and vector
    #pragma acc parallel num_gangs(4) num_workers(4) vector_length(4) copy(cube[0:DIM1*DIM2*DIM3])
    {
        int gang_id = acc_gang_id;
        int worker_id = acc_worker_id;
        int vector_id = acc_vector_id;
        
        // Each thread handles one element in the 3D space
        int idx = ((gang_id * 4 + worker_id) * 4 + vector_id);
        if (idx < DIM1 * DIM2 * DIM3) {
            cube[idx] += 1;
        }
    }
    
    free(cube);
}

// Test default case (invalid partition codes)
void test_invalid_partition() {
    // This function uses runtime conditions to potentially generate
    // invalid partition codes through macro expansion tricks
    
    int partition_type = force_runtime ? 8 : 0; // Invalid code 8
    
    // Use switch-like logic that mimics the compiler's internal mapping
    switch (partition_type) {
        case 0: case 1: case 2: case 3:
        case 4: case 5: case 6: case 7:
            // Valid cases handled elsewhere
            break;
        default:
            // This path could trigger the "<illegal>" string
            if (partition_type > 7) {
                fprintf(stderr, "Invalid partition code encountered\n");
            }
            break;
    }
    
    // Also test with OpenMP which might generate different partition codes
    #ifdef _OPENMP
    #pragma omp target teams distribute parallel for simd \
        num_teams(0) // Invalid team count might trigger edge cases
    for (int i = 0; i < 10; i++) {
        // Empty loop
    }
    #endif
}

// OpenMP versions for additional coverage
#ifdef _OPENMP
void test_omp_partitioning() {
    const int N = 100;
    int *arr = (int*)malloc(N * sizeof(int));
    
    // Test various OpenMP partitionings
    for (int i = 0; i < N; i++) arr[i] = i;
    
    // Case 1: Teams distribute (similar to gang partitioning)
    #pragma omp target teams distribute parallel for \
        num_teams(4) thread_limit(32) map(tofrom: arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] *= 2;
    }
    
    // Case 2: With simd (vector partitioning)
    #pragma omp target teams distribute parallel for simd \
        num_teams(2) map(tofrom: arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] += i;
    }
    
    free(arr);
}
#endif

int main() {
    printf("Testing OpenACC/OpenMP partition mapping coverage\n");
    
    // Execute all test cases
    test_gang_redundant();
    printf("Test 0 (gang redundant) completed\n");
    
    test_gang_partitioned();
    printf("Test 1 (gang partitioned) completed\n");
    
    test_worker_partitioned();
    printf("Test 2 (worker partitioned) completed\n");
    
    test_gang_worker_partitioned();
    printf("Test 3 (gang+worker partitioned) completed\n");
    
    test_vector_partitioned();
    printf("Test 4 (vector partitioned) completed\n");
    
    test_gang_vector_partitioned();
    printf("Test 5 (gang+vector partitioned) completed\n");
    
    test_worker_vector_partitioned();
    printf("Test 6 (worker+vector partitioned) completed\n");
    
    test_fully_partitioned();
    printf("Test 7 (fully partitioned) completed\n");
    
    test_invalid_partition();
    printf("Invalid partition test completed\n");
    
    #ifdef _OPENMP
    test_omp_partitioning();
    printf("OpenMP partitioning tests completed\n");
    #endif
    
    printf("All tests completed successfully\n");
    return 0;
}
