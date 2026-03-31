/* Test program to trigger free_sched_context in GCC's Haifa scheduler */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000
#define MAX_DEPTH 8

/* Always-inline helper functions to create complex dataflow */
static inline int __attribute__((always_inline)) 
compute_hash(int a, int b, int c) {
    /* Mix of arithmetic and bitwise operations */
    int h = a ^ b;
    h = (h << 5) | (h >> 27);
    h += c * 0x9e3779b9;
    h ^= (h >> 16);
    return h;
}

static inline int __attribute__((always_inline))
scramble_bits(int x) {
    /* Complex bit manipulation */
    x = ((x & 0xAAAAAAAA) >> 1) | ((x & 0x55555555) << 1);
    x = ((x & 0xCCCCCCCC) >> 2) | ((x & 0x33333333) << 2);
    x = ((x & 0xF0F0F0F0) >> 4) | ((x & 0x0F0F0F0F) << 4);
    x = ((x & 0xFF00FF00) >> 8) | ((x & 0x00FF00FF) << 8);
    return (x >> 16) | (x << 16);
}

/* Hot function with complex control flow */
__attribute__((hot))
void complex_control_flow(int *data, int size, int depth) {
    int i, j, k;
    volatile int barrier = 0; /* Prevent optimization */
    
    /* Nested loops with varying iteration counts */
    for (i = 0; i < size; i++) {
        /* Deep if-else chain */
        if (i % 3 == 0) {
            data[i] = compute_hash(data[i], i, depth);
        } else if (i % 3 == 1) {
            /* Switch statement with multiple cases */
            switch (i % 5) {
                case 0:
                    data[i] = scramble_bits(data[i]);
                    break;
                case 1:
                    data[i] = data[i] * 3 + 1;
                    break;
                case 2:
                    data[i] = data[i] ^ 0xDEADBEEF;
                    break;
                case 3:
                    data[i] = (data[i] << 1) | (data[i] >> 31);
                    break;
                case 4:
                    data[i] = ~data[i];
                    break;
            }
        } else {
            /* Memory operations with pointer arithmetic */
            int *ptr = &data[i];
            *ptr = (*ptr + *(ptr + (i % 8))) * 2;
        }
        
        /* Inner loop with carried dependency */
        int acc = data[i];
        for (j = 0; j < depth; j++) {
            acc = compute_hash(acc, j, i);
            /* Artificial scheduling barrier */
            asm volatile("" : "+r" (acc) : : "memory");
        }
        data[i] = acc;
        
        /* Computed goto for irreducible control flow */
        if (depth > 2) {
            static void *labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4 };
            int idx = data[i] % 5;
            goto *labels[idx];
            
            L0:
                data[i] += 1;
                goto end;
            L1:
                data[i] *= 2;
                goto end;
            L2:
                data[i] ^= 0x12345678;
                goto end;
            L3:
                data[i] = (data[i] << 3) | (data[i] >> 29);
                goto end;
            L4:
                data[i] = ~data[i] + 1;
                goto end;
            end:;
        }
    }
    
    /* Memory clobber to force scheduling constraints */
    asm volatile("" : : : "memory");
}

/* Function with vectorization candidate */
__attribute__((hot))
void vectorizable_loop(int *a, int *b, int *c, int size) {
    int i;
    
    /* Simple stride-1 array operations - good for vectorization */
    #pragma omp simd
    for (i = 0; i < size; i++) {
        a[i] = b[i] * c[i] + i;
    }
    
    /* Another loop with reduction */
    int sum = 0;
    #pragma omp simd reduction(+:sum)
    for (i = 0; i < size; i++) {
        sum += a[i] * b[i];
        /* Mix in some conditionals */
        if (sum < 0) {
            sum = -sum;
        }
    }
    
    /* Use the sum to prevent dead code elimination */
    a[0] = sum % 1000;
}

/* Function with OpenMP parallel region */
void parallel_region(int *data, int size) {
    int i;
    
    #pragma omp parallel for schedule(dynamic)
    for (i = 0; i < size; i++) {
        /* Independent but complex computation */
        int val = data[i];
        for (int j = 0; j < 10; j++) {
            val = compute_hash(val, j, i);
            val = scramble_bits(val);
        }
        data[i] = val;
    }
}

/* Recursive function with tail calls */
int __attribute__((noinline))
recursive_computation(int n, int depth) {
    if (depth <= 0 || n <= 1) {
        return n;
    }
    
    /* Mix of recursive paths */
    if (n % 2 == 0) {
        int a = recursive_computation(n / 2, depth - 1);
        int b = recursive_computation(n - 1, depth - 1);
        return compute_hash(a, b, depth);
    } else {
        int a = recursive_computation(n * 3 + 1, depth - 1);
        int b = recursive_computation(n - 2, depth - 1);
        return scramble_bits(a ^ b);
    }
}

/* Main orchestrator function */
int main() {
    int i, j;
    long checksum = 0;
    clock_t start, end;
    
    /* Allocate and initialize data */
    int *data1 = (int *)malloc(ARRAY_SIZE * sizeof(int));
    int *data2 = (int *)malloc(ARRAY_SIZE * sizeof(int));
    int *data3 = (int *)malloc(ARRAY_SIZE * sizeof(int));
    
    if (!data1 || !data2 || !data3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    srand(42);
    for (i = 0; i < ARRAY_SIZE; i++) {
        data1[i] = rand();
        data2[i] = rand();
        data3[i] = rand();
    }
    
    printf("Starting scheduler stress test...\n");
    
    /* Warm-up phase - trigger optimization heuristics */
    printf("Warm-up phase...\n");
    for (i = 0; i < ITERATIONS / 10; i++) {
        complex_control_flow(data1, ARRAY_SIZE / 4, 2);
        vectorizable_loop(data2, data1, data3, ARRAY_SIZE / 4);
    }
    
    /* Main test phase with varying parameters */
    printf("Main test phase...\n");
    start = clock();
    
    for (j = 0; j < 5; j++) {
        /* Vary array sizes and depths to trigger different scheduling paths */
        int size = ARRAY_SIZE >> j;
        int depth = MAX_DEPTH - j;
        
        for (i = 0; i < ITERATIONS; i++) {
            /* Mix different patterns */
            complex_control_flow(data1, size, depth);
            vectorizable_loop(data2, data1, data3, size);
            
            if (i % 100 == 0) {
                parallel_region(data3, size);
            }
            
            if (i % 50 == 0) {
                int result = recursive_computation(100 + i % 100, depth);
                checksum += result;
            }
            
            /* Memory barrier to prevent reordering across iterations */
            asm volatile("" : : : "memory");
        }
    }
    
    end = clock();
    
    /* Compute final checksum for verification */
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum += data1[i] + data2[i] + data3[i];
    }
    
    printf("Test completed.\n");
    printf("Checksum: %ld\n", checksum);
    printf("Time: %.2f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    
    return 0;
}
