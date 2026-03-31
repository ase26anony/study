/* Target: haifa-sched.cc free_sched_context cleanup block */
/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -finline-functions -funswitch-loops -fprofile-generate -frandom-seed=42 -o scheduler_test scheduler_test.c */
/* Then run: ./scheduler_test */
/* Then recompile with: gcc -O3 -fschedule-insns2 -fprofile-use -finline-functions -o scheduler_test_final scheduler_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <omp.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100000
#define MAX_DEPTH 8

/* Always inline helper functions to create complex dataflow */
static inline int __attribute__((always_inline)) compute_hash(int x, int y) {
    /* Mix of arithmetic and bitwise operations */
    int h = x ^ y;
    h = (h >> 16) ^ (h << 16);
    h = h * 0x5bd1e995;
    h = h ^ (h >> 15);
    return h;
}

static inline int __attribute__((always_inline)) scramble_bits(int val) {
    /* Complex bit manipulation with dependencies */
    int r = val;
    r = (r & 0x55555555) << 1 | (r & 0xAAAAAAAA) >> 1;
    r = (r & 0x33333333) << 2 | (r & 0xCCCCCCCC) >> 2;
    r = (r & 0x0F0F0F0F) << 4 | (r & 0xF0F0F0F0) >> 4;
    /* Artificial scheduling barrier */
    asm volatile("" : "+r" (r) : : "memory");
    r = r ^ (r >> 8);
    r = r * 0x9e3779b9;
    return r;
}

/* Hot function with complex control flow */
__attribute__((hot)) void complex_control_flow(int *arr, int size, int depth) {
    int i, j, k;
    volatile int counter = 0; /* Prevent optimization */
    
    /* Nested loops with varying iteration counts */
    for (i = 0; i < size; i += 2) {
        /* Deep if-else chain */
        if (arr[i] & 1) {
            if (arr[i] & 2) {
                if (arr[i] & 4) {
                    arr[i] = compute_hash(arr[i], i);
                } else {
                    arr[i] = scramble_bits(arr[i]);
                }
            } else {
                arr[i] = arr[i] * 3 + 1; /* Collatz-like */
            }
        } else {
            arr[i] = arr[i] / 2;
        }
        
        /* Switch statement with multiple cases */
        switch (arr[i] & 0x7) {
            case 0:
                arr[i] += compute_hash(i, depth);
                break;
            case 1:
                arr[i] ^= scramble_bits(depth);
                break;
            case 2:
                arr[i] *= 0x9e3779b9;
                break;
            case 3:
                arr[i] = (arr[i] << 3) | (arr[i] >> 29);
                break;
            case 4:
                arr[i] = ~arr[i];
                break;
            case 5:
                arr[i] = arr[i] + compute_hash(arr[i], i);
                break;
            case 6:
                arr[i] = arr[i] - scramble_bits(i);
                break;
            case 7:
                arr[i] = arr[i] ^ arr[i] >> 16;
                break;
        }
        
        /* Inner loop with carried dependency */
        for (j = 0; j < depth; j++) {
            int temp = arr[i];
            for (k = 0; k < 4; k++) {
                temp = compute_hash(temp, k);
                /* Memory operation creating load/store dependencies */
                arr[(i + k) % size] ^= temp;
            }
            counter++;
        }
    }
    
    /* Do-while loop */
    i = 0;
    do {
        arr[i] = scramble_bits(arr[i]);
        i = (i * 13 + 7) % size;
    } while (i != 0);
}

/* Function with irreducible control flow using computed goto */
__attribute__((noinline)) void irreducible_cfg(int *arr, int size) {
    static void *labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4, &&L5 };
    int state = arr[0] % 6;
    int i = 0;
    
    goto *labels[state];
    
L0:
    arr[i] = compute_hash(arr[i], i);
    i = (i + 1) % size;
    state = (state + 1) % 6;
    goto *labels[state];
    
L1:
    arr[i] = scramble_bits(arr[i]);
    i = (i * 3) % size;
    state = (state * 2) % 6;
    goto *labels[state];
    
L2:
    arr[i] = arr[i] ^ arr[(i + 1) % size];
    i = (i + 2) % size;
    state = (state + 3) % 6;
    goto *labels[state];
    
L3:
    arr[i] = arr[i] + compute_hash(i, state);
    i = (i + 5) % size;
    state = (state + 4) % 6;
    goto *labels[state];
    
L4:
    arr[i] = arr[i] * 0x9e3779b9;
    i = (i + 7) % size;
    state = (state * 3) % 6;
    goto *labels[state];
    
L5:
    arr[i] = ~arr[i];
    i = (i + 11) % size;
    state = (state + 2) % 6;
    if (i < size / 2) goto *labels[state];
    /* Fall through to return */
}

/* Vectorization candidate with OpenMP */
void vectorizable_loop(int *a, int *b, int *c, int size) {
    int i;
    #pragma omp simd safelen(16)
    for (i = 0; i < size; i++) {
        /* Simple stride-1 operations */
        a[i] = b[i] + c[i];
        b[i] = a[i] * 2 - c[i];
        c[i] = (b[i] << 1) | (a[i] >> 31);
    }
}

/* Tight inner loop with carried dependency */
void tight_inner_loop(int *arr, int size) {
    int i, j;
    for (i = 1; i < size; i++) {
        /* Carried dependency chain */
        int acc = arr[i - 1];
        for (j = 0; j < 8; j++) {
            acc = compute_hash(acc, j);
            acc = scramble_bits(acc);
            /* Memory dependency */
            arr[i] ^= acc;
        }
        arr[i] = acc;
    }
}

/* Function with mixed operations and memory accesses */
void mixed_operations(int *arr, int size) {
    int i, j;
    int *ptr = arr;
    
    for (i = 0; i < size; i++) {
        /* Pointer arithmetic creating complex addressing */
        int val = *(ptr + i);
        
        /* Mix of independent operations */
        int a = val * 3;
        int b = val ^ 0xdeadbeef;
        int c = val + compute_hash(i, val);
        int d = scramble_bits(val);
        
        /* Artificial asm barrier */
        asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d) : "memory");
        
        /* Store with complex addressing */
        arr[(i * 7 + 3) % size] = a + b;
        arr[(i * 13 + 5) % size] = c ^ d;
        arr[(i * 19 + 7) % size] = compute_hash(a, c);
        
        /* Function call from multiple sites */
        if (i % 3 == 0) {
            arr[i] = scramble_bits(arr[i]);
        } else if (i % 3 == 1) {
            arr[i] = compute_hash(arr[i], i);
        }
    }
}

/* Main orchestrator */
int main() {
    int i, iter;
    clock_t start, end;
    double cpu_time_used;
    
    /* Allocate arrays with different alignments */
    int *arr1 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int *arr2 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int *arr3 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    
    if (!arr1 || !arr2 || !arr3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(42);
    for (i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = rand();
        arr2[i] = rand();
        arr3[i] = rand();
    }
    
    printf("Starting scheduler stress test...\n");
    start = clock();
    
    /* Warm-up phase to trigger optimization heuristics */
    for (iter = 0; iter < ITERATIONS / 100; iter++) {
        complex_control_flow(arr1, ARRAY_SIZE / 2, 3);
        irreducible_cfg(arr2, ARRAY_SIZE / 2);
    }
    
    /* Main timed section with varying patterns */
    for (iter = 0; iter < ITERATIONS; iter++) {
        /* Vary parameters to trigger different scheduling paths */
        int depth = (iter % MAX_DEPTH) + 1;
        int size = ARRAY_SIZE / (1 + (iter % 4));
        
        /* Call different functions with complex control flow */
        if (iter % 5 == 0) {
            complex_control_flow(arr1, size, depth);
        } else if (iter % 5 == 1) {
            irreducible_cfg(arr2, size);
        } else if (iter % 5 == 2) {
            vectorizable_loop(arr1, arr2, arr3, size);
        } else if (iter % 5 == 3) {
            tight_inner_loop(arr1, size);
        } else {
            mixed_operations(arr2, size);
        }
        
        /* Cross-call dependencies */
        arr1[iter % ARRAY_SIZE] ^= compute_hash(arr2[iter % ARRAY_SIZE], iter);
        arr2[iter % ARRAY_SIZE] = scramble_bits(arr3[iter % ARRAY_SIZE]);
    }
    
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    /* Compute final checksum for verification */
    unsigned long long checksum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum += (unsigned long long)arr1[i];
        checksum ^= (unsigned long long)arr2[i] << 32;
        checksum = (checksum >> 1) | (checksum << 63); /* Rotate */
    }
    
    printf("Execution time: %.2f seconds\n", cpu_time_used);
    printf("Final checksum: 0x%016llx\n", checksum);
    printf("Array1[0]=%d, Array2[0]=%d, Array3[0]=%d\n", 
           arr1[0], arr2[0], arr3[0]);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    
    return 0;
}
