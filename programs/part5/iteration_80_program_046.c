/* Selective Scheduling Dump Test Program
 * Designed to trigger dump_insn_rtx_1 in sel-sched-dump.cc
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-dump sel_sched_test.c helper.c -o sel_sched_test
 */

#include <stdio.h>
#include <stdlib.h>

/* External declarations for multi-file compilation stress */
extern int helper_compute(int x, int y) __attribute__((noinline));
extern void helper_init(int *arr, int size);
extern volatile int g_volatile_seed;

/* Global arrays for memory operations with potential aliasing */
int global_arr1[1024];
int global_arr2[1024];
short global_short_arr[2048];
unsigned global_checksum = 0;

/* Non-inlineable functions to create scheduling boundaries */
int __attribute__((noinline)) compute_mul(int a, int b) {
    return a * b;
}

int __attribute__((noinline)) compute_div(int a, int b) {
    return b != 0 ? a / (b + 1) : a;
}

/* Pure function for varied RTL patterns */
int __attribute__((const)) pure_transform(int x) {
    return (x * 3 + 7) & 0xFF;
}

/* Function with loop-carried dependencies and mixed operations */
unsigned __attribute__((noinline)) 
complex_nested_loops(int outer_limit, int inner_limit, int threshold) {
    register int i, j, k;
    unsigned acc = 0;
    int temp_arr[64];
    volatile int vol_counter = 0;
    
    /* Initialize temp array */
    for (k = 0; k < 64; ++k) {
        temp_arr[k] = k * 2 + g_volatile_seed;
    }
    
    /* Outer loop with varying data types */
    for (i = 0; i < outer_limit; ++i) {
        short s_counter = (short)i;
        unsigned u_acc = 0;
        
        /* Memory barrier to create scheduling boundary */
        asm volatile ("" ::: "memory");
        
        /* First inner loop with arithmetic operations */
        for (j = 0; j < inner_limit; ++j) {
            /* Loop-carried dependency */
            acc = global_arr1[(i * 16 + j) & 1023] + acc;
            
            /* Mixed data type operations */
            u_acc += (unsigned)s_counter * (unsigned)j;
            
            /* Conditional branch with __builtin_expect */
            if (__builtin_expect((i * j) % 7 == 0, 0)) {
                /* Function call inside condition */
                acc += helper_compute(i, j);
            }
            
            /* Memory operation with potential aliasing */
            global_arr2[j] = global_arr1[i] + temp_arr[j & 63];
        }
        
        /* Second conditional inner loop */
        if (i > threshold) {
            int k;
            register int reg_sum = 0;
            
            /* Different loop counter type */
            for (k = inner_limit - 1; k >= 0; --k) {
                /* Multiple uses of same variable */
                reg_sum = reg_sum + global_short_arr[k * 2];
                reg_sum = pure_transform(reg_sum);
                
                /* Another memory barrier */
                asm volatile ("" ::: "memory");
                
                /* Complex expression with function calls */
                int val = compute_mul(reg_sum, k);
                if (k % 3 == 0) {
                    val = compute_div(val, i + 1);
                }
                acc += val;
                
                /* Volatile access affects scheduling */
                vol_counter++;
            }
            
            /* Store result with different scope variable */
            int scope_var = reg_sum % 256;
            global_checksum += scope_var;
        }
        
        /* Conditional execution based on outer loop */
        if (i % 4 == 0) {
            /* Innermost nested loop */
            for (j = 0; j < 8; ++j) {
                /* Mixed operations to generate varied RTL */
                double d_temp = (double)acc / (j + 1.0);
                int int_part = (int)d_temp;
                acc += int_part + (i & 0xFF);
                
                /* Access different global arrays */
                global_short_arr[(i * 8 + j) & 2047] = (short)(acc & 0xFFFF);
            }
        }
    }
    
    return acc;
}

/* Warm-up function to trigger different compilation paths */
void __attribute__((noinline)) warm_up_computation(void) {
    int i, j;
    unsigned warm_sum = 0;
    
    /* Simple warm-up loop */
    for (i = 0; i < 100; ++i) {
        for (j = 0; j < 50; ++j) {
            warm_sum += i * j + (i ^ j);
        }
    }
    
    /* Ensure warm-up isn't optimized away */
    if (warm_sum == 0) {
        printf("Warm-up completed\n");
    }
}

/* Main computation with configurable parameters */
unsigned __attribute__((noinline)) 
main_computation(int param1, int param2, int param3) {
    unsigned result = 0;
    
    /* Triple nested loops with different characteristics */
    for (int outer = 0; outer < param1; ++outer) {
        unsigned short us_outer = (unsigned short)outer;
        
        for (int middle = 0; middle < param2; ++middle) {
            /* Create register pressure */
            register int r1 = outer * middle;
            register int r2 = middle + us_outer;
            register int r3 = r1 ^ r2;
            
            /* Complex loop body with scheduling opportunities */
            for (int inner = 0; inner < param3; ++inner) {
                /* Multiple interdependent operations */
                int t1 = r1 + inner;
                int t2 = r2 - inner;
                int t3 = r3 * inner;
                
                /* Conditional with probability hint */
                if (__builtin_expect((t1 + t2 + t3) % 11 == 0, 1)) {
                    result += compute_mul(t1, t2);
                    result += global_arr1[(t3 + inner) & 1023];
                } else {
                    result += compute_div(t2, t1 + 1);
                    result += global_arr2[(t1 + outer) & 1023];
                }
                
                /* Update register variables */
                r1 = (r1 + 1) & 0xFF;
                r2 = (r2 * 3) & 0xFF;
                r3 = r3 ^ inner;
                
                /* Memory barrier every 8 iterations */
                if ((inner & 7) == 0) {
                    asm volatile ("" ::: "memory");
                }
            }
            
            /* Function call with loop-variant arguments */
            result += helper_compute(r1, r2);
        }
        
        /* Conditional inner loop block */
        if (outer % 3 == 0) {
            for (int k = 0; k < 16; ++k) {
                result += pure_transform(k + outer);
                global_short_arr[(outer * 16 + k) & 2047] = (short)(result & 0xFFFF);
            }
        }
    }
    
    return result;
}

/* Initialize data with pseudo-random values */
void init_data(void) {
    unsigned lcg = 123456789;
    
    for (int i = 0; i < 1024; ++i) {
        lcg = lcg * 1103515245 + 12345;
        global_arr1[i] = (lcg >> 16) & 0x7FFF;
        global_arr2[i] = (lcg >> 8) & 0xFF;
    }
    
    for (int i = 0; i < 2048; ++i) {
        lcg = lcg * 1103515245 + 12345;
        global_short_arr[i] = (short)(lcg & 0xFFFF);
    }
    
    /* Initialize volatile seed from environment */
    g_volatile_seed = 42;
}

int main(int argc, char *argv[]) {
    /* Use command line arguments for variability */
    int param1 = argc > 1 ? atoi(argv[1]) : 50;
    int param2 = argc > 2 ? atoi(argv[2]) : 40;
    int param3 = argc > 3 ? atoi(argv[3]) : 30;
    int threshold = argc > 4 ? atoi(argv[4]) : 20;
    
    /* Initialize data */
    init_data();
    
    /* Warm-up to trigger compilation paths */
    warm_up_computation();
    
    /* Main computation 1: Complex nested loops */
    unsigned result1 = complex_nested_loops(param1, param2, threshold);
    
    /* Main computation 2: Triple nested loops */
    unsigned result2 = main_computation(param1 / 2, param2 / 2, param3);
    
    /* Final checksum */
    unsigned final_checksum = result1 + result2 + global_checksum;
    
    printf("Result 1: %u\n", result1);
    printf("Result 2: %u\n", result2);
    printf("Global checksum: %u\n", global_checksum);
    printf("Final checksum: %u\n", final_checksum);
    
    return (final_checksum > 0) ? 0 : 1;
}
