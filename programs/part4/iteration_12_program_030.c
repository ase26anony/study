/* Target: haifa-sched.cc - free_sched_context coverage test */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000
#define MAX_DEPTH 8

/* Always inline helpers to create complex dataflow */
static inline int __attribute__((always_inline)) 
compute_hash(int a, int b, int c) {
    return ((a ^ b) << 3) | (c & 0x7F);
}

static inline int __attribute__((always_inline))
bit_reverse(int x) {
    x = ((x & 0x55555555) << 1) | ((x & 0xAAAAAAAA) >> 1);
    x = ((x & 0x33333333) << 2) | ((x & 0xCCCCCCCC) >> 2);
    x = ((x & 0x0F0F0F0F) << 4) | ((x & 0xF0F0F0F0) >> 4);
    x = ((x & 0x00FF00FF) << 8) | ((x & 0xFF00FF00) >> 8);
    return (x << 16) | (x >> 16);
}

/* Hot function with complex control flow */
__attribute__((hot, noinline))
void process_data_complex(int* restrict input, int* restrict output, int size, int mode) {
    int i, j, k;
    volatile int barrier = 0; /* Prevent dead code elimination */
    
    /* Nested loops with varying iteration patterns */
    for (i = 0; i < size; i++) {
        int acc = input[i];
        
        /* Deep if-else chain */
        if (mode & 0x01) {
            acc = compute_hash(acc, i, size);
            if (i % 3 == 0) {
                acc += bit_reverse(acc);
                if (i % 7 == 0) {
                    acc ^= 0xDEADBEEF;
                    if (i % 11 == 0) {
                        acc = (acc << 1) | (acc >> 31);
                    } else {
                        acc = (acc >> 1) | (acc << 31);
                    }
                }
            } else if (i % 5 == 0) {
                acc *= 0x9E3779B9;
            } else {
                acc -= 0x7F;
            }
        } else if (mode & 0x02) {
            /* Switch with multiple cases */
            switch (i % 8) {
                case 0: acc = acc * 3 + 1; break;
                case 1: acc = acc ^ (acc >> 16); break;
                case 2: acc = acc + (acc << 5); break;
                case 3: acc = acc - (acc << 3); break;
                case 4: acc = acc | 0x80000000; break;
                case 5: acc = acc & 0x7FFFFFFF; break;
                case 6: acc = ~acc; break;
                case 7: acc = acc % 9973; break;
            }
        }
        
        /* Memory operations with dependencies */
        for (j = 0; j < 4; j++) {
            int idx = (i + j) % size;
            acc += input[idx] * (j + 1);
            
            /* Artificial asm barrier */
            asm volatile("" : "+r" (acc) : : "memory");
        }
        
        output[i] = acc;
        barrier = output[i]; /* Use volatile store */
    }
    
    /* Irreducible control flow using computed goto */
    if (mode & 0x04) {
        static void* labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4 };
        int label_idx = 0;
        
        for (k = 0; k < 5; k++) {
            goto *labels[label_idx];
            
            L0:
                output[0] += 1;
                label_idx = (label_idx + 1) % 5;
                continue;
            L1:
                output[1] ^= output[0];
                label_idx = (label_idx + 2) % 5;
                continue;
            L2:
                output[2] = output[1] * output[0];
                label_idx = (label_idx + 3) % 5;
                continue;
            L3:
                output[3] = output[2] - output[1];
                label_idx = (label_idx + 4) % 5;
                continue;
            L4:
                output[4] = bit_reverse(output[3]);
                label_idx = (label_idx + 1) % 5;
                continue;
        }
    }
}

/* Function with vectorization candidate */
__attribute__((hot))
void vectorizable_loop(int* restrict a, int* restrict b, int* restrict c, int n) {
    int i;
    
    #pragma omp simd
    for (i = 0; i < n; i++) {
        /* Simple stride-1 operations for vectorization */
        int t = a[i] * 3;
        t = t + b[i] * 7;
        t = t ^ (t >> 4);
        c[i] = t;
    }
    
    /* Additional loop with carried dependency */
    for (i = 1; i < n; i++) {
        c[i] = c[i] + c[i-1] * 2;
    }
}

/* Complex nested loop structure */
__attribute__((hot))
void nested_loop_pattern(int* data, int size, int depth) {
    int i, j, k;
    
    for (i = 0; i < size; i += 2) {
        int sum = 0;
        
        /* Do-while with early exit */
        j = 0;
        do {
            if (j >= depth) break;
            
            for (k = 0; k < depth; k++) {
                /* Mix of operations */
                sum += data[(i + j + k) % size];
                sum = compute_hash(sum, j, k);
                
                /* Conditional store */
                if (sum & 0x100) {
                    data[(i + j) % size] = sum;
                }
            }
            
            j++;
        } while (j < MAX_DEPTH);
        
        /* Final computation with asm */
        int final = sum;
        asm volatile (
            "rorl $5, %0\n\t"
            "addl $0x9E3779B9, %0"
            : "+r" (final)
            :
            : "cc"
        );
        
        data[i] = final;
    }
}

/* Main orchestrator */
int main() {
    int i, iter;
    int* array1 = malloc(ARRAY_SIZE * sizeof(int));
    int* array2 = malloc(ARRAY_SIZE * sizeof(int));
    int* array3 = malloc(ARRAY_SIZE * sizeof(int));
    int* temp = malloc(ARRAY_SIZE * sizeof(int));
    
    if (!array1 || !array2 || !array3 || !temp) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(42);
    for (i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand();
        array2[i] = rand();
        array3[i] = 0;
        temp[i] = 0;
    }
    
    printf("Starting scheduler stress test...\n");
    
    /* Warm-up phase */
    printf("Warm-up phase...\n");
    for (iter = 0; iter < ITERATIONS / 10; iter++) {
        process_data_complex(array1, temp, ARRAY_SIZE, iter % 8);
        vectorizable_loop(array1, array2, array3, ARRAY_SIZE);
    }
    
    /* Main timed phase with varying patterns */
    printf("Main phase...\n");
    clock_t start = clock();
    
    for (iter = 0; iter < ITERATIONS; iter++) {
        int mode = iter % 16;
        
        /* Alternate between different functions and modes */
        if (mode < 4) {
            process_data_complex(array1, array3, ARRAY_SIZE, mode);
            nested_loop_pattern(array3, ARRAY_SIZE, (iter % 7) + 1);
        } else if (mode < 8) {
            vectorizable_loop(array1, array2, array3, ARRAY_SIZE);
            process_data_complex(array3, temp, ARRAY_SIZE, mode);
        } else if (mode < 12) {
            #pragma omp parallel for
            for (i = 0; i < ARRAY_SIZE; i++) {
                int val = array1[i];
                /* Independent operations for parallelization */
                val = ((val << 13) ^ val) * 0x9E3779B9;
                val = (val >> 16) ^ val;
                val = ((val << 5) ^ val) * 0x318D2A99;
                array2[i] = (val >> 16) ^ val;
            }
            vectorizable_loop(array2, array1, array3, ARRAY_SIZE);
        } else {
            nested_loop_pattern(array1, ARRAY_SIZE, (iter % MAX_DEPTH) + 1);
            process_data_complex(array1, array3, ARRAY_SIZE, mode);
        }
        
        /* Mix in some OpenMP */
        if (iter % 50 == 0) {
            #pragma omp parallel for simd
            for (i = 0; i < ARRAY_SIZE; i++) {
                array1[i] = array1[i] ^ array3[i];
                array2[i] = array2[i] + array1[i];
            }
        }
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    /* Compute final checksum for verification */
    unsigned long long checksum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum += (unsigned long long)array1[i];
        checksum += (unsigned long long)array2[i];
        checksum += (unsigned long long)array3[i];
        checksum = (checksum << 13) | (checksum >> 51); /* 64-bit rotate */
    }
    
    printf("Execution time: %.3f seconds\n", elapsed);
    printf("Final checksum: 0x%016llX\n", checksum);
    printf("Test completed successfully.\n");
    
    free(array1);
    free(array2);
    free(array3);
    free(temp);
    
    return 0;
}
