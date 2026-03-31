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
    return ((x << 5) ^ y) + (x >> 3) * 17;
}

static inline int __attribute__((always_inline))
conditional_transform(int val, int mask) {
    /* Complex conditional chain */
    if (val & 1) {
        return val ^ mask;
    } else if (val & 2) {
        return val + (mask << 1);
    } else if (val & 4) {
        return val - (mask >> 1);
    } else {
        return (val * 3) | mask;
    }
}

/* Hot function with complex control flow */
__attribute__((hot))
void complex_control_flow(int *data, int size, int depth) {
    int i, j, k;
    volatile int barrier = 0; /* Force memory dependencies */
    
    /* Nested loops with varying iteration counts */
    for (i = 0; i < size; i += 2) {
        /* Deep if-else chain */
        if (i % 3 == 0) {
            for (j = i; j < size && j < i + 10; j++) {
                /* Memory operations creating dependencies */
                data[j] = compute_hash(data[j], j);
                
                /* Switch statement with multiple cases */
                switch (j % 5) {
                    case 0:
                        data[j] += barrier;
                        break;
                    case 1:
                        data[j] ^= 0x5A5A5A5A;
                        break;
                    case 2:
                        data[j] = data[j] * 3 + 1;
                        break;
                    case 3:
                        data[j] = conditional_transform(data[j], i);
                        break;
                    case 4:
                        /* Artificial scheduling barrier */
                        asm volatile("" ::: "memory");
                        data[j] = data[j] >> (j % 8);
                        break;
                }
            }
        } else if (i % 3 == 1) {
            /* While loop with computed goto (irreducible CFG) */
            static void *labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4 };
            int counter = 0;
            
            k = i;
            while (k < size && counter < 20) {
                void *target = labels[counter % 5];
                goto *target;
                
            L0:
                data[k] = data[k] + data[k + 1];
                k += 2;
                counter++;
                continue;
            L1:
                data[k] = data[k] ^ data[k - 1];
                k++;
                counter++;
                continue;
            L2:
                data[k] = data[k] * 2 - 1;
                k++;
                counter++;
                continue;
            L3:
                /* Memory clobber to force scheduling constraints */
                asm volatile("" ::: "memory");
                barrier = data[k];
                k++;
                counter++;
                continue;
            L4:
                data[k] = conditional_transform(data[k], barrier);
                k++;
                counter++;
                continue;
            }
        } else {
            /* Do-while with pointer arithmetic */
            int *ptr = &data[i];
            int limit = (i + 16 < size) ? i + 16 : size;
            
            do {
                *ptr = compute_hash(*ptr, ptr - data);
                ptr++;
                
                /* Mix of operations to create ILP opportunities */
                int temp = *ptr;
                temp = (temp << 3) | (temp >> 29);
                temp += barrier;
                temp ^= 0xAAAAAAAA;
                *ptr = temp;
                ptr++;
            } while (ptr < &data[limit]);
        }
        
        /* Recursive call for deeper nesting */
        if (depth < MAX_DEPTH && i % 32 == 0) {
            complex_control_flow(&data[i], 
                (size - i > 64) ? 64 : size - i, 
                depth + 1);
        }
    }
}

/* Function with vectorization candidate */
__attribute__((hot))
void vectorizable_loop(int *a, int *b, int *c, int n) {
    int i;
    
    #pragma omp simd
    for (i = 0; i < n; i++) {
        /* Simple stride-1 operations for auto-vectorization */
        a[i] = b[i] * 3 + c[i];
        c[i] = a[i] - b[i] / 2;
        b[i] = (a[i] << 2) | (c[i] >> 2);
    }
}

/* Function with OpenMP parallelization */
void parallel_region(int *data, int size) {
    int i, j;
    
    #pragma omp parallel for private(j) schedule(dynamic, 16)
    for (i = 0; i < size; i += 64) {
        int chunk_size = (i + 64 < size) ? 64 : size - i;
        
        /* Inner loop with carried dependency */
        for (j = i + 1; j < i + chunk_size; j++) {
            data[j] = data[j] + data[j - 1] * 2;
        }
        
        /* Function call from multiple sites */
        for (j = 0; j < chunk_size; j += 8) {
            data[i + j] = conditional_transform(data[i + j], j);
        }
    }
}

/* Complex switch-based state machine */
__attribute__((hot))
void state_machine(int *data, int size) {
    int state = 0;
    int i = 0;
    
    while (i < size) {
        switch (state) {
            case 0:
                data[i] = (data[i] << 1) | 1;
                if (data[i] > 1000) state = 1;
                i += 2;
                break;
            case 1:
                data[i] = data[i] ^ 0xFFFF;
                if (data[i] < 100) state = 2;
                i += 3;
                break;
            case 2:
                data[i] = data[i] * 7 + 3;
                if (data[i] % 5 == 0) state = 3;
                i += 1;
                break;
            case 3:
                data[i] = compute_hash(data[i], i);
                if (i % 7 == 0) state = 0;
                i += 4;
                break;
            default:
                state = 0;
                break;
        }
        
        /* Memory barrier every 16 iterations */
        if (i % 16 == 0) {
            asm volatile("" ::: "memory");
        }
    }
}

/* Main orchestrator function */
int main() {
    int *data1, *data2, *data3;
    int i, iter;
    clock_t start, end;
    long long checksum = 0;
    
    /* Allocate and initialize data */
    data1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    data2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    data3 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    if (!data1 || !data2 || !data3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(42);
    for (i = 0; i < ARRAY_SIZE; i++) {
        data1[i] = rand() % 1000;
        data2[i] = rand() % 1000;
        data3[i] = rand() % 1000;
    }
    
    printf("Starting scheduler stress test...\n");
    start = clock();
    
    /* Warm-up phase to trigger optimization heuristics */
    for (iter = 0; iter < ITERATIONS / 10; iter++) {
        complex_control_flow(data1, ARRAY_SIZE / 2, 0);
        vectorizable_loop(data2, data1, data3, ARRAY_SIZE / 4);
    }
    
    /* Main test phase with varying patterns */
    for (iter = 0; iter < ITERATIONS; iter++) {
        /* Alternate between different complex functions */
        switch (iter % 4) {
            case 0:
                complex_control_flow(data1, ARRAY_SIZE, 0);
                break;
            case 1:
                parallel_region(data2, ARRAY_SIZE);
                break;
            case 2:
                state_machine(data3, ARRAY_SIZE);
                break;
            case 3:
                vectorizable_loop(data1, data2, data3, ARRAY_SIZE);
                break;
        }
        
        /* Mix in some OpenMP parallel sections */
        if (iter % 7 == 0) {
            #pragma omp parallel sections
            {
                #pragma omp section
                {
                    complex_control_flow(data1, ARRAY_SIZE / 2, 2);
                }
                #pragma omp section
                {
                    state_machine(data2, ARRAY_SIZE / 2);
                }
            }
        }
        
        /* Periodically reset data to create different scheduling patterns */
        if (iter % 13 == 0) {
            for (i = 0; i < ARRAY_SIZE; i++) {
                data1[i] = (data1[i] + iter) % 1000;
                data2[i] = (data2[i] ^ iter) % 1000;
                data3[i] = (data3[i] * (iter % 7 + 1)) % 1000;
            }
        }
    }
    
    end = clock();
    
    /* Compute final checksum for verification */
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum += data1[i] + data2[i] * 2 + data3[i] * 3;
        checksum = (checksum << 3) | (checksum >> 61); /* Rotate */
    }
    
    printf("Test completed in %.2f seconds\n", 
           (double)(end - start) / CLOCKS_PER_SEC);
    printf("Final checksum: %lld\n", checksum);
    printf("Expected range: 1500000000 - 2500000000\n");
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    
    return 0;
}
