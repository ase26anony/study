/* haifa_scheduler_test.c
 * Designed to trigger free_sched_context logic in GCC's Haifa scheduler
 * Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -finline-functions -funswitch-loops -fprofile-generate -o test_gen haifa_scheduler_test.c
 * Run: ./test_gen
 * Recompile: gcc -O3 -fschedule-insns2 -fprofile-use -finline-functions -funswitch-loops -o test_use haifa_scheduler_test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100000
#define WARMUP_ITERATIONS 1000

/* Helper functions marked for inlining */
static inline unsigned int __attribute__((always_inline)) 
compute_hash(unsigned int x) {
    /* Mix of arithmetic and bitwise ops for ILP */
    x = (x ^ 61) ^ (x >> 16);
    x = x + (x << 3);
    x = x ^ (x >> 4);
    x = x * 0x27d4eb2d;
    x = x ^ (x >> 15);
    return x;
}

static inline int __attribute__((always_inline))
complex_condition(int a, int b, int c) {
    /* Deep conditional chain */
    if (a > b) {
        if (b < c) {
            return a * c - b;
        } else if (a == c) {
            return (a << 3) | (b & 0xF);
        } else {
            return (a ^ b) + (c * 2);
        }
    } else if (a == b) {
        return (a & b) | (c << 16);
    } else {
        if (c > 0) {
            return (a + b) * c;
        } else {
            return (a - b) ^ c;
        }
    }
}

/* Hot function with complex control flow */
__attribute__((hot, noinline))
void process_array_complex(int* restrict arr, int size, int seed) {
    int i, j, k;
    volatile int barrier = 0; /* Prevent too much optimization */
    
    /* Nested loops with varying iteration patterns */
    for (i = 0; i < size; i++) {
        /* Switch statement inside loop */
        switch (i % 7) {
            case 0:
                arr[i] = compute_hash(arr[i] + seed) * 3;
                break;
            case 1:
                arr[i] = arr[i] ^ (seed << (i & 0xF));
                break;
            case 2:
                arr[i] = complex_condition(arr[i], seed, i);
                break;
            case 3:
                /* Memory operation with pointer arithmetic */
                *(arr + i) = *(arr + ((i + 1) % size)) + 
                            *(arr + ((i + 2) % size));
                break;
            case 4:
                /* Inline asm to create scheduling barrier */
                asm volatile("" : "+r" (arr[i]) : : "memory");
                arr[i] = arr[i] * 2 - seed;
                break;
            case 5:
                /* Loop with carried dependency */
                for (j = 0, k = arr[i]; j < 3; j++) {
                    k = compute_hash(k + j);
                }
                arr[i] = k;
                break;
            case 6:
            default:
                arr[i] = (arr[i] * 13) ^ (seed >> 3);
                break;
        }
        
        /* Additional conditional inside switch cases */
        if (arr[i] > 1000000) {
            arr[i] = arr[i] % 1000;
        } else if (arr[i] < -1000000) {
            arr[i] = -arr[i] & 0xFFF;
        }
        
        barrier = arr[i]; /* Use volatile to prevent dead code elimination */
    }
    
    /* Second loop with different pattern */
    for (i = size - 1; i >= 0; i -= 2) {
        int temp = arr[i];
        /* While loop inside for loop */
        while (temp > 0) {
            temp = temp >> 1;
            arr[i] += temp;
        }
        
        /* Function call from multiple sites */
        arr[i] = complex_condition(arr[i], arr[(i + 1) % size], seed);
    }
}

/* Function with irreducible control flow using computed goto */
__attribute__((noinline))
void irreducible_cfg(int* arr, int size) {
    static void* labels[] = { &&label0, &&label1, &&label2, &&label3, &&label4 };
    int state = 0;
    int i = 0;
    
    /* Computed goto creates irreducible control flow */
    goto *labels[state];
    
label0:
    for (i = 0; i < size; i += 4) {
        arr[i] = compute_hash(arr[i]);
        state = (arr[i] > 0) ? 1 : 2;
        goto *labels[state];
    }
    state = 3;
    goto *labels[state];
    
label1:
    for (i = 1; i < size; i += 4) {
        arr[i] = arr[i] * 7 + 13;
        state = (i % 3 == 0) ? 2 : 0;
        if (state == 2) goto *labels[state];
    }
    state = 4;
    goto *labels[state];
    
label2:
    for (i = 2; i < size; i += 4) {
        arr[i] = arr[i] ^ 0xABCDEF;
        state = (arr[i] & 1) ? 3 : 1;
        /* Mix of goto and break */
        if (state == 3) {
            goto *labels[state];
        }
    }
    state = 0;
    goto *labels[state];
    
label3:
    for (i = 3; i < size; i += 4) {
        arr[i] = complex_condition(arr[i], i, size);
        state = 4;
    }
    goto *labels[state];
    
label4:
    /* Final processing */
    for (i = 0; i < size; i++) {
        arr[i] = arr[i] & 0xFFFF;
    }
}

/* Vectorization candidate */
__attribute__((noinline))
void vectorizable_loop(int* restrict a, int* restrict b, int* restrict c, int size) {
    int i;
    /* Simple stride-1 loop for auto-vectorization */
    #pragma omp simd
    for (i = 0; i < size; i++) {
        a[i] = b[i] * 3 + c[i] * 7;
    }
    
    /* Second loop with reduction */
    int sum = 0;
    #pragma omp simd reduction(+:sum)
    for (i = 0; i < size; i++) {
        sum += a[i] * b[i];
        a[i] = sum & 0xFF;
    }
}

/* Function with mixed instruction types and dependencies */
__attribute__((hot))
void mixed_operations(int* arr, int size) {
    int i, j;
    
    /* Do-while loop */
    i = 0;
    do {
        /* Memory operations with dependencies */
        int t1 = arr[i];
        int t2 = arr[(i + 1) % size];
        int t3 = arr[(i + 2) % size];
        
        /* Chain of dependent operations */
        t1 = compute_hash(t1);
        t2 = t1 * t2 + t3;
        t3 = t2 ^ t1;
        t1 = t3 - t2;
        
        /* Store with address calculation */
        arr[i] = t1 + t2 + t3;
        
        /* Conditional with side effect */
        if (arr[i] > 1000) {
            for (j = 0; j < 2; j++) {
                arr[i] = arr[i] >> (j + 1);
            }
        }
        
        i = (i * 3 + 1) % size;
    } while (i != 0);
}

/* Main orchestrator */
int main() {
    int i, iter;
    unsigned long long checksum = 0;
    clock_t start, end;
    
    /* Allocate arrays with different alignments */
    int* array1 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int* array2 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int* array3 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    
    if (!array1 || !array2 || !array3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    srand(42);
    for (i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
        array3[i] = rand() % 1000;
    }
    
    printf("Starting scheduler stress test...\n");
    
    /* Warm-up phase */
    printf("Warm-up phase (%d iterations)...\n", WARMUP_ITERATIONS);
    start = clock();
    for (iter = 0; iter < WARMUP_ITERATIONS; iter++) {
        process_array_complex(array1, ARRAY_SIZE, iter);
        irreducible_cfg(array2, ARRAY_SIZE);
    }
    end = clock();
    printf("Warm-up completed in %.2f seconds\n", 
           (double)(end - start) / CLOCKS_PER_SEC);
    
    /* Main test phase */
    printf("\nMain test phase (%d iterations)...\n", ITERATIONS);
    start = clock();
    
    for (iter = 0; iter < ITERATIONS; iter++) {
        int mode = iter % 5;
        
        switch (mode) {
            case 0:
                process_array_complex(array1, ARRAY_SIZE, iter);
                break;
            case 1:
                irreducible_cfg(array2, ARRAY_SIZE);
                break;
            case 2:
                vectorizable_loop(array1, array2, array3, ARRAY_SIZE);
                break;
            case 3:
                mixed_operations(array3, ARRAY_SIZE);
                break;
            case 4:
                /* Combined operations */
                process_array_complex(array1, ARRAY_SIZE, iter);
                vectorizable_loop(array2, array1, array3, ARRAY_SIZE);
                irreducible_cfg(array3, ARRAY_SIZE);
                break;
        }
        
        /* Update checksum every 100 iterations */
        if (iter % 100 == 0) {
            for (i = 0; i < ARRAY_SIZE; i += 16) {
                checksum += array1[i] + array2[i] + array3[i];
            }
        }
    }
    
    end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Main test completed in %.2f seconds\n", elapsed);
    printf("Performance: %.2f iterations/sec\n", ITERATIONS / elapsed);
    
    /* Final checksum computation */
    printf("\nComputing final checksum...\n");
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum += array1[i] ^ array2[i] ^ array3[i];
        checksum = (checksum << 13) | (checksum >> (64 - 13)); /* Rotate */
    }
    
    printf("Final checksum: 0x%016llX\n", checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    
    return 0;
}
