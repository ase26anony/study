/* sel-sched-coverage.c
 * Designed to trigger debug_insn_rtx() in GCC's selective scheduler
 * Compile with: gcc -O2 -fsel-sched-pipelining -dS -fdump-rtl-all sel-sched-coverage.c -o sel-sched-coverage
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000000

/* Force memory dependencies and prevent optimization */
static volatile int g_volatile_counter = 0;

/* Function with mixed operations and dependencies */
static inline uint64_t compute_loop(int *restrict arr_a, int *arr_b, 
                                   float *restrict arr_f, double *arr_d,
                                   int start, int end, int seed) {
    uint64_t acc_int = seed;
    float acc_float = seed * 0.5f;
    double acc_double = seed * 0.25;
    
    /* Create register pressure with many variables */
    int t1, t2, t3, t4;
    float f1, f2, f3;
    double d1, d2;
    
    /* Complex loop with multiple basic blocks */
    for (int i = start; i < end; i++) {
        /* Basic block 1: Integer operations with carried dependency */
        t1 = arr_a[i] ^ (i * 3);
        t2 = arr_b[i] + (i % 17);
        acc_int = (acc_int * 1103515245 + 12345) ^ t1 ^ t2;
        
        /* Force memory barrier */
        asm volatile("" ::: "memory");
        
        /* Basic block 2: Floating point operations */
        f1 = arr_f[i] * 1.5f;
        f2 = arr_f[(i + 1) % SIZE] * 2.0f;
        acc_float = acc_float + f1 - f2;
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            /* Basic block 3: Division (expensive operation) */
            d1 = arr_d[i] / 3.14159;
            d2 = arr_d[(i + 3) % SIZE] * 2.71828;
            acc_double = acc_double * 0.99 + d1 * d2;
            
            /* Store back with potential aliasing */
            arr_f[i] = acc_float * 0.1f;
        } else if (i % 13 == 0) {
            /* Basic block 4: More complex operations */
            t3 = (arr_a[i] << 3) | (arr_b[i] >> 2);
            t4 = t3 * (i % 31);
            acc_int = acc_int + t4;
            
            /* Memory store */
            arr_b[i] = acc_int & 0xFF;
        } else {
            /* Basic block 5: Default path */
            acc_int = acc_int + (i & 0xF);
            acc_float = acc_float * 0.999f;
            acc_double = acc_double * 1.0001;
        }
        
        /* Periodic external dependency */
        if (i % 1000 == 0) {
            g_volatile_counter++;
        }
        
        /* Mix operations across types */
        arr_d[i] = acc_double + (double)acc_int * 0.001 + (double)acc_float;
        
        /* Another memory barrier */
        asm volatile("" ::: "memory");
    }
    
    /* Combine all accumulators */
    return (uint64_t)acc_int + (uint64_t)acc_float + (uint64_t)acc_double;
}

/* Another hot function to encourage inlining */
static inline uint64_t process_chunk(int *restrict a, int *b, 
                                    float *restrict f, double *d,
                                    int chunk_size, int offset) {
    uint64_t result = 0;
    
    /* Process multiple sub-loops */
    for (int chunk = 0; chunk < 4; chunk++) {
        int start = offset + chunk * (chunk_size / 4);
        int end = start + (chunk_size / 4);
        
        result ^= compute_loop(a, b, f, d, start, end, 
                              chunk * 7919 + result);
    }
    
    return result;
}

int main(void) {
    /* Allocate and initialize arrays with non-uniform data */
    int *array_a = (int*)malloc(SIZE * sizeof(int));
    int *array_b = (int*)malloc(SIZE * sizeof(int));
    float *array_f = (float*)malloc(SIZE * sizeof(float));
    double *array_d = (double*)malloc(SIZE * sizeof(double));
    
    if (!array_a || !array_b || !array_f || !array_d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = (i * 1103515245 + 12345) & 0x7FFF;
        array_b[i] = (i * 1664525 + 1013904223) & 0x7FFF;
        array_f[i] = (float)((i * 1103515245 + 12345) % 1000) * 0.001f;
        array_d[i] = (double)((i * 1664525 + 1013904223) % 1000) * 0.001;
    }
    
    uint64_t final_result = 0;
    
    /* Main computation - multiple calls to create scheduling regions */
    for (int iter = 0; iter < ITERATIONS / SIZE + 1; iter++) {
        int offset = (iter * 17) % (SIZE / 2);
        
        /* Call inlined function multiple times with different parameters */
        final_result ^= process_chunk(array_a, array_b, array_f, array_d,
                                     SIZE / 2, offset);
        
        /* Alternate between different computation patterns */
        if (iter % 3 == 0) {
            final_result += compute_loop(array_a, array_b, array_f, array_d,
                                        0, SIZE, final_result & 0xFFFF);
        }
    }
    
    /* Use result to prevent optimization */
    printf("Result checksum: 0x%016llx\n", (unsigned long long)final_result);
    printf("Volatile counter: %d\n", g_volatile_counter);
    
    /* Cleanup */
    free(array_a);
    free(array_b);
    free(array_f);
    free(array_d);
    
    return 0;
}
