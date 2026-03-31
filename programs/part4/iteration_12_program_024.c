/* Test program to trigger free_sched_context in GCC's Haifa scheduler */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <omp.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000
#define MAX_DEPTH 8

/* Always inline helper functions to create complex dataflow */
static inline int __attribute__((always_inline)) 
compute_hash(int x, int y) {
    /* Mix of arithmetic and bitwise ops */
    return ((x << 5) ^ y) + (x >> 3) * 17;
}

static inline int __attribute__((always_inline))
scramble_bits(int val) {
    /* Complex bit manipulation */
    val = (val ^ (val >> 16)) * 0x45d9f3b;
    val = (val ^ (val >> 16)) * 0x45d9f3b;
    val = val ^ (val >> 16);
    return val;
}

/* Hot function with complex control flow */
__attribute__((hot))
void complex_control_flow(int *data, int size, int depth) {
    volatile int barrier = 0; /* Prevent optimization */
    int i, j, k;
    
    /* Nested loops with varying iteration counts */
    for (i = 0; i < size; i += 2) {
        if (i % 3 == 0) {
            /* Deep if-else chain */
            if (data[i] < 100) {
                data[i] = compute_hash(data[i], i);
            } else if (data[i] < 500) {
                data[i] = scramble_bits(data[i]);
            } else if (data[i] < 1000) {
                data[i] = data[i] * 3 + 7;
            } else {
                /* Switch statement with multiple cases */
                switch (data[i] % 7) {
                    case 0: data[i] = data[i] >> 1; break;
                    case 1: data[i] = data[i] << 1; break;
                    case 2: data[i] = data[i] | 0xFF; break;
                    case 3: data[i] = data[i] & 0xFFFF; break;
                    case 4: data[i] = data[i] ^ 0xAAAA; break;
                    case 5: data[i] = data[i] + data[i]; break;
                    case 6: data[i] = data[i] - data[i] / 2; break;
                }
            }
        }
        
        /* Memory operations with pointer arithmetic */
        int *ptr = &data[i];
        for (j = 0; j < 4 && (i + j) < size; j++) {
            ptr[j] = compute_hash(ptr[j], j);
            
            /* Artificial scheduling barrier */
            asm volatile("" ::: "memory");
        }
        
        /* Computed goto for irreducible control flow */
        if (depth > 0 && (i % 17 == 0)) {
            static void *labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4 };
            int idx = data[i] % 5;
            goto *labels[idx];
            
            L0: data[i] += 1; goto end;
            L1: data[i] *= 2; goto end;
            L2: data[i] = ~data[i]; goto end;
            L3: data[i] = data[i] << 4; goto end;
            L4: data[i] = data[i] >> 4; goto end;
            end:;
        }
        
        /* Another nested loop with carried dependency */
        int acc = data[i];
        for (k = 0; k < 8; k++) {
            acc = compute_hash(acc, k);
            /* Memory clobber to force scheduling constraints */
            asm volatile("" : "+r"(acc) : : "memory");
        }
        data[i] = acc;
        
        barrier = i; /* Use volatile to prevent dead code elimination */
    }
}

/* Function with vectorization candidate */
__attribute__((hot))
void vectorizable_loop(int *a, int *b, int *c, int size) {
    int i;
    
    /* Simple stride-1 loop for auto-vectorization */
    #pragma omp simd
    for (i = 0; i < size; i++) {
        a[i] = b[i] * 3 + c[i] * 7;
    }
    
    /* Loop with reduction for parallelization */
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (i = 0; i < size; i++) {
        sum += a[i] * b[i];
    }
    
    /* Use sum to prevent optimization */
    a[0] = sum % 1000;
}

/* Function with tight inner loop and carried dependencies */
__attribute__((hot))
void tight_inner_loop(int *data, int size) {
    int i, j;
    
    for (i = 1; i < size - 1; i++) {
        /* Carried dependency chain */
        int temp = data[i - 1];
        
        /* Many independent operations */
        temp = temp * 3;
        temp = temp ^ 0x55AA55AA;
        temp = temp + i;
        temp = scramble_bits(temp);
        temp = temp >> 2;
        temp = temp * 7;
        temp = temp & 0xFFFF;
        
        /* Memory operation with dependency */
        data[i] = temp + data[i + 1];
        
        /* Mix of operations in basic block */
        if (i % 2 == 0) {
            data[i] = compute_hash(data[i], temp);
        } else {
            data[i] = scramble_bits(data[i]);
        }
    }
}

/* Function with switch and deep nesting */
void switch_nesting(int *data, int size, int mode) {
    int i;
    
    for (i = 0; i < size; i++) {
        switch (mode) {
            case 0:
                if (data[i] > 0) {
                    for (int j = 0; j < 3; j++) {
                        data[i] = compute_hash(data[i], j);
                        while (data[i] % 2 == 0) {
                            data[i] >>= 1;
                        }
                    }
                }
                break;
                
            case 1:
                do {
                    data[i] = scramble_bits(data[i]);
                    if (data[i] % 3 == 0) break;
                    data[i] += i;
                } while (data[i] < 1000);
                break;
                
            case 2:
                for (int k = 0; k < 5; k++) {
                    if (k % 2 == 0) {
                        data[i] = data[i] * 2 - 1;
                    } else {
                        data[i] = (data[i] + k) ^ 0xFF;
                    }
                }
                break;
                
            default:
                /* Complex expression with multiple operations */
                data[i] = ((data[i] << 3) | (data[i] >> 29)) + 
                          ((data[i] * 13) ^ (data[i] / 3));
                break;
        }
    }
}

/* Main orchestrator */
int main() {
    int i, iter;
    clock_t start, end;
    double total_time = 0;
    
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
        data1[i] = rand() % 10000;
        data2[i] = rand() % 10000;
        data3[i] = rand() % 10000;
    }
    
    printf("Starting scheduler stress test...\n");
    
    /* Warm-up phase to trigger optimization heuristics */
    printf("Warm-up phase...\n");
    for (iter = 0; iter < ITERATIONS / 10; iter++) {
        complex_control_flow(data1, ARRAY_SIZE / 2, 2);
        vectorizable_loop(data2, data1, data3, ARRAY_SIZE / 4);
    }
    
    /* Main timed execution with varying patterns */
    printf("Main execution phase...\n");
    start = clock();
    
    for (iter = 0; iter < ITERATIONS; iter++) {
        /* Alternate between different functions and sizes */
        int size_variant = ARRAY_SIZE / ((iter % 4) + 1);
        int depth_variant = iter % MAX_DEPTH;
        
        /* Call functions with complex control flow */
        complex_control_flow(data1, size_variant, depth_variant);
        
        /* Vectorization attempts */
        vectorizable_loop(data2, data1, data3, size_variant);
        
        /* Tight loops with dependencies */
        tight_inner_loop(data3, size_variant);
        
        /* Switch nesting with different modes */
        switch_nesting(data1, size_variant / 2, iter % 4);
        
        /* Mix all results */
        for (i = 0; i < size_variant && i < ARRAY_SIZE; i++) {
            data1[i] = compute_hash(data1[i], data2[i]);
            data2[i] = scramble_bits(data2[i] + data3[i]);
            data3[i] = (data1[i] * 3 + data2[i] * 7 + data3[i] * 11) % 1000000;
        }
    }
    
    end = clock();
    total_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    /* Compute final checksum for verification */
    unsigned long long checksum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum += (unsigned long long)data1[i];
        checksum += (unsigned long long)data2[i];
        checksum += (unsigned long long)data3[i];
        checksum = (checksum << 13) | (checksum >> 51); /* Rotate */
    }
    
    printf("Execution time: %.3f seconds\n", total_time);
    printf("Final checksum: 0x%016llX\n", checksum);
    printf("Data samples: data1[0]=%d, data2[100]=%d, data3[500]=%d\n", 
           data1[0], data2[100], data3[500]);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    
    return 0;
}
