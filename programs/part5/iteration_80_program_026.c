/* main.c - Primary file with nested loops for selective scheduling */
#include <stdio.h>
#include <stdlib.h>

/* Global arrays to create memory dependencies */
extern int global_array[1024];
extern short global_short_array[2048];
extern volatile int volatile_seed;

/* Non-inlineable functions */
int __attribute__((noinline)) non_inline_func(int x, int y) {
    return (x * y) ^ (x + y);
}

unsigned __attribute__((noinline)) non_inline_unsigned(unsigned a, unsigned b) {
    return (a << 3) | (b >> 2);
}

/* Pure function for scheduling interest */
int __attribute__((const)) pure_func(int x) {
    return x * 3 + 7;
}

/* Helper from another translation unit */
extern int external_helper(int idx);

/* Core computation with nested loops */
unsigned long long __attribute__((noinline)) 
compute_checksum(int outer_limit, int inner_limit, int conditional_limit) {
    unsigned long long checksum = 0;
    register int reg_acc = 0;  /* Hint for register allocation */
    int stack_acc = 0;
    unsigned bit_acc = 0;
    
    /* Outer loop with varying data types */
    for (int i = 0; i < outer_limit; ++i) {
        int loop_var = i;
        short short_var = (short)(i & 0xFFFF);
        
        /* Conditional inner loop execution */
        if (__builtin_expect(i > conditional_limit, 0)) {
            /* Inner loop with different counter type */
            for (unsigned j = 0; j < (unsigned)inner_limit; ++j) {
                /* Mix of arithmetic operations */
                int temp = loop_var * j;
                temp += pure_func(j);
                temp ^= non_inline_func(loop_var, j);
                
                /* Memory operations with potential aliasing */
                int mem_val = global_array[(i * 16 + j) % 1024];
                short short_val = global_short_array[(i * 32 + j) % 2048];
                
                /* Loop-carried dependency */
                reg_acc = mem_val + reg_acc;
                stack_acc = temp - stack_acc;
                
                /* Conditional branch inside innermost loop */
                if ((i % 8) == 0) {
                    bit_acc ^= (unsigned)temp;
                    checksum += non_inline_unsigned(bit_acc, j);
                } else if ((j % 5) == 0) {
                    checksum += external_helper(j);
                }
                
                /* Optimization barrier creating scheduling boundary */
                asm volatile ("" ::: "memory");
                
                /* Multiple uses of same variable */
                int multi_use = reg_acc;
                checksum += multi_use;
                multi_use ^= stack_acc;
                checksum += multi_use * 2;
                
                /* Another memory operation */
                global_array[(j * 3) % 1024] = (int)(checksum & 0x7FFFFFFF);
            }
        } else {
            /* Different path with simpler loop */
            for (short k = 0; k < (short)(inner_limit / 2); ++k) {
                int val = k * loop_var;
                val += global_short_array[k % 2048];
                checksum += val;
                
                /* Function call with loop-variant arguments */
                checksum += pure_func(val) * 2;
                
                /* Another scheduling barrier */
                asm volatile ("" ::: "memory");
            }
        }
        
        /* Additional computation between outer loop iterations */
        if ((i % 3) == 0) {
            checksum ^= (unsigned long long)reg_acc << 32;
            reg_acc = 0;  /* Reset for next iteration */
        }
    }
    
    /* Final mixing */
    checksum = checksum ^ ((unsigned long long)stack_acc << 16);
    checksum = checksum ^ bit_acc;
    
    return checksum;
}

/* Warm-up function */
void __attribute__((noinline)) warm_up_computation(void) {
    volatile int warm_limit = 10;
    int warm_acc = 0;
    
    for (int i = 0; i < warm_limit; ++i) {
        for (int j = 0; j < 5; ++j) {
            warm_acc += i * j;
            warm_acc ^= non_inline_func(i, j);
            asm volatile ("" ::: "memory");
        }
    }
    
    /* Use the result to prevent optimization */
    global_array[0] = warm_acc;
}

/* Main function with external inputs */
int main(int argc, char *argv[]) {
    /* Initialize with pseudo-random values using simple LCG */
    unsigned int seed = 123456789;
    for (int i = 0; i < 1024; ++i) {
        seed = seed * 1103515245 + 12345;
        global_array[i] = (int)(seed % 1000);
    }
    
    for (int i = 0; i < 2048; ++i) {
        seed = seed * 1103515245 + 12345;
        global_short_array[i] = (short)(seed % 1000);
    }
    
    /* Make loop bounds depend on arguments */
    int outer_bound = (argc > 1) ? atoi(argv[1]) : 50;
    int inner_bound = (argc > 2) ? atoi(argv[2]) : 100;
    int cond_bound = (argc > 3) ? atoi(argv[3]) : 25;
    
    /* Ensure bounds are reasonable */
    if (outer_bound <= 0) outer_bound = 50;
    if (inner_bound <= 0) inner_bound = 100;
    if (cond_bound < 0) cond_bound = 25;
    
    /* Warm-up execution */
    warm_up_computation();
    
    /* Main computation */
    unsigned long long result = compute_checksum(
        outer_bound, 
        inner_bound, 
        cond_bound
    );
    
    /* Print verifiable result */
    printf("Computed checksum: %llu\n", result);
    printf("Result hex: 0x%016llx\n", result);
    
    return 0;
}
