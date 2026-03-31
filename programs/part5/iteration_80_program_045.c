/* sel_sched_test.c - Test program for selective scheduling dump coverage */
#include <stdio.h>
#include <stdlib.h>

/* Global arrays to create memory dependencies */
volatile int g_volatile_seed = 42;
int g_array1[1024];
int g_array2[1024];
short g_short_array[2048];
unsigned g_unsigned_array[512];

/* Non-inlineable functions to create scheduling boundaries */
__attribute__((noinline)) int noinline_func1(int x, int y) {
    return (x * y) ^ (x + y);
}

__attribute__((noinline)) unsigned noinline_func2(unsigned a, unsigned b) {
    return (a & b) | (a ^ b);
}

__attribute__((noinline)) short noinline_func3(short s1, short s2) {
    return (short)(s1 * s2 + s1 - s2);
}

/* Pure function for loop-invariant computation */
__attribute__((const)) int pure_func(int x) {
    return x * x - x + 1;
}

/* Helper function with mixed operations */
static int helper_compute(int idx, int acc) {
    int temp = g_array1[idx] + g_array2[idx % 512];
    if (temp & 1) {
        temp = noinline_func1(temp, acc);
    }
    return temp;
}

/* Core computation with nested loops */
unsigned long long core_computation(int outer_limit, int inner_limit, 
                                   int conditional_limit) {
    unsigned long long checksum = 0;
    int register reg_acc = 0;  /* Hint for register allocation */
    int i, j, k;
    
    /* Outer loop with varying trip count */
    for (i = 0; i < outer_limit; ++i) {
        int outer_var = i * 3;
        unsigned u_var = (unsigned)i * 7;
        
        /* Conditional inner loop execution */
        if (__builtin_expect(i > conditional_limit, 0)) {
            /* First inner loop with short counter */
            for (short s = 0; s < (short)(inner_limit / 2); ++s) {
                int idx = (i * 32 + s) % 1024;
                int val = g_array1[idx] + g_short_array[s];
                
                /* Memory barrier to create scheduling boundary */
                asm volatile ("" ::: "memory");
                
                /* Loop-carried dependency */
                reg_acc = val + reg_acc;
                
                /* Conditional operation */
                if (s % 4 == 0) {
                    val = noinline_func3((short)val, (short)reg_acc);
                }
                
                checksum += val;
            }
        } else {
            /* Different inner loop for the else path */
            for (j = 0; j < inner_limit; ++j) {
                unsigned idx = (u_var + j * 5) % 512;
                unsigned val = g_unsigned_array[idx];
                
                /* Mixed-type computation */
                int mixed = (int)val + outer_var;
                
                /* Function call with loop-variant arguments */
                mixed = noinline_func2((unsigned)mixed, (unsigned)j);
                
                /* Another memory barrier */
                asm volatile ("" ::: "memory");
                
                /* Multiple uses of same variable */
                int temp = mixed;
                temp = pure_func(temp);
                temp = helper_compute(temp % 1024, reg_acc);
                
                checksum += temp;
                
                /* Update loop-carried variable */
                reg_acc ^= temp;
            }
        }
        
        /* Third level nested loop */
        for (k = 0; k < (i % 8) + 1; ++k) {
            int idx1 = (i * 17 + k * 23) % 1024;
            int idx2 = (i * 13 + k * 29) % 1024;
            
            /* Memory operation with potential aliasing */
            int diff = g_array1[idx1] - g_array2[idx2];
            
            /* Complex conditional */
            if (diff > 0 && (i % 3 == 0)) {
                diff = noinline_func1(diff, k);
            } else if (diff < 0 && (k % 2 == 0)) {
                diff = pure_func(diff);
            }
            
            checksum += diff;
            
            /* Another scheduling boundary */
            asm volatile ("" ::: "memory");
        }
    }
    
    return checksum;
}

/* Warm-up function to trigger different compilation paths */
__attribute__((noinline)) void warm_up_computation(void) {
    int temp_array[64];
    int i;
    
    /* Simple warm-up loop */
    for (i = 0; i < 64; ++i) {
        temp_array[i] = i * i - i;
        if (i % 8 == 0) {
            asm volatile ("" ::: "memory");
        }
    }
    
    /* Use the result to prevent optimization */
    g_array1[0] = temp_array[63];
}

/* Initialize data with pseudo-random values */
void init_data(void) {
    int i;
    unsigned lcg = g_volatile_seed;
    
    for (i = 0; i < 1024; ++i) {
        lcg = lcg * 1103515245 + 12345;
        g_array1[i] = (int)(lcg % 1000);
        g_array2[i % 1024] = (int)((lcg >> 16) % 1000);
    }
    
    for (i = 0; i < 2048; ++i) {
        lcg = lcg * 1103515245 + 12345;
        g_short_array[i] = (short)(lcg % 32768);
    }
    
    for (i = 0; i < 512; ++i) {
        lcg = lcg * 1103515245 + 12345;
        g_unsigned_array[i] = lcg;
    }
}

int main(int argc, char *argv[]) {
    /* Use command line arguments for variability */
    int outer_limit = (argc > 1) ? atoi(argv[1]) : 50;
    int inner_limit = (argc > 2) ? atoi(argv[2]) : 100;
    int conditional_limit = (argc > 3) ? atoi(argv[3]) : 25;
    
    if (outer_limit <= 0) outer_limit = 50;
    if (inner_limit <= 0) inner_limit = 100;
    if (conditional_limit <= 0) conditional_limit = 25;
    
    printf("Starting selective scheduling test...\n");
    printf("Parameters: outer=%d, inner=%d, cond=%d\n", 
           outer_limit, inner_limit, conditional_limit);
    
    /* Initialize data */
    init_data();
    
    /* Warm-up to potentially trigger different compilation paths */
    warm_up_computation();
    
    /* Main computation */
    unsigned long long result = core_computation(
        outer_limit, inner_limit, conditional_limit);
    
    printf("Computation result: %llu\n", result);
    
    /* Additional verification computation */
    unsigned long long verify = 0;
    for (int i = 0; i < 100; ++i) {
        verify += g_array1[i % 1024];
    }
    printf("Verification sum: %llu\n", verify);
    
    return 0;
}
