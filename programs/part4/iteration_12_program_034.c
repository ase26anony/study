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
compute_hash(int a, int b, int c) {
    /* Mix of arithmetic and bitwise ops */
    return ((a * 17) ^ (b >> 3) + (c & 0xFF)) * 31;
}

static inline int __attribute__((always_inline))
barrier_op(int x) {
    int result;
    /* Memory barrier via asm */
    asm volatile ("mfence" ::: "memory");
    result = x * 2 + 1;
    asm volatile ("" : "+r" (result) : : "memory");
    return result;
}

/* Hot function with complex control flow */
__attribute__((hot, noinline))
void complex_control_flow(int *data, int size, int *result) {
    int i, j, k;
    volatile int counter = 0; /* Prevent optimizations */
    
    /* Nested loops with varying iteration counts */
    for (i = 0; i < size; i++) {
        int temp = data[i];
        
        /* Deep if-else chain */
        if (temp < 100) {
            temp = compute_hash(temp, i, 1);
            if (temp % 3 == 0) {
                temp = barrier_op(temp);
            } else if (temp % 5 == 0) {
                temp = compute_hash(temp, temp, 2);
            } else {
                for (j = 0; j < (i % 10); j++) {
                    temp += compute_hash(j, temp, 3);
                }
            }
        } else if (temp < 500) {
            /* Switch statement with multiple cases */
            switch (temp % 7) {
                case 0: temp = temp * 2; break;
                case 1: temp = temp / 3; break;
                case 2: temp = temp ^ 0x55; break;
                case 3: temp = barrier_op(temp); break;
                case 4: temp = compute_hash(temp, i, 4); break;
                case 5: temp = temp << 2; break;
                default: temp = temp - 1; break;
            }
        } else {
            /* Irreducible control flow with computed goto */
            static void *labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4 };
            int idx = temp % 5;
            goto *labels[idx];
            
            L0: temp += 100; goto END;
            L1: temp = barrier_op(temp); goto END;
            L2: temp = compute_hash(temp, i, 5); goto END;
            L3: temp *= 3; goto END;
            L4: temp = temp ^ (temp >> 4); goto END;
            END:;
        }
        
        /* Memory operation with pointer arithmetic */
        int *ptr = &data[(i + 1) % size];
        *ptr += temp;
        counter++;
    }
    
    *result = counter;
}

/* Function with tight inner loop for vectorization */
__attribute__((hot))
void vectorizable_loop(int *a, int *b, int *c, int n) {
    int i;
    
    #pragma omp simd safelen(16)
    for (i = 0; i < n; i++) {
        /* Simple stride-1 operations */
        int t1 = a[i] * 3;
        int t2 = b[i] + 7;
        c[i] = compute_hash(t1, t2, i);
        
        /* Create carried dependency */
        if (i > 0) {
            c[i] += c[i-1] & 0xF;
        }
    }
}

/* Function with OpenMP parallelization */
void parallel_loop(int *data, int size) {
    int i;
    
    #pragma omp parallel for schedule(dynamic, 16)
    for (i = 0; i < size; i++) {
        int val = data[i];
        
        /* Independent operations that scheduler can reorder */
        int op1 = val * 2;
        int op2 = val + 5;
        int op3 = val ^ 0xAA;
        int op4 = barrier_op(val);
        
        /* Mix operations with dependencies */
        data[i] = compute_hash(op1, op2, op3) + op4;
        
        /* Additional computation to increase ILP */
        for (int j = 0; j < 4; j++) {
            data[i] += (val >> j) & 1;
        }
    }
}

/* Recursive function with complex data flow */
int __attribute__((noinline))
recursive_compute(int depth, int value, int *memo) {
    if (depth <= 0) {
        return value;
    }
    
    if (memo[depth] != 0) {
        return memo[depth];
    }
    
    int result;
    
    /* Multiple recursive calls with different operations */
    if (value % 2 == 0) {
        int r1 = recursive_compute(depth - 1, value / 2, memo);
        int r2 = recursive_compute(depth - 2, value + 1, memo);
        result = compute_hash(r1, r2, depth);
    } else {
        int r1 = recursive_compute(depth - 1, value * 3 + 1, memo);
        result = barrier_op(r1);
    }
    
    memo[depth] = result;
    return result;
}

/* Main test driver */
int main() {
    int i, j;
    clock_t start, end;
    double cpu_time_used;
    
    /* Allocate and initialize data */
    int *data1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *data2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *data3 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *result = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *memo = (int*)calloc(MAX_DEPTH + 1, sizeof(int));
    
    if (!data1 || !data2 || !data3 || !result || !memo) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(42);
    for (i = 0; i < ARRAY_SIZE; i++) {
        data1[i] = rand() % 1000;
        data2[i] = rand() % 1000;
        data3[i] = i;
    }
    
    printf("Starting scheduler stress test...\n");
    
    /* Warm-up phase to trigger optimization heuristics */
    printf("Warm-up phase...\n");
    for (j = 0; j < 10; j++) {
        int temp_result;
        complex_control_flow(data1, ARRAY_SIZE / 10, &temp_result);
        vectorizable_loop(data2, data3, result, ARRAY_SIZE / 10);
    }
    
    /* Main test phase with timing */
    printf("Main test phase...\n");
    start = clock();
    
    int total_checksum = 0;
    
    for (i = 0; i < ITERATIONS; i++) {
        int iter_checksum = 0;
        
        /* Alternate between different patterns */
        switch (i % 4) {
            case 0:
                complex_control_flow(data1, ARRAY_SIZE, &iter_checksum);
                break;
            case 1:
                vectorizable_loop(data1, data2, result, ARRAY_SIZE);
                for (j = 0; j < ARRAY_SIZE; j++) {
                    iter_checksum += result[j];
                }
                break;
            case 2:
                parallel_loop(data3, ARRAY_SIZE);
                for (j = 0; j < ARRAY_SIZE; j++) {
                    iter_checksum += data3[j];
                }
                break;
            case 3:
                for (j = 0; j < 10; j++) {
                    iter_checksum += recursive_compute(MAX_DEPTH, j, memo);
                }
                memset(memo, 0, (MAX_DEPTH + 1) * sizeof(int));
                break;
        }
        
        total_checksum += iter_checksum;
        
        /* Modify data slightly each iteration */
        if (i % 100 == 0) {
            for (j = 0; j < ARRAY_SIZE; j += 7) {
                data1[j] = (data1[j] * 13 + i) % 1000;
                data2[j] = (data2[j] * 17 + i) % 1000;
            }
        }
    }
    
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    
    /* Verification calculation */
    int final_checksum = total_checksum;
    for (i = 0; i < ARRAY_SIZE; i++) {
        final_checksum = compute_hash(final_checksum, data1[i], data2[i]);
        final_checksum = barrier_op(final_checksum);
    }
    
    printf("Test completed in %.2f seconds\n", cpu_time_used);
    printf("Final checksum: %d\n", final_checksum);
    printf("Expected range: non-zero value\n");
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    free(result);
    free(memo);
    
    return (final_checksum != 0) ? 0 : 1;
}
