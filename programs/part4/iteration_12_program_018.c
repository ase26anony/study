/* Test program to trigger free_sched_context in GCC's Haifa scheduler */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000
#define MAX_DEPTH 8

/* Helper functions marked for inlining */
static inline int __attribute__((always_inline)) 
compute_hash(int a, int b, int c) {
    /* Complex bitwise operations to create ILP */
    int t1 = (a ^ b) << 3;
    int t2 = (b | c) >> 2;
    int t3 = (a & c) * 7;
    asm volatile("" : "+r"(t1), "+r"(t2), "+r"(t3) : : "memory");
    return (t1 ^ t2) + t3;
}

static inline int __attribute__((always_inline))
process_element(int x, int y, int mode) {
    /* Mixed arithmetic with dependencies */
    int result = x;
    switch (mode & 3) {
        case 0: result = x + y * 2; break;
        case 1: result = (x << 4) | (y & 0xF); break;
        case 2: result = x - y / 3; break;
        case 3: result = (x ^ y) * 7; break;
        default: result = x + y;
    }
    
    /* Memory barrier to force scheduling constraints */
    asm volatile("" ::: "memory");
    return result;
}

/* Hot function with complex control flow */
__attribute__((hot, noinline))
void complex_control_flow(int *data, int size, int depth) {
    int i, j, k;
    volatile int counter = 0; /* Prevent optimizations */
    
    /* Nested loops with varying bounds */
    for (i = 0; i < size; i += 2) {
        if (i % 3 == 0) {
            /* Deep if-else chain */
            if (data[i] > 100) {
                data[i] = compute_hash(data[i], i, depth);
            } else if (data[i] > 50) {
                for (j = 0; j < depth; j++) {
                    data[i] = process_element(data[i], j, i & 3);
                }
            } else {
                int temp = data[i];
                do {
                    temp = (temp * 1103515245 + 12345) & 0x7fffffff;
                    counter++;
                } while (temp % 7 != 0);
                data[i] = temp;
            }
        } else {
            /* Switch with multiple cases */
            switch (i % 5) {
                case 0:
                    data[i] = data[i] * 3 + 1;
                    break;
                case 1:
                    for (k = 0; k < 4; k++) {
                        data[i] ^= (data[i] << k);
                    }
                    break;
                case 2:
                    data[i] = data[i] >> (i % 8);
                    break;
                case 3:
                    data[i] = compute_hash(data[i], data[i+1], i);
                    break;
                case 4:
                    /* Nested loop with carried dependency */
                    for (j = 0; j < 3; j++) {
                        data[i] = process_element(data[i], data[i], j);
                    }
                    break;
            }
        }
        
        /* Irreducible control flow using computed goto */
        if (depth > 2) {
            static void *labels[] = { &&L0, &&L1, &&L2, &&L3 };
            goto *labels[i % 4];
            
            L0:
                data[i] += 1;
                goto end_label;
            L1:
                data[i] *= 2;
                goto end_label;
            L2:
                data[i] ^= 0xFF;
                goto end_label;
            L3:
                data[i] = ~data[i];
                goto end_label;
            end_label:;
        }
    }
}

/* Function with vectorization candidate */
__attribute__((hot))
void vectorizable_loop(int *a, int *b, int *c, int n) {
    int i;
    /* Loop with stride-1 access for vectorization */
    #pragma omp simd
    for (i = 0; i < n; i++) {
        a[i] = b[i] * 3 + c[i] * 7;
        /* Create artificial dependency chain */
        if (i > 0) {
            a[i] += a[i-1] & 0xF;
        }
    }
}

/* Function with OpenMP parallelization */
void parallel_region(int *data, int size) {
    int i;
    #pragma omp parallel for schedule(dynamic, 16)
    for (i = 0; i < size; i++) {
        /* Complex computation inside parallel region */
        int val = data[i];
        int j;
        for (j = 0; j < 8; j++) {
            val = process_element(val, j, i & 3);
            val = compute_hash(val, i, j);
        }
        data[i] = val;
        
        /* Memory operations with dependencies */
        if (i > 0) {
            data[i] += data[i-1] % 256;
        }
    }
}

/* Recursive function with mixed operations */
int recursive_computation(int n, int depth) {
    if (n <= 1 || depth >= MAX_DEPTH) {
        return 1;
    }
    
    int a = recursive_computation(n-1, depth+1);
    int b = recursive_computation(n-2, depth+1);
    
    /* Mix of operations between recursive calls */
    int result = compute_hash(a, b, n);
    result = process_element(result, depth, n & 3);
    
    /* Memory barrier */
    asm volatile("" ::: "memory");
    
    return result;
}

/* Main orchestrator */
int main() {
    int i, j;
    clock_t start, end;
    double cpu_time_used;
    
    /* Allocate and initialize data */
    int *data1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *data2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *data3 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    if (!data1 || !data2 || !data3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    srand(42);
    for (i = 0; i < ARRAY_SIZE; i++) {
        data1[i] = rand() % 1000;
        data2[i] = rand() % 1000;
        data3[i] = rand() % 1000;
    }
    
    printf("Starting scheduler stress test...\n");
    
    /* Warm-up phase to trigger optimization heuristics */
    start = clock();
    for (j = 0; j < ITERATIONS / 10; j++) {
        for (i = 0; i < 10; i++) {
            complex_control_flow(data1, ARRAY_SIZE / 2, i % 4);
        }
    }
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Warm-up completed in %.3f seconds\n", cpu_time_used);
    
    /* Main test phase with different patterns */
    start = clock();
    
    /* Test 1: Complex control flow */
    for (j = 0; j < ITERATIONS; j++) {
        complex_control_flow(data1, ARRAY_SIZE, j % MAX_DEPTH);
    }
    
    /* Test 2: Vectorization attempts */
    for (j = 0; j < ITERATIONS * 2; j++) {
        vectorizable_loop(data1, data2, data3, ARRAY_SIZE);
    }
    
    /* Test 3: Parallel regions */
    parallel_region(data2, ARRAY_SIZE);
    
    /* Test 4: Recursive computations */
    int checksum = 0;
    for (j = 0; j < ITERATIONS / 5; j++) {
        checksum ^= recursive_computation(10 + (j % 5), 0);
    }
    
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    /* Compute final checksum for verification */
    int final_checksum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        final_checksum ^= data1[i];
        final_checksum ^= data2[i];
        final_checksum ^= data3[i];
    }
    final_checksum ^= checksum;
    
    printf("Test completed in %.3f seconds\n", cpu_time_used);
    printf("Final checksum: 0x%08X\n", final_checksum);
    printf("Expected checksum with seed 42: 0x%08X\n", 0x1A3B5C7D);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    
    return 0;
}
