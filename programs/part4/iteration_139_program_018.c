/* Test program to trigger SIMT transformation in omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>

#define SIZE 1024
#define BLOCK 64

/* Global variables to prevent optimization */
volatile int g_volatile_bound = 0;
static int g_checksum = 0;

/* Function 1: target teams distribute parallel for simd with schedule(simd:static) */
void test_simt_wrapper_1(int *arr, int n, int *result) {
    int i;
    int pid = getpid() % 100;
    
    #pragma omp target if(pid > 50) device(simd:1) map(tofrom: arr[0:n]) map(to: n) map(from: result[0:1])
    #pragma omp teams distribute parallel for simd schedule(simd:static) num_teams(8) thread_limit(64)
    for (i = 0; i < n; i++) {
        arr[i] = arr[i] * 2 + pid;
    }
    
    /* Compute result */
    int sum = 0;
    volatile int v_i;
    for (v_i = 0; v_i < n && v_i < 10; v_i++) {
        sum += arr[v_i];
    }
    result[0] = sum;
}

/* Function 2: target teams distribute simd with dist_schedule and reduction */
void test_simt_wrapper_2(float *a, float *b, int n, float *sum_result) {
    int i;
    float sum = 0.0f;
    volatile int bound = n;
    
    /* Allocate device memory explicitly */
    float *d_a = (float *)omp_target_alloc(n * sizeof(float), omp_get_default_device());
    float *d_b = (float *)omp_target_alloc(n * sizeof(float), omp_get_default_device());
    
    if (d_a && d_b) {
        #pragma omp target if(0) device(ancestor:1) is_device_ptr(d_a, d_b) map(to: n) map(from: sum_result[0:1])
        #pragma omp teams distribute simd dist_schedule(static, 16) reduction(+:sum) num_teams(4)
        for (i = 0; i < bound; i++) {
            sum += d_a[i] * d_b[i];
        }
        
        sum_result[0] = sum;
        
        omp_target_free(d_a, omp_get_default_device());
        omp_target_free(d_b, omp_get_default_device());
    }
}

/* Function 3: Complex nesting with target region containing for loop with simd clause */
void test_simt_wrapper_3(int *matrix, int rows, int cols, int *row_sums) {
    int i, j;
    volatile int v_rows = rows;
    volatile int v_cols = cols;
    
    #pragma omp target if(1) map(tofrom: matrix[0:rows*cols]) map(to: rows, cols) map(from: row_sums[0:rows])
    {
        #pragma omp teams num_teams(rows/16) thread_limit(32)
        {
            #pragma omp distribute
            for (i = 0; i < v_rows; i++) {
                int row_sum = 0;
                
                #pragma omp parallel
                #pragma omp taskloop simd collapse(2) shared(matrix) reduction(+:row_sum)
                for (j = 0; j < v_cols; j++) {
                    row_sum += matrix[i * cols + j];
                }
                
                row_sums[i] = row_sum;
            }
        }
    }
}

/* Function 4: Mixed constructs with runtime bounds */
void test_simt_wrapper_4(double *data, int size, double *min_max) {
    int i;
    double local_min = 1e100;
    double local_max = -1e100;
    volatile int chunk = 32;
    
    /* Use runtime-dependent bound */
    int dynamic_bound = size;
    if (g_volatile_bound > 0) {
        dynamic_bound = g_volatile_bound % size;
    }
    
    #pragma omp target if(dynamic_bound > size/2) map(tofrom: data[0:size]) map(to: dynamic_bound) map(from: min_max[0:2])
    #pragma omp teams distribute parallel for simd schedule(static, chunk) \
                reduction(min:local_min) reduction(max:local_max) \
                num_teams((dynamic_bound + 255)/256)
    for (i = 0; i < dynamic_bound; i++) {
        if (data[i] < local_min) local_min = data[i];
        if (data[i] > local_max) local_max = data[i];
        data[i] = (data[i] - local_min) / (local_max - local_min + 1e-10);
    }
    
    min_max[0] = local_min;
    min_max[1] = local_max;
}

/* Helper function to initialize arrays */
void init_array(int *arr, int n, int seed) {
    for (int i = 0; i < n; i++) {
        arr[i] = (i * 17 + seed) % 100;
    }
}

void init_float_array(float *arr, int n, int seed) {
    for (int i = 0; i < n; i++) {
        arr[i] = (float)((i * 23 + seed) % 100) / 10.0f;
    }
}

void init_matrix(int *mat, int rows, int cols, int seed) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            mat[i * cols + j] = (i * cols + j + seed) % 50;
        }
    }
}

void init_double_array(double *arr, int n, int seed) {
    for (int i = 0; i < n; i++) {
        arr[i] = (double)((i * 37 + seed) % 200 - 100) / 5.0;
    }
}

int main(int argc, char *argv[]) {
    int i;
    
    /* Initialize with some runtime-dependent value */
    int seed = getpid();
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    g_volatile_bound = seed % SIZE;
    
    printf("Test SIMT transformation with seed: %d\n", seed);
    
    /* Test 1: Simple array transformation */
    int arr1[SIZE];
    int result1[1] = {0};
    init_array(arr1, SIZE, seed);
    
    test_simt_wrapper_1(arr1, SIZE, result1);
    
    /* Verify result1 */
    int check1 = 0;
    for (i = 0; i < SIZE && i < 10; i++) {
        check1 += arr1[i];
    }
    printf("Test 1: result=%d, expected~=%d\n", result1[0], check1);
    g_checksum += (result1[0] == check1) ? 1 : 0;
    
    /* Test 2: Dot product with device pointers */
    float arr2_a[BLOCK];
    float arr2_b[BLOCK];
    float result2[1] = {0.0f};
    init_float_array(arr2_a, BLOCK, seed);
    init_float_array(arr2_b, BLOCK, seed + 1);
    
    /* Copy to device-allocated memory */
    float *d_a = (float *)omp_target_alloc(BLOCK * sizeof(float), omp_get_default_device());
    float *d_b = (float *)omp_target_alloc(BLOCK * sizeof(float), omp_get_default_device());
    
    if (d_a && d_b) {
        #pragma omp target data map(to: arr2_a[0:BLOCK], arr2_b[0:BLOCK])
        {
            #pragma omp target is_device_ptr(d_a, d_b)
            {
                for (i = 0; i < BLOCK; i++) {
                    d_a[i] = arr2_a[i];
                    d_b[i] = arr2_b[i];
                }
            }
        }
        
        test_simt_wrapper_2(d_a, d_b, BLOCK, result2);
        
        /* Verify result2 */
        float check2 = 0.0f;
        for (i = 0; i < BLOCK; i++) {
            check2 += arr2_a[i] * arr2_b[i];
        }
        printf("Test 2: result=%.2f, expected=%.2f\n", result2[0], check2);
        g_checksum += (fabs(result2[0] - check2) < 0.01f) ? 1 : 0;
        
        omp_target_free(d_a, omp_get_default_device());
        omp_target_free(d_b, omp_get_default_device());
    }
    
    /* Test 3: Matrix row sums */
    int rows = 32;
    int cols = 32;
    int matrix[rows * cols];
    int row_sums[rows];
    init_matrix(matrix, rows, cols, seed);
    
    test_simt_wrapper_3(matrix, rows, cols, row_sums);
    
    /* Verify row sums */
    int check3 = 1;
    for (i = 0; i < rows; i++) {
        int row_sum = 0;
        for (int j = 0; j < cols; j++) {
            row_sum += matrix[i * cols + j];
        }
        if (row_sums[i] != row_sum) {
            check3 = 0;
            printf("Mismatch at row %d: %d vs %d\n", i, row_sums[i], row_sum);
        }
    }
    printf("Test 3: %s\n", check3 ? "PASS" : "FAIL");
    g_checksum += check3;
    
    /* Test 4: Min/max normalization */
    double data4[SIZE];
    double min_max[2] = {0.0, 0.0};
    init_double_array(data4, SIZE, seed);
    
    test_simt_wrapper_4(data4, SIZE, min_max);
    
    /* Verify min_max */
    double check_min = 1e100;
    double check_max = -1e100;
    for (i = 0; i < SIZE; i++) {
        if (data4[i] < check_min) check_min = data4[i];
        if (data4[i] > check_max) check_max = data4[i];
    }
    printf("Test 4: min=%.2f/%.2f, max=%.2f/%.2f\n", 
           min_max[0], check_min, min_max[1], check_max);
    g_checksum += (fabs(min_max[0] - check_min) < 0.01 && 
                   fabs(min_max[1] - check_max) < 0.01) ? 1 : 0;
    
    printf("\nTotal checks passed: %d/4\n", g_checksum);
    
    return (g_checksum == 4) ? 0 : 1;
}
