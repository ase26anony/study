/* sel_sched_test.c - Test program for selective scheduling dump coverage */
#include <stdio.h>
#include <stdlib.h>

/* Global arrays to create memory dependencies */
extern int global_array[1024];
extern short global_short_array[2048];
static unsigned int static_array[512];

/* Volatile variables to prevent optimization */
volatile int g_volatile_seed = 42;
volatile int g_loop_control = 100;

/* Non-inlineable functions */
__attribute__((noinline)) int non_inline_func(int x, int y) {
    return (x * y) ^ (x + y);
}

__attribute__((noinline)) unsigned int non_inline_mem_access(int idx) {
    return global_array[idx & 1023] + idx;
}

/* Pure function for scheduling complexity */
__attribute__((const)) int pure_func(int a, int b) {
    return (a * a) + (b * b) - (a * b);
}

/* Helper with side effects */
__attribute__((noinline)) void side_effect_helper(int* ptr, int val) {
    *ptr += val;
    asm volatile ("" ::: "memory");
}

/* Secondary computation function */
void compute_checksum(int N, int M, int K, unsigned int* checksum) {
    int i, j;
    unsigned short us_counter;
    int acc_int = 0;
    unsigned int acc_unsigned = 0;
    register int reg_acc1 __asm__("r12") = 0;
    register int reg_acc2 __asm__("r13") = 0;
    
    /* Outer loop with varying data types */
    for (i = 0; i < N; ++i) {
        int inner_limit = M;
        
        /* Conditional inner loop execution */
        if (__builtin_expect(i > K, 0)) {
            inner_limit = M / 2;
        }
        
        /* First inner loop - integer operations */
        for (j = 0; j < inner_limit; ++j) {
            int temp = i * j;
            
            /* Loop-carried dependency */
            acc_int = acc_int + temp;
            
            /* Memory operation with potential aliasing */
            global_array[(i + j) & 1023] = temp;
            
            /* Conditional branch creating scheduling boundary */
            if (i % 7 == 0) {
                acc_int += non_inline_func(i, j);
                asm volatile ("" ::: "memory");
            }
            
            /* Pure function call */
            reg_acc1 += pure_func(i, j);
            
            /* Another conditional with different modulus */
            if (j % 5 == 0) {
                side_effect_helper(&acc_int, j);
            }
        }
        
        /* Second inner loop - unsigned/short operations */
        for (us_counter = 0; us_counter < (unsigned short)(M / 3); ++us_counter) {
            unsigned int idx = (i * 31 + us_counter) & 511;
            
            /* Different data type operations */
            acc_unsigned += static_array[idx];
            static_array[idx] = i + us_counter;
            
            /* Non-inline function with memory access */
            if (us_counter % 11 == 0) {
                acc_unsigned += non_inline_mem_access(i);
            }
            
            /* Register variable usage */
            reg_acc2 = reg_acc2 ^ (i << (us_counter & 3));
            
            /* Optimization barrier */
            asm volatile ("" ::: "memory");
        }
        
        /* Complex conditional with nested loops */
        if (i % 13 == 0) {
            int k;
            short s_counter;
            
            /* Nested loop with different counter type */
            for (s_counter = 0; s_counter < 8; ++s_counter) {
                for (k = 0; k < 4; ++k) {
                    int idx = (i + s_counter + k) & 1023;
                    global_short_array[idx] = (short)(acc_int + acc_unsigned);
                    acc_int -= k;
                }
                
                /* Function call inside innermost loop */
                if (s_counter % 3 == 0) {
                    non_inline_func(s_counter, i);
                }
            }
        }
    }
    
    /* Combine all accumulators */
    *checksum = acc_int + acc_unsigned + reg_acc1 + reg_acc2;
}

/* Warm-up function */
__attribute__((noinline)) void warm_up_computation(void) {
    int i;
    volatile int warm_acc = 0;
    
    /* Simple warm-up loop */
    for (i = 0; i < 100; ++i) {
        warm_acc += i * i;
        if (i % 17 == 0) {
            asm volatile ("" ::: "memory");
            warm_acc += non_inline_func(i, 2);
        }
    }
    
    /* Prevent unused variable warning */
    (void)warm_acc;
}

/* Main computation with nested loops */
unsigned int perform_main_computation(int outer_iter, int inner_iter, int threshold) {
    unsigned int checksum = 0;
    int loop_var;
    
    /* Triple nested loop structure */
    for (loop_var = 0; loop_var < outer_iter; ++loop_var) {
        int mid_var;
        int adjusted_inner = inner_iter;
        
        /* Adjust loop bounds based on outer index */
        if (loop_var > threshold) {
            adjusted_inner = inner_iter / 2;
        }
        
        for (mid_var = 0; mid_var < adjusted_inner; ++mid_var) {
            int inner_var;
            int temp_acc = 0;
            
            /* Innermost loop with mixed operations */
            for (inner_var = 0; inner_var < 16; ++inner_var) {
                /* Multiple operations creating scheduling dependencies */
                int val1 = loop_var * mid_var + inner_var;
                int val2 = pure_func(loop_var, inner_var);
                
                temp_acc += val1 * val2;
                
                /* Conditional store */
                if ((loop_var + mid_var + inner_var) % 19 == 0) {
                    global_array[(val1 + val2) & 1023] = temp_acc;
                    asm volatile ("" ::: "memory");
                }
                
                /* Function call with side effects */
                if (inner_var % 7 == 0) {
                    side_effect_helper(&temp_acc, inner_var);
                }
            }
            
            /* Accumulate results */
            checksum += temp_acc;
            
            /* Memory barrier */
            asm volatile ("" ::: "memory");
        }
        
        /* Additional computation between loop levels */
        if (loop_var % 23 == 0) {
            checksum ^= non_inline_mem_access(loop_var);
        }
    }
    
    return checksum;
}

/* Initialize data */
void initialize_arrays(void) {
    int i;
    unsigned int lcg = g_volatile_seed;
    
    for (i = 0; i < 1024; ++i) {
        lcg = lcg * 1103515245 + 12345;
        global_array[i] = (int)(lcg & 0x7FFF);
    }
    
    for (i = 0; i < 2048; ++i) {
        lcg = lcg * 1103515245 + 12345;
        global_short_array[i] = (short)(lcg & 0x7FFF);
    }
    
    for (i = 0; i < 512; ++i) {
        lcg = lcg * 1103515245 + 12345;
        static_array[i] = lcg;
    }
}

int main(int argc, char* argv[]) {
    unsigned int checksum1 = 0, checksum2 = 0;
    int outer_loops, inner_loops, threshold;
    
    /* Use command line arguments or defaults */
    if (argc > 3) {
        outer_loops = atoi(argv[1]);
        inner_loops = atoi(argv[2]);
        threshold = atoi(argv[3]);
    } else {
        outer_loops = 50;
        inner_loops = 100;
        threshold = 25;
    }
    
    /* Initialize data */
    initialize_arrays();
    
    /* Warm-up computation */
    warm_up_computation();
    
    /* First computation pass */
    compute_checksum(outer_loops, inner_loops, threshold, &checksum1);
    
    /* Second computation pass with different parameters */
    checksum2 = perform_main_computation(outer_loops / 2, inner_loops * 2, threshold / 2);
    
    /* Combine and output results */
    printf("Checksum 1: %u\n", checksum1);
    printf("Checksum 2: %u\n", checksum2);
    printf("Final result: %u\n", checksum1 + checksum2);
    
    return 0;
}
