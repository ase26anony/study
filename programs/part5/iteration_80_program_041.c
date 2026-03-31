/* Selective Scheduling Dump Test Program
 * Designed to trigger dump_insn_rtx_1 in sel-sched-dump.cc
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-dump sel_sched_test.c helper.c -o sel_sched_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* External declarations for helper functions */
extern int __attribute__((noinline)) non_inlineable_func(int x, int y);
extern int __attribute__((const)) pure_func(int x);
extern void init_arrays(void);
extern int process_data(int iterations);

/* Global arrays with potential aliasing */
volatile int g_volatile_seed = 42;
int g_array1[1024];
int g_array2[1024];
short g_short_array[2048];
unsigned g_unsigned_array[512];

/* Struct to create complex memory access patterns */
struct DataBlock {
    int values[8];
    short indices[16];
    unsigned checksum;
};

struct DataBlock g_data[64];

/* Non-inlineable function to create scheduling boundaries */
int __attribute__((noinline)) non_inlineable_func(int x, int y) {
    /* Complex enough to not be inlined */
    asm volatile ("" : : "r"(x), "r"(y) : "memory");
    return (x * y) ^ (x + y) ^ (x - y);
}

/* Pure function for loop-invariant removal testing */
int __attribute__((const)) pure_func(int x) {
    return (x * 3) / 2 + 7;
}

/* Warm-up function to trigger compilation paths */
void warm_up_computation(int limit) {
    register int reg_acc = 0;  /* Hint for register allocation */
    int stack_acc = 0;
    unsigned u_acc = 0;
    
    /* Mixed-type loop with varying trip counts */
    for (int i = 0; i < limit; ++i) {
        /* Loop-carried dependency */
        reg_acc += g_array1[i & 1023];
        
        /* Conditional execution */
        if (__builtin_expect((i & 3) == 0, 0)) {
            stack_acc += pure_func(i);
        }
        
        /* Memory barrier to create scheduling boundaries */
        asm volatile ("" ::: "memory");
        
        /* Mixed data type operations */
        u_acc += (unsigned)reg_acc * (unsigned)stack_acc;
        
        /* Function call with loop-variant arguments */
        if (i % 7 == 0) {
            g_array2[i & 1023] = non_inlineable_func(i, reg_acc);
        }
    }
    
    /* Use results to prevent optimization */
    g_volatile_seed = reg_acc + stack_acc + (int)u_acc;
}

/* Core computation with nested loops */
unsigned long long core_computation(int outer_limit, int inner_limit, int conditional_limit) {
    unsigned long long total_checksum = 0;
    int outer_acc = 0;
    
    /* Outer loop with varying data types */
    for (unsigned outer = 0; outer < (unsigned)outer_limit; ++outer) {
        int middle_acc = 0;
        short short_acc = 0;
        
        /* First inner loop - always executed */
        for (int inner = 0; inner < inner_limit; ++inner) {
            /* Complex addressing with potential aliasing */
            int idx = (outer * 31 + inner * 17) & 1023;
            
            /* Loop-carried dependencies on multiple variables */
            outer_acc += g_array1[idx];
            middle_acc += g_array2[idx];
            
            /* Mixed-width operations */
            short_acc += (short)g_short_array[(outer + inner) & 2047];
            
            /* Conditional inner-inner loop */
            if (__builtin_expect(outer > (unsigned)conditional_limit, 0)) {
                register int deep_acc = 0;
                /* Deeply nested loop with small trip count */
                for (unsigned char k = 0; k < 8; ++k) {
                    /* Struct member access */
                    deep_acc += g_data[(outer + k) & 63].values[k];
                    
                    /* Pure function call */
                    deep_acc += pure_func(k);
                    
                    /* Memory operation with barrier */
                    asm volatile ("" ::: "memory");
                }
                middle_acc += deep_acc;
            }
            
            /* Periodic function call */
            if (inner % 5 == 0) {
                g_unsigned_array[inner & 511] = non_inlineable_func(outer_acc, middle_acc);
            }
        }
        
        /* Second inner loop - conditionally executed */
        if (outer % 3 == 0) {
            unsigned u_acc = 0;
            for (short s = 0; s < (short)(inner_limit / 2); ++s) {
                /* Different addressing mode */
                u_acc += g_unsigned_array[s & 511];
                
                /* Complex expression with multiple uses */
                int temp = middle_acc * s + outer_acc;
                u_acc += (unsigned)temp;
                
                /* Array update with dependency */
                g_short_array[(outer * s) & 2047] = (short)(u_acc & 0xFFFF);
            }
            total_checksum += u_acc;
        }
        
        /* Update total with mixed-type operations */
        total_checksum += (unsigned long long)outer_acc * middle_acc;
        total_checksum += short_acc;
        
        /* Optimization barrier */
        asm volatile ("" ::: "memory");
    }
    
    return total_checksum;
}

/* Initialize data with pseudo-random values */
void init_data(void) {
    /* Simple LCG for pseudo-random values */
    unsigned lcg = g_volatile_seed;
    
    for (int i = 0; i < 1024; ++i) {
        lcg = lcg * 1103515245 + 12345;
        g_array1[i] = (int)(lcg & 0x7FFF) - 16384;
        g_array2[i] = (int)((lcg >> 16) & 0x7FFF) - 16384;
    }
    
    for (int i = 0; i < 2048; ++i) {
        lcg = lcg * 1103515245 + 12345;
        g_short_array[i] = (short)(lcg & 0xFFFF);
    }
    
    for (int i = 0; i < 512; ++i) {
        lcg = lcg * 1103515245 + 12345;
        g_unsigned_array[i] = lcg;
    }
    
    for (int i = 0; i < 64; ++i) {
        for (int j = 0; j < 8; ++j) {
            lcg = lcg * 1103515245 + 12345;
            g_data[i].values[j] = (int)lcg;
        }
        for (int j = 0; j < 16; ++j) {
            lcg = lcg * 1103515245 + 12345;
            g_data[i].indices[j] = (short)(lcg & 0xFFFF);
        }
        g_data[i].checksum = lcg;
    }
}

int main(int argc, char *argv[]) {
    /* Use command line arguments to make loop bounds non-constant */
    int warmup_iters = (argc > 1) ? atoi(argv[1]) : 100;
    int outer_iters = (argc > 2) ? atoi(argv[2]) : 50;
    int inner_iters = (argc > 3) ? atoi(argv[3]) : 20;
    int cond_limit = (argc > 4) ? atoi(argv[4]) : 10;
    
    /* Initialize data */
    init_data();
    
    printf("Starting selective scheduling test...\n");
    printf("Warmup iterations: %d\n", warmup_iters);
    printf("Outer iterations: %d\n", outer_iters);
    printf("Inner iterations: %d\n", inner_iters);
    printf("Conditional limit: %d\n", cond_limit);
    
    /* Warm-up phase - executed once */
    warm_up_computation(warmup_iters);
    
    /* Main computation with nested loops */
    unsigned long long result = core_computation(outer_iters, inner_iters, cond_limit);
    
    /* Additional computation with different patterns */
    int mixed_result = process_data(outer_iters * 2);
    
    /* Final result */
    printf("Computation result: %llu\n", result);
    printf("Mixed result: %d\n", mixed_result);
    printf("Total: %llu\n", result + mixed_result);
    
    return 0;
}
