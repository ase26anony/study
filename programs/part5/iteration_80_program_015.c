/* Selective Scheduling Dump Test Program
 * Designed to trigger dump_insn_rtx_1 in sel-sched-dump.cc
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Global arrays with different types to create varied RTL */
volatile int g_volatile_seed = 42;
int g_array_int[1024];
unsigned int g_array_uint[1024];
short g_array_short[2048];
float g_array_float[1024];

/* Non-inlineable functions to create scheduling boundaries */
__attribute__((noinline)) int noinline_multiply(int a, int b) {
    return a * b;
}

__attribute__((noinline)) unsigned int noinline_rotate(unsigned int x, int shift) {
    return (x << shift) | (x >> (32 - shift));
}

/* Pure function for loop-invariant but variant-argument calls */
__attribute__((const)) int pure_compute(int x, int y) {
    return (x * x + y * y) % 256;
}

/* Another non-inlineable function with side effects */
__attribute__((noinline)) void noinline_side_effect(int* ptr, int value) {
    *ptr += value;
    asm volatile("" ::: "memory");  /* Memory barrier */
}

/* Helper to initialize arrays with pseudo-random values */
void initialize_arrays(void) {
    unsigned int lcg = g_volatile_seed;
    for (int i = 0; i < 1024; i++) {
        lcg = lcg * 1103515245 + 12345;
        g_array_int[i] = (int)(lcg % 1000);
        g_array_uint[i] = lcg;
        g_array_float[i] = (float)(lcg % 1000) * 0.1f;
    }
    for (int i = 0; i < 2048; i++) {
        lcg = lcg * 1103515245 + 12345;
        g_array_short[i] = (short)(lcg % 1000);
    }
}

/* Core computation with nested loops designed for selective scheduling */
unsigned long long compute_checksum(int outer_limit, int inner_limit, int threshold) {
    unsigned long long checksum = 0;
    int loop_carried_acc = 0;
    register int reg_var1 = 0;  /* Hint for register allocation */
    int reg_var2 = 0;
    
    /* Outer loop with varying trip count */
    for (int i = 0; i < outer_limit; ++i) {
        int outer_mod = i % 7;
        
        /* Conditional inner loop execution */
        if (__builtin_expect(i > threshold, 0)) {
            /* Inner loop with different counter type */
            for (unsigned int j = 0; j < (unsigned int)inner_limit; ++j) {
                /* Mix of arithmetic operations */
                int temp1 = g_array_int[i] + g_array_short[j];
                unsigned int temp2 = g_array_uint[j] ^ (unsigned int)i;
                
                /* Loop-carried dependency */
                loop_carried_acc = temp1 + loop_carried_acc;
                
                /* Conditional branch inside innermost loop */
                if (j % 5 == 0) {
                    /* Call to non-inlineable function */
                    temp2 = noinline_rotate(temp2, outer_mod);
                    
                    /* Memory operation with potential aliasing */
                    g_array_int[(i + j) % 1024] += temp2 % 100;
                }
                
                /* Use of pure function with loop-variant arguments */
                int pure_result = pure_compute(i, j);
                
                /* Another conditional with different modulus */
                if (i % 3 == 0) {
                    /* Call to non-inlineable function with side effects */
                    noinline_side_effect(&reg_var1, pure_result);
                    
                    /* Optimization barrier */
                    asm volatile("" ::: "memory");
                }
                
                /* More arithmetic with different data types */
                short short_op = g_array_short[(i * 2 + j) % 2048];
                int mixed_op = (int)short_op * reg_var1;
                
                /* Update checksum with various operations */
                checksum += (unsigned long long)(temp1 ^ temp2);
                checksum += (unsigned long long)(mixed_op % 256);
                
                /* Register variable usage */
                reg_var2 = noinline_multiply(reg_var1, outer_mod);
                reg_var1 = (reg_var1 + 1) % 256;
            }
        } else {
            /* Different path with simpler loop */
            for (short k = 0; k < (short)(inner_limit / 2); ++k) {
                /* Different type of computation */
                float fval = g_array_float[i] * (float)k;
                int ival = (int)fval;
                
                /* Memory access pattern */
                g_array_uint[k % 1024] += (unsigned int)ival;
                
                checksum += (unsigned long long)ival;
                
                /* Another optimization barrier */
                asm volatile("" ::: "memory");
            }
        }
        
        /* Additional computation between outer loop iterations */
        if (i % 11 == 0) {
            /* Complex expression with multiple operations */
            int complex = (g_array_int[i] * reg_var2) / (loop_carried_acc + 1);
            checksum += (unsigned long long)complex;
            
            /* Call to non-inlineable function */
            reg_var2 = noinline_multiply(complex, i);
        }
    }
    
    return checksum;
}

/* Warm-up function to trigger different compilation paths */
__attribute__((noinline)) unsigned long long warmup_computation(int iterations) {
    unsigned long long warmup_sum = 0;
    volatile int warmup_counter = iterations;
    
    /* Simple warm-up loop */
    while (warmup_counter-- > 0) {
        int temp = warmup_counter * 3;
        warmup_sum += (unsigned long long)temp;
        
        /* Small amount of computation */
        for (int i = 0; i < 10; i++) {
            warmup_sum += (unsigned long long)(temp % (i + 1));
        }
    }
    
    return warmup_sum;
}

int main(int argc, char* argv[]) {
    /* Use command line arguments to make loop bounds non-constant */
    int outer_loop_limit = (argc > 1) ? atoi(argv[1]) : 100;
    int inner_loop_limit = (argc > 2) ? atoi(argv[2]) : 50;
    int threshold = (argc > 3) ? atoi(argv[3]) : 30;
    
    if (outer_loop_limit <= 0) outer_loop_limit = 100;
    if (inner_loop_limit <= 0) inner_loop_limit = 50;
    if (threshold < 0) threshold = 30;
    
    /* Initialize data */
    initialize_arrays();
    
    printf("Starting selective scheduling test...\n");
    printf("Parameters: outer=%d, inner=%d, threshold=%d\n", 
           outer_loop_limit, inner_loop_limit, threshold);
    
    /* Warm-up execution (executed once) */
    printf("Warm-up...\n");
    unsigned long long warmup_result = warmup_computation(100);
    printf("Warm-up result: %llu\n", warmup_result);
    
    /* Main computation with nested loops */
    printf("Main computation...\n");
    unsigned long long checksum = compute_checksum(
        outer_loop_limit, inner_loop_limit, threshold);
    
    printf("Final checksum: %llu\n", checksum);
    
    /* Additional verification computation */
    unsigned long long verify_sum = 0;
    for (int i = 0; i < 100; i++) {
        verify_sum += g_array_int[i % 1024];
        verify_sum += g_array_uint[i % 1024];
    }
    printf("Verification sum: %llu\n", verify_sum);
    
    return 0;
}
