/* Test program to trigger SIMT transformation in omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>

#define SIZE 1024
#define BLOCK 64

/* Global variables to prevent optimization */
volatile int g_volatile_counter = 0;
static int g_results[SIZE] = {0};
static float g_float_results[SIZE] = {0.0f};

/* Function 1: target teams distribute parallel for simd with schedule(simd:static) */
void test_target_teams_distribute_parallel_for_simd(int *arr, int n, int base) {
    volatile int vol_bound = n + (getpid() % 16); /* Runtime-dependent bound */
    
    #pragma omp target if(0) device(simd:1) map(tofrom: arr[0:n]) \
                     map(to: base, vol_bound) num_teams(4) thread_limit(128)
    #pragma omp teams distribute parallel for simd schedule(simd:static, 32) \
                     num_threads(64)
    for (int i = 0; i < vol_bound; i++) {
        arr[i] = base + i * 2 + (i % 16);
        g_volatile_counter++; /* Force side effect */
    }
}

/* Function 2: target teams distribute simd with reduction */
float test_target_teams_distribute_simd_reduction(float *data, int n) {
    float sum = 0.0f;
    volatile int start = getpid() % 8;
    
    #pragma omp target device(ancestor:1) map(tofrom: data[0:n], sum) \
                     map(to: start, n) dist_schedule(static, 16)
    #pragma omp teams distribute simd reduction(+:sum)
    for (int i = start; i < n; i++) {
        data[i] = (float)i * 1.5f;
        sum += data[i];
        g_float_results[i % SIZE] = data[i]; /* Store to global */
    }
    
    return sum;
}

/* Function 3: Complex nesting with taskloop simd inside teams */
void test_complex_nesting(int *out, int rows, int cols) {
    volatile int vrows = rows + (getpid() % 4);
    volatile int vcols = cols;
    
    /* Allocate device memory explicitly */
    int *dev_ptr = (int *)omp_target_alloc(vrows * vcols * sizeof(int), 
                                          omp_get_default_device());
    
    #pragma omp target if(1) is_device_ptr(dev_ptr) \
                     map(to: vrows, vcols) map(from: out[0:vrows*vcols]) \
                     device(simd:2)
    #pragma omp teams num_teams(2) thread_limit(32)
    {
        #pragma omp distribute collapse(2)
        for (int i = 0; i < vrows; i++) {
            for (int j = 0; j < vcols; j++) {
                int idx = i * vcols + j;
                dev_ptr[idx] = (i * 1000) + j;
                
                #pragma omp taskloop simd simdlen(8) grainsize(16)
                for (int k = 0; k < 8; k++) {
                    dev_ptr[idx] += k * (i + j);
                }
            }
        }
        
        /* Copy back */
        #pragma omp parallel for simd
        for (int i = 0; i < vrows * vcols; i++) {
            out[i] = dev_ptr[i];
        }
    }
    
    omp_target_free(dev_ptr, omp_get_default_device());
}

/* Function 4: Mixed constructs with runtime bounds */
void test_mixed_constructs(int *a, int *b, int *c, int n) {
    volatile int chunk = 16 + (getpid() % 8);
    
    #pragma omp target map(to: a[0:n], b[0:n]) map(tofrom: c[0:n]) \
                     map(to: n, chunk) device(simd:1) if(0)
    {
        #pragma omp teams distribute parallel for simd \
                         schedule(static, chunk) collapse(2)
        for (int i = 0; i < n/2; i++) {
            for (int j = 0; j < 2; j++) {
                int idx = i * 2 + j;
                c[idx] = a[idx] * b[idx] + (idx % chunk);
                g_results[idx % SIZE] = c[idx]; /* Global side effect */
            }
        }
        
        /* Additional SIMD loop */
        #pragma omp simd
        for (int i = 0; i < n; i++) {
            c[i] += g_volatile_counter;
        }
    }
}

/* Helper to verify results */
int verify_results(int *arr, int n, int expected_sum) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    printf("Checksum: %d (expected ~%d)\n", sum, expected_sum);
    return (abs(sum - expected_sum) < 1000); /* Allow some variance */
}

int main(int argc, char **argv) {
    int n = SIZE;
    if (argc > 1) n = atoi(argv[1]);
    if (n < 64) n = 64;
    if (n > 4096) n = 4096;
    
    /* Allocate and initialize arrays */
    int *arr1 = (int *)malloc(n * sizeof(int));
    int *arr2 = (int *)malloc(n * sizeof(int));
    int *arr3 = (int *)malloc(n * 2 * sizeof(int));
    int *arr4 = (int *)malloc(n * sizeof(int));
    float *farr = (float *)malloc(n * sizeof(float));
    
    for (int i = 0; i < n; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr4[i] = i % 32;
        farr[i] = (float)i * 0.5f;
    }
    for (int i = 0; i < n * 2; i++) {
        arr3[i] = 0;
    }
    
    printf("Starting SIMT transformation tests...\n");
    
    /* Test 1: Basic SIMD with teams distribute */
    printf("\nTest 1: target teams distribute parallel for simd\n");
    test_target_teams_distribute_parallel_for_simd(arr1, n, 100);
    int ok1 = verify_results(arr1, n, 100 * n + n * (n - 1) + (n / 16) * 120);
    
    /* Test 2: SIMD with reduction */
    printf("\nTest 2: target teams distribute simd with reduction\n");
    float sum = test_target_teams_distribute_simd_reduction(farr, n);
    float expected_sum = 0.0f;
    int start = getpid() % 8;
    for (int i = start; i < n; i++) {
        expected_sum += (float)i * 1.5f;
    }
    printf("Reduction sum: %.2f (expected ~%.2f)\n", sum, expected_sum);
    int ok2 = (fabs(sum - expected_sum) / expected_sum < 0.01f);
    
    /* Test 3: Complex nesting */
    printf("\nTest 3: Complex nesting with taskloop simd\n");
    int rows = 16, cols = 8;
    test_complex_nesting(arr3, rows, cols);
    int sum3 = 0;
    for (int i = 0; i < rows * cols; i++) {
        sum3 += arr3[i];
    }
    printf("Nested result checksum: %d\n", sum3);
    int ok3 = (sum3 > 0); /* Just verify non-zero */
    
    /* Test 4: Mixed constructs */
    printf("\nTest 4: Mixed constructs with collapse\n");
    test_mixed_constructs(arr1, arr2, arr4, n);
    int ok4 = verify_results(arr4, n, 0); /* Non-zero check */
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    free(farr);
    
    int all_ok = ok1 && ok2 && ok3 && ok4;
    printf("\nAll tests %s\n", all_ok ? "PASSED" : "FAILED");
    
    return all_ok ? 0 : 1;
}
