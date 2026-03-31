#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define M 512
#define MAX_ITER 5

// Variant 1: SIMD target loop
void simd_target_loop(int *a, int *b, int *c, int start, int end, int step) {
    volatile int v_start = start;  // Prevent constant folding
    volatile int v_end = end;
    
    #pragma omp target teams distribute parallel for simd \
        map(to: a[v_start:v_end], b[v_start:v_end]) \
        map(from: c[v_start:v_end]) \
        private(v_start, v_end) \
        firstprivate(step)
    for (int i = v_start; i < v_end; i += step) {
        c[i] = a[i] * 2 + b[i] / 3;
    }
}

// Variant 2: Parallel target loop without SIMD
void parallel_target_loop(float *x, float *y, float *z, int rows, int cols) {
    volatile int v_rows = rows;
    volatile int v_cols = cols;
    
    #pragma omp target teams distribute parallel for collapse(2) \
        map(to: x[0:v_rows*v_cols], y[0:v_rows*v_cols]) \
        map(from: z[0:v_rows*v_cols]) \
        shared(v_rows, v_cols)
    for (int i = 0; i < v_rows; i++) {
        for (int j = 0; j < v_cols; j++) {
            int idx = i * v_cols + j;
            z[idx] = x[idx] * 1.5f + y[idx] * 0.5f;
        }
    }
}

// Variant 3: Combined constructs with data region
void combined_constructs(double *p, double *q, double *r, int size, int offset) {
    static double static_mult = 2.0;  // Mix static storage
    const double const_add = 1.5;     // Mix const qualifier
    volatile int v_size = size;
    
    #pragma omp target data map(to: p[offset:v_size], q[offset:v_size]) \
                            map(from: r[offset:v_size])
    {
        #pragma omp target teams distribute parallel for simd \
            firstprivate(static_mult, const_add, offset) \
            private(v_size)
        for (int i = 0; i < v_size; i++) {
            int idx = i + offset;
            r[idx] = p[idx] * static_mult + q[idx] * const_add;
        }
    }
}

// Variant 4: Host-only parallel region (for conditional execution)
void host_only_parallel(int *arr1, int *arr2, int len) {
    volatile int v_len = len;
    
    #pragma omp parallel for simd
    for (int i = 0; i < v_len; i++) {
        arr1[i] = arr2[i] * 3 - i;
    }
}

// Helper function to initialize arrays
void init_arrays(int *a, int *b, int *c, 
                 float *x, float *y, float *z,
                 double *p, double *q, double *r, 
                 int size) {
    for (int i = 0; i < size; i++) {
        a[i] = i % 100;
        b[i] = (i + 1) % 100;
        c[i] = 0;
        
        x[i] = (float)i * 0.1f;
        y[i] = (float)i * 0.2f;
        z[i] = 0.0f;
        
        p[i] = (double)i * 0.05;
        q[i] = (double)i * 0.1;
        r[i] = 0.0;
    }
}

// Helper function to compute checksum
long long compute_checksum_int(int *arr, int size) {
    long long sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}

float compute_checksum_float(float *arr, int size) {
    float sum = 0.0f;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}

double compute_checksum_double(double *arr, int size) {
    double sum = 0.0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    // Use command-line argument for random seed
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    // Declare arrays with mixed storage durations
    static int static_array[N];  // Static storage
    int auto_array1[N], auto_array2[N], auto_array3[N];  // Automatic storage
    
    float float_array1[M], float_array2[M], float_array3[M];
    double double_array1[N], double_array2[N], double_array3[N];
    
    // Initialize arrays
    init_arrays(auto_array1, auto_array2, auto_array3,
                float_array1, float_array2, float_array3,
                double_array1, double_array2, double_array3,
                N);
    
    // Initialize static array separately
    for (int i = 0; i < N; i++) {
        static_array[i] = rand() % 100;
    }
    
    printf("Starting OpenMP SIMT transformation test with seed %d\n", seed);
    
    // Main test loop with varying parameters
    for (int iter = 0; iter < MAX_ITER; iter++) {
        printf("\n=== Iteration %d ===\n", iter);
        
        // Use rand() to create runtime-dependent conditions
        int use_target = (rand() % 3) > 0;  // 2/3 probability for target regions
        int loop_variant = rand() % 4;      // Choose different loop variants
        
        // Create volatile variables for loop bounds
        volatile int start_idx = rand() % (N/4);
        volatile int end_idx = N - rand() % (N/4);
        volatile int step = 1 + rand() % 3;
        
        volatile int rows = 16 + rand() % 32;
        volatile int cols = 16 + rand() % 32;
        
        volatile int offset = rand() % 100;
        volatile int size = 200 + rand() % 300;
        
        // Conditional execution based on random values
        if (use_target) {
            switch (loop_variant) {
                case 0:
                    printf("Calling simd_target_loop...\n");
                    simd_target_loop(auto_array1, auto_array2, auto_array3, 
                                    start_idx, end_idx, step);
                    {
                        long long checksum = compute_checksum_int(auto_array3, N);
                        printf("Checksum (int): %lld\n", checksum);
                    }
                    break;
                    
                case 1:
                    printf("Calling parallel_target_loop...\n");
                    parallel_target_loop(float_array1, float_array2, float_array3,
                                        rows, cols);
                    {
                        float checksum = compute_checksum_float(float_array3, M);
                        printf("Checksum (float): %f\n", checksum);
                    }
                    break;
                    
                case 2:
                    printf("Calling combined_constructs...\n");
                    combined_constructs(double_array1, double_array2, double_array3,
                                       size, offset);
                    {
                        double checksum = compute_checksum_double(double_array3, N);
                        printf("Checksum (double): %f\n", checksum);
                    }
                    break;
                    
                case 3:
                    // Nested call pattern
                    printf("Calling nested pattern...\n");
                    if (rand() % 2) {
                        simd_target_loop(auto_array1, static_array, auto_array3,
                                        start_idx, end_idx, step);
                    } else {
                        combined_constructs(double_array1, double_array2, double_array3,
                                           size, offset);
                    }
                    break;
            }
        } else {
            printf("Calling host_only_parallel...\n");
            host_only_parallel(auto_array1, auto_array2, N);
            {
                long long checksum = compute_checksum_int(auto_array1, N);
                printf("Checksum (host only): %lld\n", checksum);
            }
        }
        
        // Mix pointer arithmetic to stress data dependency analysis
        int *ptr1 = auto_array1 + start_idx;
        int *ptr2 = auto_array2 + start_idx;
        for (int i = 0; i < 10; i++) {
            ptr1[i] = ptr2[i] + i;
        }
    }
    
    // Final verification with all arrays
    printf("\n=== Final Verification ===\n");
    long long final_int_sum = compute_checksum_int(auto_array3, N);
    float final_float_sum = compute_checksum_float(float_array3, M);
    double final_double_sum = compute_checksum_double(double_array3, N);
    
    printf("Final int checksum: %lld\n", final_int_sum);
    printf("Final float checksum: %f\n", final_float_sum);
    printf("Final double checksum: %f\n", final_double_sum);
    
    return 0;
}
