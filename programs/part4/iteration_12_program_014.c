/* Test program to trigger free_sched_context in GCC's Haifa scheduler */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <omp.h>

#define ARRAY_SIZE 1024
#define MAX_DEPTH 8
#define ITERATIONS 1000

/* Always inline helper functions to create complex dataflow */
static inline int __attribute__((always_inline)) 
compute_hash(int x, int y) {
    /* Mix of arithmetic and bitwise operations */
    x = (x ^ (x >> 16)) * 0x85ebca6b;
    y = (y ^ (y >> 13)) * 0xc2b2ae35;
    return (x ^ y) + (x & y) * 2;
}

static inline int __attribute__((always_inline))
scramble_bits(int val) {
    /* Complex bit manipulation with dependencies */
    val = val ^ (val << 13);
    val = val ^ (val >> 17);
    val = val ^ (val << 5);
    /* Artificial scheduling barrier */
    asm volatile("" : "+r" (val) : : "memory");
    return val;
}

/* Hot function with complex control flow */
__attribute__((hot))
void complex_control_flow(int *data, int size, int depth) {
    int i, j, k;
    volatile int barrier; /* Prevent dead code elimination */
    
    /* Nested loops with varying iteration counts */
    for (i = 0; i < size; i++) {
        /* Deep if-else chain */
        if (i % 3 == 0) {
            data[i] = compute_hash(data[i], i);
            for (j = 0; j < depth; j++) {
                /* Memory operations with dependencies */
                int idx = (i + j) % size;
                data[idx] = scramble_bits(data[idx] + data[i]);
                
                /* Switch statement with multiple cases */
                switch (j % 5) {
                    case 0:
                        data[idx] += 1;
                        break;
                    case 1:
                        data[idx] *= 2;
                        break;
                    case 2:
                        data[idx] ^= 0xAAAAAAAA;
                        break;
                    case 3:
                        data[idx] = data[idx] >> (i % 8);
                        break;
                    case 4:
                        data[idx] = data[idx] << (j % 8);
                        break;
                    default:
                        /* Unreachable but creates control flow complexity */
                        data[idx] = 0;
                }
            }
        } else if (i % 3 == 1) {
            /* Different computation path */
            int temp = data[i];
            for (k = 0; k < 4; k++) {
                temp = compute_hash(temp, k);
                /* Memory clobber to force scheduling constraints */
                asm volatile("" : : "r"(temp) : "memory");
            }
            data[i] = temp;
        } else {
            /* Third computation path with while loop */
            int counter = 0;
            while (counter < 3) {
                data[i] ^= (1 << counter);
                data[i] = scramble_bits(data[i]);
                counter++;
            }
        }
        
        /* Computed goto for irreducible control flow */
        static void *labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4 };
        if (depth > 2) {
            goto *labels[i % 5];
        }
        
        L0:
            data[i] += 1000;
            continue;
        L1:
            data[i] -= 500;
            continue;
        L2:
            data[i] |= 0xFF;
            continue;
        L3:
            data[i] &= 0xFFFF;
            continue;
        L4:
            data[i] ^= 0x12345678;
            continue;
    }
}

/* Function with vectorization candidate loops */
__attribute__((hot))
void vectorizable_operations(float *a, float *b, float *c, int n) {
    int i;
    
    /* Simple stride-1 loop for auto-vectorization */
    #pragma omp simd
    for (i = 0; i < n; i++) {
        a[i] = b[i] * c[i] + 1.0f;
    }
    
    /* Loop with carried dependency */
    for (i = 1; i < n; i++) {
        a[i] = a[i-1] * 0.99f + b[i];
    }
    
    /* Nested loops with mixed operations */
    for (i = 0; i < n; i += 16) {
        float sum = 0.0f;
        for (int j = 0; j < 16 && (i + j) < n; j++) {
            sum += a[i + j] * b[i + j];
            /* Memory operation to create dependencies */
            c[i + j] = sum;
        }
    }
}

/* Function with OpenMP parallelization */
void parallel_region(int *data, int size) {
    int i;
    
    #pragma omp parallel for schedule(dynamic, 16)
    for (i = 0; i < size; i++) {
        /* Complex computation within parallel region */
        int val = data[i];
        
        /* Multiple dependent operations */
        for (int j = 0; j < 8; j++) {
            val = compute_hash(val, j);
            val = scramble_bits(val);
            
            /* Conditional with side effects */
            if (val % 7 == 0) {
                val += i;
            } else if (val % 7 == 1) {
                val -= j;
            } else {
                val ^= 0xDEADBEEF;
            }
        }
        
        data[i] = val;
        
        /* Memory fence for synchronization */
        #pragma omp flush(data)
    }
}

/* Recursive function with tail calls */
__attribute__((noinline))
int recursive_computation(int n, int acc) {
    if (n <= 0) return acc;
    
    /* Mix of operations */
    acc = compute_hash(acc, n);
    acc = scramble_bits(acc);
    
    /* Tail recursion with different paths */
    if (n % 2 == 0) {
        return recursive_computation(n / 2, acc + 1);
    } else {
        return recursive_computation(n - 1, acc * 2);
    }
}

/* Main orchestrator function */
int main() {
    int i, j;
    clock_t start, end;
    double cpu_time_used;
    
    /* Allocate and initialize data */
    int *int_data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_a = (float*)malloc(ARRAY_SIZE * sizeof(float));
    float *float_b = (float*)malloc(ARRAY_SIZE * sizeof(float));
    float *float_c = (float*)malloc(ARRAY_SIZE * sizeof(float));
    
    if (!int_data || !float_a || !float_b || !float_c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(42);
    for (i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = rand();
        float_a[i] = (float)rand() / RAND_MAX;
        float_b[i] = (float)rand() / RAND_MAX;
        float_c[i] = (float)rand() / RAND_MAX;
    }
    
    printf("Starting scheduler stress test...\n");
    
    /* Warm-up phase to trigger optimization heuristics */
    printf("Warm-up phase...\n");
    for (j = 0; j < 10; j++) {
        complex_control_flow(int_data, ARRAY_SIZE / 4, 3);
    }
    
    /* Main test iterations with varying parameters */
    printf("Main test phase...\n");
    start = clock();
    
    for (i = 0; i < ITERATIONS; i++) {
        /* Vary parameters to trigger different scheduling paths */
        int depth = 1 + (i % MAX_DEPTH);
        int size = ARRAY_SIZE / (1 + (i % 4));
        
        /* Call complex functions with different patterns */
        if (i % 3 == 0) {
            complex_control_flow(int_data, size, depth);
        } else if (i % 3 == 1) {
            vectorizable_operations(float_a, float_b, float_c, size);
        } else {
            parallel_region(int_data, size);
        }
        
        /* Mix in recursive computations */
        if (i % 7 == 0) {
            int_data[i % ARRAY_SIZE] = recursive_computation(
                int_data[i % ARRAY_SIZE] % 100, 
                i
            );
        }
        
        /* Progress indicator */
        if (i % 100 == 0) {
            printf("Iteration %d/%d\n", i, ITERATIONS);
        }
    }
    
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    /* Compute final checksum for verification */
    unsigned long long checksum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum += (unsigned int)int_data[i];
        checksum += (unsigned int)(float_a[i] * 1000);
    }
    
    printf("\nTest completed successfully!\n");
    printf("Execution time: %.2f seconds\n", cpu_time_used);
    printf("Final checksum: %llu\n", checksum);
    
    /* Cleanup */
    free(int_data);
    free(float_a);
    free(float_b);
    free(float_c);
    
    return 0;
}
