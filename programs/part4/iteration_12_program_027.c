/* haifa_scheduler_test.c
 * Designed to trigger free_sched_context in GCC's Haifa scheduler
 * Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -finline-functions -funswitch-loops -fprofile-generate -o test_gen haifa_scheduler_test.c
 * Run: ./test_gen
 * Recompile: gcc -O3 -fschedule-insns -fschedule-insns2 -finline-functions -funswitch-loops -fprofile-use -o test_use haifa_scheduler_test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define HOT __attribute__((hot))
#define ALWAYS_INLINE __attribute__((always_inline))
#define NOINLINE __attribute__((noinline))

/* Helper functions for inlining */
ALWAYS_INLINE static unsigned int mix_bits(unsigned int x) {
    x ^= x >> 16;
    x *= 0x85ebca6b;
    x ^= x >> 13;
    x *= 0xc2b2ae35;
    x ^= x >> 16;
    return x;
}

ALWAYS_INLINE static int compute_hash(int a, int b, int c) {
    int hash = a;
    hash = hash * 31 + b;
    hash = hash * 31 + c;
    hash ^= (hash >> 20) ^ (hash >> 12);
    return hash ^ (hash >> 7) ^ (hash >> 4);
}

/* Complex function with irreducible control flow using computed goto */
NOINLINE HOT static void irreducible_cfg(int *arr, int n, int *result) {
    static void *labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4, &&L5, &&L6, &&L7 };
    
    int i = 0;
    int state = 0;
    int sum = 0;
    
    /* Create complex control flow with computed goto */
    goto *labels[state];
    
L0:
    for (; i < n; ) {
        /* Memory operations with dependencies */
        int val = arr[i];
        int hash = compute_hash(val, i, state);
        
        /* Switch with multiple cases */
        switch (hash & 7) {
            case 0:
                sum += val * 2;
                state = 1;
                break;
            case 1:
                sum += val >> 1;
                state = 2;
                break;
            case 2:
                sum += val & 0xFF;
                state = 3;
                break;
            case 3:
                sum += mix_bits(val);
                state = 4;
                break;
            case 4:
                sum -= val;
                state = 5;
                break;
            case 5:
                sum ^= val;
                state = 6;
                break;
            case 6:
                sum |= val;
                state = 7;
                break;
            case 7:
                sum &= val;
                state = 0;
                break;
        }
        
        /* Artificial scheduling barrier */
        asm volatile("" ::: "memory");
        
        i++;
        goto *labels[state];
    }
    
L1: case 1: goto L0;
L2: case 2: goto L0;
L3: case 3: goto L0;
L4: case 4: goto L0;
L5: case 5: goto L0;
L6: case 6: goto L0;
L7: case 7: goto L0;
    
    *result = sum;
}

/* Function with nested loops and complex conditionals */
HOT static void nested_loops_mixed_ops(int *matrix, int rows, int cols, int *output) {
    int total = 0;
    
    /* Outer loop with varying iteration count */
    for (int i = 0; i < rows; i++) {
        /* Inner loop with stride-1 access (vectorization candidate) */
        int row_sum = 0;
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            
            /* Mix of arithmetic operations */
            int val = matrix[idx];
            val = val * 3 + 7;
            val = (val << 3) | (val >> 29);  /* rotate left */
            val ^= 0xAAAAAAAA;
            val = mix_bits(val);
            
            row_sum += val;
            
            /* Conditional store */
            if ((val & 1) == 0) {
                matrix[idx] = val;
            }
        }
        
        /* Deep if-else chain */
        if (row_sum < 0) {
            total -= row_sum * 2;
        } else if (row_sum < 1000) {
            total += row_sum;
        } else if (row_sum < 10000) {
            total += row_sum / 2;
        } else if (row_sum < 100000) {
            total += row_sum / 4;
        } else {
            total += 100000;
        }
        
        /* Another scheduling barrier */
        asm volatile("" ::: "memory");
    }
    
    /* Do-while loop */
    int k = 0;
    do {
        total = mix_bits(total);
        k++;
    } while (k < 3);
    
    *output = total;
}

/* Function with OpenMP parallelization attempts */
HOT static void parallelizable_work(int *data, int size, int *result) {
    int sum = 0;
    
    /* Loop with simple operations - auto-vectorization candidate */
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < size; i++) {
        int val = data[i];
        /* Independent operations for ILP */
        int a = val * 2;
        int b = val + 7;
        int c = val ^ 0xFF;
        int d = (val << 1) | (val >> 31);
        
        /* Create artificial dependencies */
        a = mix_bits(a);
        b = mix_bits(b + a);
        c = mix_bits(c + b);
        d = mix_bits(d + c);
        
        sum += a + b + c + d;
    }
    
    /* Additional complex loop */
    for (int i = 1; i < size - 1; i++) {
        /* 3-point stencil - creates carried dependencies */
        data[i] = (data[i-1] + data[i] * 2 + data[i+1]) / 4;
        
        /* More arithmetic */
        data[i] = compute_hash(data[i], i, sum);
    }
    
    *result = sum;
}

/* Function with switch statement and loop unrolling */
HOT static void switch_with_loops(int *arr, int n, int mode, int *out) {
    int acc = 0;
    
    for (int i = 0; i < n; i++) {
        /* Large switch statement */
        switch (mode) {
            case 0:
                acc += arr[i] * 3;
                break;
            case 1:
                acc += arr[i] >> 2;
                break;
            case 2:
                acc ^= arr[i];
                break;
            case 3:
                acc |= arr[i];
                break;
            case 4:
                acc &= arr[i];
                break;
            case 5:
                acc = (acc << 3) + arr[i];
                break;
            case 6:
                acc = mix_bits(acc + arr[i]);
                break;
            case 7:
                acc = compute_hash(acc, arr[i], i);
                break;
            default:
                acc -= arr[i];
                break;
        }
        
        /* Unrolled inner loop */
        int temp = arr[i];
        temp = mix_bits(temp);      /* iteration 1 */
        temp = compute_hash(temp, i, acc);  /* iteration 2 */
        temp ^= 0xDEADBEEF;         /* iteration 3 */
        arr[i] = temp;
        
        /* Memory clobber */
        asm volatile("" ::: "memory");
    }
    
    /* While loop with condition */
    int j = 0;
    while (j < 10) {
        acc = mix_bits(acc);
        j++;
    }
    
    *out = acc;
}

/* Main orchestrator */
int main() {
    const int small_size = 100;
    const int medium_size = 1000;
    const int large_size = 10000;
    
    /* Allocate test data */
    int *data1 = malloc(large_size * sizeof(int));
    int *data2 = malloc(medium_size * medium_size * sizeof(int));
    int *data3 = malloc(large_size * sizeof(int));
    
    if (!data1 || !data2 || !data3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(42);
    for (int i = 0; i < large_size; i++) {
        data1[i] = rand();
        data3[i] = rand();
    }
    for (int i = 0; i < medium_size * medium_size; i++) {
        data2[i] = rand();
    }
    
    int result1 = 0, result2 = 0, result3 = 0, result4 = 0;
    
    /* Warm-up phase */
    printf("Starting warm-up...\n");
    for (int iter = 0; iter < 5; iter++) {
        irreducible_cfg(data1, small_size, &result1);
        nested_loops_mixed_ops(data2, medium_size, medium_size, &result2);
    }
    
    /* Main computation with varying sizes */
    printf("Running main computations...\n");
    
    clock_t start = clock();
    
    /* Call complex functions with different patterns */
    irreducible_cfg(data1, large_size, &result1);
    nested_loops_mixed_ops(data2, 100, 100, &result2);
    parallelizable_work(data3, large_size, &result3);
    switch_with_loops(data1, medium_size, rand() % 8, &result4);
    
    /* Mix calls in loops */
    for (int i = 0; i < 3; i++) {
        int temp;
        irreducible_cfg(data3, small_size, &temp);
        result1 += temp;
        
        nested_loops_mixed_ops(data2, 50, 50, &temp);
        result2 += temp;
        
        switch_with_loops(data1, small_size, i % 8, &temp);
        result4 += temp;
    }
    
    clock_t end = clock();
    
    /* Compute final checksum */
    unsigned long long checksum = 0;
    checksum += result1;
    checksum += result2;
    checksum += result3;
    checksum += result4;
    
    /* Add array checksums */
    for (int i = 0; i < 1000; i++) {
        checksum += data1[i % large_size];
        checksum += data3[i % large_size];
    }
    
    printf("Results: %d, %d, %d, %d\n", result1, result2, result3, result4);
    printf("Checksum: %llu\n", checksum);
    printf("Time: %.2f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    
    return 0;
}
