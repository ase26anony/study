/* Complex control flow and instruction mix to stress GCC's Haifa scheduler */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <omp.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000
#define MAX_DEPTH 8

/* Helper functions for inlining */
static inline int __attribute__((always_inline)) 
compute_hash(int a, int b, int c) {
    /* Complex bitwise operations with dependencies */
    int t1 = a ^ (b << 3);
    int t2 = (t1 + c) * 0x5bd1e995;
    t2 = (t2 ^ (t2 >> 15)) * 0x27d4eb2d;
    return t2 ^ (t2 >> 13);
}

static inline int __attribute__((always_inline))
process_element(int x, int y, int *counter) {
    /* Memory operation with side effect */
    int old = *counter;
    *counter = old + 1;
    
    /* Arithmetic with artificial asm barrier */
    int result = x * y + old;
    asm volatile("" : "+r" (result) : : "memory");
    
    /* Conditional computation */
    if (result & 1) {
        result = (result << 1) | 1;
    } else {
        result = (result >> 1) & 0x7FFFFFFF;
    }
    
    return result;
}

/* Function with deeply nested control flow */
int __attribute__((hot))
complex_control_flow(int *data, int size, int seed) {
    int i, j, k;
    int result = seed;
    int local_counter = 0;
    
    /* Outer loop with varying iteration count */
    for (i = 0; i < size; i += (i % 3) + 1) {
        /* Switch statement inside loop */
        switch (i % 5) {
            case 0:
                /* Nested loop */
                for (j = 0; j < (i % 8); j++) {
                    result ^= compute_hash(data[i], j, result);
                }
                break;
            case 1:
                /* While loop with complex condition */
                k = 0;
                while (k < (i % 7) && result < 1000000) {
                    result = process_element(data[i], k, &local_counter);
                    k += (result % 3) + 1;
                }
                break;
            case 2:
                /* Do-while with function calls */
                do {
                    result += compute_hash(data[i], result, local_counter);
                    local_counter++;
                } while (local_counter % 4 != 0);
                break;
            case 3:
                /* Deep if-else chain */
                if (data[i] < 0) {
                    if (result > 1000) {
                        result >>= 2;
                    } else if (result > 100) {
                        result <<= 1;
                    } else {
                        result *= 3;
                    }
                } else if (data[i] > 100) {
                    result /= 2;
                } else {
                    result = process_element(data[i], result, &local_counter);
                }
                break;
            case 4:
            default:
                /* Computed goto for irreducible control flow */
                {
                    static void *labels[] = { &&L0, &&L1, &&L2, &&L3 };
                    int idx = result % 4;
                    goto *labels[idx];
                    
                    L0:
                        result += data[i] * 2;
                        goto end_case;
                    L1:
                        result -= data[i] / 2;
                        goto end_case;
                    L2:
                        result ^= data[i];
                        goto end_case;
                    L3:
                        result = compute_hash(data[i], result, local_counter);
                        goto end_case;
                    end_case:
                        break;
                }
        }
        
        /* Memory operation with pointer arithmetic */
        int *ptr = &data[i];
        if (i + 1 < size) {
            *ptr += ptr[1];
            asm volatile("" : : "r" (ptr) : "memory");
        }
    }
    
    return result;
}

/* Function with vectorization candidate */
void __attribute__((hot))
vectorizable_loop(int *in1, int *in2, int *out, int n) {
    int i;
    /* Loop with stride-1 access pattern */
    #pragma omp simd
    for (i = 0; i < n; i++) {
        /* Independent operations for ILP */
        int a = in1[i] * 3;
        int b = in2[i] / 2;
        int c = a ^ b;
        int d = (c << 1) | (c >> 31);
        out[i] = d + i;
        
        /* Artificial dependency chain */
        if (i > 0) {
            out[i] += out[i-1] & 0xFF;
        }
    }
}

/* Function with OpenMP parallelization */
int __attribute__((hot))
parallel_reduction(int *data, int size) {
    int sum = 0;
    int i;
    
    #pragma omp parallel for reduction(+:sum) schedule(dynamic, 16)
    for (i = 0; i < size; i++) {
        /* Complex computation inside parallel region */
        int val = data[i];
        
        /* Nested loops in parallel region */
        for (int j = 0; j < (val % 8); j++) {
            val = compute_hash(val, j, sum);
        }
        
        /* Conditional with function call */
        if (val % 3 == 0) {
            val = process_element(val, i, &sum);
        }
        
        sum += val;
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
    }
    
    return sum;
}

/* Function with tight inner loop and carried dependency */
int __attribute__((hot))
tight_inner_loop(int *data, int size) {
    int acc = 0;
    int i, j;
    
    for (i = 0; i < size; i++) {
        /* Carried dependency through acc */
        acc = data[i] + (acc >> 1);
        
        /* Inner loop with independent operations */
        int temp = 0;
        for (j = 0; j < 16; j++) {
            /* Multiple independent integer ops */
            int a = (i * j) & 0xFF;
            int b = (acc << j) | (acc >> (32 - j));
            int c = a ^ b;
            int d = c * 0x9e3779b9;
            temp += d;
            
            /* Memory operation */
            if (j % 4 == 0) {
                data[(i + j) % size] = temp;
            }
        }
        
        acc ^= temp;
    }
    
    return acc;
}

/* Main orchestrator */
int main() {
    int i, iter;
    int checksum = 0;
    clock_t start, end;
    
    /* Allocate and initialize data */
    int *data1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *data2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *result = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    srand(42);
    for (i = 0; i < ARRAY_SIZE; i++) {
        data1[i] = rand() % 1000;
        data2[i] = rand() % 1000;
        result[i] = 0;
    }
    
    printf("Starting scheduler stress test...\n");
    
    /* Warm-up phase */
    printf("Warm-up phase...\n");
    for (iter = 0; iter < ITERATIONS / 10; iter++) {
        checksum ^= complex_control_flow(data1, ARRAY_SIZE / 2, iter);
    }
    
    /* Main test phase with different patterns */
    start = clock();
    
    for (iter = 0; iter < ITERATIONS; iter++) {
        /* Alternate between different complex functions */
        switch (iter % 4) {
            case 0:
                checksum += complex_control_flow(data1, ARRAY_SIZE, iter);
                break;
            case 1:
                vectorizable_loop(data1, data2, result, ARRAY_SIZE);
                checksum += result[iter % ARRAY_SIZE];
                break;
            case 2:
                checksum ^= parallel_reduction(data1, ARRAY_SIZE);
                break;
            case 3:
                checksum = tight_inner_loop(data2, ARRAY_SIZE) - checksum;
                break;
        }
        
        /* Modify data periodically to change execution paths */
        if (iter % 100 == 0) {
            for (i = 0; i < ARRAY_SIZE; i += 10) {
                data1[i] = compute_hash(data1[i], iter, checksum);
            }
        }
    }
    
    end = clock();
    
    /* Verification computation */
    int final_check = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        final_check ^= data1[i];
        final_check += data2[i];
        final_check ^= result[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Verification hash: %d\n", final_check);
    printf("Time elapsed: %.2f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(result);
    
    return 0;
}
