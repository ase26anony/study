/* Complex control flow patterns to stress GCC's Haifa scheduler */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000
#define MAX_DEPTH 8

/* Helper functions for inlining */
static inline int __attribute__((always_inline)) 
compute_hash(int a, int b) {
    return (a ^ (b << 1)) + (a >> 3) * b;
}

static inline int __attribute__((always_inline))
process_element(int x, int *mask) {
    int result = x;
    asm volatile ("" : "+r" (result) : : "memory");
    result = (result & *mask) | (~result & ~*mask);
    return result ^ 0x5A5A5A5A;
}

/* Hot function with complex control flow */
__attribute__((hot))
void complex_control_flow(int *data, int size, int threshold) {
    int i, j, k;
    volatile int counter = 0; /* Prevent optimization */
    
    /* Nested loops with varying iteration counts */
    for (i = 0; i < size; i += 2) {
        if (data[i] > threshold) {
            /* Deep if-else chain */
            if (data[i] & 0x01) {
                data[i] = compute_hash(data[i], i);
            } else if (data[i] & 0x02) {
                data[i] = data[i] * 3 + 1;
            } else if (data[i] & 0x04) {
                data[i] = data[i] >> 2;
            } else if (data[i] & 0x08) {
                data[i] = data[i] << 1;
            } else {
                data[i] = ~data[i];
            }
            
            /* Inner loop with carried dependency */
            for (j = 0; j < (i % 16); j++) {
                data[i] = process_element(data[i], &threshold);
                counter++;
            }
        } else {
            /* Switch statement with multiple cases */
            switch (data[i] % 7) {
                case 0:
                    data[i] += size;
                    break;
                case 1:
                    data[i] -= i;
                    break;
                case 2:
                    data[i] *= 2;
                    break;
                case 3:
                    data[i] /= 2;
                    break;
                case 4:
                    data[i] = data[i] & 0xFF;
                    break;
                case 5:
                    data[i] = data[i] | 0xFF00;
                    break;
                case 6:
                    data[i] = data[i] ^ 0xFFFF;
                    break;
            }
        }
        
        /* While loop with computed goto (irreducible CFG) */
        int loop_ctr = data[i] % 5;
        void *labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4 };
        
        if (loop_ctr > 0) {
            goto *labels[loop_ctr];
        }
        
        L0: data[i] += 1;
        L1: data[i] *= 2;
        L2: data[i] -= 3;
        L3: data[i] ^= 0xAA;
        L4: data[i] |= 0x55;
        
        /* Memory operations with pointer arithmetic */
        int *ptr = &data[i];
        for (k = 0; k < 4 && (i + k) < size; k++) {
            *(ptr + k) = compute_hash(*(ptr + k), k);
        }
    }
}

/* Function with vectorization candidate */
__attribute__((hot))
void vectorizable_loop(int *a, int *b, int *c, int n) {
    int i;
    /* Simple stride-1 array operations */
    #pragma omp simd
    for (i = 0; i < n; i++) {
        a[i] = b[i] + c[i];
        a[i] = a[i] * 2 - b[i];
        /* Artificial dependency chain */
        if (i > 0) {
            a[i] += a[i-1] % 256;
        }
    }
}

/* Function with OpenMP parallelization */
void parallel_region(int *data, int size) {
    int i;
    #pragma omp parallel for schedule(dynamic, 16)
    for (i = 0; i < size; i++) {
        int temp = data[i];
        /* Complex operation mix */
        temp = (temp << 3) | (temp >> 29);  /* Rotate left */
        temp = temp ^ (temp * 13);
        temp = process_element(temp, &i);
        data[i] = temp;
        
        /* Additional computation to increase ILP */
        for (int j = 0; j < 8; j++) {
            data[i] += compute_hash(data[i], j);
        }
    }
}

/* Recursive function with mixed operations */
int recursive_computation(int depth, int value, int *memo) {
    if (depth <= 0) return value;
    if (memo[depth] != 0) return memo[depth];
    
    int result;
    if (value % 3 == 0) {
        result = recursive_computation(depth - 1, value * 2, memo);
        result = compute_hash(result, depth);
    } else if (value % 3 == 1) {
        result = recursive_computation(depth - 2, value / 2, memo);
        result = process_element(result, &depth);
    } else {
        result = recursive_computation(depth - 1, value + 1, memo);
        result = recursive_computation(depth - 2, result - 1, memo);
    }
    
    /* Memory barrier */
    asm volatile ("" : : : "memory");
    
    memo[depth] = result;
    return result;
}

/* Main orchestrator */
int main() {
    int i, j;
    clock_t start, end;
    double cpu_time_used;
    
    /* Allocate and initialize data */
    int *data1 = (int *)malloc(ARRAY_SIZE * sizeof(int));
    int *data2 = (int *)malloc(ARRAY_SIZE * sizeof(int));
    int *data3 = (int *)malloc(ARRAY_SIZE * sizeof(int));
    int *memo = (int *)calloc(MAX_DEPTH + 1, sizeof(int));
    
    if (!data1 || !data2 || !data3 || !memo) {
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
    
    /* Warm-up phase */
    printf("Warm-up phase...\n");
    start = clock();
    for (j = 0; j < ITERATIONS / 10; j++) {
        complex_control_flow(data1, ARRAY_SIZE, 500);
    }
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Warm-up completed in %.3f seconds\n", cpu_time_used);
    
    /* Main test phase with different patterns */
    printf("\nMain test phase...\n");
    
    /* Pattern 1: Complex control flow */
    start = clock();
    for (j = 0; j < ITERATIONS; j++) {
        complex_control_flow(data1, ARRAY_SIZE, j % 1000);
    }
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Complex control flow: %.3f seconds\n", cpu_time_used);
    
    /* Pattern 2: Vectorization attempts */
    start = clock();
    for (j = 0; j < ITERATIONS * 2; j++) {
        vectorizable_loop(data1, data2, data3, ARRAY_SIZE);
    }
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Vectorizable loops: %.3f seconds\n", cpu_time_used);
    
    /* Pattern 3: Parallel regions */
    start = clock();
    for (j = 0; j < ITERATIONS / 5; j++) {
        parallel_region(data2, ARRAY_SIZE);
    }
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Parallel regions: %.3f seconds\n", cpu_time_used);
    
    /* Pattern 4: Recursive computations */
    start = clock();
    int recursive_result = 0;
    for (j = 0; j < ITERATIONS * 10; j++) {
        memset(memo, 0, (MAX_DEPTH + 1) * sizeof(int));
        recursive_result += recursive_computation(MAX_DEPTH, j % 100, memo);
    }
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Recursive computations: %.3f seconds\n", cpu_time_used);
    
    /* Pattern 5: Mixed patterns */
    start = clock();
    for (j = 0; j < ITERATIONS / 2; j++) {
        complex_control_flow(data3, ARRAY_SIZE, 300);
        vectorizable_loop(data1, data2, data3, ARRAY_SIZE);
        parallel_region(data1, ARRAY_SIZE);
        
        /* Nested loop with heavy computation */
        for (int k = 0; k < 10; k++) {
            for (i = 0; i < ARRAY_SIZE; i += 16) {
                int sum = 0;
                for (int m = 0; m < 16 && (i + m) < ARRAY_SIZE; m++) {
                    sum += compute_hash(data1[i + m], data2[i + m]);
                    sum = process_element(sum, &data3[i + m]);
                }
                data1[i] = sum;
            }
        }
    }
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Mixed patterns: %.3f seconds\n", cpu_time_used);
    
    /* Compute final checksum for verification */
    unsigned long long checksum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum += (unsigned long long)data1[i];
        checksum += (unsigned long long)data2[i];
        checksum += (unsigned long long)data3[i];
        checksum = (checksum << 13) | (checksum >> 51); /* Rotate */
    }
    
    printf("\nFinal checksum: 0x%016llX\n", checksum);
    printf("Recursive result: %d\n", recursive_result);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    free(memo);
    
    return 0;
}
