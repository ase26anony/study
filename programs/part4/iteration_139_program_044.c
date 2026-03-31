/* Test program to trigger SIMT transformation in omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>

#define N 1024
#define M 512
#define CHUNK 16

/* Global variables to prevent optimization */
volatile int g_volatile_counter = 0;
static int g_static_results[4] = {0};

/* Function 1: target teams distribute parallel for simd with schedule(simd:static) */
void test_target_teams_distribute_parallel_for_simd(int *arr, int size, int base)
{
    volatile int vol_bound = size;
    int num_teams = (size + 255) / 256;
    if (num_teams < 1) num_teams = 1;
    if (num_teams > 8) num_teams = 8;
    
    /* Use device clause to potentially trigger SIMT path */
    #pragma omp target if(0) device(simd:1) map(tofrom: arr[0:size]) \
                     map(to: size, base) num_teams(num_teams) thread_limit(256)
    #pragma omp teams distribute parallel for simd schedule(simd:static, CHUNK) \
                     collapse(2)
    for (int i = 0; i < vol_bound; i++) {
        for (int j = 0; j < 2; j++) {
            int idx = i * 2 + j;
            if (idx < size) {
                arr[idx] = base + i * 100 + j;
            }
        }
    }
    
    /* Store result for verification */
    g_static_results[0] = arr[size-1];
}

/* Function 2: target teams distribute simd with reduction */
void test_target_teams_distribute_simd(float *results, int size, int pid)
{
    volatile int vol_size = size;
    float sum = 0.0f;
    
    /* Use ancestor device clause */
    #pragma omp target device(ancestor:1) map(tofrom: sum) \
                     map(to: results[0:size], vol_size, pid) \
                     dist_schedule(static, 16)
    #pragma omp teams distribute simd reduction(+:sum)
    for (int i = 0; i < vol_size; i++) {
        results[i] = (float)(pid + i) * 0.5f;
        sum += results[i];
    }
    
    g_static_results[1] = (int)sum;
    
    /* Nested target with is_device_ptr */
    float *dev_ptr = (float*)omp_target_alloc(size * sizeof(float), 
                                             omp_get_default_device());
    if (dev_ptr) {
        #pragma omp target is_device_ptr(dev_ptr) map(to: size) if(1)
        #pragma omp teams distribute simd
        for (int i = 0; i < size; i++) {
            dev_ptr[i] = results[i] * 2.0f;
        }
        
        /* Copy back and verify */
        #pragma omp target is_device_ptr(dev_ptr) map(from: results[0:size])
        #pragma omp teams distribute simd
        for (int i = 0; i < size; i++) {
            results[i] = dev_ptr[i];
        }
        
        omp_target_free(dev_ptr, omp_get_default_device());
    }
}

/* Function 3: Complex nesting with taskloop simd inside teams */
void test_complex_nesting(int *arr1, int *arr2, int size, int seed)
{
    volatile int vol_seed = seed;
    
    #pragma omp target map(tofrom: arr1[0:size], arr2[0:size]) \
                     map(to: size, vol_seed) device(0)
    {
        #pragma omp teams num_teams(4) thread_limit(64)
        {
            int team_id = omp_get_team_num();
            
            #pragma omp distribute simd dist_schedule(static)
            for (int i = 0; i < size; i++) {
                arr1[i] = team_id * 1000 + i + vol_seed;
            }
            
            /* Taskloop with simd inside teams region */
            #pragma omp single
            {
                #pragma omp taskloop simd grainsize(64) \
                                 shared(arr2) private(team_id)
                for (int i = 0; i < size; i++) {
                    arr2[i] = arr1[size - i - 1] * 2;
                }
            }
        }
    }
    
    /* Compute checksum */
    int checksum = 0;
    for (int i = 0; i < size && i < 10; i++) {
        checksum += arr1[i] + arr2[i];
    }
    g_static_results[2] = checksum;
}

/* Additional test: Mixed constructs with runtime bounds */
void test_mixed_constructs(double *data, int rows, int cols, int pid)
{
    volatile int vol_rows = rows;
    volatile int vol_cols = cols;
    
    /* Use if clause with runtime value */
    int use_simt = (pid % 2 == 0) ? 1 : 0;
    
    #pragma omp target if(use_simt) map(tofrom: data[0:rows*cols]) \
                     map(to: vol_rows, vol_cols) device(simd:1)
    #pragma omp teams distribute parallel for simd collapse(2) \
                     schedule(static, 8) num_teams(4)
    for (int i = 0; i < vol_rows; i++) {
        for (int j = 0; j < vol_cols; j++) {
            int idx = i * cols + j;
            data[idx] = (i * 1.5 + j * 0.7) / (pid + 1);
        }
    }
    
    /* Verify some values */
    double sum = 0.0;
    for (int i = 0; i < rows * cols && i < 100; i++) {
        sum += data[i];
    }
    g_static_results[3] = (int)(sum * 1000);
}

int main(int argc, char *argv[])
{
    int pid = getpid();
    printf("Process ID: %d\n", pid);
    
    /* Initialize data arrays */
    int *arr1 = (int*)malloc(N * sizeof(int));
    int *arr2 = (int*)malloc(N * sizeof(int));
    float *results = (float*)malloc(M * sizeof(float));
    double *matrix = (double*)malloc(256 * 256 * sizeof(double));
    
    if (!arr1 || !arr2 || !results || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1[i] = 0;
        arr2[i] = 0;
    }
    for (int i = 0; i < M; i++) {
        results[i] = 0.0f;
    }
    for (int i = 0; i < 256*256; i++) {
        matrix[i] = 0.0;
    }
    
    /* Run test functions */
    printf("Running test 1...\n");
    test_target_teams_distribute_parallel_for_simd(arr1, N, pid);
    
    printf("Running test 2...\n");
    test_target_teams_distribute_simd(results, M, pid);
    
    printf("Running test 3...\n");
    test_complex_nesting(arr1, arr2, N/2, pid);
    
    printf("Running test 4...\n");
    test_mixed_constructs(matrix, 256, 256, pid);
    
    /* Verify results */
    int checksum1 = 0, checksum2 = 0;
    float checksum3 = 0.0f;
    double checksum4 = 0.0;
    
    for (int i = 0; i < N; i++) {
        checksum1 += arr1[i];
        if (i < N/2) checksum2 += arr2[i];
    }
    
    for (int i = 0; i < M; i++) {
        checksum3 += results[i];
    }
    
    for (int i = 0; i < 256*256 && i < 1000; i++) {
        checksum4 += matrix[i];
    }
    
    printf("\nVerification results:\n");
    printf("Array1 checksum: %d\n", checksum1);
    printf("Array2 checksum: %d\n", checksum2);
    printf("Results sum: %.2f\n", checksum3);
    printf("Matrix partial sum: %.4f\n", checksum4);
    printf("Static results: [%d, %d, %d, %d]\n",
           g_static_results[0], g_static_results[1],
           g_static_results[2], g_static_results[3]);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(results);
    free(matrix);
    
    return 0;
}
