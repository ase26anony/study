/* Test program to stress GCC's Haifa scheduler and trigger free_sched_context */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <omp.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 10000
#define MAX_DEPTH 8

/* Always inline helper functions to create complex dataflow */
static inline int __attribute__((always_inline)) 
compute_hash(int a, int b, int c) {
    /* Mix of arithmetic and bitwise operations */
    int t = a ^ (b << 3);
    t = t + (c * 0x5bd1e995);
    t = t ^ (t >> 16);
    return t;
}

static inline int __attribute__((always_inline))
barrier_op(int x) {
    /* Memory barrier via asm to create scheduling constraints */
    int result;
    asm volatile ("movl %1, %0\n\t"
                  "mfence\n\t"
                  : "=r"(result)
                  : "r"(x)
                  : "memory");
    return result;
}

/* Hot function with complex control flow */
__attribute__((hot, noinline))
void complex_control_flow(int *data, int size, int *result) {
    int i, j, k;
    volatile int counter = 0; /* Prevent too much optimization */
    
    /* Nested loops with varying iteration counts */
    for (i = 0; i < size; i++) {
        int temp = data[i];
        
        /* Deep if-else chain */
        if (temp < 0) {
            temp = compute_hash(temp, i, 1);
            if (temp & 1) {
                temp = barrier_op(temp);
                for (j = 0; j < (i % 16); j++) {
                    temp ^= (j << (temp & 0xF));
                }
            } else {
                temp = temp * 0x9e3779b9;
            }
        } else if (temp < 100) {
            temp = compute_hash(temp, i, 2);
            switch (temp % 5) {
                case 0: temp += 1; break;
                case 1: temp *= 3; break;
                case 2: temp ^= 0xFF; break;
                case 3: temp = barrier_op(temp); break;
                case 4: temp = temp >> 2; break;
            }
        } else {
            temp = compute_hash(temp, i, 3);
            /* Another nested loop */
            for (k = 0; k < (temp % 8); k++) {
                temp = (temp * 1103515245 + 12345) & 0x7FFFFFFF;
            }
        }
        
        /* Memory operation with pointer arithmetic */
        result[i] = temp;
        counter++;
    }
}

/* Function with irreducible control flow using computed goto */
__attribute__((noinline))
void irreducible_cfg(int *data, int size) {
    static void *labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4, &&L5 };
    int i = 0, state = 0;
    
    /* Computed goto creates irreducible control flow */
    void *next_label = labels[0];
    
    while (i < size) {
        goto *next_label;
        
    L0:
        data[i] = compute_hash(data[i], i, state);
        state = (state + 1) % 6;
        next_label = labels[state];
        i++;
        continue;
        
    L1:
        data[i] = barrier_op(data[i]);
        state = (state * 3 + 1) % 6;
        next_label = labels[state];
        i += 2;
        continue;
        
    L2:
        data[i] = data[i] ^ data[i-1];
        state = (state + 5) % 6;
        next_label = labels[state];
        i++;
        continue;
        
    L3:
        data[i] = data[i] * 0x9e3779b9;
        state = (state * 2) % 6;
        next_label = labels[state];
        i += 3;
        continue;
        
    L4:
        data[i] = data[i] >> (data[i] & 0x7);
        state = (state + 3) % 6;
        next_label = labels[state];
        i++;
        continue;
        
    L5:
        data[i] = ~data[i];
        state = 0;
        next_label = labels[state];
        i += 2;
        continue;
    }
}

/* Function with vectorization candidate */
__attribute__((hot))
void vectorizable_loop(int *a, int *b, int *c, int size) {
    int i;
    
    /* Simple stride-1 loop that can be vectorized */
    #pragma omp simd
    for (i = 0; i < size; i++) {
        a[i] = b[i] * c[i] + compute_hash(i, b[i], c[i]);
    }
    
    /* Another loop with carried dependency */
    for (i = 1; i < size; i++) {
        a[i] += a[i-1] * 0x5A827999;
    }
}

/* Function with OpenMP parallelization */
void parallel_region(int *data, int size) {
    int i;
    
    #pragma omp parallel for schedule(dynamic)
    for (i = 0; i < size; i++) {
        int temp = data[i];
        
        /* Complex operations inside parallel region */
        for (int j = 0; j < (i % 32); j++) {
            temp = compute_hash(temp, j, i);
            if (j % 3 == 0) {
                temp = barrier_op(temp);
            }
        }
        
        data[i] = temp;
    }
}

/* Recursive function with tail recursion */
__attribute__((noinline))
int recursive_computation(int n, int acc) {
    if (n <= 0) return acc;
    
    /* Mix of operations */
    int new_acc = compute_hash(acc, n, recursive_computation(n-1, acc));
    
    /* Conditional tail recursion */
    if (n % 2 == 0) {
        return recursive_computation(n-2, new_acc);
    } else {
        return recursive_computation(n-1, new_acc ^ 0xDEADBEEF);
    }
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
    int *result = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    if (!data1 || !data2 || !data3 || !result) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(42);
    for (i = 0; i < ARRAY_SIZE; i++) {
        data1[i] = rand();
        data2[i] = rand();
        data3[i] = rand();
        result[i] = 0;
    }
    
    printf("Starting scheduler stress test...\n");
    
    /* Warm-up phase */
    printf("Warm-up phase...\n");
    start = clock();
    for (j = 0; j < ITERATIONS/10; j++) {
        complex_control_flow(data1, ARRAY_SIZE/4, result);
    }
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Warm-up completed in %.2f seconds\n", cpu_time_used);
    
    /* Main test phase with different patterns */
    printf("\nMain test phase...\n");
    
    /* Test 1: Complex control flow */
    printf("Test 1: Complex control flow\n");
    start = clock();
    for (j = 0; j < ITERATIONS; j++) {
        complex_control_flow(data1, ARRAY_SIZE, result);
    }
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Completed in %.2f seconds\n", cpu_time_used);
    
    /* Test 2: Irreducible CFG */
    printf("\nTest 2: Irreducible control flow graph\n");
    start = clock();
    for (j = 0; j < ITERATIONS/2; j++) {
        irreducible_cfg(data2, ARRAY_SIZE);
    }
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Completed in %.2f seconds\n", cpu_time_used);
    
    /* Test 3: Vectorization */
    printf("\nTest 3: Vectorizable loops\n");
    start = clock();
    for (j = 0; j < ITERATIONS*2; j++) {
        vectorizable_loop(result, data1, data2, ARRAY_SIZE);
    }
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Completed in %.2f seconds\n", cpu_time_used);
    
    /* Test 4: Parallel region */
    printf("\nTest 4: OpenMP parallel region\n");
    start = clock();
    for (j = 0; j < ITERATIONS/4; j++) {
        parallel_region(data3, ARRAY_SIZE);
    }
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Completed in %.2f seconds\n", cpu_time_used);
    
    /* Test 5: Recursive computation */
    printf("\nTest 5: Recursive computation\n");
    start = clock();
    int recursive_result = 0;
    for (j = 0; j < ITERATIONS*10; j++) {
        recursive_result ^= recursive_computation(MAX_DEPTH, j);
    }
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Completed in %.2f seconds\n", cpu_time_used);
    
    /* Compute final checksum for verification */
    printf("\nComputing final checksum...\n");
    unsigned long long checksum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum += (unsigned long long)result[i];
        checksum += (unsigned long long)data1[i];
        checksum += (unsigned long long)data2[i];
        checksum += (unsigned long long)data3[i];
    }
    checksum ^= (unsigned long long)recursive_result;
    
    printf("\nFinal checksum: 0x%016llX\n", checksum);
    printf("Test completed successfully.\n");
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    free(result);
    
    return 0;
}
