/* Selective Scheduling Dump Test Program
 * Designed to trigger dump_insn_rtx_1 in sel-sched-dump.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Global arrays to create memory dependencies */
extern int global_array[1024];
extern short global_short_array[2048];
static unsigned int static_array[512];

/* Non-inlineable functions to create scheduling boundaries */
__attribute__((noinline)) int noinline_func(int x, int y) {
    return x * y + (x ^ y);
}

__attribute__((noinline, const)) int pure_func(int a, int b) {
    return (a * a + b * b) >> 1;
}

__attribute__((noinline)) void memory_barrier(void) {
    asm volatile("" ::: "memory");
}

/* Helper with mixed operations */
__attribute__((noinline)) int mixed_ops(int a, int b, int c) {
    int result = 0;
    if (__builtin_expect((a & 1) == 0, 1)) {
        result = a + b * c;
    } else {
        result = a - b / (c ? c : 1);
    }
    return result;
}

/* Core computation with nested loops */
__attribute__((noinline)) 
unsigned long long compute_checksum(int outer_limit, int inner_limit, 
                                   volatile int* control) {
    unsigned long long checksum = 0;
    register int reg_acc = 0;  /* Hint for register allocation */
    int stack_acc = 0;
    
    /* Outer loop with varying trip count */
    for (int i = 0; i < outer_limit; ++i) {
        int outer_mod = i % 8;
        
        /* Conditional inner loop execution */
        if (__builtin_expect(i > (*control) >> 2, 0)) {
            /* First inner loop with short counter */
            for (unsigned short j = 0; j < (inner_limit & 0xFFFF); ++j) {
                int temp = pure_func(i, j);
                
                /* Loop-carried dependency */
                reg_acc += temp;
                
                /* Memory operation with potential aliasing */
                global_array[(i * 16 + j) & 1023] = temp;
                
                /* Scheduling barrier */
                if (j % 4 == 0) {
                    memory_barrier();
                }
                
                /* Mixed data type computation */
                checksum += (unsigned long long)reg_acc * 
                           (unsigned)global_short_array[j & 2047];
            }
        }
        
        /* Second inner loop with different characteristics */
        if (outer_mod == 0 || outer_mod == 3) {
            for (int k = 0; k < (inner_limit >> 1); ++k) {
                /* Complex expression with function call */
                int val = noinline_func(i, k) + mixed_ops(i, k, reg_acc);
                
                /* Multiple uses of same variable */
                stack_acc = val;
                global_short_array[k & 2047] = (short)(stack_acc & 0xFFFF);
                stack_acc = stack_acc * 3 - val / 2;
                
                /* Conditional with unpredictable branch */
                if (__builtin_expect((i * k) & 0xFF, 1)) {
                    checksum += (unsigned)val * (unsigned)stack_acc;
                } else {
                    checksum += (unsigned)val | (unsigned)stack_acc;
                }
                
                /* Another scheduling barrier */
                asm volatile("" ::: "memory");
            }
        }
        
        /* Third nested loop level */
        for (int m = 0; m < 4; ++m) {
            int inner_acc = 0;
            for (int n = 0; n < 8; ++n) {
                /* Access static array */
                inner_acc += static_array[(i + m + n) & 511];
                
                /* Complex conditional */
                if ((i + m * n) % 5 == 0) {
                    inner_acc = inner_acc * 7 + 1;
                }
            }
            checksum += inner_acc;
        }
    }
    
    return checksum + reg_acc + stack_acc;
}

/* Warm-up function */
__attribute__((noinline)) 
void warm_up_computation(int iterations) {
    int dummy = 0;
    for (int i = 0; i < iterations; ++i) {
        /* Simple warm-up loop */
        dummy += i * i - i;
        if (i % 7 == 0) {
            dummy = dummy >> 1;
        }
    }
    /* Use dummy to prevent optimization */
    global_array[0] = dummy & 1;
}

/* Initialize arrays with pseudo-random values */
void initialize_arrays(void) {
    unsigned int lcg = 123456789;
    
    for (int i = 0; i < 1024; ++i) {
        lcg = lcg * 1103515245 + 12345;
        global_array[i] = (int)(lcg >> 16) & 0x7FFF;
    }
    
    for (int i = 0; i < 2048; ++i) {
        lcg = lcg * 1103515245 + 12345;
        global_short_array[i] = (short)(lcg & 0xFFFF);
    }
    
    for (int i = 0; i < 512; ++i) {
        lcg = lcg * 1103515245 + 12345;
        static_array[i] = lcg;
    }
}

int main(int argc, char* argv[]) {
    /* Use arguments for variability */
    int outer_limit = argc > 1 ? atoi(argv[1]) : 100;
    int inner_limit = argc > 2 ? atoi(argv[2]) : 50;
    volatile int control = argc > 3 ? atoi(argv[3]) : 25;
    
    /* Ensure non-zero limits */
    if (outer_limit <= 0) outer_limit = 100;
    if (inner_limit <= 0) inner_limit = 50;
    
    printf("Starting selective scheduling test...\n");
    printf("Parameters: outer=%d, inner=%d, control=%d\n", 
           outer_limit, inner_limit, control);
    
    /* Initialize data */
    initialize_arrays();
    
    /* Warm-up execution */
    printf("Warm-up phase...\n");
    warm_up_computation(100);
    
    /* Main computation */
    printf("Main computation phase...\n");
    unsigned long long result = compute_checksum(outer_limit, inner_limit, &control);
    
    printf("Result checksum: %llu\n", result);
    printf("Test completed.\n");
    
    return 0;
}
