/* sel-sched-coverage.c
 * Designed to trigger selective scheduler debug output (debug_insn_rtx)
 * Compile with: gcc -O2 -fsel-sched-pipelining -dS -fdump-rtl-all sel-sched-coverage.c -o sel-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force memory dependencies and prevent optimization */
static volatile int memory_barrier;

/* Inline function with hot loop - will be inlined multiple times */
static inline uint64_t compute_loop(int start, int end, 
                                   double* restrict arr1, 
                                   float* restrict arr2,
                                   int* arr3) /* non-restrict for aliasing */
{
    uint64_t acc_int = 0;
    double acc_fp = 0.0;
    float acc_float = 0.0f;
    
    /* Hot loop with multiple dependencies and operations */
    for (int i = start; i < end; i++) {
        /* Integer operations with carried dependency */
        acc_int = (acc_int * 6364136223846793005ULL) + 1442695040888963407ULL;
        
        /* Floating-point operations */
        acc_fp = acc_fp * 1.0000001 + (double)i * 0.001;
        acc_float = acc_float * 1.0001f + (float)i * 0.01f;
        
        /* Memory operations with potential aliasing */
        int temp = arr3[i % 256];
        arr1[i % 128] = acc_fp + temp;
        arr2[i % 64] = acc_float - temp;
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            /* Division operation - expensive and creates complex RTL */
            acc_int /= (uint64_t)(temp + 1);
            acc_fp /= (temp != 0) ? (double)(temp + 1) : 1.0;
        } else if (i % 13 == 0) {
            /* Another branch with different operations */
            acc_int ^= (uint64_t)temp << 32;
            acc_fp = acc_fp * 0.9999;
        } else {
            /* Default path with mixed operations */
            acc_int += (uint64_t)temp;
            acc_fp += (double)temp * 0.5;
        }
        
        /* Complex expression with multiple operations */
        double complex_expr = (acc_fp * 1.5) / ((double)(i % 29) + 1.0);
        arr1[(i + 1) % 128] = complex_expr;
        
        /* Inline assembly to prevent optimization and create memory clobber */
        asm volatile("" : "+r" (acc_int), "+r" (temp) : : "memory");
        
        /* Volatile store to force memory dependency */
        memory_barrier = temp;
    }
    
    /* Combine results */
    return acc_int + (uint64_t)acc_fp + (uint64_t)acc_float;
}

/* Another inline function with different pattern */
static inline uint64_t compute_loop2(int start, int end,
                                    double* arr1,
                                    float* arr2,
                                    int* restrict arr3,
                                    int* restrict arr4)
{
    uint64_t sum = 0;
    double prod = 1.0;
    
    for (int i = start; i < end; i++) {
        /* Different dependency pattern */
        int idx1 = i & 0xFF;
        int idx2 = (i * 37) & 0x7F;
        
        /* Load operations */
        int val1 = arr3[idx1];
        int val2 = arr4[idx2];
        
        /* Mixed integer/FP operations */
        sum += (uint64_t)val1 * val2;
        prod *= (double)val1 / ((double)val2 + 1.0);
        
        /* Store operations */
        arr1[idx1] = prod;
        arr2[idx2] = (float)sum * 0.001f;
        
        /* Nested conditionals for control flow complexity */
        if (val1 > 0) {
            if (val2 < 100) {
                sum ^= (uint64_t)val1 << 16;
            } else {
                prod = prod * 0.99;
            }
        }
        
        /* Periodic expensive operation */
        if (i % 17 == 0) {
            prod = prod / ((double)(val1 % 19) + 2.0);
        }
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
    }
    
    return sum + (uint64_t)prod;
}

int main(void) {
    /* Allocate and initialize arrays */
    const int size = 1024;
    double* arr1 = (double*)aligned_alloc(64, size * sizeof(double));
    float* arr2 = (float*)aligned_alloc(64, size * sizeof(float));
    int* arr3 = (int*)aligned_alloc(64, size * sizeof(int));
    int* arr4 = (int*)aligned_alloc(64, size * sizeof(int));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < size; i++) {
        arr1[i] = (double)(i * 1.2345);
        arr2[i] = (float)(i * 0.9876f);
        arr3[i] = (i * 1103515245 + 12345) & 0x7FFF;
        arr4[i] = (i * 1664525 + 1013904223) & 0x7FFF;
    }
    
    uint64_t total_checksum = 0;
    
    /* Call compute_loop multiple times with different parameters */
    for (int iter = 0; iter < 100; iter++) {
        int start = iter * 10;
        int end = start + 1000;
        
        /* Alternate between two different loop functions */
        if (iter % 2 == 0) {
            total_checksum ^= compute_loop(start, end, arr1, arr2, arr3);
        } else {
            total_checksum ^= compute_loop2(start, end, arr1, arr2, arr3, arr4);
        }
        
        /* Modify array data to create varying patterns */
        arr3[iter % 256] = total_checksum & 0xFFFF;
        arr4[iter % 256] = (total_checksum >> 16) & 0xFFFF;
        
        /* Memory barrier between iterations */
        asm volatile("" : : : "memory");
    }
    
    /* Additional complex loop in main */
    uint64_t final_acc = 0;
    for (int i = 0; i < 5000; i++) {
        /* Mixed operations with array accesses */
        double val = arr1[i % 128];
        float fval = arr2[i % 64];
        int ival = arr3[i % 256];
        
        /* Complex expression tree */
        double result = (val * 2.5 + (double)fval) / ((double)(ival % 31) + 1.0);
        arr1[i % 128] = result;
        
        /* Integer computation with branching */
        if (i % 11 == 0) {
            final_acc += (uint64_t)(result * 1000.0);
        } else {
            final_acc ^= (uint64_t)ival;
        }
        
        /* Periodic division operation */
        if (i % 23 == 0) {
            final_acc /= (uint64_t)((ival & 0xF) + 1);
        }
    }
    
    total_checksum ^= final_acc;
    
    /* Print checksum to prevent dead code elimination */
    printf("Result checksum: 0x%016llX\n", (unsigned long long)total_checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    
    return 0;
}
