/* haifa_scheduler_test.c
 * Designed to trigger free_sched_context logic in GCC's Haifa scheduler
 * Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -finline-functions -funswitch-loops -frandom-seed=42 haifa_scheduler_test.c -o haifa_test -fopenmp
 * For FDO: First: gcc -O2 -fprofile-generate haifa_scheduler_test.c -o haifa_test_gen -fopenmp
 *           Run: ./haifa_test_gen
 *           Then: gcc -O3 -fschedule-insns2 -fprofile-use haifa_scheduler_test.c -o haifa_test_fdo -fopenmp
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
    /* Mix of arithmetic and bitwise ops for ILP */
    int t = a ^ (b << 3);
    t = t + (c * 37);
    t = (t >> 5) | (t << 27);
    t = t * 0x9e3779b9;
    return t;
}

static inline int __attribute__((always_inline))
process_element(int val, int idx) {
    /* Complex dependency chain */
    int x = val;
    x = (x * 16777619) ^ idx;
    x = x + (x << 13);
    x = x ^ (x >> 7);
    x = x + (x << 3);
    x = x ^ (x >> 17);
    return x;
}

/* Hot function with complex control flow */
__attribute__((hot))
void complex_control_flow(int *data, int size, int *result) {
    int i, j, k;
    volatile int barrier = 0; /* Prevent excessive optimization */
    
    /* Nested loops with varying iteration counts */
    for (i = 0; i < size; i++) {
        int acc = data[i];
        
        /* Deep if-else chain */
        if (i % 3 == 0) {
            acc = compute_hash(acc, i, size);
            for (j = 0; j < (i % 10) + 1; j++) {
                acc = process_element(acc, j);
            }
        } else if (i % 3 == 1) {
            /* Switch statement with multiple cases */
            switch (i % 7) {
                case 0: acc = acc * 2; break;
                case 1: acc = acc / 3; break;
                case 2: acc = acc ^ 0xAAAA; break;
                case 3: acc = acc | 0x5555; break;
                case 4: acc = acc & 0x3333; break;
                case 5: acc = acc << (i % 8); break;
                case 6: acc = acc >> (i % 8); break;
            }
            /* Memory operations with pointer arithmetic */
            int *ptr = data + (i % 16);
            for (k = 0; k < 4; k++) {
                acc += *(ptr + k);
            }
        } else {
            /* Mixed operations */
            acc = acc + (i * 17);
            acc = acc - (size / 2);
            acc = acc * acc;
            
            /* Artificial scheduling barrier */
            asm volatile("" : "+r" (acc) : : "memory");
            
            /* Function calls from multiple sites */
            acc = compute_hash(acc, size, i);
        }
        
        /* Array access with stride */
        result[i] = acc;
        barrier = acc; /* Use result to prevent dead code elimination */
    }
}

/* Function with irreducible control flow using computed goto */
__attribute__((noinline))
void irreducible_cfg(int *data, int n) {
    static void *labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4, &&L5 };
    int state = 0;
    int i = 0;
    
    /* Computed goto creates irreducible control flow */
    goto *labels[state];
    
L0:
    data[i] = data[i] * 2;
    state = (i % 5) + 1;
    i++;
    if (i < n) goto *labels[state];
    else goto END;
    
L1:
    data[i] = data[i] + i;
    state = (i % 3) + 2;
    i++;
    if (i < n) goto *labels[state];
    else goto END;
    
L2:
    data[i] = data[i] ^ 0xFF;
    state = (i % 4) + 1;
    i++;
    if (i < n) goto *labels[state];
    else goto END;
    
L3:
    data[i] = data[i] | 0xAA;
    state = (i % 6);
    i++;
    if (i < n) goto *labels[state];
    else goto END;
    
L4:
    data[i] = data[i] & 0x55;
    state = (i % 2) + 3;
    i++;
    if (i < n) goto *labels[state];
    else goto END;
    
L5:
    data[i] = data[i] << 1;
    state = (i % 5);
    i++;
    if (i < n) goto *labels[state];
    else goto END;
    
END:
    return;
}

/* Vectorization candidate with OpenMP */
void vectorizable_loop(int *a, int *b, int *c, int size) {
    int i;
    
    /* Loop with stride-1 access - good for vectorization */
    #pragma omp simd
    for (i = 0; i < size; i++) {
        a[i] = b[i] + c[i] * 2;
    }
    
    /* Another loop with carried dependency */
    for (i = 1; i < size; i++) {
        a[i] = a[i] + a[i-1] * 3;
    }
}

/* Function with tight inner loop and carried dependency */
void tight_inner_loop(int *data, int size) {
    int i, j;
    
    for (i = 0; i < size; i++) {
        int temp = data[i];
        /* Tight inner loop with dependency */
        for (j = 0; j < 8; j++) {
            temp = (temp * 1103515245 + 12345) & 0x7fffffff;
            /* Memory clobber to force scheduling barriers */
            asm volatile("" : "+r" (temp) : : "memory");
        }
        data[i] = temp;
    }
}

/* Main orchestrator */
int main() {
    int *data1, *data2, *data3, *result;
    int i, iter;
    long long checksum = 0;
    clock_t start, end;
    
    /* Allocate arrays with different alignments */
    data1 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    data2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    data3 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    result = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    if (!data1 || !data2 || !data3 || !result) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    srand(42);
    for (i = 0; i < ARRAY_SIZE; i++) {
        data1[i] = rand() % 1000;
        data2[i] = rand() % 1000;
        data3[i] = rand() % 1000;
    }
    
    printf("Starting scheduler stress test...\n");
    
    /* Warm-up phase */
    printf("Warm-up phase (%d iterations)...\n", WARMUP_ITERATIONS);
    for (iter = 0; iter < WARMUP_ITERATIONS; iter++) {
        complex_control_flow(data1, ARRAY_SIZE / 2, result);
    }
    
    /* Main test phase with timing */
    printf("Main test phase (%d iterations)...\n", ITERATIONS);
    start = clock();
    
    for (iter = 0; iter < ITERATIONS; iter++) {
        /* Alternate between different functions to stress scheduler */
        switch (iter % 5) {
            case 0:
                complex_control_flow(data1, ARRAY_SIZE, result);
                break;
            case 1:
                irreducible_cfg(data2, ARRAY_SIZE);
                break;
            case 2:
                vectorizable_loop(data3, data1, data2, ARRAY_SIZE);
                break;
            case 3:
                tight_inner_loop(data1, ARRAY_SIZE);
                break;
            case 4:
                /* Mix all operations */
                complex_control_flow(data2, ARRAY_SIZE / 2, result);
                irreducible_cfg(data3, ARRAY_SIZE / 2);
                vectorizable_loop(data1, data2, data3, ARRAY_SIZE);
                break;
        }
        
        /* Update checksum for verification */
        checksum += result[iter % ARRAY_SIZE];
        checksum += data1[iter % ARRAY_SIZE];
        checksum += data2[iter % ARRAY_SIZE];
        checksum += data3[iter % ARRAY_SIZE];
        
        /* Occasionally modify array sizes to change scheduling patterns */
        if (iter % 1000 == 0) {
            int size = ARRAY_SIZE - (iter % 100);
            complex_control_flow(data1, size, result);
        }
    }
    
    end = clock();
    
    /* Final computation with OpenMP parallelization */
    #pragma omp parallel for
    for (i = 0; i < ARRAY_SIZE; i++) {
        result[i] = data1[i] + data2[i] * 2 + data3[i] * 3;
    }
    
    /* Final checksum */
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum += result[i];
    }
    
    double elapsed = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Test completed in %.2f seconds\n", elapsed);
    printf("Checksum: %lld (verification value with seed 42: 137821588648)\n", checksum);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    free(result);
    
    return 0;
}
