/* haifa_scheduler_test.c
 * Complex program designed to stress GCC's Haifa scheduler and trigger
 * free_sched_context cleanup logic.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

/* Helper functions marked for inlining */
static inline int __attribute__((always_inline)) 
compute_hash(int a, int b, int c) {
    /* Mix of arithmetic and bitwise ops for ILP */
    return ((a ^ b) << 3) | ((b + c) & 0xFF) ^ (a * 7);
}

static inline int __attribute__((always_inline))
conditional_transform(int x, int selector) {
    /* Complex conditional chain */
    if (selector == 0) return x * 2;
    else if (selector == 1) return x ^ 0xAAAAAAAA;
    else if (selector == 2) return x + (x >> 3);
    else if (selector == 3) return x * x;
    else if (selector == 4) return ~x;
    else return x;
}

/* Hot function with complex control flow */
__attribute__((hot))
void process_data(int* input, int* output, int size, int mode) {
    int i, j, k;
    
    /* Outer loop with varying iteration count */
    for (i = 0; i < size; i += 16) {
        int chunk_size = (size - i) > 16 ? 16 : (size - i);
        
        /* Nested loops with dependencies */
        for (j = 0; j < chunk_size; j++) {
            int idx = i + j;
            int temp = input[idx];
            
            /* Deep if-else chain */
            switch (mode % 6) {
                case 0:
                    for (k = 0; k < 4; k++) {
                        temp = compute_hash(temp, j, k);
                    }
                    break;
                case 1:
                    temp = conditional_transform(temp, idx % 6);
                    /* Memory dependency chain */
                    if (j > 0) temp ^= output[idx - 1];
                    break;
                case 2:
                    /* Loop with carried dependency */
                    for (k = 0; k < 8; k++) {
                        temp = (temp << 1) | ((temp >> 31) & 1);
                        if (k % 3 == 0) temp ^= 0xDEADBEEF;
                    }
                    break;
                case 3:
                    /* Mixed operations */
                    temp = (temp * 1103515245 + 12345) & 0x7FFFFFFF;
                    temp = temp ^ (temp >> 16);
                    break;
                case 4:
                    /* Artificial scheduling barrier */
                    asm volatile("" : "+r" (temp) : : "memory");
                    temp = temp * 3 + 1;
                    while (temp > 1000) temp >>= 1;
                    break;
                case 5:
                    /* More complex computation */
                    temp = (temp & 0x55555555) << 1 | (temp & 0xAAAAAAAA) >> 1;
                    temp = compute_hash(temp, i, j);
                    break;
            }
            
            /* Store with potential aliasing */
            output[idx] = temp;
            
            /* Memory clobber to force scheduling constraints */
            asm volatile("" : : "r"(temp) : "memory");
        }
        
        /* Computed goto to create irreducible CFG */
        if (chunk_size < 16) {
            static void* labels[] = { &&L1, &&L2, &&L3, &&L4, &&L5 };
            goto *labels[mode % 5];
        }
        
        L1: if (i % 64 == 0) mode ^= 1;
        L2: if (i % 128 == 0) mode = (mode + 1) % 6;
        L3: continue;
        L4: i += 4; /* Skip some iterations */
        L5: /* Fall through */;
    }
}

/* Function with vectorization candidate */
__attribute__((hot))
void vectorizable_operation(float* a, float* b, float* c, int n) {
    int i;
    
    /* Loop with stride-1 access for vectorization */
    #pragma omp simd
    for (i = 0; i < n; i++) {
        /* Independent operations with some dependencies */
        float t1 = a[i] * 2.0f;
        float t2 = b[i] + 1.0f;
        c[i] = t1 * t2 - a[i] / (b[i] + 0.001f);
        
        /* Conditional to prevent simple vectorization */
        if (i % 8 == 0) {
            c[i] = c[i] * 0.5f;
        }
    }
}

/* Function with OpenMP parallelization */
void parallel_processing(int* data, int size) {
    int i;
    
    #pragma omp parallel for schedule(dynamic, 16)
    for (i = 0; i < size; i++) {
        /* Complex per-iteration computation */
        int val = data[i];
        
        /* Loop with carried dependency within iteration */
        for (int j = 0; j < 10; j++) {
            val = (val * 1664525 + 1013904223) & 0x7FFFFFFF;
            if (j % 3 == 0) {
                val ^= compute_hash(val, i, j);
            }
        }
        
        /* Memory operation */
        data[i] = val;
        
        /* Barrier-like operation */
        #pragma omp atomic
        data[size % 1024] ^= val & 0xFF;
    }
}

/* Irreducible control flow using computed gotos */
__attribute__((noinline))
int irreducible_cfg(int x, int iterations) {
    static void* jump_table[] = { &&start, &&loop, &&cond1, &&cond2, &&end };
    
    int result = x;
    int counter = 0;
    
    goto *jump_table[0];
    
start:
    if (iterations <= 0) goto *jump_table[4];
    
loop:
    result = compute_hash(result, counter, iterations);
    counter++;
    
cond1:
    if (counter % 2 == 0) {
        result = conditional_transform(result, 1);
        goto *jump_table[2];
    } else {
        result = conditional_transform(result, 2);
        goto *jump_table[3];
    }
    
cond2:
    if (counter < iterations) {
        goto *jump_table[1];
    }
    
end:
    return result;
}

/* Main orchestrator */
int main() {
    const int sizes[] = {1024, 2048, 4096, 8192};
    const int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    int total_checksum = 0;
    
    /* Warm-up phase */
    printf("Starting warm-up phase...\n");
    for (int warmup = 0; warmup < 3; warmup++) {
        for (int s = 0; s < num_sizes; s++) {
            int size = sizes[s] >> (warmup + 1);
            if (size < 16) continue;
            
            int* data1 = malloc(size * sizeof(int));
            int* data2 = malloc(size * sizeof(int));
            float* fdata1 = malloc(size * sizeof(float));
            float* fdata2 = malloc(size * sizeof(float));
            float* fdata3 = malloc(size * sizeof(float));
            
            /* Initialize data */
            for (int i = 0; i < size; i++) {
                data1[i] = i ^ 0x12345678;
                data2[i] = 0;
                fdata1[i] = i * 0.1f;
                fdata2[i] = i * 0.2f;
                fdata3[i] = 0.0f;
            }
            
            /* Call complex functions */
            process_data(data1, data2, size, warmup);
            vectorizable_operation(fdata1, fdata2, fdata3, size);
            
            /* Compute checksum */
            int checksum = 0;
            for (int i = 0; i < size; i++) {
                checksum ^= data2[i];
                checksum += (int)(fdata3[i] * 1000);
            }
            total_checksum ^= checksum;
            
            free(data1);
            free(data2);
            free(fdata1);
            free(fdata2);
            free(fdata3);
        }
    }
    
    /* Main processing phase */
    printf("Starting main processing phase...\n");
    clock_t start = clock();
    
    for (int iter = 0; iter < 10; iter++) {
        for (int s = 0; s < num_sizes; s++) {
            int size = sizes[s];
            
            /* Allocate with different alignments */
            int* data = aligned_alloc(64, size * sizeof(int));
            int* buffer = aligned_alloc(32, size * sizeof(int));
            
            /* Initialize with pattern */
            for (int i = 0; i < size; i++) {
                data[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
                buffer[i] = 0;
            }
            
            /* Stress scheduler with different modes */
            for (int mode = 0; mode < 6; mode++) {
                process_data(data, buffer, size, mode);
                
                /* Parallel processing */
                parallel_processing(buffer, size);
                
                /* Irreducible CFG */
                int cfg_result = irreducible_cfg(buffer[0], 100);
                total_checksum ^= cfg_result;
                
                /* Mix array accesses */
                for (int i = 1; i < size; i++) {
                    buffer[i] ^= buffer[i-1] + compute_hash(i, mode, iter);
                }
            }
            
            /* Final checksum */
            int final_sum = 0;
            for (int i = 0; i < size; i++) {
                final_sum += buffer[i];
            }
            total_checksum ^= final_sum;
            
            free(data);
            free(buffer);
        }
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Total checksum: %d\n", total_checksum);
    printf("Elapsed time: %.3f seconds\n", elapsed);
    printf("Test completed successfully.\n");
    
    return 0;
}
