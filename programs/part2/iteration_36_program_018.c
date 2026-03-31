/* sel-sched-test.c
 * Designed to trigger debug_insn_rtx in GCC's selective scheduler
 * Compile with: gcc -O2 -fsel-sched-pipelining -dS -fdump-rtl-all sel-sched-test.c -o sel-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force memory dependencies and prevent optimization */
static volatile int memory_barrier;

/* Complex loop with mixed operations and dependencies */
static inline uint64_t compute_loop(int start, int end, 
                                   float* restrict arr_f, 
                                   double* restrict arr_d,
                                   int* arr_i,
                                   const int* mod_arr) 
{
    double d_acc = 1.0;
    float f_acc = 2.0f;
    int i_acc = start;
    uint64_t bit_acc = 0;
    
    /* Hot loop with multiple dependencies and operations */
    for (int i = start; i < end; i++) {
        /* Integer operations with carried dependency */
        i_acc = i_acc * 1103515245 + 12345;
        
        /* Floating-point operations */
        f_acc = f_acc * 1.5f + arr_f[i % 256];
        d_acc = d_acc / 1.7 + arr_d[i % 256] * 0.5;
        
        /* Memory operations with potential aliasing */
        arr_i[i % 256] = i_acc % 1000;
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            f_acc = f_acc - 3.14f;
            /* Additional operations in taken branch */
            d_acc = d_acc * 2.0 - 1.0;
            memory_barrier = i; /* Volatile write */
        } else if (i % 13 == 0) {
            /* Another basic block */
            i_acc = i_acc ^ mod_arr[i % 16];
            f_acc = f_acc + 2.71f;
        }
        
        /* More arithmetic diversity */
        if (i % 3 == 0) {
            d_acc = d_acc + (double)(i % 5);
        }
        
        /* Bit manipulation */
        bit_acc ^= ((uint64_t)i_acc << 32) | (*(uint32_t*)&f_acc);
        
        /* Complex expression with multiple operations */
        arr_f[i % 256] = (f_acc * 0.25f) + (float)(d_acc * 0.1);
        
        /* Prevent loop invariant code motion */
        asm volatile("" : "+r" (i_acc), "+r" (f_acc) : : "memory");
    }
    
    /* Combine accumulators */
    return bit_acc ^ *(uint64_t*)&d_acc ^ i_acc;
}

/* Another variant to encourage inlining */
static inline uint64_t compute_loop_variant(int start, int end,
                                          float* arr1, double* arr2,
                                          int* arr3, const int* mods) 
{
    uint64_t result = compute_loop(start, end, arr1, arr2, arr3, mods);
    
    /* Additional computation */
    for (int i = 0; i < 16; i++) {
        result = (result >> 1) ^ (result * 0x9e3779b97f4a7c15);
    }
    
    return result;
}

int main(void) 
{
    /* Initialize data arrays */
    float arr_f[256];
    double arr_d[256];
    int arr_i[256];
    int mod_arr[16];
    
    /* Fill with pseudo-random but deterministic values */
    for (int i = 0; i < 256; i++) {
        arr_f[i] = (i * 1.2345f) / 256.0f;
        arr_d[i] = (i * 3.14159) / 256.0;
        arr_i[i] = i * 1103515245;
    }
    
    for (int i = 0; i < 16; i++) {
        mod_arr[i] = (i << 4) | (i >> 4);
    }
    
    uint64_t final_result = 0;
    
    /* Call hot functions multiple times to create scheduling regions */
    for (int iter = 0; iter < 100; iter++) {
        int base = iter * 100;
        
        /* Mix calls to different variants */
        if (iter % 3 == 0) {
            final_result ^= compute_loop(base, base + 500, 
                                        arr_f, arr_d, arr_i, mod_arr);
        } else {
            final_result ^= compute_loop_variant(base, base + 300,
                                               arr_f, arr_d, arr_i, mod_arr);
        }
        
        /* Modify array data between iterations */
        for (int i = 0; i < 16; i++) {
            arr_f[(iter * 17 + i) % 256] += 0.1f;
            arr_d[(iter * 23 + i) % 256] *= 1.01;
        }
        
        /* Memory barrier */
        memory_barrier = iter;
    }
    
    /* Additional complex region */
    {
        double temp_d = 1.0;
        float temp_f = 2.0f;
        
        for (int i = 0; i < 1000; i++) {
            /* Mixed operations in a tight loop */
            temp_d = temp_d * 1.1 + (double)temp_f;
            temp_f = temp_f * 0.9f - (float)temp_d;
            
            /* Conditional with side effects */
            if (i % 11 == 0) {
                arr_i[i % 256] = (int)(temp_d * 1000);
                temp_f = -temp_f;
            }
            
            /* Inline assembly to prevent optimization */
            asm volatile("" : "+r" (temp_d), "+r" (temp_f) : : "memory");
        }
        
        final_result ^= *(uint64_t*)&temp_d;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: 0x%016lx\n", final_result);
    
    return 0;
}
