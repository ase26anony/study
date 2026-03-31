/* Selective Scheduling Dump Test Program
 * Designed to trigger dump_insn_rtx_1 in sel-sched-dump.cc
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-dump sel_sched_test.c helper.c -o sel_sched_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* External declarations for helper functions */
extern int __attribute__((noinline)) helper_compute(int a, int b);
extern int __attribute__((const)) pure_helper(int x);
extern void __attribute__((noinline)) memory_barrier(void);

/* Global arrays to create memory dependencies */
volatile int g_volatile_seed = 42;
int g_array1[1024];
int g_array2[1024];
short g_short_array[2048];
unsigned g_unsigned_array[512];

/* Struct to create complex memory access patterns */
struct DataPoint {
    int value;
    unsigned timestamp;
    short tag;
    char padding[2];
};

struct DataPoint g_data[256];

/* Non-inlineable function to force function calls in loops */
int __attribute__((noinline)) complex_calculation(int x, int y) {
    /* Mix of operations to create varied RTL */
    int result = (x * y) + (x >> 3) - (y & 0xFF);
    result ^= (result << 13);
    result ^= (result >> 17);
    result ^= (result << 5);
    return result;
}

/* Pure function with const attribute */
int __attribute__((const)) pure_transform(int val) {
    return (val * 1103515245 + 12345) & 0x7FFFFFFF;
}

/* Warm-up function to trigger compilation paths */
void warm_up_computation(int iterations) {
    int temp = 0;
    for (int i = 0; i < iterations; ++i) {
        /* Mix of operations */
        temp += i * 3;
        temp ^= (temp << 3);
        if (i % 7 == 0) {
            temp -= i;
        }
    }
    /* Use the result to prevent optimization */
    asm volatile ("" : : "r"(temp) : "memory");
}

/* Core computation with nested loops targeting selective scheduling */
unsigned long long core_computation(int outer_limit, int inner_limit, 
                                   int conditional_limit) {
    unsigned long long checksum = 0;
    int register reg_acc = 0;  /* Hint for register allocation */
    int stack_var = 0;
    
    /* Outer loop with varying trip count */
    for (int i = 0; i < outer_limit; ++i) {
        int loop_carried = i;  /* Loop-carried dependency */
        
        /* Conditional inner loop execution */
        if (__builtin_expect(i > conditional_limit, 0)) {
            /* Inner loop with different data type */
            for (unsigned j = 0; j < (unsigned)inner_limit; ++j) {
                /* Mix of arithmetic operations */
                int temp = (int)j * 3 + loop_carried;
                
                /* Memory operations with potential aliasing */
                g_array1[i & 1023] = temp;
                int mem_val = g_array2[j & 1023];
                
                /* Loop-carried dependency */
                reg_acc += mem_val + temp;
                
                /* Conditional branch inside innermost loop */
                if ((j % 13) == 0) {
                    /* Function call with loop-variant arguments */
                    int func_result = complex_calculation(i, (int)j);
                    reg_acc ^= func_result;
                    
                    /* Pure function call */
                    int pure_result = pure_transform(reg_acc);
                    stack_var += pure_result;
                }
                
                /* Memory barrier to create scheduling boundary */
                asm volatile ("" ::: "memory");
                
                /* Access struct array */
                g_data[(i + j) & 255].value = reg_acc;
                g_data[(i + j) & 255].timestamp = j;
                
                /* Use helper function from another translation unit */
                int helper_val = helper_compute(i, (int)j);
                checksum += helper_val;
            }
        } else {
            /* Alternative path with different operations */
            for (short k = 0; k < (short)(inner_limit / 2); ++k) {
                /* Different data type operations */
                short short_val = (short)(i * k);
                g_short_array[(i + k) & 2047] = short_val;
                
                /* More arithmetic with type mixing */
                unsigned unsigned_val = (unsigned)(reg_acc + k);
                g_unsigned_array[k & 511] = unsigned_val;
                
                /* Complex expression with multiple uses of same variable */
                int x = reg_acc;
                x = x * 3 + 1;
                x = x ^ (x >> 7);
                x = x * 5 - 2;
                reg_acc = x;
                
                /* Conditional with __builtin_expect */
                if (__builtin_expect((k & 31) == 0, 1)) {
                    checksum += (unsigned long long)reg_acc * k;
                }
            }
        }
        
        /* Cross-iteration dependency */
        loop_carried = reg_acc - stack_var;
        
        /* Another memory barrier */
        memory_barrier();
        
        /* Update checksum with mixed operations */
        checksum += (unsigned long long)reg_acc * i + stack_var;
        
        /* Reset some variables for next iteration */
        if (i % 5 == 0) {
            stack_var = 0;
        }
    }
    
    return checksum;
}

/* Additional computation with triple nested loops */
int triple_nested_computation(int limit_a, int limit_b, int limit_c) {
    int total = 0;
    
    for (int a = 0; a < limit_a; ++a) {
        int a_acc = a;
        
        for (int b = 0; b < limit_b; ++b) {
            register int b_acc = b;  /* Register hint */
            
            for (int c = 0; c < limit_c; ++c) {
                /* Complex expression tree */
                int val = (a_acc * b_acc + c) & 0xFFF;
                val = (val << 4) | (val >> 8);
                
                /* Memory store with different index calculation */
                g_array1[(a + b + c) & 1023] = val;
                
                /* Load with potential aliasing */
                int loaded = g_array2[(a * b + c) & 1023];
                
                /* Conditional with function call */
                if ((val + loaded) % 11 == 0) {
                    b_acc += complex_calculation(val, loaded);
                } else {
                    b_acc -= pure_transform(loaded);
                }
                
                /* Update total with overflow */
                total += b_acc;
                total &= 0xFFFFFF;
            }
            
            a_acc ^= b_acc;
        }
        
        total += a_acc;
    }
    
    return total;
}

int main(int argc, char *argv[]) {
    /* Initialize with volatile to prevent constant propagation */
    int outer_lim = g_volatile_seed + (argc > 1 ? atoi(argv[1]) : 100);
    int inner_lim = g_volatile_seed * 2 + (argc > 2 ? atoi(argv[2]) : 50);
    int cond_lim = outer_lim / 3;
    
    /* Initialize arrays with pseudo-random values */
    unsigned lcg = 123456789;
    for (int i = 0; i < 1024; ++i) {
        lcg = lcg * 1103515245 + 12345;
        g_array1[i] = (int)(lcg & 0x7FFF);
        g_array2[i] = (int)((lcg >> 16) & 0x7FFF);
    }
    
    for (int i = 0; i < 256; ++i) {
        lcg = lcg * 1103515245 + 12345;
        g_data[i].value = (int)(lcg & 0xFFF);
        g_data[i].timestamp = (unsigned)(lcg >> 12);
        g_data[i].tag = (short)(lcg & 0xFFFF);
    }
    
    printf("Starting selective scheduling test...\n");
    
    /* Warm-up execution */
    warm_up_computation(100);
    
    /* Main computation with nested loops */
    unsigned long long checksum1 = core_computation(outer_lim, inner_lim, cond_lim);
    
    /* Additional computation with triple nesting */
    int result2 = triple_nested_computation(
        outer_lim / 4, 
        inner_lim / 2, 
        cond_lim
    );
    
    /* Final checksum */
    unsigned long long final_checksum = checksum1 + result2;
    
    printf("Computation complete.\n");
    printf("Checksum: %llu\n", final_checksum);
    printf("Array1[0]=%d, Array2[0]=%d\n", g_array1[0], g_array2[0]);
    
    return (final_checksum > 0) ? 0 : 1;
}
