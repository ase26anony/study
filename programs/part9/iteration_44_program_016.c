#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define M 512

/* Variant 1: SIMD target loop with complex data environment */
void simd_target_loop(int *a, int *b, int *c, int start, int end, int step, 
                      int n, int m, volatile int bound) {
    static int static_buffer[N];
    const int const_limit = 100;
    int auto_buffer[M];
    
    /* Initialize local arrays with mixed patterns */
    for (int i = 0; i < M; i++) {
        auto_buffer[i] = i % 10;
    }
    for (int i = 0; i < N; i++) {
        static_buffer[i] = i % 20;
    }
    
    /* Complex target region with SIMD clause */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[start:end:step], b[start:end:step], static_buffer[0:N]) \
        map(from: c[start:end:step]) \
        map(tofrom: auto_buffer[0:m]) \
        private(start) firstprivate(end) shared(step) \
        collapse(2) num_teams(bound % 8 + 1) thread_limit(64)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            if (idx >= start && idx < end && idx % step == 0) {
                /* Complex indexing with pointer arithmetic */
                int *ptr_a = a + idx;
                int *ptr_b = b + idx;
                int *ptr_c = c + idx;
                
                *ptr_c = *ptr_a + *ptr_b + static_buffer[idx % N] + 
                         auto_buffer[idx % M] + const_limit;
                
                /* Additional computation to prevent optimization */
                auto_buffer[idx % M] = (auto_buffer[idx % M] + 1) % 256;
            }
        }
    }
}

/* Variant 2: Target loop without explicit SIMD clause */
void parallel_target_loop(float *x, float *y, float *z, int low, int high, 
                         int stride, volatile int use_simd) {
    float local_buffer[256];
    const float scale = 2.5f;
    
    /* Initialize with trigonometric pattern */
    for (int i = 0; i < 256; i++) {
        local_buffer[i] = (float)(i * 3.14159f / 128.0f);
    }
    
    /* Conditional execution path */
    if (use_simd % 3 == 0) {
        #pragma omp target teams distribute parallel for \
            map(to: x[low:high:stride], y[low:high:stride], local_buffer) \
            map(from: z[low:high:stride]) \
            firstprivate(scale) private(low, high) \
            num_teams(use_simd % 4 + 2)
        for (int i = low; i < high; i += stride) {
            /* Complex computation with conditionals */
            if (i % 2 == 0) {
                z[i] = x[i] * scale + y[i] + local_buffer[i % 256];
            } else {
                z[i] = x[i] / scale - y[i] + local_buffer[i % 256];
            }
            
            /* Update local buffer */
            local_buffer[i % 256] = local_buffer[i % 256] * 0.99f;
        }
    } else {
        /* Host-only parallel region for comparison */
        #pragma omp parallel for private(scale)
        for (int i = low; i < high; i += stride) {
            z[i] = x[i] + y[i];
        }
    }
}

/* Variant 3: Combined constructs with nested data regions */
void combined_constructs(double *p, double *q, double *r, int size, 
                        int offset, volatile int mode) {
    double *dynamic_buf = (double*)malloc(size * sizeof(double));
    static double static_matrix[64][64];
    
    /* Initialize matrices */
    for (int i = 0; i < size; i++) {
        dynamic_buf[i] = (double)(i % 100) / 10.0;
    }
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            static_matrix[i][j] = (double)(i * j) / 4096.0;
        }
    }
    
    /* Combined target data and parallel region */
    #pragma omp target data map(to: p[offset:size], static_matrix) \
                            map(tofrom: q[offset:size]) \
                            map(from: r[offset:size])
    {
        /* Nested loops with collapse */
        #pragma omp target teams distribute parallel for simd \
            map(alloc: dynamic_buf[0:size]) \
            collapse(2) if(mode > 0)
        for (int i = 0; i < 64; i++) {
            for (int j = 0; j < 64; j++) {
                int idx = i * 64 + j;
                if (idx < size) {
                    double temp = p[offset + idx] * q[offset + idx];
                    r[offset + idx] = temp + static_matrix[i][j] + 
                                     dynamic_buf[idx] * (mode % 10);
                    
                    /* Update dynamic buffer */
                    dynamic_buf[idx] = dynamic_buf[idx] * 0.95 + 
                                      static_matrix[i][j] * 0.05;
                }
            }
        }
    }
    
    free(dynamic_buf);
}

/* Helper function to compute checksum */
long long compute_checksum(void *data, size_t size) {
    long long sum = 0;
    unsigned char *bytes = (unsigned char*)data;
    for (size_t i = 0; i < size; i++) {
        sum += bytes[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    /* Use command-line argument for random seed */
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    /* Volatile variables to prevent constant folding */
    volatile int v_size = N;
    volatile int v_offset = M;
    volatile int v_mode = rand() % 10;
    
    /* Allocate and initialize arrays with different types */
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    int *c = (int*)malloc(N * sizeof(int));
    
    float *x = (float*)malloc(N * sizeof(float));
    float *y = (float*)malloc(N * sizeof(float));
    float *z = (float*)malloc(N * sizeof(float));
    
    double *p = (double*)malloc(N * sizeof(double));
    double *q = (double*)malloc(N * sizeof(double));
    double *r = (double*)malloc(N * sizeof(double));
    
    /* Initialize with random patterns */
    for (int i = 0; i < N; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
        c[i] = 0;
        
        x[i] = (float)(rand() % 1000) / 10.0f;
        y[i] = (float)(rand() % 1000) / 10.0f;
        z[i] = 0.0f;
        
        p[i] = (double)(rand() % 1000) / 100.0;
        q[i] = (double)(rand() % 1000) / 100.0;
        r[i] = 0.0;
    }
    
    /* Execute multiple iterations with varying parameters */
    for (int iter = 0; iter < 5; iter++) {
        printf("Iteration %d:\n", iter);
        
        /* Vary parameters each iteration */
        int start = rand() % 100;
        int end = N - rand() % 100;
        int step = (rand() % 5) + 1;
        int n = (rand() % 64) + 32;
        int m = (rand() % 16) + 8;
        volatile int bound = rand() % 100 + 50;
        
        /* Call variant functions */
        simd_target_loop(a, b, c, start, end, step, n, m, bound);
        
        long long checksum1 = compute_checksum(c + start, 
                                             (end - start) * sizeof(int));
        printf("  SIMD target checksum: %lld\n", checksum1);
        
        int low = rand() % 200;
        int high = N - rand() % 200;
        int stride = (rand() % 3) + 1;
        volatile int use_simd = rand() % 6;
        
        parallel_target_loop(x, y, z, low, high, stride, use_simd);
        
        long long checksum2 = compute_checksum(z + low, 
                                             (high - low) * sizeof(float));
        printf("  Parallel target checksum: %lld\n", checksum2);
        
        int size = v_size - iter * 100;
        int offset = v_offset + iter * 50;
        v_mode = (v_mode + iter) % 10;
        
        combined_constructs(p, q, r, size, offset, v_mode);
        
        long long checksum3 = compute_checksum(r + offset, 
                                             size * sizeof(double));
        printf("  Combined constructs checksum: %lld\n", checksum3);
        
        printf("\n");
    }
    
    /* Cleanup */
    free(a); free(b); free(c);
    free(x); free(y); free(z);
    free(p); free(q); free(r);
    
    return 0;
}
