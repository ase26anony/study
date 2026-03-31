#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define M 512
#define MAX_ITER 5

/* Variant 1: SIMD target loop */
void simd_target_loop(int *a, int *b, int *c, int start, int end, int step, 
                      float *fa, float *fb, volatile int n) {
    #pragma omp target teams distribute parallel for simd \
        map(to: a[start:end], b[start:end], fa[0:n], fb[0:n]) \
        map(from: c[start:end]) \
        private(start) firstprivate(end, step) shared(a, b, c, fa, fb) \
        collapse(2) num_teams(32) thread_limit(128)
    for (int i = start; i < end; i += step) {
        for (int j = 0; j < n; j++) {
            int idx = i * n + j;
            if (idx < N * M) {
                c[idx] = a[idx] + b[idx] * (i % 8);
                fa[idx] = fb[idx] * 2.0f + (float)(j % 4);
            }
        }
    }
}

/* Variant 2: Parallel target loop without SIMD */
void parallel_target_loop(double *da, double *db, double *dc, 
                          int low, int high, int stride,
                          volatile int limit) {
    static int counter = 0;
    const int chunk_size = 16;
    
    #pragma omp target data map(to: da[low:high:stride], db[low:high:stride]) \
                            map(from: dc[low:high:stride])
    {
        #pragma omp target teams distribute parallel for \
            num_teams(64) thread_limit(256) collapse(1) \
            firstprivate(low, high, stride, chunk_size, counter)
        for (int i = low; i < high && i < limit; i += stride) {
            int local_idx = i - low;
            dc[i] = da[i] * db[i] + (double)(counter + local_idx % 8);
            for (int k = 0; k < chunk_size; k++) {
                if (k < local_idx % 16) {
                    dc[i] += 0.5 * (da[i - k] + db[i + k]);
                }
            }
        }
    }
}

/* Variant 3: Combined constructs with complex data environment */
void combined_constructs(int *arr1, int *arr2, int *arr3,
                         float *farr1, float *farr2,
                         int dim1, int dim2, volatile int iter) {
    int *local_ptr = arr1;
    const int offset = 32;
    static int persistent = 0;
    
    #pragma omp target data map(to: arr1[0:dim1*dim2], arr2[0:dim1*dim2], \
                                      farr1[0:dim1*dim2], farr2[0:dim1*dim2]) \
                            map(tofrom: arr3[0:dim1*dim2])
    {
        #pragma omp target teams distribute parallel for simd \
            private(local_ptr) firstprivate(offset, persistent, iter) \
            shared(arr1, arr2, arr3, farr1, farr2) \
            collapse(2) num_teams(16) thread_limit(64)
        for (int i = 0; i < dim1; i++) {
            for (int j = 0; j < dim2; j++) {
                int idx = i * dim2 + j;
                int temp = (i + iter) % 8;
                
                /* Complex indexing with pointer arithmetic */
                local_ptr = arr1 + idx;
                int val1 = *local_ptr;
                int val2 = arr2[idx + (j % offset)];
                
                /* Conditional computation */
                if (temp > 3) {
                    arr3[idx] = val1 * val2 + (int)(farr1[idx] * 100.0f);
                } else {
                    arr3[idx] = val1 + val2 - (int)(farr2[idx] * 50.0f);
                }
                
                /* Update floating point arrays */
                farr1[idx] = farr2[idx] * (float)(temp + 1) + (float)persistent;
                farr2[idx] = farr1[idx] / (float)((idx % 16) + 1);
            }
        }
        persistent++;
    }
}

/* Host-only parallel region for conditional execution */
void host_only_parallel(int *data, int size, volatile int factor) {
    #pragma omp parallel for simd schedule(dynamic, 8) \
        firstprivate(factor) shared(data)
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * factor + (i % 64);
    }
}

int main(int argc, char *argv[]) {
    /* Initialize with random seed from command line */
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    /* Declare arrays with mixed storage durations */
    static int static_array[N * M];
    int auto_array[N * M];
    int *heap_array = (int *)malloc(N * M * sizeof(int));
    
    float float_array1[N * M];
    float float_array2[N * M];
    static float static_float_array[N * M];
    
    double double_array1[N * M];
    double double_array2[N * M];
    double double_array3[N * M];
    
    /* Initialize arrays with random/sequential data */
    for (int i = 0; i < N * M; i++) {
        static_array[i] = rand() % 1000;
        auto_array[i] = i % 256;
        heap_array[i] = rand() % 500;
        
        float_array1[i] = (float)(rand() % 100) / 10.0f;
        float_array2[i] = (float)(rand() % 100) / 5.0f;
        static_float_array[i] = (float)(i % 50);
        
        double_array1[i] = (double)(rand() % 1000) / 100.0;
        double_array2[i] = (double)(rand() % 1000) / 50.0;
        double_array3[i] = 0.0;
    }
    
    /* Volatile variables to prevent constant folding */
    volatile int v_bound1 = (rand() % 512) + 256;
    volatile int v_bound2 = (rand() % 256) + 128;
    volatile int v_step = (rand() % 8) + 1;
    volatile int v_limit = (rand() % 1000) + 500;
    
    int checksum = 0;
    
    /* Main loop with varying parameters */
    for (int iter = 0; iter < MAX_ITER; iter++) {
        printf("Iteration %d:\n", iter);
        
        /* Random condition to select execution path */
        int use_target = (rand() % 3) > 0;
        
        if (use_target) {
            /* Call target region variants with different parameters */
            int start = iter * 64;
            int end = start + v_bound1;
            int step = v_step + (iter % 4);
            
            printf("  Executing SIMD target loop...\n");
            simd_target_loop(static_array, auto_array, heap_array,
                            start, end, step, float_array1, float_array2, v_bound2);
            
            /* Update checksum */
            for (int i = start; i < end && i < N*M; i += step) {
                checksum += heap_array[i] % 256;
            }
            
            printf("  Executing parallel target loop...\n");
            int low = iter * 32;
            int high = low + v_bound2;
            int stride = (iter % 3) + 1;
            
            parallel_target_loop(double_array1, double_array2, double_array3,
                                low, high, stride, v_limit);
            
            /* Update checksum */
            for (int i = low; i < high && i < N*M; i += stride) {
                checksum += (int)(double_array3[i] * 10) % 256;
            }
            
            printf("  Executing combined constructs...\n");
            int dim1 = 64 + (iter * 16);
            int dim2 = 32 + (iter * 8);
            
            combined_constructs(static_array, auto_array, heap_array,
                              float_array1, static_float_array,
                              dim1, dim2, iter);
            
            /* Update checksum */
            for (int i = 0; i < dim1 * dim2 && i < N*M; i++) {
                checksum += heap_array[i] % 256;
            }
        } else {
            /* Host-only execution path */
            printf("  Executing host-only parallel...\n");
            int size = v_bound1 + (iter * 64);
            int factor = (iter % 7) + 2;
            
            host_only_parallel(heap_array, size < N*M ? size : N*M, factor);
            
            /* Update checksum */
            for (int i = 0; i < size && i < N*M; i++) {
                checksum += heap_array[i] % 256;
            }
        }
        
        /* Array section operations */
        int section_start = iter * 128;
        int section_len = 256;
        
        #pragma omp target teams distribute parallel for simd \
            map(to: static_array[section_start:section_len]) \
            map(from: auto_array[section_start:section_len])
        for (int i = section_start; 
             i < section_start + section_len && i < N*M; i++) {
            auto_array[i] = static_array[i] * 2 + (i % 32);
        }
        
        /* Update checksum */
        for (int i = section_start; 
             i < section_start + section_len && i < N*M; i++) {
            checksum += auto_array[i] % 256;
        }
        
        printf("  Checksum after iteration %d: %d\n", iter, checksum);
    }
    
    /* Final verification */
    printf("\nFinal checksum: %d\n", checksum);
    
    /* Cleanup */
    free(heap_array);
    
    return 0;
}
