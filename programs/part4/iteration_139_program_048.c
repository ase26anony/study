/* Test program to trigger SIMT transformation in omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>

#define SIZE 1024
#define BLOCK 64

/* Global variables to prevent optimization */
volatile int g_bound1 = 0;
volatile int g_bound2 = 0;
static int g_checksum = 0;

/* Device memory pointers */
static int *dev_ptr1 = NULL;
static float *dev_ptr2 = NULL;

/* Function 1: target teams distribute parallel for simd with schedule(simd:static) */
void test_simt_wrapper_1(int *arr, int n, int base) {
    volatile int local_bound = n + getpid() % 16; /* Runtime-dependent bound */
    
    #pragma omp target if(0) device(simd:1) map(tofrom: arr[0:n]) map(to: n, base) \
                     num_teams(4) thread_limit(128)
    #pragma omp teams distribute parallel for simd schedule(simd:static, 32) \
                     num_threads(64)
    for (int i = 0; i < n; i++) {
        arr[i] = base + i * (i % 16);
    }
    
    /* Nested loop to potentially trigger collapse */
    #pragma omp target if(1) device(ancestor:1) map(tofrom: arr[0:n/2]) \
                     num_teams(2)
    #pragma omp teams distribute parallel for simd collapse(2) \
                     schedule(static, 8)
    for (int i = 0; i < local_bound/2; i++) {
        for (int j = 0; j < 8; j++) {
            int idx = i * 8 + j;
            if (idx < n) {
                arr[idx] += (i * j) & 0xFF;
            }
        }
    }
}

/* Function 2: target teams distribute simd with reduction */
float test_simt_wrapper_2(float *data, int m, float init) {
    float sum = init;
    volatile int dynamic_m = m + (getpid() & 0xF); /* Prevent constant propagation */
    
    #pragma omp target if(dynamic_m > 100) device(simd:2) \
                     map(tofrom: sum) map(to: data[0:m], dynamic_m) \
                     dist_schedule(static, 16)
    #pragma omp teams distribute simd reduction(+:sum) num_teams(8)
    for (int i = 0; i < dynamic_m; i++) {
        sum += data[i] * (i % 7);
    }
    
    /* Additional loop with device pointer */
    if (!dev_ptr1) {
        dev_ptr1 = (int*)omp_target_alloc(SIZE * sizeof(int), 
                                         omp_get_default_device());
    }
    
    #pragma omp target if(1) is_device_ptr(dev_ptr1) map(to: dynamic_m) \
                     device(ancestor:2)
    #pragma omp teams distribute parallel for simd
    for (int i = 0; i < dynamic_m && i < SIZE; i++) {
        dev_ptr1[i] = i * i;
    }
    
    return sum;
}

/* Function 3: Complex nesting with taskloop simd inside teams */
void test_simt_wrapper_3(int *out, float *in, int size) {
    volatile int chunk = 16 + (getpid() % 8);
    
    #pragma omp target if(size > 256) map(to: in[0:size]) map(from: out[0:size]) \
                     device(simd:3) num_teams(4)
    {
        #pragma omp teams distribute
        for (int team = 0; team < 4; team++) {
            int start = team * (size / 4);
            int end = (team == 3) ? size : (team + 1) * (size / 4);
            
            #pragma omp parallel
            {
                #pragma omp taskloop simd grainsize(chunk) \
                                 if(end-start > 32) shared(in, out)
                for (int i = start; i < end; i++) {
                    float val = in[i];
                    out[i] = (int)(val * val) + (i % 64);
                }
            }
        }
        
        /* SIMD loop with device-allocated memory */
        if (!dev_ptr2) {
            dev_ptr2 = (float*)omp_target_alloc(SIZE * sizeof(float), 
                                               omp_get_default_device());
        }
        
        #pragma omp teams distribute simd is_device_ptr(dev_ptr2) \
                     dist_schedule(static, 32)
        for (int i = 0; i < size && i < SIZE; i++) {
            dev_ptr2[i] = in[i] * 0.5f;
        }
    }
}

/* Helper to compute checksum */
int compute_checksum(int *arr, int n) {
    int sum = 0;
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += arr[i] & 0xFF;
    }
    return sum;
}

float compute_fchecksum(float *arr, int n) {
    float sum = 0.0f;
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char **argv) {
    int *array1 = (int*)malloc(SIZE * sizeof(int));
    int *array2 = (int*)malloc(SIZE * sizeof(int));
    float *farray = (float*)malloc(SIZE * sizeof(float));
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = i % 100;
        array2[i] = (i * 3) % 100;
        farray[i] = (i % 50) * 0.1f;
    }
    
    printf("Starting SIMT transformation tests...\n");
    
    /* Test 1: Basic SIMT wrapper */
    printf("\nTest 1: target teams distribute parallel for simd\n");
    test_simt_wrapper_1(array1, SIZE, 1000);
    int checksum1 = compute_checksum(array1, SIZE);
    printf("Checksum 1: %d\n", checksum1);
    g_checksum += checksum1;
    
    /* Test 2: SIMD with reduction */
    printf("\nTest 2: target teams distribute simd with reduction\n");
    float sum2 = test_simt_wrapper_2(farray, SIZE, 10.0f);
    printf("Reduction sum: %.2f\n", sum2);
    g_checksum += (int)sum2;
    
    /* Test 3: Complex nesting */
    printf("\nTest 3: Nested taskloop simd in teams\n");
    test_simt_wrapper_3(array2, farray, SIZE);
    int checksum3 = compute_checksum(array2, SIZE);
    printf("Checksum 3: %d\n", checksum3);
    g_checksum += checksum3;
    
    /* Additional test with runtime bounds from argv */
    if (argc > 1) {
        int custom_size = atoi(argv[1]);
        if (custom_size > 0 && custom_size <= SIZE) {
            printf("\nAdditional test with size %d\n", custom_size);
            test_simt_wrapper_1(array1, custom_size, 500);
            int custom_cs = compute_checksum(array1, custom_size);
            printf("Custom checksum: %d\n", custom_cs);
            g_checksum += custom_cs;
        }
    }
    
    printf("\nTotal checksum: %d\n", g_checksum);
    
    /* Cleanup device memory */
    if (dev_ptr1) {
        omp_target_free(dev_ptr1, omp_get_default_device());
    }
    if (dev_ptr2) {
        omp_target_free(dev_ptr2, omp_get_default_device());
    }
    
    free(array1);
    free(array2);
    free(farray);
    
    return 0;
}
