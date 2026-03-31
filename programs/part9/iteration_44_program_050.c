#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define M 512
#define MAX_ITER 5

/* Variant 1: SIMD target loop with complex data environment */
void simd_target_loop(int *a, int *b, int *c, int start, int end, int step, 
                      volatile int n_iter, int use_simd) {
    static int static_counter = 0;
    const int const_offset = 10;
    int private_var = static_counter++;
    
    /* Mixed storage duration and qualifiers */
    static float static_weights[N];
    volatile int vol_bound = end;
    
    #pragma omp target teams distribute parallel for simd \
        if(use_simd) collapse(2) \
        map(to: a[start:end-start], b[start:end-start]) \
        map(from: c[start:end-start]) \
        private(private_var) firstprivate(const_offset) \
        shared(static_weights)
    for (int i = start; i < vol_bound; i += step) {
        for (int j = 0; j < n_iter; ++j) {
            int idx = i * M + j;
            /* Complex indexing with pointer arithmetic */
            int *ptr_a = a + i;
            int *ptr_b = b + i;
            int *ptr_c = c + i;
            
            /* SIMD-friendly operation with runtime-dependent index */
            float weight = static_weights[j % N];
            ptr_c[0] = (int)((*ptr_a + *ptr_b) * weight) + const_offset + private_var;
            
            /* Conditional execution within SIMD loop */
            if (j % 2 == 0) {
                ptr_c[0] += (i % 3) * 2;
            }
        }
    }
}

/* Variant 2: Parallel target loop without SIMD clause */
void parallel_target_loop(double *x, double *y, double *z, 
                         int low, int high, int stride,
                         volatile int iter_count) {
    int local_private = low * high;
    const double scale = 2.5;
    
    /* Array section with stride in map clause */
    #pragma omp target teams distribute parallel for \
        map(to: x[low:high-low:stride], y[low:high-low:stride]) \
        map(from: z[low:high-low:stride]) \
        firstprivate(local_private, scale)
    for (int i = low; i < high; i += stride) {
        double temp = 0.0;
        /* Nested loop to create complex iteration space */
        for (int k = 0; k < iter_count; ++k) {
            temp += x[i] * y[i + k % stride] * (k + 1);
        }
        z[i] = temp * scale + local_private;
        
        /* Runtime-dependent array access */
        if (i % 4 == 0) {
            z[i] /= (iter_count % 7 + 1);
        }
    }
}

/* Variant 3: Combined constructs with target data region */
void combined_constructs(float *src1, float *src2, float *dst,
                        int dim1, int dim2, int offset,
                        volatile int mode) {
    /* Static array with mixed qualifiers */
    static volatile int shared_counter = 0;
    const float pi = 3.14159f;
    int thread_specific = shared_counter++;
    
    /* Target data region enclosing parallel region */
    #pragma omp target data map(to: src1[0:dim1*dim2]) \
                            map(to: src2[offset:dim1*dim2-offset]) \
                            map(from: dst[0:dim1*dim2])
    {
        /* Complex loop with conditional SIMD */
        #pragma omp target teams distribute parallel for simd \
            collapse(2) if(mode > 0) \
            private(thread_specific) firstprivate(pi)
        for (int i = 0; i < dim1; ++i) {
            for (int j = 0; j < dim2; ++j) {
                int idx = i * dim2 + j;
                /* Pointer arithmetic with different strides */
                float *p1 = src1 + idx;
                float *p2 = src2 + offset + idx;
                float *p3 = dst + idx;
                
                /* SIMD-able computation with trigonometric functions */
                float angle = (*p1 + *p2) * pi / 180.0f;
                *p3 = (*p1 * *p2) + sinf(angle) * cosf(angle);
                
                /* Conditional store with runtime check */
                if (thread_specific % 3 == (i + j) % 3) {
                    *p3 += (float)(i * j) / (dim1 * dim2);
                }
            }
        }
    }
}

/* Variant 4: Host-only parallel region for comparison */
void host_only_parallel(int *arr, int size, int factor) {
    int local_sum = 0;
    #pragma omp parallel for reduction(+:local_sum) \
        if(factor % 2 == 0)
    for (int i = 0; i < size; ++i) {
        arr[i] = (arr[i] * factor) + (i % 8);
        local_sum += arr[i];
    }
    /* Use result to prevent elimination */
    arr[0] = local_sum % 1000;
}

/* Helper to initialize arrays with pseudo-random data */
void init_array(void *arr, size_t size, int seed) {
    unsigned char *ptr = (unsigned char *)arr;
    for (size_t i = 0; i < size; ++i) {
        ptr[i] = (unsigned char)((i * seed + i * 13) % 256);
    }
}

int main(int argc, char *argv[]) {
    /* Use command-line argument for variability */
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    /* Mixed-type arrays with different storage */
    static int array1[N * M];
    int array2[N * M];
    int array3[N * M];
    double darray1[N], darray2[N], darray3[N];
    float farray1[N * M / 2], farray2[N * M / 2], farray3[N * M / 2];
    
    /* Initialize with random patterns */
    init_array(array1, sizeof(array1), seed);
    init_array(array2, sizeof(array2), seed + 1);
    init_array(array3, sizeof(array3), seed + 2);
    init_array(darray1, sizeof(darray1), seed + 3);
    init_array(darray2, sizeof(darray2), seed + 4);
    init_array(darray3, sizeof(darray3), seed + 5);
    init_array(farray1, sizeof(farray1), seed + 6);
    init_array(farray2, sizeof(farray2), seed + 7);
    init_array(farray3, sizeof(farray3), seed + 8);
    
    /* Volatile loop bounds to prevent constant folding */
    volatile int v_start = rand() % 100;
    volatile int v_end = N - (rand() % 100);
    volatile int v_step = (rand() % 5) + 1;
    volatile int v_iter = (rand() % 10) + 1;
    
    printf("Starting OpenMP SIMT transformation test (seed=%d)\n", seed);
    
    /* Main test loop with varying parameters */
    for (int iter = 0; iter < MAX_ITER; ++iter) {
        int use_target = (iter + seed) % 3;  /* Variable target usage */
        int simd_mode = iter % 2;
        
        printf("Iteration %d: use_target=%d, simd_mode=%d\n", 
               iter, use_target, simd_mode);
        
        /* Call different variants based on runtime conditions */
        if (use_target == 0) {
            /* Force SIMT path with target region */
            simd_target_loop(array1, array2, array3, 
                           v_start, v_end, v_step, 
                           v_iter + iter, 1);
        } else if (use_target == 1) {
            /* Target without explicit SIMD clause */
            parallel_target_loop(darray1, darray2, darray3,
                               iter * 10, N - iter * 5,
                               (iter % 3) + 1,
                               v_iter);
        } else {
            /* Combined constructs */
            combined_constructs(farray1, farray2, farray3,
                              N / 4, M / 2,
                              iter * 8,
                              simd_mode);
        }
        
        /* Also call host-only version to create mixed context */
        host_only_parallel(array1, 100, iter);
        
        /* Compute checksums to prevent dead code elimination */
        int sum1 = 0, sum2 = 0;
        double sum3 = 0.0;
        float sum4 = 0.0f;
        
        #pragma omp parallel for reduction(+:sum1, sum2, sum3, sum4) \
            if(iter % 2 == 0)
        for (int i = 0; i < 100; ++i) {
            sum1 += array1[i];
            sum2 += array2[i];
            if (i < N) {
                sum3 += darray1[i];
                if (i < N * M / 2) {
                    sum4 += farray1[i];
                }
            }
        }
        
        printf("  Checksums: int=%d, int2=%d, double=%.2f, float=%.2f\n",
               sum1 % 1000, sum2 % 1000, sum3, sum4);
        
        /* Modify volatile bounds for next iteration */
        v_start = (v_start + 17) % 50;
        v_end = N - ((v_end + 23) % 50);
        v_step = (v_step % 4) + 1;
        v_iter = (v_iter + 5) % 15 + 1;
    }
    
    /* Final verification */
    int final_check = 0;
    #pragma omp parallel for reduction(+:final_check)
    for (int i = 0; i < N * M; ++i) {
        if (i % 100 == 0) {
            final_check += array1[i] + array2[i] + array3[i];
        }
    }
    
    printf("Final check: %d\n", final_check % 10000);
    return 0;
}
