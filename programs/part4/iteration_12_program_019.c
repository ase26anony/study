/* Target: haifa-sched.cc - free_sched_context coverage test */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define HOT __attribute__((hot))
#define ALWAYS_INLINE __attribute__((always_inline)) inline
#define NOINLINE __attribute__((noinline))

/* Helper functions for inlining */
ALWAYS_INLINE unsigned mix_bits(unsigned a, unsigned b) {
    return (a ^ (b << 1)) + (a >> 3) * b;
}

ALWAYS_INLINE int compute_hash(int x, int y) {
    int h = x * 0x9e3779b9;
    h ^= y * 0x9e3779b1;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    return h;
}

/* Complex control flow with irreducible CFG using computed goto */
HOT NOINLINE unsigned complex_flow(int *arr, int n) {
    static void *labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4, &&L5 };
    unsigned sum = 0;
    int i = 0, state = 0;
    
    /* Nested loops with switch inside */
    for (int outer = 0; outer < n; outer++) {
        int inner = 0;
        while (inner < n) {
            switch (state % 6) {
                case 0: sum += arr[inner] * 2; break;
                case 1: sum ^= arr[inner] | 0xFF; break;
                case 2: sum = (sum << 3) | (sum >> 29); break;
                case 3: sum += compute_hash(inner, outer); break;
                case 4: sum = mix_bits(sum, arr[inner]); break;
                case 5: sum -= arr[inner] * 3; break;
            }
            
            /* Computed goto for irreducible flow */
            goto *labels[(inner + state) % 6];
            
            L0: inner += 1; state ^= 0x1; continue;
            L1: inner += 2; state ^= 0x3; continue;
            L2: inner += 3; state ^= 0x7; continue;
            L3: inner += 1; state ^= 0xF; continue;
            L4: inner += 4; state ^= 0xF0; continue;
            L5: inner += 2; state ^= 0xFF; continue;
        }
        
        /* Deep if-else chain */
        if (outer % 2 == 0) {
            if (sum % 3 == 0) {
                sum = (sum * 7) >> 1;
            } else if (sum % 5 == 0) {
                sum = (sum * 11) >> 2;
            } else if (sum % 7 == 0) {
                sum = (sum * 13) >> 3;
            } else {
                sum = (sum * 17) >> 4;
            }
        } else if (outer % 3 == 0) {
            sum = sum ^ 0xAAAAAAAA;
        } else if (outer % 7 == 0) {
            sum = sum | 0x55555555;
        } else {
            sum = sum & 0x33333333;
        }
    }
    return sum;
}

/* Function with instruction mix and memory dependencies */
HOT NOINLINE unsigned instruction_mix(int *a, int *b, int n) {
    unsigned result = 0;
    int temp[256];
    
    /* Memory operations with dependencies */
    for (int i = 0; i < n; i++) {
        /* Load-store chain */
        int x = a[i];
        int y = b[i];
        
        /* Arithmetic mix */
        int t1 = x + y;
        int t2 = x - y;
        int t3 = x * y;
        int t4 = x ^ y;
        int t5 = x | y;
        int t6 = x & y;
        
        /* Artificial scheduling barrier */
        asm volatile("" : "+r"(t1), "+r"(t2) : : "memory");
        
        /* More operations with dependencies */
        t1 = t1 * t2;
        t2 = t3 ^ t4;
        t3 = t5 | t6;
        t4 = mix_bits(t1, t2);
        
        /* Store with address calculation */
        temp[i % 256] = t1 + t2 + t3 + t4;
        
        /* Complex expression tree */
        result += ((t1 << 2) + (t2 >> 1)) * ((t3 & 0xFF) | (t4 & 0xFF00));
        
        /* Function call to inline candidate */
        result ^= compute_hash(i, result);
    }
    
    /* Second loop with carried dependency */
    int carry = 0;
    for (int i = 0; i < 256; i++) {
        carry = temp[i] + carry * 3;
        result += carry;
    }
    
    return result;
}

/* Vectorization candidate with OpenMP */
HOT NOINLINE unsigned vectorizable_loop(int *data, int n) {
    unsigned sum = 0;
    
    #pragma omp simd reduction(+:sum) simdlen(8)
    for (int i = 0; i < n; i++) {
        /* Simple stride-1 access pattern */
        int val = data[i];
        val = val * 3 + 7;
        val = val ^ (val >> 4);
        val = val * 0x9e3779b9;
        sum += val;
    }
    
    /* Nested loop for more scheduling complexity */
    for (int i = 0; i < n; i += 64) {
        int block_sum = 0;
        #pragma omp parallel for reduction(+:block_sum)
        for (int j = i; j < i + 64 && j < n; j++) {
            block_sum += data[j] * (j - i + 1);
        }
        sum ^= block_sum * 0x5A827999;
    }
    
    return sum;
}

/* Main orchestrator with varying inputs */
int main() {
    const int sizes[] = {256, 512, 1024, 2048, 4096};
    const int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    unsigned final_result = 0;
    
    /* Warm-up phase */
    printf("Starting warm-up...\n");
    for (int iter = 0; iter < 3; iter++) {
        for (int s = 0; s < num_sizes; s++) {
            int n = sizes[s];
            int *arr1 = malloc(n * sizeof(int));
            int *arr2 = malloc(n * sizeof(int));
            
            /* Initialize with pseudo-random data */
            for (int i = 0; i < n; i++) {
                arr1[i] = (i * 0x9e3779b9) ^ 0x12345678;
                arr2[i] = (i * 0x85ebca6b) ^ 0x87654321;
            }
            
            /* Call complex functions */
            unsigned r1 = complex_flow(arr1, n);
            unsigned r2 = instruction_mix(arr1, arr2, n);
            unsigned r3 = vectorizable_loop(arr1, n);
            
            final_result ^= r1 + r2 * 3 + r3 * 7;
            
            free(arr1);
            free(arr2);
        }
    }
    
    /* Timed execution with different patterns */
    printf("Starting timed execution...\n");
    clock_t start = clock();
    
    for (int phase = 0; phase < 5; phase++) {
        for (int s = 0; s < num_sizes; s++) {
            int n = sizes[s] * (phase + 1);
            int *arr1 = malloc(n * sizeof(int));
            int *arr2 = malloc(n * sizeof(int));
            
            /* Different initialization patterns */
            for (int i = 0; i < n; i++) {
                if (phase % 2 == 0) {
                    arr1[i] = (i * 0x1234567) >> (i % 8);
                    arr2[i] = (i * 0x89ABCDEF) << (i % 8);
                } else {
                    arr1[i] = i ^ 0xF0F0F0F0;
                    arr2[i] = i | 0x0F0F0F0F;
                }
            }
            
            /* Mix of function calls */
            unsigned r1 = complex_flow(arr1, n);
            unsigned r2 = instruction_mix(arr1, arr2, n / 2);
            unsigned r3 = vectorizable_loop(arr2, n);
            
            /* Complex final computation */
            final_result = mix_bits(final_result, r1);
            final_result ^= compute_hash(r2, r3);
            final_result = (final_result << 5) | (final_result >> 27);
            
            free(arr1);
            free(arr2);
        }
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Final result: 0x%08X\n", final_result);
    printf("Execution time: %.3f seconds\n", elapsed);
    
    /* Verification (should be deterministic with fixed seed) */
    if (final_result == 0x7A3B9C1D) {
        printf("VERIFICATION PASSED\n");
    } else {
        printf("VERIFICATION: Got 0x%08X\n", final_result);
    }
    
    return 0;
}
