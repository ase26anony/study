/* Test program to stress GCC's Haifa scheduler and trigger free_sched_context */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <omp.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100000
#define MAX_DEPTH 8

/* Always inline helper functions to create complex dataflow */
static inline int __attribute__((always_inline)) 
compute_hash(int a, int b, int c) {
    /* Mix of arithmetic and bitwise operations */
    int t = a ^ (b << 3);
    t = t + (c * 0x5bd1e995);
    t = t ^ (t >> 16);
    t = t * 0x27d4eb2d;
    return t ^ (t >> 15);
}

static inline int __attribute__((always_inline))
scramble_bits(int x, int mask) {
    /* Complex bit manipulation with dependencies */
    int r = x;
    r = (r & 0x55555555) << 1 | (r & 0xAAAAAAAA) >> 1;
    r = (r & 0x33333333) << 2 | (r & 0xCCCCCCCC) >> 2;
    r = (r & 0x0F0F0F0F) << 4 | (r & 0xF0F0F0F0) >> 4;
    r = (r & 0x00FF00FF) << 8 | (r & 0xFF00FF00) >> 8;
    r = (r & 0x0000FFFF) << 16 | (r & 0xFFFF0000) >> 16;
    return r ^ mask;
}

/* Hot function with complex control flow */
__attribute__((hot))
void complex_control_flow(int *data, int size, int *result) {
    int i, j, k;
    volatile int barrier = 0; /* Force memory dependencies */
    
    /* Nested loops with varying iteration counts */
    for (i = 0; i < size; i++) {
        int acc = data[i];
        
        /* Deep if-else chain */
        if (i % 3 == 0) {
            acc = compute_hash(acc, i, size);
            if (i % 7 == 0) {
                acc = scramble_bits(acc, 0xDEADBEEF);
                for (j = 0; j < (i % 5) + 1; j++) {
                    acc += data[(i + j) % size];
                }
            } else if (i % 11 == 0) {
                /* Switch statement with multiple cases */
                switch (i % 4) {
                    case 0: acc = acc * 2; break;
                    case 1: acc = acc / 3; break;
                    case 2: acc = acc | 0xFF; break;
                    case 3: acc = acc & 0xFFFF; break;
                }
            } else {
                acc = acc ^ 0x12345678;
            }
        } else if (i % 5 == 0) {
            /* While loop with computed goto (irreducible CFG) */
            int counter = 0;
            void *labels[] = { &&L1, &&L2, &&L3, &&L4 };
            
            goto *labels[i % 4];
            
            L1:
                acc += 1;
                if (counter++ < 3) goto L2;
                else goto L4;
            L2:
                acc *= 2;
                if (counter++ < 3) goto L3;
                else goto L1;
            L3:
                acc -= 1;
                if (counter++ < 3) goto L4;
                else goto L2;
            L4:
                acc ^= 0xFF;
                if (counter++ < 3) goto L1;
                /* fall through */
        } else {
            /* Do-while with memory operations */
            int k = 0;
            do {
                /* Memory dependency chain */
                barrier = acc;
                asm volatile("" : : "r"(barrier) : "memory");
                acc = data[(i + k) % size] + barrier;
                k++;
            } while (k < (i % 8) + 1);
        }
        
        /* Artificial scheduling barrier */
        asm volatile("" : : : "memory");
        
        result[i] = acc;
    }
}

/* Function with vectorization candidate */
__attribute__((hot))
void vectorizable_loop(int *a, int *b, int *c, int size) {
    int i;
    
    #pragma omp simd
    for (i = 0; i < size; i++) {
        /* Simple stride-1 operations for vectorization */
        int t = a[i] * 3 + 7;
        t = t ^ b[i];
        t = t * 0x9e3779b9;
        c[i] = t + (t >> 13);
    }
    
    /* Additional loop with carried dependency */
    for (i = 1; i < size; i++) {
        c[i] = c[i] + c[i-1] * 2;
    }
}

/* Function with OpenMP parallelization */
void parallel_region(int *data, int size, int depth) {
    int i;
    
    if (depth >= MAX_DEPTH) return;
    
    #pragma omp parallel for private(i) schedule(dynamic, 4)
    for (i = 0; i < size; i += 4) {
        int j, sum = 0;
        
        /* Nested loop inside parallel region */
        for (j = 0; j < 4 && (i + j) < size; j++) {
            int idx = i + j;
            
            /* Complex computation with multiple dependencies */
            int val = data[idx];
            val = compute_hash(val, idx, size);
            val = scramble_bits(val, depth * 0x11111111);
            
            /* Memory operation with pointer arithmetic */
            int *ptr = &data[idx];
            *ptr = val;
            
            sum += val;
        }
        
        /* Reduction-like operation */
        #pragma omp atomic
        data[i % size] ^= sum;
    }
    
    /* Recursive call for deeper nesting */
    if (size > 16) {
        parallel_region(data, size / 2, depth + 1);
    }
}

/* Mixed workload function */
__attribute__((hot))
void mixed_workload(int *array, int size, int iterations) {
    int i, j;
    int *temp = malloc(size * sizeof(int));
    
    for (i = 0; i < iterations; i++) {
        /* Alternate between different patterns */
        switch (i % 4) {
            case 0:
                complex_control_flow(array, size, temp);
                break;
            case 1:
                vectorizable_loop(array, temp, array, size);
                break;
            case 2:
                parallel_region(array, size, 0);
                break;
            case 3:
                /* Manual unrolled loop with asm barriers */
                for (j = 0; j < size; j += 4) {
                    int t0 = array[j];
                    int t1 = array[j+1];
                    int t2 = array[j+2];
                    int t3 = array[j+3];
                    
                    asm volatile("" : : : "memory");
                    
                    t0 = compute_hash(t0, j, size);
                    t1 = compute_hash(t1, j+1, size);
                    t2 = compute_hash(t2, j+2, size);
                    t3 = compute_hash(t3, j+3, size);
                    
                    asm volatile("" : : : "memory");
                    
                    array[j] = t0 ^ t1;
                    array[j+1] = t1 ^ t2;
                    array[j+2] = t2 ^ t3;
                    array[j+3] = t3 ^ t0;
                }
                break;
        }
        
        /* Periodic array reordering */
        if (i % 100 == 0) {
            for (j = 0; j < size / 2; j++) {
                int tmp = array[j];
                array[j] = array[size - j - 1];
                array[size - j - 1] = tmp;
            }
        }
    }
    
    free(temp);
}

int main(int argc, char **argv) {
    int *data1, *data2, *data3;
    int i, size;
    clock_t start, end;
    unsigned long long checksum = 0;
    
    /* Test different sizes to trigger different optimization paths */
    int sizes[] = {64, 128, 256, 512, 1024, 2048};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    
    /* Warm-up phase */
    printf("Starting warm-up phase...\n");
    data1 = malloc(sizes[0] * sizeof(int));
    data2 = malloc(sizes[0] * sizeof(int));
    
    for (i = 0; i < sizes[0]; i++) {
        data1[i] = i * 3 + 7;
        data2[i] = i ^ 0x55;
    }
    
    mixed_workload(data1, sizes[0], 1000);
    
    free(data1);
    free(data2);
    
    /* Main test phase with different array sizes */
    printf("Starting main test phase...\n");
    
    for (int s = 0; s < num_sizes; s++) {
        size = sizes[s];
        printf("Testing with size %d\n", size);
        
        data1 = malloc(size * sizeof(int));
        data2 = malloc(size * sizeof(int));
        data3 = malloc(size * sizeof(int));
        
        /* Initialize with pattern */
        for (i = 0; i < size; i++) {
            data1[i] = i;
            data2[i] = size - i;
            data3[i] = i * i;
        }
        
        start = clock();
        
        /* Run complex functions */
        complex_control_flow(data1, size, data2);
        vectorizable_loop(data1, data2, data3, size);
        parallel_region(data3, size, 0);
        mixed_workload(data1, size, ITERATIONS / (size / 64 + 1));
        
        end = clock();
        
        /* Compute checksum for verification */
        for (i = 0; i < size; i++) {
            checksum += (unsigned long long)data1[i];
            checksum += (unsigned long long)data2[i];
            checksum += (unsigned long long)data3[i];
        }
        
        printf("  Size %d: %f seconds, checksum = 0x%016llx\n", 
               size, (double)(end - start) / CLOCKS_PER_SEC, checksum);
        
        free(data1);
        free(data2);
        free(data3);
    }
    
    /* Final verification */
    printf("\nFinal checksum: 0x%016llx\n", checksum);
    printf("Test completed successfully.\n");
    
    return 0;
}
