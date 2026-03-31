/* Test program to trigger free_sched_context in GCC's Haifa scheduler */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

#define ARRAY_SIZE 1024
#define MAX_ITER 100
#define NEST_DEPTH 8

/* Always inline helper functions to create complex dataflow */
static inline int __attribute__((always_inline)) 
compute_hash(int x, int y) {
    /* Mix of arithmetic and bitwise ops */
    return ((x << 3) ^ y) + (x >> 2) * 17 - (x & 0xFF) | (y & 0x7F);
}

static inline int __attribute__((always_inline))
scramble_bits(int val) {
    /* Complex bit manipulation */
    val = (val ^ (val >> 16)) * 0x45d9f3b;
    val = (val ^ (val >> 16)) * 0x45d9f3b;
    val = val ^ (val >> 16);
    return val;
}

/* Hot function with complex control flow */
__attribute__((hot, noinline))
void complex_control_flow(int *data, int size, int seed) {
    int i, j, k;
    volatile int barrier = 0; /* Prevent optimization */
    
    /* Nested loops with varying iteration counts */
    for (i = 0; i < size; i += 2) {
        /* Deep if-else chain */
        if (i % 3 == 0) {
            data[i] = compute_hash(i, seed);
            if (i % 5 == 0) {
                data[i] ^= scramble_bits(data[i]);
                for (j = 0; j < (i % 10); j++) {
                    data[i] += (j * 7) & 0xFF;
                }
            } else if (i % 7 == 0) {
                data[i] = data[i] * 13 - (i << 2);
                /* Artificial scheduling barrier */
                asm volatile("" ::: "memory");
            } else {
                data[i] = (data[i] << 1) | (data[i] >> 31);
            }
        } else if (i % 4 == 0) {
            /* Switch statement with multiple cases */
            switch (i % 6) {
                case 0:
                    data[i] = seed * i;
                    break;
                case 1:
                    data[i] = seed + i * 3;
                    break;
                case 2:
                    data[i] = seed ^ i ^ 0xDEADBEEF;
                    break;
                case 3:
                    data[i] = (seed * i) % 7919;
                    break;
                case 4:
                    data[i] = scramble_bits(seed + i);
                    break;
                case 5:
                    data[i] = compute_hash(seed, i);
                    break;
            }
        } else {
            data[i] = i * seed;
        }
        
        /* Memory operations with pointer arithmetic */
        int *ptr = &data[i];
        for (k = 0; k < NEST_DEPTH; k++) {
            if (k % 2 == 0) {
                *ptr += *(ptr + (k % (size - i)));
            } else {
                *ptr -= *(ptr - (k % i));
            }
        }
        
        /* Computed goto to create irreducible CFG */
        static void *labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4 };
        goto *labels[i % 5];
        
        L0:
            data[i] += 1;
            continue;
        L1:
            data[i] *= 2;
            continue;
        L2:
            data[i] ^= 0x55;
            continue;
        L3:
            data[i] = ~data[i];
            continue;
        L4:
            data[i] = data[i] << 3;
            continue;
    }
    
    /* Prevent dead code elimination */
    barrier = data[0];
}

/* Function with tight inner loop and carried dependencies */
__attribute__((hot))
void tight_inner_loop(int *a, int *b, int *c, int n) {
    int i, j;
    
    /* Loop with carried dependency */
    for (i = 1; i < n; i++) {
        a[i] = a[i-1] * 3 + b[i];
    }
    
    /* Independent operations between branches */
    for (i = 0; i < n; i++) {
        if (i % 2 == 0) {
            c[i] = a[i] * 7 - b[i];
            /* Inline asm to create scheduling barrier */
            asm volatile("" ::: "memory");
        } else {
            c[i] = a[i] / 3 + b[i];
        }
        
        /* Mix of operations */
        c[i] = (c[i] << 2) | (c[i] >> 30);
        c[i] ^= 0xAAAAAAAA;
        c[i] += i * 11;
    }
}

/* Vectorization candidate */
__attribute__((hot))
void vectorizable_loop(float *x, float *y, float *z, int n) {
    int i;
    
    /* Simple stride-1 operations - good for vectorization */
    #pragma omp simd
    for (i = 0; i < n; i++) {
        x[i] = y[i] * 2.5f + z[i];
        z[i] = x[i] * 0.7f - y[i];
    }
    
    /* Another loop with reduction */
    float sum = 0.0f;
    #pragma omp simd reduction(+:sum)
    for (i = 0; i < n; i++) {
        sum += x[i] * y[i];
        z[i] += sum * 0.01f;
    }
}

/* Function with irreducible control flow using computed gotos */
__attribute__((noinline))
void irreducible_cfg(int *arr, int n) {
    int i = 0;
    int state = 0;
    
    /* Labels for computed goto */
    void *states[] = { &&STATE_A, &&STATE_B, &&STATE_C, &&STATE_D, &&STATE_E };
    
    while (i < n) {
        goto *states[state];
        
        STATE_A:
            arr[i] = compute_hash(i, arr[i]);
            state = (state + 1) % 5;
            i++;
            continue;
            
        STATE_B:
            arr[i] = scramble_bits(arr[i]);
            state = (state + 2) % 5;
            i++;
            continue;
            
        STATE_C:
            arr[i] = arr[i] * 13 + 7;
            state = (state + 3) % 5;
            i++;
            continue;
            
        STATE_D:
            arr[i] = arr[i] ^ 0x12345678;
            state = (state + 4) % 5;
            i++;
            continue;
            
        STATE_E:
            arr[i] = ~arr[i];
            state = (state + 1) % 5;
            i++;
            continue;
    }
}

/* Main orchestrator */
int main() {
    int i, iter;
    clock_t start, end;
    double cpu_time_used;
    
    /* Allocate arrays with different alignments */
    int *data1 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int *data2 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int *data3 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    float *fdata1 = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float *fdata2 = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float *fdata3 = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    
    if (!data1 || !data2 || !data3 || !fdata1 || !fdata2 || !fdata3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    srand(42);
    for (i = 0; i < ARRAY_SIZE; i++) {
        data1[i] = rand() % 1000;
        data2[i] = rand() % 1000;
        data3[i] = rand() % 1000;
        fdata1[i] = (float)rand() / RAND_MAX;
        fdata2[i] = (float)rand() / RAND_MAX;
        fdata3[i] = (float)rand() / RAND_MAX;
    }
    
    printf("Starting scheduler stress test...\n");
    start = clock();
    
    /* Warm-up phase - trigger optimization heuristics */
    for (iter = 0; iter < MAX_ITER / 10; iter++) {
        complex_control_flow(data1, ARRAY_SIZE / 2, iter);
        tight_inner_loop(data1, data2, data3, ARRAY_SIZE / 4);
    }
    
    /* Main test phase with timing */
    #pragma omp parallel for schedule(dynamic)
    for (iter = 0; iter < MAX_ITER; iter++) {
        int chunk = ARRAY_SIZE / 4;
        
        /* Call from multiple sites with different parameters */
        complex_control_flow(data1 + (iter % 4) * chunk, chunk, iter);
        complex_control_flow(data2 + ((iter + 1) % 4) * chunk, chunk, iter + 1);
        
        tight_inner_loop(data1, data2, data3, chunk);
        tight_inner_loop(data2, data3, data1, chunk);
        
        vectorizable_loop(fdata1, fdata2, fdata3, ARRAY_SIZE);
        
        if (iter % 3 == 0) {
            irreducible_cfg(data1, chunk);
        }
        
        /* Mix of memory operations */
        for (i = 0; i < chunk; i++) {
            int idx = (i * 7) % ARRAY_SIZE;
            data1[idx] = compute_hash(data1[idx], data2[i]);
            data2[i] = scramble_bits(data3[idx]);
            
            /* Memory clobber to force scheduling constraints */
            asm volatile("" ::: "memory");
        }
    }
    
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    /* Compute checksum for verification */
    unsigned long long checksum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum += (unsigned long long)data1[i];
        checksum += (unsigned long long)data2[i];
        checksum += (unsigned long long)data3[i];
        checksum += (unsigned long long)(fdata1[i] * 1000);
    }
    
    printf("Test completed in %.2f seconds\n", cpu_time_used);
    printf("Checksum: %llu\n", checksum);
    printf("Expected checksum range: 1500000000 - 2500000000\n");
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    free(fdata1);
    free(fdata2);
    free(fdata3);
    
    return 0;
}
