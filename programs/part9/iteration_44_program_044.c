#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_SIZE 1024
#define MIN_SIZE 64

// Variant 1: SIMD target loop
void simd_target_loop(int *a, int *b, int *c, int start, int end, int stride) {
    volatile int vsize = end - start;  // Prevent constant folding
    const int chunk = 16;
    static int counter = 0;  // Static variable for complexity
    
    #pragma omp target teams distribute parallel for simd \
        map(to: a[start:end:stride], b[start:end:stride]) \
        map(from: c[start:end:stride]) \
        num_teams(vsize/chunk) thread_limit(64) \
        private(counter) firstprivate(chunk)
    for (int i = start; i < end; i += stride) {
        counter = (i % 8) + 1;  // Varying private value
        c[i] = a[i] * counter + b[i] / (counter + 1);
    }
}

// Variant 2: Parallel target loop without SIMD clause
void parallel_target_loop(float *x, float *y, float *z, 
                         int low, int high, int step) {
    volatile int vlow = low, vhigh = high;
    float local_scale = 2.5f;
    
    #pragma omp target teams distribute parallel for \
        map(to: x[low:high:step], y[low:high:step]) \
        map(tofrom: z[low:high:step]) \
        collapse(2) num_teams(8) \
        firstprivate(local_scale)
    for (int i = vlow; i < vhigh; i += step) {
        for (int j = 0; j < 4; j++) {  // Nested loop for collapse
            int idx = i * 4 + j;
            if (idx < high * 4) {
                z[idx] = x[idx] * local_scale + y[idx] / local_scale;
                // Complex indexing to prevent optimization
                z[idx] += (i % 3 == 0) ? 1.0f : -1.0f;
            }
        }
    }
}

// Variant 3: Combined constructs with data region
void combined_constructs(double *p, double *q, double *r, 
                        int n, int offset, int mode) {
    volatile int vn = n;
    const double pi = 3.14159;
    double *temp = (double*)malloc(vn * sizeof(double));
    
    #pragma omp target data map(to: p[offset:n], q[offset:n]) \
                            map(alloc: temp[0:n])
    {
        // Initialize temp array on device
        #pragma omp target teams distribute parallel for simd \
            map(always, to: pi) \
            num_teams((vn+127)/128)
        for (int i = 0; i < vn; i++) {
            temp[i] = (mode == 0) ? p[i+offset] : q[i+offset];
        }
        
        // Main computation with pointer arithmetic
        double *src1 = p + offset;
        double *src2 = q + offset;
        double *dst = r + offset;
        
        #pragma omp target teams distribute parallel for simd \
            map(to: src1[0:n], src2[0:n]) \
            map(from: dst[0:n]) \
            firstprivate(pi, mode) \
            simdlen(8)
        for (int i = 0; i < vn; i++) {
            double t = temp[i];
            if (mode == 0) {
                dst[i] = src1[i] * pi + src2[i] / pi + t;
            } else {
                dst[i] = src2[i] * pi - src1[i] / pi + t * t;
            }
        }
    }
    
    free(temp);
}

// Host-only parallel region for conditional execution
void host_only_parallel(int *arr, int size) {
    volatile int vsize = size;
    
    #pragma omp parallel for simd schedule(static, 8)
    for (int i = 0; i < vsize; i++) {
        arr[i] = arr[i] * 2 + (i % 7);
    }
}

int main(int argc, char *argv[]) {
    // Initialize with command-line seed
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    printf("Seed: %d\n", seed);
    
    // Allocate arrays with different types and storage durations
    static int arr1[MAX_SIZE];  // Static storage
    int arr2[MAX_SIZE];         // Automatic storage
    const int arr3[MAX_SIZE/2]; // Const array (uninitialized)
    float farr1[MAX_SIZE];
    float farr2[MAX_SIZE];
    double darr1[MAX_SIZE];
    double darr2[MAX_SIZE];
    double darr3[MAX_SIZE];
    
    // Initialize with random data
    for (int i = 0; i < MAX_SIZE; i++) {
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
        farr1[i] = (float)(rand() % 1000) / 10.0f;
        farr2[i] = (float)(rand() % 1000) / 10.0f;
        darr1[i] = (double)(rand() % 1000) / 10.0;
        darr2[i] = (double)(rand() % 1000) / 10.0;
    }
    
    // Initialize const array through pointer (non-constant initialization)
    int *ptr = (int*)arr3;
    for (int i = 0; i < MAX_SIZE/2; i++) {
        ptr[i] = i * 2;
    }
    
    int total_checksum = 0;
    
    // Run multiple iterations with varying parameters
    for (int iter = 0; iter < 5; iter++) {
        volatile int base = rand() % (MAX_SIZE - MIN_SIZE);
        volatile int size = MIN_SIZE + (rand() % (MAX_SIZE - MIN_SIZE));
        volatile int stride = 1 + (rand() % 3);
        volatile int mode = rand() % 2;
        
        printf("\nIteration %d: base=%d, size=%d, stride=%d, mode=%d\n",
               iter, base, size, stride, mode);
        
        // Conditional execution based on random value
        if (rand() % 2 == 0) {
            printf("  Calling simd_target_loop\n");
            simd_target_loop(arr1, arr2, arr1, base, base + size, stride);
            
            // Compute checksum
            int sum = 0;
            for (int i = base; i < base + size && i < MAX_SIZE; i += stride) {
                sum += arr1[i];
            }
            total_checksum += sum;
            printf("  Checksum: %d\n", sum);
        } else {
            printf("  Calling host_only_parallel\n");
            host_only_parallel(arr2, size);
        }
        
        // Always call parallel_target_loop
        printf("  Calling parallel_target_loop\n");
        parallel_target_loop(farr1, farr2, farr1, base, base + size/2, stride);
        
        // Compute checksum for float array
        float fsum = 0.0f;
        for (int i = base; i < base + size/2 && i < MAX_SIZE; i += stride) {
            fsum += farr1[i];
        }
        total_checksum += (int)fsum;
        printf("  Float checksum: %d\n", (int)fsum);
        
        // Call combined_constructs with varying offset
        volatile int offset = rand() % (MAX_SIZE/4);
        printf("  Calling combined_constructs (offset=%d)\n", offset);
        combined_constructs(darr1, darr2, darr3, size/4, offset, mode);
        
        // Compute checksum for double array
        double dsum = 0.0;
        for (int i = offset; i < offset + size/4 && i < MAX_SIZE; i++) {
            dsum += darr3[i];
        }
        total_checksum += (int)dsum;
        printf("  Double checksum: %d\n", (int)dsum);
    }
    
    printf("\nTotal checksum: %d\n", total_checksum);
    
    // Final verification with target update
    int final_sum = 0;
    #pragma omp target update from(arr1[0:MAX_SIZE])
    for (int i = 0; i < MAX_SIZE; i++) {
        final_sum += arr1[i];
    }
    printf("Final array sum: %d\n", final_sum);
    
    return 0;
}
