#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define M 512
#define MAX_ITER 5

/* Variant 1: SIMD target loop with complex data environment */
void simd_target_loop(int *a, int *b, int *c, int start, int end, 
                      int stride, volatile int n, volatile int m) {
    static int static_counter = 0;
    const int chunk_size = 64;
    int private_var = static_counter++;
    
    #pragma omp target teams distribute parallel for simd \
        collapse(2) map(to: a[start:end:stride], b[start:end:stride]) \
        map(from: c[start:end:stride]) firstprivate(private_var) \
        num_teams(m/64) thread_limit(256)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            if (idx >= start && idx < end && (idx - start) % stride == 0) {
                int local_idx = (idx - start) / stride;
                c[local_idx] = a[local_idx] * private_var + b[local_idx] * (i + j);
            }
        }
    }
}

/* Variant 2: Parallel target loop without SIMD clause */
void parallel_target_loop(float *x, float *y, float *z, 
                         int low, int high, volatile int limit) {
    float *ptr_x = x + low;
    float *ptr_y = y + low;
    float *ptr_z = z + low;
    int range = high - low;
    
    #pragma omp target teams distribute parallel for \
        map(to: ptr_x[0:range], ptr_y[0:range]) \
        map(from: ptr_z[0:range]) shared(limit) \
        num_teams(range/128) thread_limit(128)
    for (int i = 0; i < range; i++) {
        float temp = ptr_x[i] * ptr_y[i];
        for (int k = 0; k < (i % 8); k++) {
            temp += 0.1f * k;
        }
        ptr_z[i] = temp * limit;
    }
}

/* Variant 3: Combined constructs with data region */
void combined_constructs(double *d1, double *d2, double *d3,
                        int offset, int length, volatile int scale) {
    const double pi = 3.141592653589793;
    double *section1 = d1 + offset;
    double *section2 = d2 + offset;
    double *section3 = d3 + offset;
    
    #pragma omp target data map(to: section1[0:length], section2[0:length]) \
                            map(from: section3[0:length])
    {
        int block_size = length / 4;
        for (int block = 0; block < 4; block++) {
            int block_start = block * block_size;
            int block_end = (block == 3) ? length : (block + 1) * block_size;
            
            #pragma omp target teams distribute parallel for simd \
                map(always, to: section1[block_start:block_end-block_start], \
                           section2[block_start:block_end-block_start]) \
                map(always, from: section3[block_start:block_end-block_start]) \
                firstprivate(pi, scale) private(block_start, block_end) \
                num_teams(block_size/32) thread_limit(64)
            for (int i = block_start; i < block_end; i++) {
                double angle = pi * i / length;
                section3[i] = section1[i] * cos(angle) + 
                             section2[i] * sin(angle) * scale;
            }
        }
    }
}

/* Host-only parallel region for conditional execution */
void host_only_parallel(int *arr, int size, volatile int factor) {
    #pragma omp parallel for simd schedule(static, 16)
    for (int i = 0; i < size; i++) {
        arr[i] = (arr[i] * factor) % 1000;
    }
}

/* Function that conditionally calls target or host version */
void conditional_computation(int *data, int size, int use_target, 
                            volatile int param) {
    if (use_target) {
        int *temp1 = (int*)malloc(size * sizeof(int));
        int *temp2 = (int*)malloc(size * sizeof(int));
        
        #pragma omp target teams distribute parallel for simd \
            map(to: data[0:size]) map(from: temp1[0:size], temp2[0:size]) \
            num_teams(size/256) thread_limit(128)
        for (int i = 0; i < size; i++) {
            temp1[i] = data[i] * param;
            temp2[i] = data[i] + param;
        }
        
        #pragma omp target teams distribute parallel for simd \
            map(to: temp1[0:size], temp2[0:size]) map(from: data[0:size])
        for (int i = 0; i < size; i++) {
            data[i] = temp1[i] + temp2[i] * (i % 16);
        }
        
        free(temp1);
        free(temp2);
    } else {
        host_only_parallel(data, size, param);
    }
}

int main(int argc, char *argv[]) {
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    /* Declare arrays with different storage durations */
    static int static_array[N];
    int auto_array[N];
    const int const_size = M;
    volatile int vol_bound = N / 2;
    
    float float_array[N];
    double double_array[N];
    
    /* Initialize arrays with random data */
    for (int i = 0; i < N; i++) {
        static_array[i] = rand() % 100;
        auto_array[i] = rand() % 100;
        float_array[i] = (float)(rand() % 100) / 10.0f;
        double_array[i] = (double)(rand() % 100) / 10.0;
    }
    
    int checksum = 0;
    
    /* Main loop with varying parameters */
    for (int iter = 0; iter < MAX_ITER; iter++) {
        volatile int dynamic_bound = (rand() % (N/2)) + N/4;
        volatile int stride = (rand() % 5) + 1;
        int use_target = (rand() % 2);
        int offset = rand() % (N/4);
        int length = (rand() % (N/2)) + N/4;
        
        /* Call variant functions with different parameters */
        if (iter % 3 == 0) {
            int *temp_c = (int*)malloc(N * sizeof(int));
            simd_target_loop(static_array, auto_array, temp_c, 
                           offset, offset + length, stride, 
                           dynamic_bound, const_size);
            
            for (int i = 0; i < length; i += stride) {
                checksum += temp_c[i/stride];
            }
            free(temp_c);
        }
        else if (iter % 3 == 1) {
            float *temp_z = (float*)malloc(N * sizeof(float));
            parallel_target_loop(float_array, float_array + N/2, temp_z,
                               offset, offset + length, dynamic_bound);
            
            for (int i = 0; i < length; i++) {
                checksum += (int)temp_z[i];
            }
            free(temp_z);
        }
        else {
            double *temp_d3 = (double*)malloc(N * sizeof(double));
            combined_constructs(double_array, double_array + N/2, temp_d3,
                              offset, length, dynamic_bound);
            
            for (int i = 0; i < length; i++) {
                checksum += (int)temp_d3[i];
            }
            free(temp_d3);
        }
        
        /* Conditional computation that may use target or host */
        conditional_computation(auto_array, dynamic_bound, use_target, iter);
        
        for (int i = 0; i < dynamic_bound && i < N; i++) {
            checksum += auto_array[i];
        }
        
        /* Nested loops with pointer arithmetic */
        int *ptr = static_array;
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: ptr[0:vol_bound]) firstprivate(iter) \
            num_teams(vol_bound/64) thread_limit(256)
        for (int i = 0; i < vol_bound; i++) {
            ptr[i] = ptr[i] * (i % 32) + iter;
        }
        
        for (int i = 0; i < vol_bound; i++) {
            checksum += ptr[i];
        }
    }
    
    printf("Final checksum: %d (seed: %d)\n", checksum, seed);
    
    /* Additional test with array sections */
    int section_start = rand() % (N/2);
    int section_len = (rand() % (N/4)) + N/8;
    int section_stride = (rand() % 3) + 1;
    
    #pragma omp target teams distribute parallel for simd \
        map(to: static_array[section_start:section_len:section_stride]) \
        map(from: auto_array[section_start:section_len:section_stride]) \
        num_teams(section_len/32) thread_limit(128)
    for (int i = 0; i < section_len; i += section_stride) {
        int idx = section_start + i;
        auto_array[idx] = static_array[idx] * 2 + i;
    }
    
    /* Verify results */
    int final_check = 0;
    for (int i = 0; i < section_len; i += section_stride) {
        int idx = section_start + i;
        final_check += auto_array[idx];
    }
    printf("Section checksum: %d\n", final_check);
    
    return 0;
}
