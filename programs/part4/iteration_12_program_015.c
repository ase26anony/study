/* Test program to trigger free_sched_context in GCC's Haifa scheduler */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <omp.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000
#define MAX_DEPTH 8

/* Helper functions marked for inlining */
static inline int __attribute__((always_inline)) 
compute_hash(int a, int b, int c) {
    /* Complex bitwise operations creating ILP opportunities */
    int t1 = (a ^ b) & (b | c);
    int t2 = (a << 3) | (b >> 2);
    int t3 = (c * 7) ^ (t1 & 0xFF);
    return (t1 + t2) ^ t3;
}

static inline int __attribute__((always_inline))
conditional_transform(int x, int selector) {
    /* Multiple conditional paths creating complex CFG */
    switch (selector & 0x7) {
        case 0: return x * 2;
        case 1: return x + (x >> 1);
        case 2: return x ^ 0x55AA55AA;
        case 3: return (x * 3) / 2;
        case 4: return ~x;
        case 5: return x + compute_hash(x, selector, x >> 2);
        case 6: return x | (selector << 16);
        case 7: return x & (selector | 0xFFFF);
        default: return x;
    }
}

/* Hot function with complex control flow */
__attribute__((hot))
void process_array_complex(int *input, int *output, int size, int depth) {
    int i, j, k;
    
    /* Outer loop with varying iteration count */
    for (i = 0; i < size; i += (depth + 1)) {
        /* Nested loops creating scheduling pressure */
        for (j = i; j < i + depth && j < size; j++) {
            int acc = input[j];
            
            /* Deep if-else chain */
            if (acc & 1) {
                acc = conditional_transform(acc, j);
                if (acc > 1000) {
                    acc = acc % 1000;
                } else if (acc < -1000) {
                    acc = -acc;
                } else {
                    /* Artificial memory dependency */
                    asm volatile("" : "+r"(acc) : : "memory");
                }
            } else if (acc & 2) {
                acc = compute_hash(acc, j, depth);
                for (k = 0; k < 4; k++) {
                    acc ^= (acc << k);
                }
            } else {
                /* Switch statement inside loop */
                switch (acc % 5) {
                    case 0: acc += input[(j + 1) % size]; break;
                    case 1: acc -= input[(j + 2) % size]; break;
                    case 2: acc *= 2; break;
                    case 3: acc /= (depth + 1); break;
                    case 4: acc = acc ^ input[(j + 3) % size]; break;
                }
            }
            
            /* Store with potential aliasing */
            output[j] = acc;
            
            /* Computed goto creating irreducible CFG */
            if (depth > 2) {
                static void *labels[] = { &&L0, &&L1, &&L2, &&L3 };
                goto *labels[acc % 4];
                
                L0: acc += 1; goto end_label;
                L1: acc -= 1; goto end_label;
                L2: acc ^= 0xFF; goto end_label;
                L3: acc *= 3; goto end_label;
                end_label: output[j] = acc;
            }
        }
        
        /* Memory barrier using asm */
        asm volatile("" ::: "memory");
    }
}

/* Function with vectorization opportunities */
__attribute__((hot))
void vectorizable_operation(float *a, float *b, float *c, int n) {
    int i;
    
    /* Loop with stride-1 access pattern */
    #pragma omp simd
    for (i = 0; i < n; i++) {
        /* Independent operations creating ILP */
        float t1 = a[i] * b[i];
        float t2 = a[i] + b[i];
        float t3 = t1 / (t2 + 1.0f);
        float t4 = t3 * t3 - t1;
        
        /* Conditional store */
        c[i] = (t4 > 0) ? t4 : -t4;
        
        /* Artificial dependency chain */
        asm volatile("" : "+r"(i) : : "memory");
    }
}

/* Function with OpenMP parallelization */
void parallel_processing(int *data, int size) {
    int i;
    
    #pragma omp parallel for schedule(dynamic)
    for (i = 0; i < size; i++) {
        /* Complex computation inside parallel region */
        int val = data[i];
        
        /* Nested loop with carried dependency */
        for (int j = 0; j < 8; j++) {
            val = compute_hash(val, j, i);
            
            /* Memory operation creating scheduling constraints */
            if (j % 3 == 0) {
                data[(i + j) % size] ^= val;
            }
        }
        
        /* Multiple conditional paths */
        if (val > 0) {
            for (int k = 0; k < 4; k++) {
                val = conditional_transform(val, k);
            }
        } else {
            val = val * 2 - 1;
        }
        
        data[i] = val;
    }
}

/* Recursive function with mixed operations */
int recursive_computation(int n, int depth) {
    if (depth >= MAX_DEPTH || n <= 1) {
        return n;
    }
    
    int result;
    
    /* Different computation paths based on parity */
    if (n % 2 == 0) {
        result = recursive_computation(n / 2, depth + 1);
        
        /* Bit manipulation creating ILP */
        result ^= (result << 1);
        result |= 0x1;
        
        /* Function call in middle of computation */
        result = compute_hash(result, n, depth);
    } else {
        result = recursive_computation(n * 3 + 1, depth + 1);
        
        /* Complex arithmetic chain */
        result = result * 7 - 3;
        result = result / 4 + 1;
        
        /* Memory clobber forcing scheduling barrier */
        asm volatile("" : "+r"(result) : : "memory");
    }
    
    return conditional_transform(result, depth);
}

/* Main orchestrator function */
int main() {
    int i, j;
    clock_t start, end;
    double cpu_time_used;
    
    /* Allocate and initialize test data */
    int *int_data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_a = (float*)malloc(ARRAY_SIZE * sizeof(float));
    float *float_b = (float*)malloc(ARRAY_SIZE * sizeof(float));
    float *float_c = (float*)malloc(ARRAY_SIZE * sizeof(float));
    int *output = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    srand(42);  /* Fixed seed for reproducibility */
    
    for (i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = rand() % 1000;
        float_a[i] = (float)(rand() % 100) / 10.0f;
        float_b[i] = (float)(rand() % 100) / 10.0f;
    }
    
    printf("Starting complex scheduling test...\n");
    
    /* Warm-up phase to trigger optimization heuristics */
    printf("Warm-up phase...\n");
    for (i = 0; i < 10; i++) {
        process_array_complex(int_data, output, ARRAY_SIZE / 4, 3);
    }
    
    /* Main test phase with varying parameters */
    printf("Main test phase...\n");
    start = clock();
    
    long long checksum = 0;
    
    for (j = 0; j < ITERATIONS; j++) {
        /* Vary parameters to trigger different scheduling paths */
        int depth = (j % 7) + 1;
        int size = ARRAY_SIZE / (depth + 1);
        
        /* Call complex function with varying control flow */
        process_array_complex(int_data, output, size, depth);
        
        /* Perform vectorizable operations */
        vectorizable_operation(float_a, float_b, float_c, ARRAY_SIZE);
        
        /* Every 10 iterations, do parallel processing */
        if (j % 10 == 0) {
            parallel_processing(int_data, ARRAY_SIZE);
        }
        
        /* Recursive computations */
        for (i = 0; i < 100; i++) {
            int idx = (i + j) % ARRAY_SIZE;
            output[idx] = recursive_computation(int_data[idx], 0);
        }
        
        /* Update checksum for verification */
        for (i = 0; i < size; i++) {
            checksum += output[i];
            checksum ^= (checksum << 13);
            checksum ^= (checksum >> 17);
            checksum ^= (checksum << 5);
        }
    }
    
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("Test completed in %.2f seconds\n", cpu_time_used);
    printf("Final checksum: %llx\n", checksum);
    
    /* Verification step */
    printf("Verifying results...\n");
    int verify_sum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        verify_sum += output[i];
    }
    printf("Output sum: %d\n", verify_sum);
    
    /* Cleanup */
    free(int_data);
    free(float_a);
    free(float_b);
    free(float_c);
    free(output);
    
    return 0;
}
