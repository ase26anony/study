/* Test program to trigger free_sched_context in GCC's Haifa scheduler */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <omp.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000
#define MAX_DEPTH 8

/* Always inline helper functions to create complex dataflow */
static inline int __attribute__((always_inline)) 
compute_hash(int x, int y) {
    /* Mix of arithmetic and bitwise operations */
    return ((x << 3) ^ (y >> 2)) + (x * y) - (x & y) | (x ^ y);
}

static inline int __attribute__((always_inline))
conditional_transform(int val, int selector) {
    /* Complex conditional chain */
    if (selector & 0x01) val = (val * 3) + 1;
    if (selector & 0x02) val ^= (val << 4);
    if (selector & 0x04) val = (val >> 1) | (val << 31);
    if (selector & 0x08) val = ~val + selector;
    return val;
}

/* Hot function with complex control flow */
__attribute__((hot))
void complex_control_flow(int *arr, int size, int depth) {
    int i, j, k;
    volatile int barrier = 0; /* Force memory dependencies */
    
    /* Nested loops with varying iteration counts */
    for (i = 0; i < size; i += 2) {
        /* Switch statement inside loop */
        switch (i % 5) {
            case 0:
                arr[i] = compute_hash(arr[i], i);
                /* Fall through */
            case 1:
                arr[i] = conditional_transform(arr[i], i);
                break;
            case 2:
                /* Nested loop */
                for (j = 0; j < (i % 10); j++) {
                    arr[i] += compute_hash(j, arr[i]);
                }
                break;
            case 3:
                /* Memory operation with pointer arithmetic */
                int *ptr = &arr[i];
                for (k = 0; k < 3; k++) {
                    *ptr = (*ptr << k) ^ compute_hash(k, *ptr);
                    ptr++;
                    if (ptr >= &arr[size]) break;
                }
                break;
            case 4:
                /* Do-while loop */
                j = 0;
                do {
                    arr[i] ^= (arr[i] << j);
                    j++;
                } while (j < 4);
                break;
        }
        
        /* Artificial scheduling barrier */
        asm volatile("" ::: "memory");
        barrier = arr[i];
        
        /* Computed goto for irreducible control flow */
        if (depth > 0 && (i % 7 == 0)) {
            static void *labels[] = { &&L0, &&L1, &&L2, &&L3 };
            goto *labels[i % 4];
            
            L0:
                arr[i] = arr[i] * 2 + 1;
                goto end_label;
            L1:
                arr[i] = arr[i] / 3 | 0x5555;
                goto end_label;
            L2:
                arr[i] = ~arr[i];
                goto end_label;
            L3:
                arr[i] = arr[i] ^ 0xAAAA;
                /* fall through */
            end_label:;
        }
    }
    
    /* Recursive call for deeper nesting */
    if (depth > 1) {
        complex_control_flow(arr + size/2, size/2, depth - 1);
    }
}

/* Function with vectorization candidate */
__attribute__((hot))
void vectorizable_loop(int *a, int *b, int *c, int n) {
    int i;
    /* Simple stride-1 operations - good for auto-vectorization */
    #pragma omp simd
    for (i = 0; i < n; i++) {
        a[i] = b[i] * c[i] + compute_hash(i, b[i]);
    }
    
    /* Another loop with carried dependency */
    for (i = 1; i < n; i++) {
        a[i] += a[i-1] * 2 - conditional_transform(i, a[i-1]);
    }
}

/* Function with OpenMP parallelization */
void parallel_region(int *data, int size) {
    int i;
    #pragma omp parallel for schedule(dynamic, 16)
    for (i = 0; i < size; i++) {
        /* Independent but complex computation */
        int val = data[i];
        for (int j = 0; j < 8; j++) {
            val = conditional_transform(val, j);
            val = compute_hash(val, data[(i + j) % size]);
        }
        data[i] = val;
        
        /* Memory clobber to force scheduling constraints */
        asm volatile("" : "+m" (data[i]) : : "memory");
    }
}

/* Irreducible control flow using computed gotos */
void irreducible_cfg(int *arr, int n) {
    int i = 0;
    void *next_label = &&start;
    
    start:
    if (i >= n) goto end;
    
    /* Jump table simulation */
    switch (arr[i] % 6) {
        case 0: next_label = &&label_a; break;
        case 1: next_label = &&label_b; break;
        case 2: next_label = &&label_c; break;
        case 3: next_label = &&label_d; break;
        case 4: next_label = &&label_e; break;
        default: next_label = &&label_f; break;
    }
    
    goto *next_label;
    
    label_a:
        arr[i] = arr[i] * 3 + 1;
        i++;
        goto start;
    label_b:
        arr[i] = arr[i] ^ 0x1234;
        i += 2;
        goto start;
    label_c:
        arr[i] = ~arr[i];
        i += 3;
        goto start;
    label_d:
        arr[i] = arr[i] | 0xABCD;
        i += 4;
        goto start;
    label_e:
        arr[i] = arr[i] & 0xF0F0;
        i += 5;
        goto start;
    label_f:
        arr[i] = arr[i] << 2;
        i += 6;
        goto start;
    end:;
}

/* Main orchestrator */
int main() {
    int i, iter;
    clock_t start, end;
    double total_time = 0;
    
    /* Allocate and initialize arrays */
    int *array1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *array2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *array3 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    if (!array1 || !array2 || !array3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(42);
    for (i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
        array3[i] = rand() % 1000;
    }
    
    printf("Starting scheduler stress test...\n");
    
    /* Warm-up phase to trigger optimization heuristics */
    printf("Warm-up phase...\n");
    for (iter = 0; iter < ITERATIONS / 10; iter++) {
        complex_control_flow(array1, ARRAY_SIZE / 2, 3);
        vectorizable_loop(array2, array1, array3, ARRAY_SIZE / 4);
    }
    
    /* Main test phase with timing */
    printf("Main test phase...\n");
    start = clock();
    
    for (iter = 0; iter < ITERATIONS; iter++) {
        /* Alternate between different patterns */
        switch (iter % 4) {
            case 0:
                complex_control_flow(array1, ARRAY_SIZE, MAX_DEPTH);
                break;
            case 1:
                vectorizable_loop(array2, array1, array3, ARRAY_SIZE);
                break;
            case 2:
                parallel_region(array3, ARRAY_SIZE);
                break;
            case 3:
                irreducible_cfg(array1, ARRAY_SIZE / 2);
                break;
        }
        
        /* Mix in some inlined computations */
        for (i = 0; i < ARRAY_SIZE; i += 16) {
            array1[i] = compute_hash(array1[i], array2[i]);
            array2[i] = conditional_transform(array2[i], array3[i]);
        }
    }
    
    end = clock();
    total_time = (double)(end - start) / CLOCKS_PER_SEC;
    
    /* Compute checksum for verification */
    unsigned long long checksum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum += (unsigned long long)array1[i];
        checksum ^= (unsigned long long)array2[i] << 16;
        checksum += (unsigned long long)array3[i] * 3;
    }
    
    printf("Test completed in %.2f seconds\n", total_time);
    printf("Final checksum: 0x%016llx\n", checksum);
    printf("Array1[0]=%d, Array2[0]=%d, Array3[0]=%d\n", 
           array1[0], array2[0], array3[0]);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    
    return 0;
}
