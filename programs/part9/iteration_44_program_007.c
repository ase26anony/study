#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define M 512
#define MAX_ITER 5

static int static_array[N];
const int const_array[N] = {[0 ... N-1] = 1};
volatile int volatile_bound = N;

/* Function 1: SIMD target loop with collapse */
void simd_target_loop(int *a, int *b, int *c, int start, int end, int step) {
    volatile int v_start = start;
    volatile int v_end = end;
    
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(to: a[start:end], b[start:end]) map(from: c[start:end]) \
        private(v_start, v_end) firstprivate(step)
    for (int i = v_start; i < v_end; i += step) {
        for (int j = 0; j < M; j++) {
            int idx = i * M + j;
            if (idx < N * M) {
                c[idx] = a[idx] + b[idx] * (i % 8 + 1);
            }
        }
    }
}

/* Function 2: Parallel target loop without SIMD clause */
void parallel_target_loop(float *x, float *y, float alpha, int low, int high) {
    float local_alpha = alpha;
    int stride = 2;
    
    #pragma omp target teams distribute parallel for \
        map(to: x[low:high:stride]) map(tofrom: y[low:high:stride]) \
        firstprivate(local_alpha, stride)
    for (int i = low; i < high; i += stride) {
        y[i] = x[i] * local_alpha + (i % 16) * 0.5f;
        /* Complex indexing to prevent optimization */
        if (i > 0 && i < high - 1) {
            y[i] += x[i-1] * 0.3f + x[i+1] * 0.7f;
        }
    }
}

/* Function 3: Combined constructs with data region */
void combined_constructs(double *p, double *q, int size, int offset) {
    double *temp = (double*)malloc(size * sizeof(double));
    static double static_temp[N];
    
    #pragma omp target data map(to: p[offset:size]) map(from: q[offset:size]) \
        map(alloc: temp[0:size])
    {
        #pragma omp target teams distribute parallel for simd \
            firstprivate(offset, size) private(temp)
        for (int i = 0; i < size; i++) {
            int idx = (i + offset) % N;
            temp[i] = p[idx] * 2.0;
            q[idx] = temp[i] + static_temp[idx] + (idx % 32) * 0.25;
        }
    }
    
    free(temp);
}

/* Function 4: Host-only parallel region (alternative path) */
void host_only_parallel(int *arr, int n, int factor) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * factor + (i % 64);
    }
}

/* Function 5: Nested target with conditional execution */
void nested_target_conditional(int *data, int dim1, int dim2, int use_simt) {
    volatile int use_simt_flag = use_simt;
    
    if (use_simt_flag > 0) {
        #pragma omp target teams distribute parallel for simd collapse(2) \
            map(tofrom: data[0:dim1*dim2])
        for (int i = 0; i < dim1; i++) {
            for (int j = 0; j < dim2; j++) {
                int idx = i * dim2 + j;
                data[idx] = (data[idx] + i * j) % 256;
            }
        }
    } else {
        #pragma omp target teams distribute parallel for \
            map(tofrom: data[0:dim1*dim2])
        for (int i = 0; i < dim1 * dim2; i++) {
            data[i] = (data[i] + (i / dim2) * (i % dim2)) % 256;
        }
    }
}

int main(int argc, char *argv[]) {
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    printf("Seed: %d\n", seed);
    
    /* Initialize arrays with different storage durations */
    int *dynamic_a = (int*)malloc(N * M * sizeof(int));
    int *dynamic_b = (int*)malloc(N * M * sizeof(int));
    int *dynamic_c = (int*)malloc(N * M * sizeof(int));
    
    float *float_x = (float*)malloc(N * sizeof(float));
    float *float_y = (float*)malloc(N * sizeof(float));
    
    double *double_p = (double*)malloc(N * sizeof(double));
    double *double_q = (double*)malloc(N * sizeof(double));
    
    /* Initialize with random data */
    for (int i = 0; i < N * M; i++) {
        dynamic_a[i] = rand() % 100;
        dynamic_b[i] = rand() % 100;
        dynamic_c[i] = 0;
    }
    
    for (int i = 0; i < N; i++) {
        float_x[i] = (float)(rand() % 1000) / 10.0f;
        float_y[i] = 0.0f;
        double_p[i] = (double)(rand() % 1000) / 10.0;
        double_q[i] = 0.0;
        static_array[i] = i % 50;
    }
    
    /* Main loop with varying parameters */
    for (int iter = 0; iter < MAX_ITER; iter++) {
        volatile int mode = rand() % 3;
        volatile int bound_var = N / (iter + 1) + 10;
        
        printf("\nIteration %d, Mode %d:\n", iter, mode);
        
        /* Vary which functions are called based on random input */
        if (mode == 0) {
            /* Call SIMD target function with complex bounds */
            int start = rand() % 10;
            int end = bound_var + start;
            int step = (rand() % 3) + 1;
            
            simd_target_loop(dynamic_a, dynamic_b, dynamic_c, start, end, step);
            
            /* Verify results */
            long checksum = 0;
            for (int i = start * M; i < end * M && i < N * M; i++) {
                checksum += dynamic_c[i];
            }
            printf("  SIMD target checksum: %ld\n", checksum);
            
        } else if (mode == 1) {
            /* Call parallel target function */
            int low = rand() % 20;
            int high = bound_var + low;
            float alpha = (rand() % 100) / 10.0f;
            
            parallel_target_loop(float_x, float_y, alpha, low, high);
            
            /* Verify results */
            float sum = 0.0f;
            for (int i = low; i < high; i += 2) {
                sum += float_y[i];
            }
            printf("  Parallel target sum: %.2f\n", sum);
            
        } else {
            /* Call combined constructs */
            int size = bound_var;
            int offset = rand() % 50;
            
            combined_constructs(double_p, double_q, size, offset);
            
            /* Verify results */
            double total = 0.0;
            for (int i = offset; i < offset + size && i < N; i++) {
                total += double_q[i];
            }
            printf("  Combined constructs total: %.2f\n", total);
        }
        
        /* Sometimes call host-only function to create alternative paths */
        if (rand() % 2 == 0) {
            int factor = (rand() % 5) + 1;
            host_only_parallel(static_array, N / 2, factor);
            
            int host_sum = 0;
            for (int i = 0; i < N / 2; i++) {
                host_sum += static_array[i];
            }
            printf("  Host-only sum: %d\n", host_sum);
        }
        
        /* Call nested conditional function */
        int use_simt = rand() % 2;
        int dim1 = 16 + (rand() % 16);
        int dim2 = 16 + (rand() % 16);
        int *temp_data = (int*)malloc(dim1 * dim2 * sizeof(int));
        
        for (int i = 0; i < dim1 * dim2; i++) {
            temp_data[i] = rand() % 100;
        }
        
        nested_target_conditional(temp_data, dim1, dim2, use_simt);
        
        int cond_sum = 0;
        for (int i = 0; i < dim1 * dim2; i++) {
            cond_sum += temp_data[i];
        }
        printf("  Conditional SIMT sum: %d (use_simt=%d)\n", cond_sum, use_simt);
        
        free(temp_data);
    }
    
    /* Cleanup */
    free(dynamic_a);
    free(dynamic_b);
    free(dynamic_c);
    free(float_x);
    free(float_y);
    free(double_p);
    free(double_q);
    
    return 0;
}
