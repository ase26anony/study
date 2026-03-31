#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERATIONS 10

// Simple LCG for deterministic pseudo-random values
static unsigned int seed = 123456789;
static inline unsigned int lcg_rand() {
    seed = seed * 1103515245 + 12345;
    return seed;
}

// Function containing multiple SIMD loops with various clauses
void compute(float *a, float *b, float *c, float *d, int n, int iter) {
    float k = 1.5f + iter * 0.1f;
    float sum1 = 0.0f, sum2 = 0.0f;
    
    // Loop 1: Basic SIMD with safelen and simdlen
    #pragma omp simd safelen(16) simdlen(8) aligned(a, b, c: 64) linear(i:1)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + k;
    }
    
    // Loop 2: SIMD with reduction and conditional
    #pragma omp simd reduction(+:sum1) simdlen(4) aligned(c, d: 32)
    for (int i = 0; i < n; i++) {
        float val = c[i] * 2.0f;
        if (val > 0.5f) {  // Non-trivial conditional
            d[i] = val - 0.3f;
            sum1 += d[i];
        } else {
            d[i] = val + 0.7f;
        }
    }
    
    // Loop 3: SIMD with linear clause on reference
    float ref = iter * 0.01f;
    #pragma omp simd linear(ref:0.001f) simdlen(16) aligned(d: 64)
    for (int i = 0; i < n; i++) {
        d[i] = d[i] * ref + (float)i * 0.001f;
        ref += 0.001f;  // Modified by linear clause
    }
    
    // Loop 4: Mixed operations with different data types
    int *indices = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) indices[i] = i % 16;
    
    #pragma omp simd simdlen(8) reduction(+:sum2)
    for (int i = 0; i < n; i++) {
        // Mixed-type operations
        float temp = d[i] * (float)indices[i];
        sum2 += temp;
        
        // Built-in function call (trigonometric)
        #ifdef __FAST_MATH__
        c[i] = __builtin_sinf(temp * 0.01f);
        #else
        c[i] = temp * 0.5f;  // Fallback without fast math
        #endif
    }
    
    free(indices);
    
    // Nested SIMD loop structure
    for (int block = 0; block < 4; block++) {
        int start = block * (n / 4);
        int end = (block + 1) * (n / 4);
        
        #pragma omp simd simdlen(4) safelen(8)
        for (int i = start; i < end; i++) {
            // Complex operation with multiple array accesses
            a[i] = b[i] + c[i % 256] * d[i] - k;
        }
    }
    
    // Final reduction
    printf("Iter %d: sums = %f, %f\n", iter, sum1, sum2);
}

// Function that could be called from different contexts
void process_chunk(float *a, float *b, float *c, float *d, int start, int end) {
    // SIMD loop inside a function that might be inlined
    #pragma omp simd simdlen(8) aligned(a, b, c, d: 32)
    for (int i = start; i < end; i++) {
        float t1 = a[i] * 0.3f;
        float t2 = b[i] * 0.7f;
        c[i] = t1 + t2;
        d[i] = t1 - t2;
    }
}

int main() {
    // Allocate and initialize arrays
    float *a = (float*)aligned_alloc(64, SIZE * sizeof(float));
    float *b = (float*)aligned_alloc(64, SIZE * sizeof(float));
    float *c = (float*)aligned_alloc(64, SIZE * sizeof(float));
    float *d = (float*)aligned_alloc(64, SIZE * sizeof(float));
    
    // Initialize with pseudo-random pattern
    for (int i = 0; i < SIZE; i++) {
        a[i] = (float)(lcg_rand() % 1000) / 1000.0f;
        b[i] = (float)(lcg_rand() % 1000) / 1000.0f;
        c[i] = 0.0f;
        d[i] = 0.0f;
    }
    
    // Multiple calls to create different contexts
    for (int iter = 0; iter < ITERATIONS; iter++) {
        compute(a, b, c, d, SIZE, iter);
        
        // Process in chunks to create additional SIMD contexts
        for (int chunk = 0; chunk < 4; chunk++) {
            int start = chunk * (SIZE / 4);
            int end = start + (SIZE / 4);
            process_chunk(a, b, c, d, start, end);
        }
    }
    
    // Target construct for offloading (triggers different SIMT paths)
    #ifdef _OFFLOAD
    #pragma omp target teams distribute parallel for simd \
            map(to: a[0:SIZE], b[0:SIZE]) map(from: c[0:SIZE]) \
            simdlen(8) safelen(16)
    for (int i = 0; i < SIZE; i++) {
        c[i] = a[i] * b[i] + sinf((float)i * 0.01f);
    }
    #endif
    
    // Final checksum to prevent dead code elimination
    double checksum = 0.0;
    #pragma omp simd reduction(+:checksum) simdlen(4)
    for (int i = 0; i < SIZE; i++) {
        checksum += (double)a[i] + (double)b[i] + (double)c[i] + (double)d[i];
    }
    
    printf("Final checksum: %f\n", checksum);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
