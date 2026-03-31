/* Target: haifa-sched.cc free_sched_context logic */
/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -finline-functions -funswitch-loops -fprofile-generate -frandom-seed=42 -o scheduler_test scheduler_test.c */
/* For profile feedback: Run once, then recompile with -fprofile-use */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000
#define MAX_DEPTH 8

/* Always inline helper functions */
__attribute__((always_inline)) static inline int compute_hash(int x, int y) {
    /* Mix of arithmetic and bitwise ops */
    return ((x << 3) ^ (y >> 1)) + (x * y) - (x & y) | (x ^ y);
}

__attribute__((always_inline)) static inline int barrier_op(int val) {
    int result;
    /* Memory barrier asm to create scheduling constraints */
    asm volatile ("mfence" ::: "memory");
    result = val * 37 + 12345;
    asm volatile ("sfence" ::: "memory");
    return result;
}

__attribute__((always_inline)) static inline void process_chunk(int *arr, int start, int end, int modifier) {
    /* Complex loop with carried dependency */
    int temp = modifier;
    for (int i = start; i < end; i++) {
        arr[i] = compute_hash(arr[i], temp);
        temp = arr[i] % 256;  /* Carried dependency */
        if (i % 3 == 0) {
            arr[i] = barrier_op(arr[i]);
        }
    }
}

/* Hot function with complex control flow */
__attribute__((hot)) void complex_control_flow(int *data, int size, int depth) {
    if (depth >= MAX_DEPTH) return;
    
    /* Nested loops with varying bounds */
    for (int i = 0; i < size; i += 64) {
        int chunk_size = (i + 64 > size) ? size - i : 64;
        
        /* Switch with multiple cases */
        switch (depth % 4) {
            case 0:
                for (int j = 0; j < chunk_size; j++) {
                    data[i + j] = compute_hash(data[i + j], j);
                    if (j % 7 == 0) {
                        data[i + j] = barrier_op(data[i + j]);
                    }
                }
                break;
            case 1:
                /* While loop with computed goto */
                {
                    int k = 0;
                    void *labels[] = { &&label0, &&label1, &&label2, &&label3 };
                    
                label0:
                    data[i + k] += (k * 3) & 0xFF;
                    goto *labels[(k++) % 4];
                    
                label1:
                    data[i + k] ^= (k << 2);
                    if (k < chunk_size) goto *labels[k % 4];
                    else goto done;
                    
                label2:
                    data[i + k] *= 17;
                    if (k < chunk_size) goto *labels[(k * 7) % 4];
                    else goto done;
                    
                label3:
                    data[i + k] = barrier_op(data[i + k]);
                    if (k < chunk_size) goto *labels[(k * 11) % 4];
                    else goto done;
                    
                done:;
                }
                break;
            case 2:
                /* Do-while with early exit */
                {
                    int j = 0;
                    do {
                        if (data[i + j] > 1000) {
                            data[i + j] /= 2;
                        } else {
                            data[i + j] = compute_hash(data[i + j], depth);
                        }
                        j++;
                        if (j % 5 == 0) {
                            data[i + j - 1] = barrier_op(data[i + j - 1]);
                        }
                    } while (j < chunk_size && data[i + j - 1] != 0);
                }
                break;
            case 3:
                /* Deep if-else chain */
                for (int j = 0; j < chunk_size; j++) {
                    int val = data[i + j];
                    if (val < 10) {
                        val = val * 2 + 1;
                    } else if (val < 50) {
                        val = (val << 1) ^ 0x5A;
                    } else if (val < 100) {
                        val = barrier_op(val);
                    } else if (val < 200) {
                        val = compute_hash(val, j);
                    } else if (val < 300) {
                        val = (val * 3) / 2;
                    } else {
                        val = val % 127;
                    }
                    data[i + j] = val;
                }
                break;
        }
        
        /* Recursive call with different parameters */
        if (chunk_size > 16) {
            complex_control_flow(data + i, chunk_size / 2, depth + 1);
        }
    }
}

/* Function with vectorization candidate */
__attribute__((hot)) void vectorizable_loop(int *a, int *b, int *c, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        /* Independent operations for vectorization */
        int t1 = a[i] * 3;
        int t2 = b[i] + 7;
        int t3 = t1 ^ t2;
        int t4 = barrier_op(t3);
        c[i] = t4 - (a[i] & b[i]);
    }
}

/* Function with OpenMP parallel region */
void parallel_region(int *data, int size) {
    #pragma omp parallel for schedule(dynamic, 16)
    for (int i = 0; i < size; i++) {
        int val = data[i];
        /* Complex operation mix */
        val = compute_hash(val, i);
        val = barrier_op(val);
        
        /* Nested loop in parallel region */
        for (int j = 0; j < 4; j++) {
            val = (val << j) | (val >> (32 - j));
            if (j % 2 == 0) {
                val ^= 0xDEADBEEF;
            }
        }
        
        data[i] = val;
    }
}

/* Irreducible control flow using computed goto */
void irreducible_cfg(int *arr, int n) {
    void *jump_table[] = { &&block_a, &&block_b, &&block_c, &&block_d, &&block_e };
    int counter = 0;
    int i = 0;
    
block_a:
    arr[i] += counter * 3;
    counter = (counter + 1) % 5;
    goto *jump_table[counter];
    
block_b:
    arr[i] ^= 0xAA;
    if (++i >= n) goto done;
    counter = (counter * 7) % 5;
    goto *jump_table[counter];
    
block_c:
    arr[i] = barrier_op(arr[i]);
    counter = (counter + 3) % 5;
    goto *jump_table[counter];
    
block_d:
    arr[i] = compute_hash(arr[i], i);
    counter = (counter * 2 + 1) % 5;
    goto *jump_table[counter];
    
block_e:
    arr[i] = arr[i] * 2 - 1;
    counter = (counter + 4) % 5;
    goto *jump_table[counter];
    
done:
    return;
}

/* Main orchestrator */
int main() {
    /* Allocate with different alignments to stress scheduler */
    int *data1 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int *data2 = (int*)aligned_alloc(32, ARRAY_SIZE * sizeof(int));
    int *data3 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *temp = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    /* Initialize with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data1[i] = i * 3 + 7;
        data2[i] = i ^ 0x55;
        data3[i] = i % 97;
    }
    
    /* Warm-up phase */
    printf("Warm-up phase...\n");
    for (int iter = 0; iter < ITERATIONS / 10; iter++) {
        complex_control_flow(data1, ARRAY_SIZE / 2, 0);
        process_chunk(data2, 0, ARRAY_SIZE, iter);
    }
    
    /* Main computation with timing */
    printf("Main computation phase...\n");
    clock_t start = clock();
    
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Alternate between different patterns */
        switch (iter % 5) {
            case 0:
                complex_control_flow(data1, ARRAY_SIZE, 0);
                break;
            case 1:
                vectorizable_loop(data1, data2, temp, ARRAY_SIZE);
                /* Copy back with dependency */
                for (int i = 0; i < ARRAY_SIZE; i++) {
                    data1[i] = temp[i] + data3[i];
                }
                break;
            case 2:
                parallel_region(data2, ARRAY_SIZE);
                break;
            case 3:
                irreducible_cfg(data3, ARRAY_SIZE);
                break;
            case 4:
                /* Mixed operations */
                for (int i = 0; i < ARRAY_SIZE; i += 8) {
                    process_chunk(data1, i, i + 8, iter);
                    process_chunk(data2, i, i + 8, iter + 1);
                    process_chunk(data3, i, i + 8, iter + 2);
                }
                break;
        }
        
        /* Cross-data dependency */
        if (iter % 10 == 0) {
            for (int i = 0; i < ARRAY_SIZE; i++) {
                data1[i] = compute_hash(data1[i], data2[i]);
                data2[i] = compute_hash(data2[i], data3[i]);
                data3[i] = compute_hash(data3[i], data1[i]);
            }
        }
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    /* Compute checksum for verification */
    unsigned long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += (unsigned long long)data1[i];
        checksum ^= (unsigned long long)data2[i] << 32;
        checksum += (unsigned long long)data3[i] * 3;
    }
    
    printf("Execution time: %.3f seconds\n", elapsed);
    printf("Checksum: 0x%016llx\n", checksum);
    printf("Result sample: data1[0]=%d, data2[100]=%d, data3[500]=%d\n", 
           data1[0], data2[100], data3[500]);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    free(temp);
    
    return 0;
}
