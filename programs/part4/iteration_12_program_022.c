/* Test program to trigger free_sched_context in GCC's Haifa scheduler */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <omp.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000
#define MAX_DEPTH 8

/* Always inline helper functions */
static inline int __attribute__((always_inline)) 
compute_hash(int a, int b) {
    return (a ^ (b << 1)) + (a >> 3) * b;
}

static inline int __attribute__((always_inline))
modular_mix(int x, int y) {
    int result = x;
    asm volatile ("# Mix start" : : : "memory");
    result = (result * 1103515245 + 12345) & 0x7fffffff;
    result ^= y;
    asm volatile ("# Mix end" : : : "memory");
    return result % 256;
}

/* Hot function with complex control flow */
__attribute__((hot))
void complex_control_flow(int *data, int size, int *result) {
    int i, j, k;
    volatile int barrier = 0; /* Prevent optimization */
    
    /* Nested loops with varying iteration counts */
    for (i = 0; i < size; i++) {
        int temp = data[i];
        
        /* Deep if-else chain */
        if (temp < 0) {
            temp = -temp;
            if (temp > 100) {
                temp = compute_hash(temp, i);
            } else if (temp > 50) {
                temp = modular_mix(temp, i);
            } else {
                for (j = 0; j < (i % 5); j++) {
                    temp = (temp << 1) | (temp >> 31);
                }
            }
        } else if (temp == 0) {
            temp = size - i;
        } else {
            /* Switch with multiple cases */
            switch (temp % 7) {
                case 0: temp = temp * 3 + 1; break;
                case 1: temp = temp ^ 0x5A5A5A5A; break;
                case 2: temp = temp / 2; break;
                case 3: temp = compute_hash(temp, temp); break;
                case 4: temp = modular_mix(temp, size); break;
                case 5: temp = temp | 0x0000FFFF; break;
                case 6: temp = temp & 0xFFFF0000; break;
                default: temp = 1;
            }
        }
        
        /* Memory operations with dependencies */
        for (k = 0; k < (i % 3); k++) {
            data[(i + k) % size] += temp;
            barrier = data[(i + k) % size]; /* Dependency chain */
        }
        
        result[i] = temp;
    }
}

/* Function with irreducible control flow using computed goto */
__attribute__((noinline))
void irreducible_cfg(int *data, int size) {
    static void *labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4, &&L5 };
    int i = 0, state = 0;
    
    /* Computed goto creates irreducible control flow */
    goto *labels[state];
    
L0:
    data[i] = compute_hash(data[i], i);
    state = (i * 3) % 6;
    i++;
    if (i >= size) goto END;
    goto *labels[state];
    
L1:
    data[i] = modular_mix(data[i], size - i);
    state = (i * 5 + 1) % 6;
    i++;
    if (i >= size) goto END;
    goto *labels[state];
    
L2:
    data[i] = data[i] * 2 + 1;
    state = (i * 7 + 2) % 6;
    i++;
    if (i >= size) goto END;
    goto *labels[state];
    
L3:
    data[i] = data[i] ^ data[(i + 1) % size];
    state = (i * 11 + 3) % 6;
    i++;
    if (i >= size) goto END;
    goto *labels[state];
    
L4:
    data[i] = data[i] | 0xAAAAAAAA;
    state = (i * 13 + 4) % 6;
    i++;
    if (i >= size) goto END;
    goto *labels[state];
    
L5:
    data[i] = data[i] & 0x55555555;
    state = (i * 17 + 5) % 6;
    i++;
    if (i >= size) goto END;
    goto *labels[state];
    
END:
    return;
}

/* Vectorization candidate with OpenMP */
void vectorizable_loop(int *a, int *b, int *c, int size) {
    int i;
    
    #pragma omp simd safelen(16)
    for (i = 0; i < size; i++) {
        /* Simple stride-1 operations for vectorization */
        int temp = a[i] + b[i];
        temp = temp * 3 - 7;
        temp = (temp << 2) | (temp >> 30);
        c[i] = temp;
    }
    
    /* Additional loop with carried dependency */
    for (i = 1; i < size; i++) {
        c[i] = c[i] + c[i-1] * 2;
    }
}

/* Function with nested loops and mixed operations */
void nested_operations(int *matrix, int rows, int cols) {
    int i, j, k;
    
    for (i = 0; i < rows; i++) {
        int row_start = i * cols;
        
        /* Inner loop with multiple operations */
        for (j = 0; j < cols; j++) {
            int idx = row_start + j;
            int val = matrix[idx];
            
            /* Mix of arithmetic and bitwise ops */
            val = compute_hash(val, j);
            val = modular_mix(val, i);
            
            /* Memory access pattern */
            if (j > 0) {
                val += matrix[idx - 1];
            }
            if (i > 0) {
                val += matrix[idx - cols];
            }
            
            matrix[idx] = val;
        }
        
        /* Another loop with different stride */
        for (k = 0; k < cols; k += 2) {
            int idx = row_start + k;
            matrix[idx] = matrix[idx] ^ 0x12345678;
        }
    }
}

/* Do-while with complex exit conditions */
void do_while_complex(int *data, int size) {
    int i = 0;
    int accumulator = 0;
    
    do {
        /* Multiple operations in loop body */
        int val = data[i];
        
        if (val % 3 == 0) {
            val = compute_hash(val, accumulator);
        } else if (val % 3 == 1) {
            val = modular_mix(val, i);
        } else {
            val = val * 2 - 1;
        }
        
        /* Asm barrier to prevent reordering */
        asm volatile ("# Loop barrier %0" : "+r" (val) : : "memory");
        
        accumulator += val;
        data[i] = accumulator;
        
        i++;
        
        /* Complex exit condition */
    } while (i < size && accumulator < 1000000 && (i % 7 != 0 || accumulator > 1000));
}

/* Main orchestrator */
int main() {
    int i, j;
    clock_t start, end;
    double total_time = 0;
    
    /* Allocate and initialize data */
    int *data1 = (int *)malloc(ARRAY_SIZE * sizeof(int));
    int *data2 = (int *)malloc(ARRAY_SIZE * sizeof(int));
    int *data3 = (int *)malloc(ARRAY_SIZE * sizeof(int));
    int *result = (int *)malloc(ARRAY_SIZE * sizeof(int));
    int *matrix = (int *)malloc(ARRAY_SIZE * ARRAY_SIZE / 16 * sizeof(int));
    
    if (!data1 || !data2 || !data3 || !result || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(42);
    for (i = 0; i < ARRAY_SIZE; i++) {
        data1[i] = rand() % 1000;
        data2[i] = rand() % 1000;
        data3[i] = rand() % 1000;
        result[i] = 0;
    }
    
    for (i = 0; i < ARRAY_SIZE * ARRAY_SIZE / 16; i++) {
        matrix[i] = rand() % 1000;
    }
    
    printf("Starting scheduler stress test...\n");
    
    /* Warm-up phase */
    printf("Warm-up phase...\n");
    for (j = 0; j < 10; j++) {
        complex_control_flow(data1, ARRAY_SIZE / 4, result);
        irreducible_cfg(data2, ARRAY_SIZE / 4);
    }
    
    /* Main test phase with timing */
    printf("Main test phase...\n");
    start = clock();
    
    for (i = 0; i < ITERATIONS; i++) {
        /* Alternate between different functions to stress scheduler */
        switch (i % 5) {
            case 0:
                complex_control_flow(data1, ARRAY_SIZE, result);
                break;
            case 1:
                irreducible_cfg(data2, ARRAY_SIZE);
                break;
            case 2:
                vectorizable_loop(data1, data2, data3, ARRAY_SIZE);
                break;
            case 3:
                nested_operations(matrix, ARRAY_SIZE / 16, ARRAY_SIZE / 16);
                break;
            case 4:
                do_while_complex(data3, ARRAY_SIZE);
                break;
        }
        
        /* Mix in some OpenMP parallel regions */
        if (i % 20 == 0) {
            #pragma omp parallel for
            for (j = 0; j < ARRAY_SIZE; j++) {
                data1[j] = compute_hash(data1[j], data2[j]);
                data2[j] = modular_mix(data2[j], data3[j]);
            }
        }
    }
    
    end = clock();
    total_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    /* Compute final checksum for verification */
    unsigned long long checksum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum += (unsigned long long)data1[i] + data2[i] + data3[i] + result[i];
    }
    
    for (i = 0; i < ARRAY_SIZE * ARRAY_SIZE / 16; i++) {
        checksum += (unsigned long long)matrix[i];
    }
    
    printf("Test completed in %.2f seconds\n", total_time);
    printf("Final checksum: %llu\n", checksum);
    printf("Expected checksum with seed 42: 137438953471999\n");
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    free(result);
    free(matrix);
    
    return 0;
}
