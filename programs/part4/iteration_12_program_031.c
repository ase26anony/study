/* Test program to trigger free_sched_context in GCC's Haifa scheduler */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 10000
#define MAX_DEPTH 8

/* Always inline helper functions */
static inline int __attribute__((always_inline)) 
compute_hash(int a, int b, int c) {
    return (a * 31 + b) ^ (c * 17);
}

static inline int __attribute__((always_inline))
bitwise_mix(int x, int y) {
    return (x & 0xFF00FF00) | (y & 0x00FF00FF);
}

static inline void __attribute__((always_inline))
memory_barrier(void) {
    asm volatile("" ::: "memory");
}

/* Hot function with complex control flow */
__attribute__((hot))
void complex_control_flow(int *data, int size, int *result) {
    int i, j, k;
    volatile int counter = 0; /* Prevent optimization */
    
    /* Nested loops with varying iteration counts */
    for (i = 0; i < size; i++) {
        /* Deep if-else chain */
        if (data[i] < 0) {
            for (j = 0; j < i % 16; j++) {
                /* Mix of arithmetic operations */
                int temp = data[i] * j;
                temp += compute_hash(i, j, temp);
                temp = bitwise_mix(temp, ~temp);
                
                /* Memory operation with dependency */
                result[j % MAX_DEPTH] += temp;
                
                /* Artificial scheduling barrier */
                memory_barrier();
            }
        } else if (data[i] < 100) {
            int sum = 0;
            /* Switch statement with multiple cases */
            switch (data[i] % 7) {
                case 0: sum = data[i] * 2; break;
                case 1: sum = data[i] / 2; break;
                case 2: sum = data[i] << 1; break;
                case 3: sum = data[i] >> 1; break;
                case 4: sum = data[i] | 0x5555; break;
                case 5: sum = data[i] & 0xAAAA; break;
                case 6: sum = data[i] ^ 0x3333; break;
            }
            result[i % MAX_DEPTH] += sum;
        } else {
            /* Do-while loop */
            k = 0;
            do {
                /* Pointer arithmetic and array access */
                int *ptr = &data[i];
                ptr += k % 4;
                result[k % MAX_DEPTH] += *ptr * k;
                k++;
            } while (k < 4);
        }
        
        counter++;
    }
}

/* Function with irreducible control flow using computed goto */
__attribute__((noinline))
void irreducible_cfg(int *data, int size) {
    static void *labels[] = {
        &&L0, &&L1, &&L2, &&L3, &&L4, &&L5
    };
    
    int i = 0;
    int state = 0;
    
    /* Computed goto creates irreducible control flow */
    goto *labels[state];
    
L0:
    data[i] = compute_hash(data[i], i, state);
    state = (state + 1) % 6;
    if (++i < size) goto *labels[state];
    else return;
    
L1:
    data[i] = bitwise_mix(data[i], i);
    state = (state * 3 + 1) % 6;
    if (++i < size) goto *labels[state];
    else return;
    
L2:
    data[i] = data[i] * 31 + 17;
    state = (state + 5) % 6;
    if (++i < size) goto *labels[state];
    else return;
    
L3:
    data[i] = data[i] ^ 0xDEADBEEF;
    state = (state * 2) % 6;
    if (++i < size) goto *labels[state];
    else return;
    
L4:
    data[i] = ~data[i];
    state = (state + 3) % 6;
    if (++i < size) goto *labels[state];
    else return;
    
L5:
    data[i] = (data[i] << 3) | (data[i] >> 29);
    state = (state * 7 + 1) % 6;
    if (++i < size) goto *labels[state];
    else return;
}

/* Function with vectorization candidate */
__attribute__((hot))
void vectorizable_loop(int *a, int *b, int *c, int size) {
    int i;
    
    /* Simple stride-1 loop for auto-vectorization */
    #pragma omp simd
    for (i = 0; i < size; i++) {
        a[i] = b[i] * 3 + c[i] * 7;
    }
    
    /* Loop with carried dependency */
    for (i = 1; i < size; i++) {
        a[i] += a[i-1] * 2;
    }
}

/* Function with OpenMP parallelization */
void parallel_region(int *data, int size) {
    int i;
    
    #pragma omp parallel for schedule(dynamic, 16)
    for (i = 0; i < size; i++) {
        int j;
        /* Inner loop with varying bounds */
        for (j = 0; j < (i % 32) + 1; j++) {
            data[i] = compute_hash(data[i], j, i);
        }
    }
}

/* Main orchestrator */
int main(void) {
    int i, j;
    int *data1, *data2, *data3, *result;
    int checksum = 0;
    clock_t start, end;
    
    /* Allocate arrays with different alignments */
    data1 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    data2 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    data3 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    result = (int*)calloc(MAX_DEPTH, sizeof(int));
    
    if (!data1 || !data2 || !data3 || !result) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    srand(42);
    for (i = 0; i < ARRAY_SIZE; i++) {
        data1[i] = rand() % 256;
        data2[i] = rand() % 256;
        data3[i] = rand() % 256;
    }
    
    printf("Starting scheduler stress test...\n");
    
    /* Warm-up phase */
    printf("Warm-up phase...\n");
    start = clock();
    for (j = 0; j < ITERATIONS / 100; j++) {
        complex_control_flow(data1, ARRAY_SIZE / 4, result);
        irreducible_cfg(data2, ARRAY_SIZE / 4);
    }
    end = clock();
    printf("Warm-up completed in %.2f seconds\n", 
           (double)(end - start) / CLOCKS_PER_SEC);
    
    /* Main test phase with varying array sizes */
    printf("\nMain test phase...\n");
    start = clock();
    
    for (j = 0; j < ITERATIONS; j++) {
        int size = ARRAY_SIZE / ((j % 8) + 1);
        
        /* Alternate between different patterns */
        switch (j % 4) {
            case 0:
                complex_control_flow(data1, size, result);
                break;
            case 1:
                irreducible_cfg(data2, size);
                break;
            case 2:
                vectorizable_loop(data1, data2, data3, size);
                break;
            case 3:
                parallel_region(data3, size);
                break;
        }
        
        /* Mix in some inline assembly for scheduling barriers */
        if (j % 100 == 0) {
            asm volatile(
                "mfence\n\t"
                "lfence\n\t"
                ::: "memory"
            );
        }
    }
    
    end = clock();
    printf("Main test completed in %.2f seconds\n", 
           (double)(end - start) / CLOCKS_PER_SEC);
    
    /* Compute final checksum */
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum ^= data1[i];
        checksum ^= data2[i];
        checksum ^= data3[i];
    }
    for (i = 0; i < MAX_DEPTH; i++) {
        checksum ^= result[i];
    }
    
    printf("\nFinal checksum: 0x%08X\n", checksum);
    printf("Test completed successfully.\n");
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    free(result);
    
    return 0;
}
