/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -finline-functions -funswitch-loops -frandom-seed=1 -fprofile-generate -o test test.c */
/* After running: ./test */
/* Recompile with: gcc -O3 -fschedule-insns2 -fprofile-use -finline-functions -o test_opt test.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100000
#define WARMUP_ITERATIONS 1000

/* Helper functions marked for inlining */
__attribute__((always_inline)) static inline int compute_hash(int x, int y) {
    /* Mix of arithmetic and bitwise operations */
    return ((x << 5) ^ y) + (x >> 3) * 7;
}

__attribute__((always_inline)) static inline int barrier_op(int x) {
    int result;
    /* Memory barrier via asm to create scheduling constraints */
    asm volatile ("mfence" ::: "memory");
    result = x * 3 + 1;
    asm volatile ("" ::: "memory");
    return result;
}

__attribute__((always_inline)) static inline int complex_transform(int val) {
    /* Multiple dependent operations */
    int a = val * 2;
    int b = a ^ 0x5A5A5A5A;
    int c = b + (val >> 4);
    int d = c * 3 - 1;
    return barrier_op(d);
}

/* Hot function with complex control flow */
__attribute__((hot)) void process_array_complex(int *arr, int size, int mode) {
    int i, j, k;
    
    /* Nested loops with varying patterns */
    for (i = 0; i < size; i++) {
        /* Deep if-else chain */
        if (mode == 0) {
            arr[i] = compute_hash(arr[i], i);
        } else if (mode == 1) {
            for (j = 0; j < 3; j++) {
                arr[i] += complex_transform(arr[i] + j);
            }
        } else if (mode == 2) {
            /* Switch statement */
            switch (arr[i] & 0x7) {
                case 0: arr[i] = arr[i] * 2; break;
                case 1: arr[i] = arr[i] ^ 0xFF; break;
                case 2: arr[i] = arr[i] + compute_hash(arr[i], i); break;
                case 3: arr[i] = barrier_op(arr[i]); break;
                case 4: arr[i] = arr[i] >> 2; break;
                case 5: arr[i] = arr[i] << 1; break;
                case 6: arr[i] = arr[i] | 0xAA; break;
                case 7: arr[i] = arr[i] & 0x55; break;
                default: arr[i] = 0;
            }
        } else {
            /* Mixed operations */
            arr[i] = (arr[i] * 3 + 1) ^ (arr[i] >> 1);
        }
        
        /* Inner loop with carried dependency */
        int temp = arr[i];
        for (k = 0; k < 2; k++) {
            temp = (temp * 1103515245 + 12345) & 0x7FFFFFFF;
        }
        arr[i] = temp;
    }
}

/* Function with irreducible control flow using computed goto */
__attribute__((noinline)) void irreducible_cfg(int *arr, int size) {
    static void *labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4, &&L5 };
    int i = 0;
    int state = 0;
    
    L0:
        arr[i] += 1;
        state = (state + 1) % 6;
        goto *labels[state];
    
    L1:
        arr[i] ^= 0xAA;
        i = (i + 1) % size;
        state = (state * 3) % 6;
        goto *labels[state];
    
    L2:
        arr[i] *= 2;
        state = (state + 2) % 6;
        goto *labels[state];
    
    L3:
        arr[i] = barrier_op(arr[i]);
        i = (i + size - 1) % size;
        state = (state + 4) % 6;
        goto *labels[state];
    
    L4:
        arr[i] = compute_hash(arr[i], i);
        state = (state + 1) % 6;
        if (i == 0) goto L5;
        goto *labels[state];
    
    L5:
        return;
}

/* Function with vectorization candidate */
__attribute__((hot)) void vectorizable_loop(int *a, int *b, int *c, int size) {
    #pragma omp simd
    for (int i = 0; i < size; i++) {
        /* Simple stride-1 operations */
        a[i] = b[i] * 3 + c[i];
        c[i] = a[i] ^ b[i];
        b[i] = barrier_op(c[i]);
    }
}

/* Function with OpenMP parallelization */
void parallel_processing(int *arr, int size) {
    #pragma omp parallel for schedule(dynamic, 16)
    for (int i = 0; i < size; i++) {
        int val = arr[i];
        /* Complex operation mix */
        for (int j = 0; j < 8; j++) {
            val = complex_transform(val);
            if (j % 3 == 0) {
                val += compute_hash(val, j);
            } else if (j % 3 == 1) {
                val ^= 0xCCCCCCCC;
            } else {
                val = (val << 3) | (val >> 29);
            }
        }
        arr[i] = val;
    }
}

/* Main orchestrator */
int main() {
    int *array1 = malloc(ARRAY_SIZE * sizeof(int));
    int *array2 = malloc(ARRAY_SIZE * sizeof(int));
    int *array3 = malloc(ARRAY_SIZE * sizeof(int));
    
    /* Initialize arrays */
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
        array3[i] = rand() % 1000;
    }
    
    clock_t start, end;
    long long checksum = 0;
    
    /* Warm-up phase */
    printf("Warm-up phase...\n");
    for (int iter = 0; iter < WARMUP_ITERATIONS; iter++) {
        process_array_complex(array1, ARRAY_SIZE, iter % 4);
    }
    
    /* Main processing with different patterns */
    printf("Main processing phase...\n");
    start = clock();
    
    for (int iter = 0; iter < ITERATIONS; iter++) {
        int mode = iter % 5;
        
        /* Alternate between different functions */
        switch (mode) {
            case 0:
                process_array_complex(array1, ARRAY_SIZE, iter % 4);
                break;
            case 1:
                irreducible_cfg(array2, ARRAY_SIZE);
                break;
            case 2:
                vectorizable_loop(array1, array2, array3, ARRAY_SIZE);
                break;
            case 3:
                parallel_processing(array3, ARRAY_SIZE);
                break;
            case 4:
                /* Nested loop with complex dependencies */
                for (int i = 0; i < ARRAY_SIZE; i++) {
                    int val = array1[i];
                    for (int j = 0; j < 4; j++) {
                        val = complex_transform(val);
                        if (val % 2 == 0) {
                            val = compute_hash(val, j);
                        } else {
                            val = barrier_op(val);
                        }
                    }
                    array1[i] = val;
                }
                break;
        }
        
        /* Update checksum */
        if (iter % 100 == 0) {
            for (int i = 0; i < ARRAY_SIZE; i++) {
                checksum += array1[i] + array2[i] * 2 + array3[i] * 3;
            }
        }
    }
    
    end = clock();
    
    /* Final checksum computation */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += array1[i] ^ array2[i] ^ array3[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    printf("Time: %.2f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    
    free(array1);
    free(array2);
    free(array3);
    
    return 0;
}
