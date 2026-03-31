/* Target: haifa-sched.cc - free_sched_context block coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

#define HOT __attribute__((hot))
#define ALWAYS_INLINE __attribute__((always_inline)) inline
#define NOINLINE __attribute__((noinline))

/* Helper functions for inlining */
ALWAYS_INLINE unsigned int mix_bits(unsigned int a, unsigned int b, unsigned int c) {
    /* Complex bit mixing to create ILP opportunities */
    unsigned int t = a ^ (b << 3);
    t = (t >> 5) | (t << 27);
    t = t + c;
    t = t ^ (t >> 11);
    t = t * 0x9e3779b9;
    return t;
}

ALWAYS_INLINE int compute_hash(int x, int y, int z) {
    /* Multiple arithmetic operations with dependencies */
    int h = x * 0xcc9e2d51;
    h = (h << 15) | (h >> 17);
    h = h * 0x1b873593;
    h = h ^ y;
    h = (h << 13) | (h >> 19);
    h = h * 5 + 0xe6546b64;
    h = h ^ z;
    h = h ^ (h >> 16);
    h = h * 0x85ebca6b;
    h = h ^ (h >> 13);
    h = h * 0xc2b2ae35;
    h = h ^ (h >> 16);
    return h;
}

/* Function with complex control flow and nested loops */
HOT NOINLINE unsigned long process_matrix(int *matrix, int size, int threshold) {
    unsigned long checksum = 0;
    int i, j, k;
    
    /* Outer loop with varying iteration count */
    for (i = 0; i < size; i++) {
        /* Deeply nested if-else chain */
        if (i % 3 == 0) {
            /* Switch statement with multiple cases */
            switch (i % 7) {
                case 0: matrix[i] += 1; break;
                case 1: matrix[i] *= 2; break;
                case 2: matrix[i] ^= 0x5A; break;
                case 3: matrix[i] = ~matrix[i]; break;
                case 4: matrix[i] = (matrix[i] << 1) | (matrix[i] >> 31); break;
                case 5: matrix[i] = matrix[i] - threshold; break;
                case 6: matrix[i] = matrix[i] + compute_hash(i, threshold, matrix[i]); break;
            }
        } else if (i % 3 == 1) {
            /* Nested loop with carried dependency */
            for (j = 0; j < (i % 8); j++) {
                matrix[i] = mix_bits(matrix[i], j, threshold);
                /* Memory operation creating load/store dependencies */
                if (j > 0) {
                    matrix[i] += matrix[i-1] * 0x1b873593;
                }
            }
        } else {
            /* While loop with complex condition */
            k = 0;
            while (k < (i % 5)) {
                /* Inline assembly creating scheduling barriers */
                asm volatile("" : "+r" (matrix[i]) : : "memory");
                matrix[i] = (matrix[i] * 0x9e3779b9) ^ k;
                k++;
            }
        }
        
        /* Computed goto for irreducible control flow */
        static void *labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4 };
        goto *labels[i % 5];
        
        L0:
            checksum += matrix[i] * 3;
            continue;
        L1:
            checksum += matrix[i] ^ 0xFF;
            continue;
        L2:
            checksum += matrix[i] << 2;
            continue;
        L3:
            checksum += matrix[i] >> 1;
            continue;
        L4:
            checksum += ~matrix[i];
            continue;
    }
    
    return checksum;
}

/* Function with vectorization candidate loops */
HOT NOINLINE void vectorizable_operations(float *a, float *b, float *c, int n) {
    int i;
    
    /* Loop with simple stride-1 operations (auto-vectorization candidate) */
    #pragma omp simd
    for (i = 0; i < n; i++) {
        a[i] = b[i] * c[i] + 1.0f;
    }
    
    /* Another vectorization candidate with reduction */
    float sum = 0.0f;
    #pragma omp simd reduction(+:sum)
    for (i = 0; i < n; i++) {
        sum += a[i] * b[i];
        /* Artificial dependency chain */
        c[i] = sum * 0.5f + c[i-1 < 0 ? 0 : i-1] * 0.3f;
    }
    
    /* Do-while loop for control flow variety */
    i = 0;
    do {
        /* Mix of operations */
        a[i] = a[i] + b[i] * 2.0f - c[i];
        /* Inline assembly with memory clobber */
        asm volatile("" : : "r"(&a[i]), "r"(&b[i]) : "memory");
        i++;
    } while (i < n);
}

/* Function with OpenMP parallel region */
NOINLINE void parallel_region_work(int *data, int size) {
    int i;
    
    #pragma omp parallel for schedule(dynamic)
    for (i = 0; i < size; i++) {
        /* Complex per-iteration work */
        int val = data[i];
        
        /* Nested switch in parallel region */
        switch (val % 6) {
            case 0:
                val = compute_hash(val, i, size);
                break;
            case 1:
                val = mix_bits(val, val >> 16, i);
                break;
            case 2:
                #pragma omp critical
                {
                    val = val * 0x9e3779b9;
                }
                break;
            case 3:
                val = (val << 3) | (val >> 29);
                break;
            case 4:
                val = val ^ 0xDEADBEEF;
                break;
            case 5:
                val = ~val + i;
                break;
        }
        
        /* Memory barrier */
        #pragma omp barrier
        
        data[i] = val;
    }
}

/* Main orchestrator function */
int main(int argc, char **argv) {
    const int sizes[] = {256, 512, 1024, 2048, 4096};
    const int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    unsigned long total_checksum = 0;
    int i, iter;
    
    /* Warm-up phase to trigger optimization heuristics */
    printf("Starting warm-up phase...\n");
    for (iter = 0; iter < 3; iter++) {
        for (i = 0; i < num_sizes; i++) {
            int size = sizes[i];
            int *matrix = (int*)malloc(size * sizeof(int));
            float *fa = (float*)malloc(size * sizeof(float));
            float *fb = (float*)malloc(size * sizeof(float));
            float *fc = (float*)malloc(size * sizeof(float));
            
            /* Initialize data */
            for (int j = 0; j < size; j++) {
                matrix[j] = j * 0x9e3779b9;
                fa[j] = j * 0.1f;
                fb[j] = j * 0.2f;
                fc[j] = j * 0.3f;
            }
            
            /* Call complex functions */
            total_checksum += process_matrix(matrix, size, iter * 100);
            vectorizable_operations(fa, fb, fc, size);
            parallel_region_work(matrix, size);
            
            free(matrix);
            free(fa);
            free(fb);
            free(fc);
        }
    }
    
    /* Timed execution phase */
    printf("Starting timed execution phase...\n");
    clock_t start = clock();
    
    for (iter = 0; iter < 10; iter++) {
        for (i = 0; i < num_sizes; i++) {
            int size = sizes[i] * (iter + 1);
            int *matrix = (int*)malloc(size * sizeof(int));
            float *fa = (float*)malloc(size * sizeof(float));
            float *fb = (float*)malloc(size * sizeof(float));
            float *fc = (float*)malloc(size * sizeof(float));
            
            /* Different initialization patterns */
            for (int j = 0; j < size; j++) {
                matrix[j] = compute_hash(j, iter, size);
                fa[j] = (j % 10) * 0.1f;
                fb[j] = (j % 7) * 0.2f;
                fc[j] = (j % 5) * 0.3f;
            }
            
            /* Stress different scheduling paths */
            if (iter % 2 == 0) {
                total_checksum += process_matrix(matrix, size, 1000);
            } else {
                #pragma omp parallel sections
                {
                    #pragma omp section
                    {
                        total_checksum += process_matrix(matrix, size/2, 500);
                    }
                    #pragma omp section
                    {
                        vectorizable_operations(fa, fb, fc, size);
                    }
                }
            }
            
            parallel_region_work(matrix, size);
            
            free(matrix);
            free(fa);
            free(fb);
            free(fc);
        }
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Total checksum: %lu\n", total_checksum);
    printf("Execution time: %.3f seconds\n", elapsed);
    
    return 0;
}
