/* haifa_scheduler_test.c
 * Designed to trigger GCC's Haifa scheduler context saving/restoration
 * and ensure free_sched_context is called during compilation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

/* Helper functions for inlining */
static inline int __attribute__((always_inline)) 
compute_hash(int x, int y) {
    /* Mix of arithmetic and bitwise ops for ILP */
    return ((x << 3) ^ y) + (x >> 2) * 7 - (y & 0xFF);
}

static inline int __attribute__((always_inline))
conditional_transform(int val, int threshold) {
    /* Complex conditional chain */
    if (val < threshold >> 1) {
        return val * 2 + 1;
    } else if (val < threshold) {
        return (val ^ 0x5A) & 0x7F;
    } else if (val < threshold * 2) {
        return val - threshold / 3;
    } else {
        return (val % 127) | 0x80;
    }
}

/* Hot function with complex control flow */
__attribute__((hot))
void process_data(int* restrict input, int* restrict output, int size, int mode) {
    int i, j, k;
    
    /* Outer loop with switch inside */
    for (i = 0; i < size; i++) {
        switch (mode) {
            case 0: {
                /* Nested loops with dependencies */
                int acc = input[i];
                for (j = 0; j < 8; j++) {
                    acc = compute_hash(acc, j);
                    /* Memory dependency chain */
                    for (k = 0; k < 4; k++) {
                        acc += input[(i + k) % size] * (j + 1);
                    }
                }
                output[i] = conditional_transform(acc, 1000);
                break;
            }
            case 1: {
                /* Irreducible control flow using computed goto */
                static void* labels[] = { &&L0, &&L1, &&L2, &&L3 };
                int idx = input[i] & 3;
                goto *labels[idx];
                
                L0:
                    output[i] = input[i] * 3 + 1;
                    continue;
                L1:
                    output[i] = (input[i] << 2) | (input[i] >> 30);
                    continue;
                L2:
                    output[i] = input[i] ^ 0xDEADBEEF;
                    continue;
                L3:
                    output[i] = ~input[i];
                    continue;
            }
            case 2: {
                /* Loop with carried dependency */
                int prev = (i > 0) ? output[i-1] : 0;
                int temp = input[i];
                
                /* Artificial scheduling barrier */
                asm volatile("" : "+r" (temp) : : "memory");
                
                for (j = 0; j < 16; j++) {
                    temp = (temp * 1103515245 + 12345) & 0x7FFFFFFF;
                    prev = (prev + temp) % 10007;
                }
                
                output[i] = prev;
                break;
            }
            default: {
                /* Mixed operations with memory accesses */
                int* ptr = &input[i];
                int val = *ptr;
                
                /* Multiple independent operations for ILP */
                int a = val + 5;
                int b = val * 3;
                int c = val & 0xFF00;
                int d = val >> 4;
                
                /* Force dependencies */
                a = compute_hash(a, b);
                c = conditional_transform(c, 256);
                d = compute_hash(d, c);
                
                output[i] = a + b - c + d;
                break;
            }
        }
    }
}

/* Vectorization candidate with OpenMP */
__attribute__((hot))
void vectorizable_operation(float* restrict a, float* restrict b, 
                           float* restrict c, int n) {
    int i;
    
    #pragma omp simd safelen(16)
    for (i = 0; i < n; i++) {
        /* Simple stride-1 operations */
        float t = a[i] * 2.5f;
        t += b[i] * 1.5f;
        t = t / (a[i] + 1.0f);
        c[i] = t;
        
        /* Additional ops to create scheduling opportunities */
        if (i % 32 == 0) {
            /* Scheduling barrier in vector loop */
            asm volatile("" ::: "memory");
        }
    }
}

/* Function with deeply nested control flow */
int complex_decision_tree(int x, int y, int z) {
    /* Deep if-else chain */
    if (x < y) {
        if (y < z) {
            if (x * 2 > z) {
                return compute_hash(x, y) | z;
            } else {
                for (int i = 0; i < 8; i++) {
                    x = (x ^ y) + (z << i);
                }
                return x;
            }
        } else {
            switch (z % 4) {
                case 0: return (x & y) | (z << 16);
                case 1: return (x | y) ^ (z & 0xFFFF);
                case 2: {
                    do {
                        x = (x * 3 + 1) & 0xFFF;
                        z--;
                    } while (z > 0 && x != 1);
                    return x + y;
                }
                case 3: return conditional_transform(x, y) + z;
                default: return 0;
            }
        }
    } else {
        int acc = 0;
        for (int i = 0; i < 16; i++) {
            acc += (x >> i) & 1;
            acc -= (y >> i) & 1;
            acc *= (z >> (i % 8)) & 1 ? 2 : 1;
        }
        return acc;
    }
}

/* Main orchestrator */
int main() {
    const int sizes[] = {1024, 2048, 4096, 8192};
    const int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    int total_checksum = 0;
    
    /* Warm-up phase */
    printf("Starting warm-up phase...\n");
    for (int warmup = 0; warmup < 3; warmup++) {
        int size = sizes[warmup % num_sizes];
        int* in = malloc(size * sizeof(int));
        int* out = malloc(size * sizeof(int));
        
        /* Initialize with pattern */
        for (int i = 0; i < size; i++) {
            in[i] = (i * 1103515245 + 12345) & 0x7FFF;
        }
        
        /* Call with different modes */
        for (int mode = 0; mode < 4; mode++) {
            process_data(in, out, size, mode);
            
            /* Compute checksum */
            int sum = 0;
            for (int i = 0; i < size; i++) {
                sum = (sum + out[i]) & 0xFFFF;
            }
            total_checksum ^= sum;
        }
        
        free(in);
        free(out);
    }
    
    /* Main test phase with timing */
    printf("Starting main test phase...\n");
    clock_t start = clock();
    
    for (int iter = 0; iter < 5; iter++) {
        for (int s = 0; s < num_sizes; s++) {
            int size = sizes[s];
            
            /* Test integer processing */
            int* int_in = malloc(size * sizeof(int));
            int* int_out = malloc(size * sizeof(int));
            
            for (int i = 0; i < size; i++) {
                int_in[i] = (i * iter + 123) ^ 0xABCD;
            }
            
            /* Multiple calls with different control flows */
            process_data(int_in, int_out, size, iter % 4);
            
            /* Complex decision tree on selected elements */
            for (int i = 0; i < size; i += 64) {
                int_out[i] = complex_decision_tree(
                    int_in[i], 
                    int_in[(i + 1) % size],
                    int_in[(i + 2) % size]
                );
            }
            
            /* Update checksum */
            for (int i = 0; i < size; i += 128) {
                total_checksum = (total_checksum * 31 + int_out[i]) & 0xFFFFFF;
            }
            
            free(int_in);
            free(int_out);
            
            /* Test vectorizable operations */
            if (size <= 4096) {  /* Avoid huge allocations */
                float* float_a = malloc(size * sizeof(float));
                float* float_b = malloc(size * sizeof(float));
                float* float_c = malloc(size * sizeof(float));
                
                for (int i = 0; i < size; i++) {
                    float_a[i] = (i % 255) * 0.1f;
                    float_b[i] = ((i * 7) % 255) * 0.05f;
                }
                
                /* Multiple vectorized passes */
                for (int pass = 0; pass < 3; pass++) {
                    vectorizable_operation(float_a, float_b, float_c, size);
                    
                    /* Swap buffers */
                    float* temp = float_a;
                    float_a = float_b;
                    float_b = float_c;
                    float_c = temp;
                }
                
                /* Incorporate into checksum */
                for (int i = 0; i < size; i += 256) {
                    total_checksum ^= (int)(float_c[i] * 1000);
                }
                
                free(float_a);
                free(float_b);
                free(float_c);
            }
        }
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Test completed in %.3f seconds\n", elapsed);
    printf("Final checksum: 0x%06X\n", total_checksum & 0xFFFFFF);
    
    /* Verify checksum is non-zero (basic correctness check) */
    if (total_checksum == 0) {
        printf("WARNING: Zero checksum detected\n");
        return 1;
    }
    
    return 0;
}
