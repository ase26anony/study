/* Test program to trigger SIMT transformation in omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>

#define SIZE 1024
#define BLOCK 64

/* Global variables to prevent optimization */
volatile int g_volatile_counter = 0;
static int g_checksum = 0;

/* Function 1: target teams distribute parallel for simd with schedule(simd:static) */
void test_simt_wrapper_1(int *arr, int n, int *result) {
    int i, j;
    volatile int bound = n;
    
    /* Use device clause that might trigger SIMT transformation */
    #pragma omp target map(tofrom: arr[0:n], result[0:n]) \
                      map(to: bound) if(0) device(simd:1)
    #pragma omp teams num_teams(8) thread_limit(32)
    #pragma omp distribute parallel for simd \
                schedule(simd:static, BLOCK) collapse(2)
    for (i = 0; i < bound; i++) {
        for (j = 0; j < BLOCK/4; j++) {
            int idx = i * (BLOCK/4) + j;
            if (idx < n) {
                arr[idx] = idx * 2 + (i % 4);
                result[idx] = arr[idx] + (j % 8);
            }
        }
    }
    
    /* Compute checksum */
    #pragma omp simd reduction(+:g_checksum)
    for (i = 0; i < n && i < SIZE; i++) {
        g_checksum += result[i];
    }
}

/* Function 2: target teams distribute simd with reduction */
void test_simt_wrapper_2(float *data, int n, float *sum_ptr) {
    int i;
    volatile int start = getpid() % 100;
    float sum = 0.0f;
    
    /* Use ancestor device clause which might trigger SIMT path */
    #pragma omp target map(to: data[0:n]) map(from: sum) \
                      device(ancestor:1) if(start > 50)
    #pragma omp teams num_teams(4) dist_schedule(static, 16)
    #pragma omp distribute simd reduction(+:sum)
    for (i = 0; i < n; i++) {
        data[i] = (float)i / (n + 1.0f);
        sum += data[i] * (i % 3 + 1);
    }
    
    *sum_ptr = sum;
    
    /* Additional nested loop to create more complex control flow */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: data[0:n]) if(1) device(simd:0)
    for (i = 0; i < n; i += 2) {
        if (i + 1 < n) {
            float tmp = data[i];
            data[i] = data[i + 1];
            data[i + 1] = tmp;
        }
    }
}

/* Function 3: Complex nesting with taskloop simd inside teams */
void test_simt_wrapper_3(int *a, int *b, int *c, int n) {
    int i, j;
    volatile int chunk = 32;
    
    /* Allocate device memory explicitly */
    int *dev_a = (int *)omp_target_alloc(n * sizeof(int), 
                                         omp_get_default_device());
    int *dev_b = (int *)omp_target_alloc(n * sizeof(int), 
                                         omp_get_default_device());
    
    if (dev_a && dev_b) {
        /* Copy data to device */
        #pragma omp target enter data map(to: a[0:n], b[0:n]) \
                          device(omp_get_default_device())
        
        /* Complex target region with mixed constructs */
        #pragma omp target is_device_ptr(dev_a, dev_b) \
                          map(from: c[0:n]) if(0) device(simd:1)
        #pragma omp teams num_teams(2)
        {
            #pragma omp distribute
            for (i = 0; i < n; i += chunk) {
                int end = i + chunk;
                if (end > n) end = n;
                
                #pragma omp parallel
                {
                    #pragma omp taskloop simd nogroup \
                                shared(dev_a, dev_b, c) \
                                firstprivate(i, end)
                    for (j = i; j < end; j++) {
                        dev_a[j] = j * 3;
                        dev_b[j] = j * 5;
                        c[j] = dev_a[j] + dev_b[j] + (j % 7);
                    }
                }
            }
        }
        
        /* Cleanup */
        omp_target_free(dev_a, omp_get_default_device());
        omp_target_free(dev_b, omp_get_default_device());
        
        #pragma omp target exit data map(from: a[0:n], b[0:n]) \
                          device(omp_get_default_device())
    }
}

/* Helper function with runtime-dependent bounds */
void test_variable_bounds(int *output, int max_size) {
    int i;
    volatile int actual_size = (getpid() % 100) + 100;
    if (actual_size > max_size) actual_size = max_size;
    
    /* Multiple SIMD constructs with variable bounds */
    #pragma omp target teams distribute parallel for simd \
                map(from: output[0:actual_size]) \
                if(actual_size > 150) device(simd:1)
    for (i = 0; i < actual_size; i++) {
        output[i] = i * i - (i % 11);
    }
    
    /* Nested loop with collapse */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: output[0:actual_size]) collapse(2) \
                schedule(simd:guided) if(0)
    for (i = 0; i < actual_size/16; i++) {
        for (int k = 0; k < 16; k++) {
            int idx = i * 16 + k;
            if (idx < actual_size) {
                output[idx] += (i * k) % 17;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    int arr[SIZE];
    int result[SIZE];
    float fdata[SIZE/2];
    float sum;
    int output[SIZE];
    int i;
    
    /* Initialize arrays with pattern */
    for (i = 0; i < SIZE; i++) {
        arr[i] = i;
        result[i] = 0;
        if (i < SIZE/2) fdata[i] = 0.0f;
        output[i] = 0;
    }
    
    printf("Starting SIMT transformation tests...\n");
    
    /* Test 1: Basic SIMT wrapper */
    printf("Test 1: target teams distribute parallel for simd\n");
    test_simt_wrapper_1(arr, SIZE, result);
    
    /* Verify results */
    int local_sum = 0;
    #pragma omp simd reduction(+:local_sum)
    for (i = 0; i < SIZE; i++) {
        local_sum += result[i];
    }
    printf("  Checksum 1: %d (global: %d)\n", local_sum, g_checksum);
    
    /* Test 2: Reduction with SIMD */
    printf("Test 2: target teams distribute simd with reduction\n");
    test_simt_wrapper_2(fdata, SIZE/2, &sum);
    printf("  Sum: %f\n", sum);
    
    /* Test 3: Complex nesting with device pointers */
    printf("Test 3: Complex nesting with taskloop simd\n");
    int a[SIZE/4], b[SIZE/4], c[SIZE/4];
    for (i = 0; i < SIZE/4; i++) {
        a[i] = i * 2;
        b[i] = i * 3;
        c[i] = 0;
    }
    test_simt_wrapper_3(a, b, c, SIZE/4);
    
    /* Verify c array */
    int sum_c = 0;
    for (i = 0; i < SIZE/4; i++) {
        sum_c += c[i];
    }
    printf("  Array C sum: %d\n", sum_c);
    
    /* Test 4: Variable bounds */
    printf("Test 4: Variable bounds with runtime condition\n");
    test_variable_bounds(output, SIZE);
    
    /* Final verification */
    int final_check = 0;
    #pragma omp parallel for simd reduction(+:final_check)
    for (i = 0; i < SIZE; i++) {
        final_check += output[i] % 100;
    }
    printf("Final check: %d\n", final_check);
    
    /* Print compilation hint */
    printf("\nCompile with: gcc -O2 -fopenmp -foffload=nvptx-none ");
    printf("-fdump-tree-omplower -o simt_test simt_test.c\n");
    
    return 0;
}
