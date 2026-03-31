#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define M 512
#define MAX_ITER 5

/* Variant 1: SIMD target loop with complex data environment */
void simd_target_loop(int *a, int *b, int *c, int start, int end, int stride, 
                      volatile int n, volatile int m) {
    static int static_counter = 0;
    const int chunk_size = 32;
    int private_var = start;
    
    #pragma omp target teams distribute parallel for simd \
        collapse(2) map(to: a[start:end:stride], b[start:end:stride]) \
        map(from: c[start:end:stride]) firstprivate(private_var) \
        num_teams(n/64) thread_limit(128)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            if (idx >= start && idx < end && (idx - start) % stride == 0) {
                int local_idx = (idx - start) / stride;
                c[local_idx] = a[local_idx] * private_var + b[local_idx] + 
                              static_counter + omp_get_team_num();
            }
        }
    }
    static_counter++;
}

/* Variant 2: Parallel target loop without SIMD clause */
void parallel_target_loop(float *x, float *y, float *z, int offset, 
                         volatile int rows, volatile int cols) {
    float firstprivate_val = (float)offset;
    int *shared_indices = (int*)malloc(rows * sizeof(int));
    
    #pragma omp target data map(to: x[0:rows*cols], shared_indices[0:rows]) \
                            map(tofrom: y[0:rows*cols]) \
                            map(from: z[offset:rows*cols-offset])
    {
        // Initialize shared indices on host
        for (int i = 0; i < rows; i++) {
            shared_indices[i] = i * cols;
        }
        
        #pragma omp target update to(shared_indices[0:rows])
        
        #pragma omp target teams distribute parallel for \
            num_teams(rows/16) thread_limit(64) \
            private(firstprivate_val) shared(shared_indices)
        for (int i = 0; i < rows; i++) {
            int base = shared_indices[i];
            float scale = firstprivate_val * omp_get_thread_num();
            for (int j = 0; j < cols; j++) {
                int idx = base + j;
                if (idx + offset < rows * cols) {
                    z[idx + offset] = x[idx] * scale + y[idx] / (scale + 1.0f);
                }
            }
        }
    }
    free(shared_indices);
}

/* Variant 3: Combined constructs with pointer arithmetic */
void combined_constructs(double *src, double *dst, int dim1, int dim2, 
                        volatile int iter, volatile int seed) {
    const int tile_size = 16;
    double *ptr_array[2] = {src, dst};
    int mod = iter % 2;
    
    #pragma omp target data map(to: src[0:dim1*dim2]) \
                            map(tofrom: dst[0:dim1*dim2])
    {
        #pragma omp target teams distribute parallel for simd \
            collapse(2) num_teams(dim1/tile_size) thread_limit(256) \
            firstprivate(ptr_array, mod, seed)
        for (int i = 0; i < dim1; i += tile_size) {
            for (int j = 0; j < dim2; j += tile_size) {
                double local_sum = 0.0;
                for (int ti = 0; ti < tile_size && (i + ti) < dim1; ti++) {
                    for (int tj = 0; tj < tile_size && (j + tj) < dim2; tj++) {
                        int idx = (i + ti) * dim2 + (j + tj);
                        double *current = ptr_array[mod];
                        double *other = ptr_array[(mod + 1) % 2];
                        
                        // Complex indexing with runtime-dependent condition
                        if ((idx + seed) % 3 == 0) {
                            dst[idx] = current[idx] * 2.5 - other[idx] / 1.7;
                        } else if ((idx + seed) % 5 == 0) {
                            dst[idx] = current[idx] + other[idx] * 
                                      (1.0 + (double)omp_get_thread_num() / 100.0);
                        } else {
                            dst[idx] = current[idx] * 0.8 + other[idx] * 0.2;
                        }
                        local_sum += dst[idx];
                    }
                }
                // Write reduction-like result
                if (omp_get_thread_num() == 0) {
                    int tile_idx = (i/tile_size) * (dim2/tile_size) + (j/tile_size);
                    if (tile_idx < dim1 * dim2 / (tile_size * tile_size)) {
                        dst[tile_idx] = local_sum;
                    }
                }
            }
        }
    }
}

/* Host-only parallel region for conditional execution */
void host_only_parallel(int *arr, int size, volatile int factor) {
    int local_factor = factor;
    #pragma omp parallel for simd schedule(dynamic, 8) \
        firstprivate(local_factor)
    for (int i = 0; i < size; i++) {
        arr[i] = arr[i] * local_factor + omp_get_thread_num();
    }
}

int main(int argc, char *argv[]) {
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    // Declare arrays with different storage durations
    static int static_array[N];
    int auto_array[N];
    const int const_size = N;
    volatile int vol_bound = M;
    
    float float_array[N * M];
    double double_array[N * M];
    
    // Initialize with random data
    for (int i = 0; i < N; i++) {
        static_array[i] = rand() % 100;
        auto_array[i] = rand() % 100;
    }
    
    for (int i = 0; i < N * M; i++) {
        float_array[i] = (float)(rand() % 1000) / 10.0f;
        double_array[i] = (double)(rand() % 1000) / 10.0;
    }
    
    int checksum = 0;
    
    // Loop with varying parameters to expose different transformation contexts
    for (int iter = 0; iter < MAX_ITER; iter++) {
        volatile int dynamic_bound = (rand() % (N/2)) + N/4;
        volatile int stride = (rand() % 5) + 1;
        int offset = rand() % 100;
        
        // Runtime condition to choose between target and host execution
        if ((seed + iter) % 3 == 0) {
            printf("Iteration %d: Using SIMD target loop\n", iter);
            int *temp_result = (int*)malloc(N * sizeof(int));
            memcpy(temp_result, auto_array, N * sizeof(int));
            
            simd_target_loop(static_array, auto_array, temp_result, 
                           offset, dynamic_bound, stride, 
                           vol_bound, dynamic_bound/2);
            
            // Compute checksum
            for (int i = 0; i < dynamic_bound; i += stride) {
                checksum += temp_result[i/stride];
            }
            free(temp_result);
        } 
        else if ((seed + iter) % 3 == 1) {
            printf("Iteration %d: Using parallel target loop\n", iter);
            float *temp_float = (float*)malloc(N * M * sizeof(float));
            memcpy(temp_float, float_array, N * M * sizeof(float));
            
            parallel_target_loop(float_array, float_array + N*M/2, temp_float,
                               offset, dynamic_bound, dynamic_bound/2);
            
            // Compute checksum from float array (convert to int)
            for (int i = 0; i < dynamic_bound * dynamic_bound/2; i++) {
                checksum += (int)temp_float[i + offset];
            }
            free(temp_float);
        }
        else {
            printf("Iteration %d: Using combined constructs\n", iter);
            double *temp_double = (double*)malloc(N * M * sizeof(double));
            memcpy(temp_double, double_array, N * M * sizeof(double));
            
            combined_constructs(double_array, temp_double, 
                              dynamic_bound, dynamic_bound/2, 
                              iter, seed);
            
            // Compute checksum from double array
            for (int i = 0; i < dynamic_bound * dynamic_bound/2; i++) {
                checksum += (int)temp_double[i];
            }
            free(temp_double);
        }
        
        // Also call host-only function sometimes
        if ((seed + iter) % 4 == 0) {
            host_only_parallel(auto_array, dynamic_bound, iter + 1);
            for (int i = 0; i < dynamic_bound; i++) {
                checksum += auto_array[i];
            }
        }
        
        // Modify volatile bounds for next iteration
        vol_bound = (vol_bound + 17) % (N/2) + N/4;
    }
    
    printf("Final checksum: %d\n", checksum);
    
    // Additional test with array sections and complex map clauses
    {
        int section_array[1000];
        for (int i = 0; i < 1000; i++) {
            section_array[i] = i;
        }
        
        #pragma omp target teams distribute parallel for simd \
            map(to: section_array[100:400:2]) \
            map(from: section_array[500:400:3]) \
            num_teams(8) thread_limit(64)
        for (int i = 0; i < 200; i++) {
            int src_idx = 100 + i * 2;
            int dst_idx = 500 + i * 3;
            if (dst_idx < 900) {
                section_array[dst_idx] = section_array[src_idx] * 
                                        omp_get_team_num() + 
                                        omp_get_thread_num();
            }
        }
        
        // Verify some results
        int verify_sum = 0;
        for (int i = 500; i < 900; i += 3) {
            verify_sum += section_array[i];
        }
        printf("Section array verify sum: %d\n", verify_sum);
    }
    
    return 0;
}
