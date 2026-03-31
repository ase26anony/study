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
void test_target_teams_distribute_parallel_for_simd(int *arr, int n) {
    int i;
    int pid = getpid() % 100;
    
    #pragma omp target map(tofrom: arr[0:n]) if(pid > 50) device(simd:1)
    #pragma omp teams num_teams(8) thread_limit(64)
    #pragma omp distribute parallel for simd schedule(simd:static, 32) \
                private(i) shared(arr)
    for (i = 0; i < n; i++) {
        arr[i] = (i + pid) * 2;
    }
}

/* Function 2: target teams distribute simd with reduction */
void test_target_teams_distribute_simd_reduction(float *results, int m) {
    float sum = 0.0f;
    int j;
    volatile int vol_bound = m;
    
    #pragma omp target map(to: results[0:m]) map(from: sum) \
                device(ancestor:1) if(0)
    #pragma omp teams num_teams(4) dist_schedule(static, 16)
    #pragma omp distribute simd reduction(+:sum)
    for (j = 0; j < vol_bound; j++) {
        sum += results[j] * 0.5f;
    }
    
    /* Store result to global to prevent dead code elimination */
    #pragma omp atomic
    g_checksum += (int)sum;
}

/* Function 3: Complex nesting with taskloop simd inside teams */
void test_complex_nesting(int *data, int rows, int cols) {
    int i, j;
    int *dev_ptr = NULL;
    int host_data[BLOCK][BLOCK];
    
    /* Initialize host data */
    for (i = 0; i < BLOCK; i++) {
        for (j = 0; j < BLOCK; j++) {
            host_data[i][j] = i * cols + j;
        }
    }
    
    /* Allocate device memory */
    size_t dev_size = BLOCK * BLOCK * sizeof(int);
    dev_ptr = (int *)omp_target_alloc(dev_size, omp_get_default_device());
    
    if (dev_ptr) {
        #pragma omp target map(to: host_data) is_device_ptr(dev_ptr) \
                    if(rows > cols) device(simd:2)
        #pragma omp teams
        {
            #pragma omp distribute
            for (i = 0; i < BLOCK; i++) {
                #pragma omp parallel
                {
                    #pragma omp taskloop simd collapse(2) \
                                shared(dev_ptr, host_data) private(j)
                    for (int ii = 0; ii < 8; ii++) {
                        for (int jj = 0; jj < 8; jj++) {
                            int idx = (i * 8 + ii) * BLOCK + jj;
                            dev_ptr[idx] = host_data[ii][jj] + i;
                        }
                    }
                }
            }
        }
        
        /* Copy back and accumulate */
        #pragma omp target map(from: data[0:BLOCK*BLOCK]) \
                    is_device_ptr(dev_ptr)
        #pragma omp teams distribute parallel for simd \
                    collapse(2) schedule(static)
        for (i = 0; i < BLOCK; i++) {
            for (j = 0; j < BLOCK; j++) {
                data[i * BLOCK + j] = dev_ptr[i * BLOCK + j];
            }
        }
        
        omp_target_free(dev_ptr, omp_get_default_device());
    }
}

/* Function 4: Mixed constructs with runtime bounds */
void test_mixed_constructs(double *vec, int len, int iter) {
    int k;
    double local_sum = 0.0;
    volatile int dynamic_bound = len;
    
    #pragma omp target map(to: vec[0:len]) map(tofrom: local_sum) \
                if(iter % 2 == 0) device(simd:3)
    #pragma omp teams distribute parallel for simd \
                reduction(+:local_sum) schedule(static, 8) \
                num_teams(iter % 4 + 1)
    for (k = 0; k < dynamic_bound; k++) {
        local_sum += vec[k] * (k + 1);
    }
    
    /* Use result to affect global state */
    #pragma omp atomic
    g_checksum += (int)(local_sum * 100);
}

/* Function 5: Nested loops with collapse */
void test_collapse_nested(int *matrix, int dim) {
    int i, j;
    
    #pragma omp target map(tofrom: matrix[0:dim*dim]) \
                device(ancestor:2) if(1)
    #pragma omp teams distribute parallel for simd \
                collapse(2) num_teams(16)
    for (i = 0; i < dim; i++) {
        for (j = 0; j < dim; j++) {
            int idx = i * dim + j;
            matrix[idx] = (i * j + getpid()) % 256;
        }
    }
}

int main(int argc, char *argv[]) {
    int arr[SIZE];
    float results[SIZE/2];
    int matrix[BLOCK][BLOCK];
    double vector[SIZE];
    int i, j;
    
    /* Initialize data with runtime-dependent values */
    int seed = getpid();
    srand(seed);
    
    for (i = 0; i < SIZE; i++) {
        arr[i] = rand() % 100;
        if (i < SIZE/2) {
            results[i] = (float)(rand() % 1000) / 10.0f;
        }
        vector[i] = (double)(rand() % 100) / 3.0;
    }
    
    for (i = 0; i < BLOCK; i++) {
        for (j = 0; j < BLOCK; j++) {
            matrix[i][j] = 0;
        }
    }
    
    printf("Starting SIMT transformation tests...\n");
    printf("PID: %d, Seed: %d\n", getpid(), seed);
    
    /* Test 1: Basic SIMT transformation */
    printf("\nTest 1: target teams distribute parallel for simd\n");
    test_target_teams_distribute_parallel_for_simd(arr, SIZE);
    
    /* Verify results */
    int sum1 = 0;
    for (i = 0; i < SIZE; i++) {
        sum1 += arr[i];
    }
    printf("Checksum 1: %d\n", sum1);
    
    /* Test 2: Reduction with SIMD */
    printf("\nTest 2: target teams distribute simd with reduction\n");
    test_target_teams_distribute_simd_reduction(results, SIZE/2);
    
    /* Test 3: Complex nesting */
    printf("\nTest 3: Complex nesting with taskloop simd\n");
    test_complex_nesting(&matrix[0][0], BLOCK, BLOCK);
    
    int sum3 = 0;
    for (i = 0; i < BLOCK; i++) {
        for (j = 0; j < BLOCK; j++) {
            sum3 += matrix[i][j];
        }
    }
    printf("Checksum 3: %d\n", sum3);
    
    /* Test 4: Mixed constructs */
    printf("\nTest 4: Mixed constructs with runtime bounds\n");
    test_mixed_constructs(vector, SIZE, argc);
    
    /* Test 5: Collapse nested loops */
    printf("\nTest 5: Nested loops with collapse\n");
    test_collapse_nested(&matrix[0][0], BLOCK);
    
    int sum5 = 0;
    for (i = 0; i < BLOCK; i++) {
        for (j = 0; j < BLOCK; j++) {
            sum5 += matrix[i][j];
        }
    }
    printf("Checksum 5: %d\n", sum5);
    
    printf("\nFinal global checksum: %d\n", g_checksum);
    printf("All tests completed.\n");
    
    return 0;
}
