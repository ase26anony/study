#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define M 512
#define MAX_ITER 5

// Variant 1: SIMD target loop
void simd_target_loop(int *a, int *b, int *c, int start, int end, int step, 
                      volatile int n, volatile int m) {
    #pragma omp target teams distribute parallel for simd \
                map(to: a[start:end-start+1], b[start:end-start+1]) \
                map(from: c[start:end-start+1]) \
                collapse(2) \
                private(n, m) \
                firstprivate(start, end, step)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            if (idx >= start && idx <= end && idx % step == 0) {
                c[idx] = a[idx] * 2 + b[idx] / 3;
            }
        }
    }
}

// Variant 2: Parallel target loop without SIMD
void parallel_target_loop(float *x, float *y, float *z, 
                          int low, int high, int stride,
                          volatile int rows, volatile int cols) {
    static const float scale = 1.5f;
    
    #pragma omp target data map(to: x[low:high-low+1:stride]) \
                            map(tofrom: y[low:high-low+1:stride]) \
                            map(from: z[low:high-low+1:stride])
    {
        #pragma omp target teams distribute parallel for \
                    collapse(2) \
                    shared(scale) \
                    firstprivate(low, high, stride, rows, cols)
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                int idx = i * cols + j;
                if (idx >= low && idx <= high && (idx - low) % stride == 0) {
                    z[idx] = x[idx] * scale + y[idx] * (1.0f - scale);
                    y[idx] = z[idx] * 0.5f;
                }
            }
        }
    }
}

// Variant 3: Combined constructs with complex data environment
void combined_constructs(double *p, double *q, double *r,
                         int *mask, int offset, int range,
                         volatile int dim1, volatile int dim2) {
    int local_offset = offset;
    const double pi = 3.141592653589793;
    double *temp = (double*)malloc(range * sizeof(double));
    
    // Initialize temp with some values
    for (int i = 0; i < range; i++) {
        temp[i] = (double)i / range;
    }
    
    #pragma omp target enter data map(to: temp[0:range])
    
    #pragma omp target teams distribute parallel for simd \
                map(to: p[offset:range], mask[offset:range]) \
                map(tofrom: q[offset:range]) \
                map(from: r[offset:range]) \
                firstprivate(local_offset, pi, dim1, dim2) \
                private(temp)
    for (int i = 0; i < dim1; i++) {
        for (int j = 0; j < dim2; j++) {
            int idx = i * dim2 + j;
            if (idx >= offset && idx < offset + range && mask[idx]) {
                double tval = temp[idx - offset];
                r[idx] = p[idx] * sin(pi * tval) + q[idx] * cos(pi * tval);
                q[idx] = r[idx] * tval;
            }
        }
    }
    
    #pragma omp target exit data map(delete: temp[0:range])
    
    free(temp);
}

// Host-only parallel region for conditional execution
void host_only_parallel(int *arr, int size, int factor) {
    #pragma omp parallel for simd
    for (int i = 0; i < size; i++) {
        arr[i] = arr[i] * factor + i;
    }
}

int main(int argc, char *argv[]) {
    // Use command-line argument for random seed
    unsigned int seed = (unsigned int)time(NULL);
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    // Declare arrays with different storage durations
    static int static_array[N * M];
    int auto_array[N * M];
    const int const_size = N * M;
    volatile int vol_bound = N;
    
    float float_array[N * M];
    double double_array[N * M];
    int mask_array[N * M];
    
    // Initialize arrays with random data
    for (int i = 0; i < N * M; i++) {
        static_array[i] = rand() % 100;
        auto_array[i] = rand() % 100;
        float_array[i] = (float)rand() / RAND_MAX * 100.0f;
        double_array[i] = (double)rand() / RAND_MAX * 100.0;
        mask_array[i] = rand() % 2;
    }
    
    // Main loop with varying parameters
    for (int iter = 0; iter < MAX_ITER; iter++) {
        // Vary parameters using random values and iteration index
        int start = rand() % (N * M / 2);
        int end = start + 100 + rand() % 200;
        int step = 1 + rand() % 3;
        
        int low = rand() % (N * M / 4);
        int high = low + 200 + rand() % 300;
        int stride = 1 + rand() % 4;
        
        int offset = rand() % (N * M / 3);
        int range = 150 + rand() % 250;
        
        // Use volatile variables for loop bounds
        volatile int n_val = N - iter * 10;
        volatile int m_val = M + iter * 5;
        volatile int rows_val = N / (iter + 1);
        volatile int cols_val = M * (iter + 1) / 2;
        volatile int dim1_val = N - iter * 20;
        volatile int dim2_val = M + iter * 10;
        
        // Conditional execution based on random value
        int choice = rand() % 3;
        
        if (choice == 0) {
            // Call SIMD target variant
            printf("Iteration %d: Calling simd_target_loop\n", iter);
            simd_target_loop(static_array, auto_array, static_array,
                           start, end, step, n_val, m_val);
            
            // Verify results with checksum
            long long checksum = 0;
            for (int i = start; i <= end && i < N * M; i += step) {
                checksum += static_array[i];
            }
            printf("  Checksum: %lld\n", checksum);
        }
        else if (choice == 1) {
            // Call parallel target variant
            printf("Iteration %d: Calling parallel_target_loop\n", iter);
            parallel_target_loop(float_array, float_array, float_array,
                               low, high, stride, rows_val, cols_val);
            
            // Verify results with checksum
            double fchecksum = 0.0;
            for (int i = low; i <= high && i < N * M; i += stride) {
                fchecksum += float_array[i];
            }
            printf("  Float checksum: %f\n", fchecksum);
        }
        else {
            // Call combined constructs variant
            printf("Iteration %d: Calling combined_constructs\n", iter);
            combined_constructs(double_array, double_array, double_array,
                              mask_array, offset, range, dim1_val, dim2_val);
            
            // Verify results with checksum
            double dchecksum = 0.0;
            for (int i = offset; i < offset + range && i < N * M; i++) {
                if (mask_array[i]) {
                    dchecksum += double_array[i];
                }
            }
            printf("  Double checksum: %f\n", dchecksum);
        }
        
        // Occasionally call host-only parallel function
        if (rand() % 4 == 0) {
            printf("  Also calling host_only_parallel\n");
            int factor = 1 + rand() % 5;
            host_only_parallel(auto_array, N * M / 2, factor);
        }
        
        // Add some pointer arithmetic
        int *ptr = auto_array + start;
        for (int i = 0; i < 10; i++) {
            *(ptr + i) = *(ptr + i) * 2;
        }
    }
    
    // Final verification
    printf("\nFinal verification:\n");
    int final_sum = 0;
    for (int i = 0; i < N * M; i += 100) {
        final_sum += static_array[i] + auto_array[i] + (int)float_array[i];
    }
    printf("Aggregate sum: %d\n", final_sum);
    
    return 0;
}
