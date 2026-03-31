/* sel-sched-coverage.c
 * Designed to trigger debug_insn_rtx() in GCC's selective scheduler
 * Compile with: gcc -O2 -fsel-sched-pipelining -dS -fdump-rtl-all sel-sched-coverage.c -o sel-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000000

/* Use volatile to prevent optimization of dependencies */
static volatile int global_seed = 42;

/* Function with potential aliasing */
static inline void compute_loop(int *restrict dest, const int *src1, 
                               float *src2, double *accum, int n) {
    double local_accum = *accum;
    float fp_temp = 0.0f;
    int int_temp = 0;
    
    /* Hot loop with mixed operations and dependencies */
    for (int i = 0; i < n; i++) {
        /* Integer arithmetic with carried dependency */
        int_temp = src1[i] * global_seed;
        int_temp += (i & 0xFF);
        
        /* Floating-point operations */
        fp_temp = src2[i] * 1.5f;
        if (fp_temp > 100.0f) {
            fp_temp = fp_temp / 2.0f;
        }
        
        /* Memory operations with potential aliasing */
        dest[i] = int_temp + (int)fp_temp;
        
        /* Complex dependency chain */
        local_accum += (double)dest[i] * 0.01;
        local_accum -= (double)src1[i] * 0.005;
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            local_accum *= 1.1;
            int_temp >>= 1;
        } else if (i % 13 == 0) {
            local_accum /= 1.05;
            int_temp <<= 1;
        }
        
        /* Mixed division operations */
        if (i % 100 == 0) {
            local_accum = local_accum / 1.5;
            fp_temp = fp_temp / 3.0f;
        }
        
        /* Inline asm to create memory clobber */
        asm volatile("" : : "r"(dest[i]), "r"(src1[i]) : "memory");
    }
    
    /* Store back accumulated value */
    *accum = local_accum;
    
    /* Another dependency to prevent tail merging */
    global_seed = (global_seed * 1103515245 + 12345) & 0x7FFFFFFF;
}

/* Secondary computation with different pattern */
static inline void compute_loop2(double *restrict dest, const float *src1,
                                int *src2, double *accum, int n) {
    double local_accum = *accum;
    
    for (int i = 0; i < n; i++) {
        /* Different operation mix */
        double val = (double)src1[i] * 2.5;
        val += (double)src2[i % 256] * 0.75;
        
        /* More complex floating point */
        if (val > 50.0) {
            val = val / 1.8;
        } else {
            val = val * 1.2;
        }
        
        dest[i] = val;
        local_accum += val * (i % 10 + 1);
        
        /* Nested conditionals */
        if (i % 11 == 0) {
            if (local_accum > 1000.0) {
                local_accum = local_accum * 0.9;
            }
            dest[i] += 1.0;
        }
        
        /* Integer division (expensive) */
        if (i % 23 == 0) {
            src2[i % 256] = src2[i % 256] / 3;
        }
    }
    
    *accum = local_accum;
}

int main(void) {
    /* Allocate and initialize arrays */
    int *array1 = (int*)malloc(SIZE * sizeof(int));
    int *array2 = (int*)malloc(SIZE * sizeof(int));
    float *array3 = (float*)malloc(SIZE * sizeof(float));
    double *array4 = (double*)malloc(SIZE * sizeof(double));
    double *array5 = (double*)malloc(SIZE * sizeof(double));
    
    if (!array1 || !array2 || !array3 || !array4 || !array5) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (i * 13 + 7) & 0xFFF;
        array2[i] = 0;
        array3[i] = (float)((i * 17 + 11) % 100) / 3.0f;
        array4[i] = (double)((i * 19 + 13) % 200) / 5.0;
        array5[i] = 0.0;
    }
    
    double accum1 = 0.0;
    double accum2 = 0.0;
    
    /* Perform multiple iterations to create hot loop */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call inline functions multiple times */
        compute_loop(array2, array1, array3, &accum1, SIZE);
        
        if (iter % 3 == 0) {
            compute_loop2(array5, array3, array1, &accum2, SIZE);
        }
        
        /* Modify input data slightly each iteration */
        for (int i = 0; i < SIZE; i += 64) {
            array1[i] = (array1[i] + iter) & 0xFFF;
            array3[i] += 0.1f;
        }
        
        /* Prevent complete optimization */
        asm volatile("" : : "r"(array2), "r"(array5) : "memory");
    }
    
    /* Compute checksum to ensure computations aren't optimized away */
    uint64_t checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum ^= (uint64_t)array2[i];
        checksum ^= (uint64_t)(array5[i] * 1000);
    }
    
    checksum ^= (uint64_t)(accum1 * 10000);
    checksum ^= (uint64_t)(accum2 * 10000);
    
    printf("Result checksum: 0x%016llx\n", (unsigned long long)checksum);
    printf("Accum1: %f, Accum2: %f\n", accum1, accum2);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(array4);
    free(array5);
    
    return 0;
}
