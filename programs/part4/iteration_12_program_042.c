/* haifa_scheduler_stress.c
 * Designed to stress GCC's Haifa scheduler and trigger free_sched_context
 * Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -finline-functions -funswitch-loops -frandom-seed=1 haifa_scheduler_stress.c -o haifa_test
 * For FDO: gcc -O2 -fprofile-generate haifa_scheduler_stress.c -o haifa_test_gen
 *           ./haifa_test_gen
 *           gcc -O3 -fschedule-insns2 -fprofile-use haifa_scheduler_stress.c -o haifa_test_fdo
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100000
#define WARMUP_ITERATIONS 1000

/* Always inline helper functions to create complex dataflow */
static inline int __attribute__((always_inline)) 
compute_hash(int a, int b, int c) {
    /* Mix of arithmetic and bitwise ops */
    int t = a ^ b;
    t = t * 0x5bd1e995;
    t = t ^ (t >> 15);
    t = t + c;
    t = t * 0x27d4eb2d;
    return t ^ (t >> 16);
}

static inline int __attribute__((always_inline))
conditional_transform(int x, int selector) {
    /* Deep if-else chain */
    if (selector == 0) return x + 1;
    else if (selector == 1) return x * 2;
    else if (selector == 2) return x ^ 0xAAAAAAAA;
    else if (selector == 3) return x - (x >> 3);
    else if (selector == 4) return (x << 1) | (x >> 31);
    else if (selector == 5) return x * 3 + 7;
    else if (selector == 6) return x ^ (x << 13);
    else if (selector == 7) return x | 0x55555555;
    else return x;
}

/* Hot function with complex control flow */
__attribute__((hot))
void complex_control_flow(int *data, int size, int *result) {
    int i, j, k;
    volatile int barrier = 0; /* Force memory barrier */
    
    /* Nested loops with varying iteration counts */
    for (i = 0; i < size; i++) {
        int acc = data[i];
        
        /* Inner loop with carried dependency */
        for (j = 0; j < 8; j++) {
            acc = compute_hash(acc, j, i);
            
            /* Switch statement inside loop */
            switch (acc & 7) {
                case 0: acc += data[(i + j) % size]; break;
                case 1: acc ^= data[(i - j + size) % size]; break;
                case 2: acc *= 3; break;
                case 3: acc = (acc << 3) | (acc >> 29); break;
                case 4: acc -= barrier; break;
                case 5: acc = acc & 0x0F0F0F0F; break;
                case 6: acc = acc | 0x80808080; break;
                case 7: acc = ~acc; break;
            }
            
            /* Inline asm to create scheduling barrier */
            asm volatile("" ::: "memory");
        }
        
        result[i] = conditional_transform(acc, i & 7);
    }
}

/* Function with irreducible control flow using computed goto */
__attribute__((noinline))
void irreducible_cfg(int *data, int size) {
    static void *labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4, &&L5, &&L6, &&L7 };
    int i = 0;
    int state = 0;
    
    L0:
        data[i] += 1;
        state = (state + 1) & 7;
        goto *labels[state];
    
    L1:
        data[i] ^= 0x55;
        state = (state * 3 + 1) & 7;
        i = (i + 1) % size;
        goto *labels[state];
    
    L2:
        data[i] *= 2;
        state = (state ^ 5) & 7;
        goto *labels[state];
    
    L3:
        data[i] = data[i] >> 1;
        state = (state + 2) & 7;
        goto *labels[state];
    
    L4:
        data[i] = ~data[i];
        state = (state * 5) & 7;
        i = (i + 2) % size;
        goto *labels[state];
    
    L5:
        data[i] = data[i] & 0x0F;
        state = (state - 1) & 7;
        goto *labels[state];
    
    L6:
        data[i] = data[i] | 0xF0;
        state = (state ^ 3) & 7;
        i = (i + 3) % size;
        goto *labels[state];
    
    L7:
        if (i < size - 1) {
            data[i] += data[i + 1];
            state = 0;
            goto *labels[state];
        }
        return;
}

/* Loop with auto-vectorization potential */
#pragma omp declare simd
__attribute__((hot))
void vectorizable_loop(int *a, int *b, int *c, int size) {
    #pragma omp simd
    for (int i = 0; i < size; i++) {
        /* Simple stride-1 operations for vectorization */
        int t = a[i] * 3 + b[i];
        t = t ^ (t >> 4);
        c[i] = t * 7 - b[i];
    }
}

/* Function with mixed operations and memory dependencies */
void mixed_operations(int *arr, int n) {
    int i, j;
    
    /* Outer loop with loop unrolling candidate */
    for (i = 0; i < n; i += 4) {
        int sum0 = 0, sum1 = 0, sum2 = 0, sum3 = 0;
        
        /* Inner loop with multiple accumulators */
        for (j = 0; j < 16; j++) {
            /* Memory operations with pointer arithmetic */
            int idx = (i + j) % n;
            sum0 += arr[idx] * j;
            sum1 ^= arr[(idx + 1) % n];
            sum2 = sum2 * 3 + arr[(idx + 2) % n];
            sum3 = (sum3 << 1) | (arr[(idx + 3) % n] & 1);
            
            /* Function call that will be inlined */
            sum0 = conditional_transform(sum0, j & 3);
        }
        
        /* Store results with memory barrier */
        arr[i] = sum0;
        asm volatile("" ::: "memory");
        arr[i + 1] = sum1;
        arr[i + 2] = sum2;
        arr[i + 3] = sum3;
    }
}

/* Main orchestrator */
int main() {
    int *data1 = malloc(ARRAY_SIZE * sizeof(int));
    int *data2 = malloc(ARRAY_SIZE * sizeof(int));
    int *data3 = malloc(ARRAY_SIZE * sizeof(int));
    int *result = malloc(ARRAY_SIZE * sizeof(int));
    
    /* Initialize with pseudo-random data */
    srand(42);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data1[i] = rand();
        data2[i] = rand();
        data3[i] = rand();
        result[i] = 0;
    }
    
    printf("Starting scheduler stress test...\n");
    clock_t start = clock();
    
    /* Warm-up phase */
    printf("Warm-up phase...\n");
    for (int iter = 0; iter < WARMUP_ITERATIONS; iter++) {
        complex_control_flow(data1, ARRAY_SIZE / 4, result);
        irreducible_cfg(data2, ARRAY_SIZE / 4);
    }
    
    /* Main test phase with varying patterns */
    printf("Main test phase...\n");
    long long checksum = 0;
    
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Alternate between different functions to stress different scheduling paths */
        switch (iter & 3) {
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
                mixed_operations(data3, ARRAY_SIZE);
                break;
        }
        
        /* Compute checksum for verification */
        for (int i = 0; i < ARRAY_SIZE; i += 16) {
            checksum += data1[i] + data2[i] + data3[i] + result[i];
            checksum = compute_hash(checksum, i, iter);
        }
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    /* Final verification computation */
    int final_hash = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_hash ^= data1[i];
        final_hash = final_hash * 0x5bd1e995 + data2[i];
        final_hash ^= data3[i];
        final_hash = (final_hash << 13) | (final_hash >> 19);
    }
    
    printf("Test completed in %.2f seconds\n", elapsed);
    printf("Checksum: %lld\n", checksum);
    printf("Final hash: %d\n", final_hash);
    printf("Result[0] = %d, Result[%d] = %d\n", 
           result[0], ARRAY_SIZE-1, result[ARRAY_SIZE-1]);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    free(result);
    
    return 0;
}
