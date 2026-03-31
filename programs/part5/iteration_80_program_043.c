/* Selective Scheduling Dump Test Program
 * Designed to trigger dump_insn_rtx_1 in sel-sched-dump.cc
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-dump sel_sched_test.c helper.c -o sel_sched_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* External declarations for multi-file compilation stress */
extern int global_array[1024];
extern volatile int g_volatile_seed;

/* Non-inlineable functions to create scheduling boundaries */
__attribute__((noinline)) int noinline_multiply(int a, int b);
__attribute__((noinline)) unsigned noinline_modulo(unsigned a, unsigned b);
__attribute__((noinline)) short noinline_short_op(short a, short b);

/* Pure functions for varied RTL patterns */
__attribute__((const)) int pure_square(int x);
__attribute__((const)) unsigned pure_cube(unsigned x);

/* Global arrays for memory operations with potential aliasing */
int arr1[256];
int arr2[256];
short short_arr[512];
unsigned mixed_arr[384];

/* Struct with different sized members for varied RTL */
struct MixedData {
    int id;
    short count;
    unsigned flags;
    char tag;
    long long big_val;
};

struct MixedData struct_arr[128];

/* Warm-up function to trigger different compilation paths */
__attribute__((noinline)) void warm_up_computation(int iterations) {
    volatile int warm_acc = 0;
    for (int w = 0; w < iterations; ++w) {
        warm_acc += w * (w % 7);
        asm volatile("" ::: "memory");  /* Optimization barrier */
    }
    /* Use the result to prevent dead code elimination */
    if (warm_acc == 0) {
        printf("Warm-up completed\n");
    }
}

/* Core computation with nested loops targeting selective scheduler */
__attribute__((noinline)) long long compute_checksum(int outer_limit, 
                                                    unsigned inner_limit,
                                                    short threshold) {
    long long checksum = 0;
    int loop_carried_dep = 0;
    register int reg_var1 = 0;  /* Hint for register allocation */
    register unsigned reg_var2 = 0;
    
    /* Outer loop with varying data types */
    for (int i = 0; i < outer_limit; ++i) {
        int temp_i = i;
        unsigned u_i = (unsigned)i;
        
        /* Conditional inner loop execution */
        if (__builtin_expect(i > threshold, 0)) {
            /* First inner loop with short counter */
            for (short s = 0; s < (short)(inner_limit % 256); ++s) {
                /* Mix of arithmetic operations */
                int calc = temp_i * s + reg_var1;
                unsigned u_calc = u_i + (unsigned)s * reg_var2;
                
                /* Loop-carried dependency */
                loop_carried_dep = arr1[calc % 256] + loop_carried_dep;
                
                /* Memory operations with potential aliasing */
                arr2[s % 256] = calc;
                short_arr[(u_calc % 512)] = (short)calc;
                
                /* Non-inlineable function call */
                checksum += noinline_multiply(calc, s);
                
                /* Conditional branch inside innermost loop */
                if (i % 8 == 0) {
                    checksum -= noinline_modulo(u_calc, 17);
                }
                
                /* Pure function call with loop-variant arguments */
                checksum += pure_square(calc % 64);
                
                /* Optimization barrier creating scheduling boundary */
                asm volatile("" ::: "memory");
            }
        }
        
        /* Second inner loop with different characteristics */
        for (unsigned j = 0; j < (inner_limit % 128); ++j) {
            /* Different scope variables */
            {
                int local_var = i * j;
                unsigned local_uvar = u_i ^ j;
                
                /* Struct access for varied RTL patterns */
                struct_arr[j % 128].id = local_var;
                struct_arr[j % 128].count = (short)local_uvar;
                struct_arr[j % 128].flags = local_uvar;
                
                /* Complex expression with multiple uses */
                reg_var1 = local_var + reg_var1 - loop_carried_dep;
                reg_var2 = local_uvar | reg_var2;
                
                /* Memory barrier */
                asm volatile("" ::: "memory");
                
                /* Conditional with __builtin_expect */
                if (__builtin_expect((j & 0xF) == 0, 1)) {
                    checksum += pure_cube(local_uvar % 32);
                }
                
                /* Access global array from another translation unit */
                checksum += global_array[(i + j) % 1024];
            }
        }
        
        /* Third loop with short data type */
        for (short k = 0; k < (short)(threshold % 64); ++k) {
            /* Non-inlineable short operation */
            short result = noinline_short_op(k, (short)i);
            
            /* Mixed array access */
            mixed_arr[(i * k) % 384] = (unsigned)result;
            
            /* Update checksum with varied operations */
            checksum += (long long)result * k;
            
            /* Another optimization barrier */
            asm volatile("" ::: "memory");
        }
        
        /* Update register variables for next iteration */
        reg_var1 ^= temp_i;
        reg_var2 += u_i;
    }
    
    return checksum;
}

/* Helper function with loop-carried dependency pattern */
__attribute__((noinline)) int process_array_slice(int start, int end, int stride) {
    int acc = 0;
    for (int idx = start; idx < end; idx += stride) {
        /* Strong loop-carried dependency */
        acc = arr1[idx % 256] + acc;
        
        /* Conditional store */
        if (acc % 3 == 0) {
            arr2[(idx + 1) % 256] = acc;
        }
        
        /* Function call inside loop */
        acc = noinline_multiply(acc, idx % 7);
    }
    return acc;
}

/* Initialize data with pseudo-random values using LCG */
void initialize_data(void) {
    unsigned seed = 123456789;
    for (int i = 0; i < 256; ++i) {
        /* Simple LCG: seed = (a * seed + c) mod m */
        seed = (1103515245 * seed + 12345) & 0x7fffffff;
        arr1[i] = (int)(seed % 1000);
        arr2[i] = (int)(seed % 500);
    }
    
    for (int i = 0; i < 512; ++i) {
        seed = (1103515245 * seed + 12345) & 0x7fffffff;
        short_arr[i] = (short)(seed % 10000);
    }
    
    for (int i = 0; i < 128; ++i) {
        seed = (1103515245 * seed + 12345) & 0x7fffffff;
        struct_arr[i].id = (int)seed;
        struct_arr[i].count = (short)(seed % 100);
        struct_arr[i].flags = seed;
        struct_arr[i].big_val = (long long)seed * seed;
    }
    
    for (int i = 0; i < 384; ++i) {
        seed = (1103515245 * seed + 12345) & 0x7fffffff;
        mixed_arr[i] = seed;
    }
}

int main(int argc, char *argv[]) {
    /* Make loop bounds depend on external inputs */
    int outer_bound = (argc > 1) ? atoi(argv[1]) : 50;
    unsigned inner_bound = (argc > 2) ? (unsigned)atoi(argv[2]) : 100;
    short threshold = (argc > 3) ? (short)atoi(argv[3]) : 10;
    
    /* Use volatile to prevent optimization */
    volatile int vol_bound = outer_bound;
    outer_bound = vol_bound;
    
    printf("Initializing data...\n");
    initialize_data();
    
    printf("Starting warm-up...\n");
    warm_up_computation(100);
    
    printf("Running main computation...\n");
    long long checksum = compute_checksum(outer_bound, inner_bound, threshold);
    
    /* Additional processing with different patterns */
    int slice_result = process_array_slice(0, 256, 3);
    checksum += slice_result;
    
    printf("Final checksum: %lld\n", checksum);
    printf("Slice result: %d\n", slice_result);
    
    return 0;
}

/* Pure function implementations */
__attribute__((const)) int pure_square(int x) {
    return x * x;
}

__attribute__((const)) unsigned pure_cube(unsigned x) {
    return x * x * x;
}
