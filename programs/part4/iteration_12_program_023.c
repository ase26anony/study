/* Test program to trigger free_sched_context in GCC's Haifa scheduler */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000
#define MAX_DEPTH 8

/* Always-inline helper functions to create scheduling complexity */
static inline int __attribute__((always_inline)) 
compute_hash(int x, int y) {
    /* Mix of arithmetic and bitwise operations */
    return ((x << 3) ^ (y >> 5)) + (x * y) - (x & y) | (x ^ y);
}

static inline int __attribute__((always_inline))
conditional_transform(int val, int threshold) {
    /* Complex conditional chain */
    if (val < threshold) {
        return val * 2 + 1;
    } else if (val < threshold * 2) {
        return (val ^ 0x5A) & 0xFF;
    } else if (val < threshold * 3) {
        return val % 256;
    } else {
        return ~val & 0x7F;
    }
}

/* Hot function with complex control flow */
__attribute__((hot, noinline))
void complex_control_flow(int *data, int size, int depth) {
    int i, j, k;
    volatile int barrier = 0; /* Prevent optimization */
    
    /* Nested loops with varying iteration counts */
    for (i = 0; i < size; i += 2) {
        /* Switch statement inside loop */
        switch (i % 5) {
            case 0:
                data[i] = compute_hash(data[i], i);
                /* Nested if-else chain */
                if (depth > 0) {
                    if (i % 3 == 0) {
                        data[i] = conditional_transform(data[i], 100);
                    } else if (i % 3 == 1) {
                        data[i] = data[i] ^ 0xAA;
                    } else {
                        data[i] = data[i] * 3 - 1;
                    }
                }
                break;
            case 1:
                /* Memory operations with pointer arithmetic */
                int *ptr = &data[i];
                *ptr = (*ptr << 2) | 0x0F;
                ptr++;
                *ptr = compute_hash(*ptr, *ptr);
                break;
            case 2:
                /* While loop inside for loop */
                j = 0;
                while (j < 3) {
                    data[i] += j * depth;
                    j++;
                }
                break;
            case 3:
                /* Do-while loop */
                k = 0;
                do {
                    data[i] = (data[i] + k) % 256;
                    k++;
                } while (k < 4);
                break;
            case 4:
                /* Artificial scheduling barrier */
                asm volatile("" ::: "memory");
                data[i] = data[i] * data[i] - data[i];
                break;
        }
        
        /* Computed goto for irreducible control flow */
        if (depth > 1 && (i % 7 == 0)) {
            static void *labels[] = { &&L0, &&L1, &&L2, &&L3 };
            goto *labels[i % 4];
            
            L0:
                data[i] += 1;
                goto end_label;
            L1:
                data[i] *= 2;
                goto end_label;
            L2:
                data[i] ^= 0x55;
                goto end_label;
            L3:
                data[i] = ~data[i];
                goto end_label;
            end_label:;
        }
        
        /* Memory clobber to force scheduler work */
        asm volatile("" : : : "memory");
    }
    
    /* Prevent dead code elimination */
    barrier = data[0];
}

/* Function with vectorization candidate */
__attribute__((hot))
void vectorizable_loop(int *a, int *b, int *c, int n) {
    int i;
    
    /* Simple stride-1 loop - good for auto-vectorization */
    #pragma omp simd
    for (i = 0; i < n; i++) {
        a[i] = b[i] * c[i] + i;
    }
    
    /* Loop with carried dependency */
    for (i = 1; i < n; i++) {
        a[i] = a[i-1] + b[i] * 2;
    }
}

/* Function with OpenMP parallelization */
void parallel_region(int *data, int size) {
    int i;
    
    #pragma omp parallel for schedule(dynamic)
    for (i = 0; i < size; i++) {
        /* Independent operations */
        int temp = data[i];
        temp = compute_hash(temp, i);
        temp = conditional_transform(temp, 128);
        
        /* Memory operation */
        data[i] = temp;
        
        /* Inline asm for scheduling complexity */
        asm volatile("" : "+r" (temp) : : "cc", "memory");
    }
}

/* Deeply recursive control flow */
__attribute__((noinline))
void nested_control_structures(int *arr, int n, int depth) {
    int i, j;
    
    for (i = 0; i < n; i++) {
        /* Deep if-else chain */
        if (i % 2 == 0) {
            if (i % 4 == 0) {
                if (i % 8 == 0) {
                    if (i % 16 == 0) {
                        arr[i] = 1;
                    } else {
                        arr[i] = 2;
                    }
                } else {
                    arr[i] = 3;
                }
            } else {
                arr[i] = 4;
            }
        } else {
            arr[i] = 5;
        }
        
        /* Nested switch */
        switch (depth % 4) {
            case 0:
                for (j = 0; j < 4; j++) {
                    arr[i] += j;
                }
                break;
            case 1:
                j = 0;
                while (j < 3) {
                    arr[i] *= (j + 1);
                    j++;
                }
                break;
            case 2:
                do {
                    arr[i] ^= 0xFF;
                } while (0); /* Single iteration do-while */
                break;
            case 3:
                /* Mixed operations */
                arr[i] = (arr[i] << 3) | (arr[i] >> 5);
                break;
        }
    }
}

/* Main orchestrator */
int main() {
    int i, iter;
    clock_t start, end;
    double cpu_time_used;
    
    /* Allocate and initialize data */
    int *data1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *data2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *data3 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    if (!data1 || !data2 || !data3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    srand(42);
    for (i = 0; i < ARRAY_SIZE; i++) {
        data1[i] = rand() % 256;
        data2[i] = rand() % 256;
        data3[i] = rand() % 256;
    }
    
    printf("Starting scheduler stress test...\n");
    start = clock();
    
    /* Warm-up phase to trigger optimization heuristics */
    printf("Warm-up phase...\n");
    for (iter = 0; iter < ITERATIONS / 10; iter++) {
        complex_control_flow(data1, ARRAY_SIZE / 2, 1);
        vectorizable_loop(data2, data1, data3, ARRAY_SIZE / 4);
    }
    
    /* Main test phase with varying parameters */
    printf("Main test phase...\n");
    for (iter = 0; iter < ITERATIONS; iter++) {
        int depth = iter % MAX_DEPTH;
        int size = ARRAY_SIZE / (1 + (iter % 4));
        
        /* Alternate between different patterns */
        switch (iter % 5) {
            case 0:
                complex_control_flow(data1, size, depth);
                break;
            case 1:
                vectorizable_loop(data2, data1, data3, size);
                break;
            case 2:
                parallel_region(data3, size);
                break;
            case 3:
                nested_control_structures(data1, size, depth);
                break;
            case 4:
                /* Mix all patterns */
                complex_control_flow(data1, size / 2, depth);
                vectorizable_loop(data2, data1, data3, size / 2);
                parallel_region(data3, size / 2);
                break;
        }
        
        /* Cross-data dependencies */
        if (iter % 3 == 0) {
            for (i = 0; i < size; i++) {
                data1[i] = compute_hash(data1[i], data2[i]);
                data2[i] = compute_hash(data2[i], data3[i]);
                data3[i] = compute_hash(data3[i], data1[i]);
            }
        }
    }
    
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    /* Compute checksum for verification */
    unsigned long long checksum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum += data1[i] + data2[i] * 2 + data3[i] * 3;
    }
    
    printf("Test completed in %.2f seconds\n", cpu_time_used);
    printf("Final checksum: %llu\n", checksum);
    printf("Data1[0]=%d, Data2[0]=%d, Data3[0]=%d\n", 
           data1[0], data2[0], data3[0]);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    
    return 0;
}
