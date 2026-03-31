/* Test program to stress GCC's Haifa scheduler and trigger free_sched_context */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <omp.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100000
#define MAX_DEPTH 8

/* Always inline helper functions to create complex dataflow */
static inline int __attribute__((always_inline)) 
compute_hash(int a, int b, int c) {
    /* Mix of arithmetic and bitwise operations */
    int t = a ^ (b << 3);
    t = t + (c * 0x5bd1e995);
    t = t ^ (t >> 16);
    t = t * 0x27d4eb2d;
    return t ^ (t >> 15);
}

static inline int __attribute__((always_inline))
conditional_transform(int x, int selector) {
    /* Complex conditional chain */
    if (selector & 1) {
        x = x * 3 + 1;
    } else if (selector & 2) {
        x = x ^ 0xAAAAAAAA;
    } else if (selector & 4) {
        x = (x << 1) | (x >> 31);
    } else if (selector & 8) {
        x = x - (x / 3);
    }
    
    switch (x & 7) {
        case 0: return x + 1;
        case 1: return x * 2;
        case 2: return x ^ 0x55555555;
        case 3: return x - 17;
        case 4: return x | 0x12345678;
        case 5: return x & 0xF0F0F0F0;
        case 6: return x << 4;
        case 7: return x >> 4;
        default: return x;
    }
}

/* Hot function with complex control flow */
__attribute__((hot))
void complex_control_flow(int *data, int size, int depth) {
    int i, j, k;
    volatile int barrier = 0; /* Force memory dependencies */
    
    /* Nested loops with varying iteration counts */
    for (i = 0; i < size; i += 2) {
        int acc = data[i];
        
        /* Deep if-else chain */
        if (i % 3 == 0) {
            for (j = 0; j < depth; j++) {
                acc = compute_hash(acc, j, data[i + 1]);
                /* Memory operation creating dependencies */
                barrier = acc;
                data[i + 1] = barrier;
            }
        } else if (i % 3 == 1) {
            int temp = data[i];
            /* While loop with computed goto (irreducible CFG) */
            void *labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4 };
            int label_idx = 0;
            
            while (temp > 0) {
                goto *labels[label_idx % 5];
                
                L0:
                    temp = conditional_transform(temp, 1);
                    label_idx++;
                    continue;
                L1:
                    temp = compute_hash(temp, i, size);
                    label_idx++;
                    continue;
                L2:
                    /* Inline asm to create scheduling barrier */
                    asm volatile ("nop" : : : "memory");
                    temp = temp ^ 0xDEADBEEF;
                    label_idx++;
                    continue;
                L3:
                    temp = temp * 0x9e3779b9;
                    label_idx++;
                    continue;
                L4:
                    temp = temp >> 1;
                    label_idx++;
                    continue;
            }
            data[i] = temp;
        } else {
            /* Do-while with switch inside */
            k = 0;
            do {
                switch (k % 4) {
                    case 0:
                        data[i] += data[(i + k) % size];
                        break;
                    case 1:
                        data[i] ^= data[(i + k * 2) % size];
                        break;
                    case 2:
                        data[i] *= 1 + (k % 7);
                        break;
                    case 3:
                        /* Another asm barrier */
                        asm volatile ("# scheduler barrier" : : : "memory");
                        data[i] = data[i] - data[(i + k * 3) % size];
                        break;
                }
                k++;
            } while (k < 8);
        }
        
        /* Array access with pointer arithmetic */
        int *ptr = &data[i];
        for (k = 0; k < 4; k++) {
            *(ptr + k) = compute_hash(*(ptr + k), i, k);
        }
    }
}

/* Function with vectorization candidate */
__attribute__((hot))
void vectorizable_loop(int *a, int *b, int *c, int n) {
    int i;
    /* Loop with stride-1 access - good for vectorization */
    #pragma omp simd
    for (i = 0; i < n; i++) {
        a[i] = b[i] * c[i] + i;
        /* Create some dependencies to force scheduling decisions */
        if (i > 0) {
            a[i] += a[i-1] & 0xFF;
        }
    }
    
    /* Another loop with carried dependency */
    for (i = 1; i < n; i++) {
        b[i] = b[i-1] * 3 + a[i];
        /* Mix in function calls */
        c[i] = conditional_transform(c[i], i % 16);
    }
}

/* Function with OpenMP parallel region */
void parallel_region(int *data, int size) {
    int i;
    #pragma omp parallel for schedule(dynamic, 16)
    for (i = 0; i < size; i++) {
        int val = data[i];
        /* Complex operation inside parallel region */
        for (int j = 0; j < 32; j++) {
            val = compute_hash(val, j, i);
            val = conditional_transform(val, j);
            /* Memory clobber to force scheduling constraints */
            asm volatile ("# parallel op" : "+r" (val) : : "memory");
        }
        data[i] = val;
    }
}

/* Irreducible control flow using computed gotos */
__attribute__((noinline))
void irreducible_cfg(int *data, int n) {
    static void *jump_table[] = { &&block_a, &&block_b, &&block_c, 
                                  &&block_d, &&block_e };
    
    int i = 0;
    int counter = 0;
    
    block_a:
        data[i] += 1;
        i = (i + 1) % n;
        goto *jump_table[(counter++) % 5];
    
    block_b:
        data[i] ^= 0xAA;
        i = (i * 2) % n;
        goto *jump_table[(counter++) % 5];
    
    block_c:
        data[i] *= 3;
        i = (i + 3) % n;
        goto *jump_table[(counter++) % 5];
    
    block_d:
        data[i] = compute_hash(data[i], i, n);
        i = (i * 7) % n;
        goto *jump_table[(counter++) % 5];
    
    block_e:
        data[i] = conditional_transform(data[i], counter);
        if (counter++ < 1000) {
            goto *jump_table[counter % 5];
        }
}

/* Main orchestrator function */
int main() {
    int i, j;
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
        data1[i] = rand();
        data2[i] = rand();
        data3[i] = rand();
    }
    
    printf("Starting scheduler stress test...\n");
    start = clock();
    
    /* Warm-up phase - trigger optimization heuristics */
    printf("Warm-up phase...\n");
    for (j = 0; j < ITERATIONS / 100; j++) {
        complex_control_flow(data1, ARRAY_SIZE / 2, 3);
    }
    
    /* Main test phase with varying parameters */
    printf("Main test phase...\n");
    for (i = 1; i <= 8; i++) {
        int size = ARRAY_SIZE / i;
        int depth = i;
        
        /* Mix different patterns */
        complex_control_flow(data1, size, depth);
        vectorizable_loop(data2, data3, data1, size);
        
        if (i % 2 == 0) {
            parallel_region(data3, size);
        }
        
        if (i % 3 == 0) {
            irreducible_cfg(data1, size);
        }
        
        /* Call helper functions from multiple sites */
        for (j = 0; j < 100; j++) {
            int idx = j % size;
            data2[idx] = compute_hash(data2[idx], data3[idx], j);
            data3[idx] = conditional_transform(data3[idx], data1[idx] & 0xF);
        }
    }
    
    /* Final verification phase */
    printf("Verification phase...\n");
    unsigned long long checksum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum += (unsigned long long)data1[i];
        checksum ^= (unsigned long long)data2[i] << (i % 32);
        checksum = compute_hash(checksum & 0xFFFFFFFF, 
                               (checksum >> 32) & 0xFFFFFFFF, i);
    }
    
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("Checksum: 0x%016llX\n", checksum);
    printf("Execution time: %.2f seconds\n", cpu_time_used);
    printf("Test completed successfully.\n");
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    
    return 0;
}
