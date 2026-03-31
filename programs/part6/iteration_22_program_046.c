/* Test case 1: Tight floating-point loop for software pipelining */
#include <math.h>

#define SIZE 1024

void compute_fp_loop(float *a, float *b, float *c, int n) {
    for (int i = 0; i < n; i++) {
        // Complex FP operations to create scheduling pressure
        float t1 = a[i] * b[i];
        float t2 = sinf(a[i]) * cosf(b[i]);
        float t3 = t1 * t2 + a[i] / (b[i] + 1.0f);
        c[i] = t3 * t3 - sqrtf(fabsf(t1 - t2));
        
        // Additional dependent operations
        if (i > 0) {
            c[i] += c[i-1] * 0.5f;
        }
    }
}

void nested_loops_fp(float *data, int n, int m) {
    for (int i = 0; i < n; i++) {
        float acc = 0.0f;
        for (int j = 0; j < m; j++) {
            // Mixed operations to prevent vectorization
            acc += data[i * m + j] * (j % 8);
            acc = acc * 0.99f + sinf(acc * 0.01f);
        }
        data[i * m] = acc;
        
        // Conditional with data-dependent branch
        if (acc > 100.0f) {
            for (int k = 0; k < i; k++) {
                data[k * m] *= 0.9f;
            }
        }
    }
}

volatile int g_volatile = 0;

int main_test1() {
    float a[SIZE], b[SIZE], c[SIZE];
    
    // Initialize with non-constant values
    for (int i = 0; i < SIZE; i++) {
        a[i] = (i % 37) * 0.1f;
        b[i] = (i % 41) * 0.2f;
    }
    
    // Multiple calls with different parameters
    compute_fp_loop(a, b, c, SIZE);
    compute_fp_loop(b, c, a, SIZE / 2);
    compute_fp_loop(c, a, b, SIZE / 4);
    
    nested_loops_fp(a, 32, 32);
    nested_loops_fp(b, 16, 64);
    
    // Use volatile to prevent optimization
    return g_volatile + (int)(c[0] * 1000);
}
