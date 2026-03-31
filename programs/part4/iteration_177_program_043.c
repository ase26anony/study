#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

// Prevent inlining to force scheduler state saves/restores
__attribute__((noinline)) 
float helper_float_ops(float a, float b, float c, float d, float e) {
    volatile float barrier = a + b;
    asm volatile("" ::: "memory");
    float t1 = a * b + c;
    float t2 = d / (e + 1.0f);
    float t3 = t1 - t2;
    asm volatile("" ::: "memory");
    return t3 * barrier;
}

__attribute__((noinline))
int helper_int_ops(int a, int b, int c, int d, int e) {
    volatile int barrier = a ^ b;
    asm volatile("" ::: "memory");
    int t1 = (a & b) | c;
    int t2 = (d << 3) ^ e;
    int t3 = t1 * t2;
    asm volatile("" ::: "memory");
    return t3 + barrier;
}

__attribute__((noinline))
double helper_mixed_ops(int a, float b, double c, int d) {
    volatile double barrier = c;
    asm volatile("" ::: "memory");
    double t1 = (double)a * (double)b;
    double t2 = c / (d + 1.0);
    double t3 = sin(t1) + cos(t2);
    asm volatile("" ::: "memory");
    return t3 * barrier;
}

// Complex function with high register pressure and mixed operations
void complex_scheduling_test(volatile int outer_iterations) {
    // Many local variables to create register pressure (30+)
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f, f6 = 6.6f, f7 = 7.7f, f8 = 8.8f;
    double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04, d5 = 5.05, d6 = 6.06, d7 = 7.07, d8 = 8.08;
    int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15, v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    float f9 = 9.9f, f10 = 10.10f, f11 = 11.11f, f12 = 12.12f;
    double d9 = 9.09, d10 = 10.10, d11 = 11.11, d12 = 12.12;
    int v21 = 21, v22 = 22, v23 = 23, v24 = 24, v25 = 25;
    
    volatile int checksum = 0;
    volatile float f_checksum = 0.0f;
    volatile double d_checksum = 0.0;
    
    // Outer loop with volatile limit
    for (volatile int outer = 0; outer < outer_iterations; outer++) {
        // Nested loops with variable bounds
        int inner_limit = (outer % 10) + 5;  // Variable trip count
        volatile int inner_volatile = inner_limit;
        
        for (int i = 0; i < inner_volatile; i++) {
            // Mixed operation dependency chains
            // int -> float -> memory -> int chain
            v1 = v2 + v3;
            f1 = (float)v1 * f2;
            asm volatile("" ::: "memory");  // Barrier
            
            // float -> double -> int chain
            d1 = (double)f1 + d2;
            v4 = (int)d1 ^ v5;
            asm volatile("" ::: "memory");
            
            // Memory access pattern
            int* ptr1 = &v6;
            float* ptr2 = &f3;
            *ptr1 = v7 * v8;
            *ptr2 = f4 / f5;
            
            // Call helper functions with dependencies
            if (i % 3 == 0) {
                f6 = helper_float_ops(f1, f2, f3, f4, f5);
                v9 = helper_int_ops(v1, v2, v3, v4, v5);
                d3 = helper_mixed_ops(v6, f6, d1, v9);
            }
            
            // Conditional execution paths
            switch (i % 4) {
                case 0:
                    // FP math branch
                    f7 = f1 * f2 + f3 - f4 / f5;
                    d4 = sin(d1) * cos(d2);
                    v10 = (int)(f7 * 100.0f);
                    break;
                case 1:
                    // Integer bit manipulation branch
                    v11 = (v1 << 2) | (v2 >> 1);
                    v12 = v3 ^ v4 & v5;
                    v13 = ~v6;
                    f8 = (float)(v11 ^ v12 ^ v13);
                    break;
                case 2:
                    // Mixed operations branch
                    v14 = v7 * v8 + v9;
                    f9 = sqrt(fabs(f3 * f4 - f5));
                    d5 = (double)v14 / (double)(v15 + 1);
                    v16 = (int)(d5 * 1000.0);
                    break;
                case 3:
                    // Memory intensive branch
                    int arr[8];
                    for (int j = 0; j < 8; j++) {
                        arr[j] = v17 + j;
                        v17 = arr[j] ^ v18;
                    }
                    f10 = (float)arr[i % 8];
                    d6 = (double)arr[(i + 1) % 8];
                    break;
            }
            
            // More dependency chains
            v18 = v10 + v11;
            f11 = f7 * f8 + f9;
            d7 = d4 * d5 + d6;
            
            // Another barrier
            asm volatile("" ::: "memory");
            
            // Update checksums
            checksum ^= v1 ^ v4 ^ v10 ^ v14 ^ v18;
            f_checksum += f1 + f6 + f7 + f11;
            d_checksum += d1 + d3 + d5 + d7;
        }
        
        // Second level of nesting with different pattern
        for (volatile int k = 0; k < (outer % 5) + 2; k++) {
            // Different operation mix
            v19 = v20 * v21 - v22;
            f12 = f10 / (f11 + 1.0f);
            d8 = exp(d7 * 0.1) - 1.0;
            
            // Call helper in inner loop
            v23 = helper_int_ops(v19, v20, v21, v22, v23);
            d9 = helper_mixed_ops(v23, f12, d8, v24);
            
            // Memory barrier
            asm volatile("" ::: "memory");
            
            // Update variables with complex expression
            v24 = (v19 * v23) / (v20 + 1);
            v25 = (v21 << (k % 3)) | (v22 >> (k % 2));
            
            checksum ^= v19 ^ v23 ^ v24 ^ v25;
            f_checksum += f12;
            d_checksum += d8 + d9;
        }
    }
    
    // Final complex calculation to use all variables
    volatile int final_result = 0;
    final_result = v1 + v5 + v10 + v15 + v20 + v25;
    final_result ^= (int)f_checksum;
    final_result ^= (int)d_checksum;
    final_result ^= checksum;
    
    // Print to prevent dead code elimination
    printf("Result: %d (checksum: %d, f: %f, d: %f)\n", 
           final_result, checksum, f_checksum, d_checksum);
}

int main() {
    // Volatile to prevent constant propagation
    volatile int iterations = 1000;
    
    // Run the complex scheduling test
    complex_scheduling_test(iterations);
    
    // Additional test with different iteration count
    volatile int more_iterations = 500;
    complex_scheduling_test(more_iterations);
    
    return 0;
}
