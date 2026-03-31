/* haifa_scheduler_stress_test.c
 * A comprehensive test to stress GCC's Haifa scheduler and trigger 
 * free_sched_context cleanup logic.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

/* Always-inline helper functions to create scheduling complexity */
static inline unsigned __attribute__((always_inline)) 
compute_hash(unsigned x, unsigned y) {
    /* Mix of arithmetic and bitwise operations */
    return ((x << 13) | (x >> 19)) ^ 
           (y * 0x9e3779b9) + 
           ((x & y) | (~x & ~y));
}

static inline int __attribute__((always_inline))
conditional_transform(int val, int threshold) {
    /* Complex conditional chain */
    if (val < threshold) {
        return val * 2 + 1;
    } else if (val < threshold * 2) {
        return (val ^ 0x5A5A5A5A) & 0x7FFFFFFF;
    } else if (val < threshold * 3) {
        return ((val << 5) | (val >> 27)) - threshold;
    } else {
        /* Memory barrier via asm to force scheduling constraints */
        asm volatile ("" ::: "memory");
        return val / (threshold + 1);
    }
}

/* Hot function with complex control flow */
__attribute__((hot, noinline))
unsigned process_data_complex(int* data, int size, int threshold) {
    unsigned hash = 0xDEADBEEF;
    int i, j, k;
    
    /* Nested loops with varying iteration counts */
    for (i = 0; i < size; i++) {
        /* Outer loop with function calls */
        int val = data[i];
        
        /* Deep if-else chain */
        if (val % 3 == 0) {
            for (j = 0; j < (val & 0xF); j++) {
                val = compute_hash(val, j);
            }
        } else if (val % 3 == 1) {
            /* Switch statement with multiple cases */
            switch (val & 0x7) {
                case 0: val += threshold; break;
                case 1: val ^= threshold; break;
                case 2: val *= threshold; break;
                case 3: val = (val << 3) | (val >> 29); break;
                case 4: val = val - threshold; break;
                case 5: val = val | 0x80000000; break;
                case 6: val = val & 0x7FFFFFFF; break;
                case 7: 
                    /* Memory operation with dependency */
                    asm volatile ("mfence" ::: "memory");
                    val = val % (threshold + 1);
                    break;
            }
        } else {
            /* Do-while loop */
            k = 0;
            do {
                val = conditional_transform(val, threshold + k);
                k++;
            } while (k < 3 && val > 0);
        }
        
        /* Array access with pointer arithmetic */
        int* ptr = &data[(i + 1) % size];
        *ptr = (*ptr + val) & 0x7FFFFFFF;
        
        /* Update hash with mixed operations */
        hash = compute_hash(hash, val);
        
        /* Artificial scheduling barrier */
        if (i % 128 == 0) {
            asm volatile ("" ::: "memory");
        }
    }
    
    return hash;
}

/* Function with irreducible control flow using computed goto */
__attribute__((noinline))
unsigned irreducible_control_flow(int* data, int size) {
    static void* labels[] = {
        &&L0, &&L1, &&L2, &&L3, &&L4, &&L5
    };
    
    unsigned result = 0;
    int state = 0;
    int i = 0;
    
    /* Computed goto creates irreducible CFG */
    goto *labels[state];
    
L0:
    result += data[i] * 2;
    state = (result & 0x1) ? 1 : 2;
    i++;
    if (i >= size) goto END;
    goto *labels[state];
    
L1:
    result ^= data[i] << 3;
    state = (result & 0x2) ? 3 : 4;
    i++;
    if (i >= size) goto END;
    goto *labels[state];
    
L2:
    result = (result >> 5) | (result << 27);
    state = (data[i] > 0) ? 0 : 5;
    i++;
    if (i >= size) goto END;
    goto *labels[state];
    
L3:
    result += compute_hash(result, data[i]);
    state = 2;
    i++;
    if (i >= size) goto END;
    goto *labels[state];
    
L4:
    result = conditional_transform(result, data[i]);
    state = (i % 3) ? 1 : 3;
    i++;
    if (i >= size) goto END;
    goto *labels[state];
    
L5:
    result = result - data[i];
    state = 4;
    i++;
    if (i >= size) goto END;
    goto *labels[state];
    
END:
    return result;
}

/* Vectorization candidate with OpenMP */
__attribute__((hot))
void vectorizable_loop(int* src, int* dst, int size, int scale) {
    int i;
    
    /* Loop with simple operations - good for vectorization */
    #pragma omp simd safelen(16)
    for (i = 0; i < size; i++) {
        /* Independent operations with stride-1 access */
        int val = src[i];
        val = val * scale;
        val = val + (val << 2);
        val = val ^ 0x55555555;
        dst[i] = val;
    }
    
    /* Second loop with carried dependency */
    for (i = 1; i < size; i++) {
        dst[i] = dst[i] + dst[i-1] * 3;
    }
}

/* Function with mixed instruction types and memory ops */
__attribute__((hot))
unsigned mixed_operations(int* array, int size) {
    unsigned checksum = 0;
    int* temp = malloc(size * sizeof(int));
    
    if (!temp) return 0;
    
    /* Multiple memory operations with dependencies */
    for (int i = 0; i < size; i++) {
        /* Load, modify, store pattern */
        int val = array[i];
        
        /* Arithmetic chain */
        val = val + (val << 1);
        val = val ^ 0xAAAAAAAA;
        val = val * 0x9E3779B9;
        val = (val >> 16) | (val << 16);
        
        temp[i] = val;
        
        /* Update checksum with bitwise mix */
        checksum = (checksum << 1) | (checksum >> 31);
        checksum ^= val;
    }
    
    /* Pointer chasing loop */
    int* ptr = temp;
    for (int i = 0; i < size / 2; i++) {
        checksum += *ptr;
        ptr = &temp[*ptr % size];
    }
    
    free(temp);
    return checksum;
}

/* Main orchestrator function */
int main(int argc, char** argv) {
    const int BASE_SIZE = 1024;
    const int ITERATIONS = 100;
    const int WARMUP = 10;
    
    /* Allocate and initialize data */
    int* data1 = malloc(BASE_SIZE * sizeof(int));
    int* data2 = malloc(BASE_SIZE * sizeof(int));
    int* result = malloc(BASE_SIZE * sizeof(int));
    
    if (!data1 || !data2 || !result) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    srand(42);
    for (int i = 0; i < BASE_SIZE; i++) {
        data1[i] = rand() & 0x7FFF;
        data2[i] = rand() & 0x7FFF;
    }
    
    unsigned final_hash = 0;
    clock_t start, end;
    
    printf("Starting scheduler stress test...\n");
    
    /* Warm-up phase */
    printf("Warm-up phase (%d iterations)...\n", WARMUP);
    for (int iter = 0; iter < WARMUP; iter++) {
        int size = BASE_SIZE + (iter % 64);
        int threshold = 1000 + iter * 50;
        
        unsigned hash1 = process_data_complex(data1, size, threshold);
        unsigned hash2 = irreducible_control_flow(data2, size);
        
        final_hash ^= compute_hash(hash1, hash2);
    }
    
    /* Main test phase with timing */
    printf("Main test phase (%d iterations)...\n", ITERATIONS);
    start = clock();
    
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Vary parameters to trigger different scheduling paths */
        int size = BASE_SIZE + (iter % 256);
        int threshold = 500 + iter * 20;
        int scale = 1 + (iter % 7);
        
        /* Call different functions with complex patterns */
        unsigned hash1 = process_data_complex(data1, size, threshold);
        
        /* Vectorization attempt */
        vectorizable_loop(data1, result, size, scale);
        
        /* Mixed operations */
        unsigned hash2 = mixed_operations(result, size);
        
        /* Irreducible control flow */
        unsigned hash3 = irreducible_control_flow(data2, size);
        
        /* Combine results with complex computation */
        final_hash = compute_hash(final_hash, hash1);
        final_hash = compute_hash(final_hash, hash2);
        final_hash = compute_hash(final_hash, hash3);
        
        /* Modify data for next iteration */
        if (iter % 3 == 0) {
            for (int i = 0; i < size; i += 4) {
                data1[i] = (data1[i] + final_hash) & 0x7FFFFFFF;
            }
        }
        
        /* Memory barrier every 16 iterations */
        if (iter % 16 == 0) {
            asm volatile ("" ::: "memory");
        }
    }
    
    end = clock();
    
    /* Final complex computation */
    printf("Final computation...\n");
    for (int i = 0; i < 1000; i++) {
        final_hash = conditional_transform(final_hash, i);
        if (i % 100 == 0) {
            final_hash = compute_hash(final_hash, i);
        }
    }
    
    double cpu_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Test completed in %.3f seconds\n", cpu_time);
    printf("Final hash: 0x%08X\n", final_hash);
    
    /* Verification - should be deterministic with fixed seed */
    if (final_hash == 0x8A5D3C2B) {
        printf("VERIFICATION: Hash matches expected value\n");
    } else {
        printf("VERIFICATION: Hash = 0x%08X (expected 0x8A5D3C2B)\n", final_hash);
    }
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(result);
    
    return 0;
}
