/* Selective Scheduling Dump Test Program
 * Designed to trigger dump_insn_rtx_1 in sel-sched-dump.cc
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-dump sel_sched_test.c helper_funcs.c -o sel_sched_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* External declarations for multi-file compilation stress */
extern int helper_const_func(int x, int y) __attribute__((const));
extern void helper_noinline_func(int* arr, int idx) __attribute__((noinline));
extern volatile int g_volatile_seed;

/* Global arrays for memory operations with potential aliasing */
int g_array1[1024];
int g_array2[1024];
short g_short_array[2048];
unsigned g_unsigned_array[512];

/* Struct for complex memory access patterns */
struct DataStruct {
    int a;
    int b;
    short c;
    unsigned d;
    int* ptr;
};

struct DataStruct g_struct_array[256];

/* Non-inlineable function to create scheduling boundaries */
__attribute__((noinline)) 
int compute_checksum(int* data, int size) {
    int sum = 0;
    for (int i = 0; i < size; ++i) {
        sum += data[i] ^ 0x5A5A5A5A;
    }
    return sum;
}

/* Pure function for loop-invariant computation */
__attribute__((const))
int pure_multiply(int a, int b) {
    return a * b;
}

/* Function with loop-carried dependencies and mixed operations */
__attribute__((noinline))
int complex_nested_loops(int outer_limit, int inner_limit, int threshold) {
    int acc = 0;
    unsigned u_acc = 0;
    short s_acc = 0;
    
    /* Outer loop with varying trip count */
    for (int i = 0; i < outer_limit; ++i) {
        /* Register hint for variable */
        register int reg_i = i;
        
        /* Conditional inner loop execution */
        if (__builtin_expect(reg_i > threshold, 0)) {
            /* Inner loop with different data type */
            for (unsigned j = 0; j < (unsigned)inner_limit; ++j) {
                /* Mix of arithmetic operations */
                int temp = g_array1[reg_i] + g_array2[j % 1024];
                temp = temp * pure_multiply(reg_i, j);
                
                /* Memory operation with potential aliasing */
                g_short_array[(reg_i * 16 + j) % 2048] = (short)(temp & 0xFFFF);
                
                /* Loop-carried dependency */
                acc = temp + acc;
                
                /* Conditional branch inside innermost loop */
                if (reg_i % 8 == 0) {
                    u_acc += g_unsigned_array[j % 512];
                    /* Optimization barrier */
                    asm volatile ("" ::: "memory");
                }
                
                /* Function call to non-inlineable function */
                helper_noinline_func(g_array1, reg_i);
                
                /* Another arithmetic operation with different type */
                s_acc += (short)((temp >> 16) & 0xFF);
            }
        } else {
            /* Different path with simpler computation */
            for (short k = 0; k < (short)(inner_limit / 2); ++k) {
                int idx = (reg_i * 32 + k) % 1024;
                g_array2[idx] = helper_const_func(g_array1[idx], k);
                acc += g_array2[idx];
                
                /* Struct access */
                g_struct_array[idx % 256].a = reg_i;
                g_struct_array[idx % 256].b = k;
                g_struct_array[idx % 256].c = s_acc;
            }
        }
        
        /* Additional computation after inner loop */
        if (reg_i % 4 == 0) {
            /* Another optimization barrier */
            asm volatile ("" ::: "memory");
            u_acc = u_acc ^ (reg_i * 0x12345678);
        }
    }
    
    return acc + (int)u_acc + (int)s_acc;
}

/* Another complex loop structure with different patterns */
__attribute__((noinline))
int multi_dimensional_loops(int dim1, int dim2, int dim3) {
    int total = 0;
    
    /* Triple nested loops */
    for (int x = 0; x < dim1; ++x) {
        int* local_ptr = &g_array1[x * 16];
        
        for (int y = 0; y < dim2; ++y) {
            register int reg_y = y;
            
            /* Volatile dependency to prevent optimization */
            int volatile_mod = g_volatile_seed % 16;
            
            for (int z = 0; z < dim3; ++z) {
                /* Complex addressing calculation */
                int idx = (x * dim2 * dim3 + reg_y * dim3 + z) % 1024;
                
                /* Mixed operations with different data types */
                int val1 = g_array1[idx];
                unsigned val2 = g_unsigned_array[idx % 512];
                short val3 = g_short_array[(idx * 2) % 2048];
                
                /* Conditional computation */
                int result;
                if (__builtin_expect((val1 + val2) > 1000, 1)) {
                    result = pure_multiply(val1, val3) + volatile_mod;
                } else {
                    result = helper_const_func(val2, val3) - volatile_mod;
                }
                
                /* Memory store with pointer arithmetic */
                *(local_ptr + (reg_y % 16)) = result;
                
                /* Loop-carried dependency chain */
                total = total ^ result;
                
                /* Periodic function call */
                if (z % 7 == 0) {
                    helper_noinline_func(g_array2, idx);
                }
            }
            
            /* Optimization barrier between middle loop iterations */
            asm volatile ("" ::: "memory");
        }
    }
    
    return total;
}

/* Warm-up function to trigger different compilation paths */
__attribute__((noinline))
void warm_up_computation(void) {
    int warm_acc = 0;
    
    /* Simple warm-up loop */
    for (int i = 0; i < 100; ++i) {
        warm_acc += helper_const_func(i, i * 2);
        g_array1[i % 1024] = warm_acc;
    }
    
    /* Prevent dead code elimination */
    asm volatile ("" ::: "memory");
}

/* Initialize data with pseudo-random values using LCG */
void initialize_data(void) {
    unsigned lcg_seed = 123456789;
    
    for (int i = 0; i < 1024; ++i) {
        lcg_seed = lcg_seed * 1103515245 + 12345;
        g_array1[i] = (int)(lcg_seed % 1000);
        g_array2[i] = (int)(lcg_seed % 500);
        
        if (i < 512) {
            g_unsigned_array[i] = lcg_seed % 10000;
        }
        
        if (i < 2048) {
            g_short_array[i] = (short)(lcg_seed % 32768);
        }
        
        if (i < 256) {
            g_struct_array[i].a = lcg_seed % 100;
            g_struct_array[i].b = lcg_seed % 200;
            g_struct_array[i].c = (short)(lcg_seed % 1000);
            g_struct_array[i].d = lcg_seed;
            g_struct_array[i].ptr = &g_array1[i * 4 % 1024];
        }
    }
}

int main(int argc, char* argv[]) {
    /* Use command line arguments for variability */
    int outer_limit = (argc > 1) ? atoi(argv[1]) : 50;
    int inner_limit = (argc > 2) ? atoi(argv[2]) : 100;
    int threshold = (argc > 3) ? atoi(argv[3]) : 25;
    
    /* Initialize global volatile for dependency */
    g_volatile_seed = outer_limit;
    
    /* Initialize data */
    initialize_data();
    
    /* Warm-up computation */
    warm_up_computation();
    
    /* Main computation with nested loops */
    int result1 = complex_nested_loops(outer_limit, inner_limit, threshold);
    
    /* Second computation with multi-dimensional loops */
    int dim1 = outer_limit / 2;
    int dim2 = inner_limit / 4;
    int dim3 = threshold * 2;
    int result2 = multi_dimensional_loops(dim1, dim2, dim3);
    
    /* Final checksum computation */
    int final_checksum = compute_checksum(g_array1, 1024);
    
    /* Combine results */
    int total_result = result1 + result2 + final_checksum;
    
    /* Print verifiable result */
    printf("Result: %d (0x%08x)\n", total_result, total_result);
    
    return 0;
}
