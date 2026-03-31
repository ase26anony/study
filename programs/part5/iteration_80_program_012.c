/* sel-sched-trigger.c - Program to trigger selective scheduling dump logic */

#include <stdio.h>
#include <stdlib.h>

/* Global arrays for memory operations */
extern int global_arr[1024];
extern short global_short_arr[2048];
static unsigned int static_arr[512];

/* Non-inlineable functions */
__attribute__((noinline)) int non_inline_func(int x, int y) {
    return x * y + (x ^ y);
}

__attribute__((noinline, const)) int pure_func(int a, int b) {
    return (a * a + b * b) >> 1;
}

__attribute__((noinline)) void memory_barrier(void) {
    asm volatile("" ::: "memory");
}

/* Helper function with loop-carried dependency */
__attribute__((noinline)) int compute_checksum(int *arr, int n) {
    register int acc = 0;
    for (int i = 0; i < n; ++i) {
        acc = arr[i] + acc;
        if (__builtin_expect((acc & 0xFF) == 0, 0)) {
            acc ^= 0xABCD;
        }
    }
    return acc;
}

/* Complex nested loop structure */
unsigned long long complex_loops(int outer_limit, int inner_limit, 
                                 volatile int *control) {
    unsigned long long total = 0;
    unsigned short us_counter;
    int i, j, k;
    
    /* Outer loop with volatile dependency */
    for (i = 0; i < outer_limit + *control; ++i) {
        int loop_acc = 0;
        
        /* First inner loop with arithmetic mix */
        for (j = 0; j < inner_limit; ++j) {
            int temp = i * j + (i ^ j);
            global_arr[j % 1024] = temp;
            
            /* Conditional function call */
            if (i % 8 == 0) {
                temp = non_inline_func(temp, j);
                memory_barrier();
            }
            
            /* Pure function call with loop-variant args */
            temp += pure_func(i, j);
            
            /* Different data type computation */
            us_counter = (unsigned short)(temp & 0xFFFF);
            global_short_arr[j % 2048] = us_counter;
            
            loop_acc += temp;
            
            /* Scheduling barrier */
            asm volatile("" ::: "memory");
        }
        
        /* Second conditional inner loop */
        if (i > outer_limit / 2) {
            register int reg_var = loop_acc;
            for (k = 0; k < (inner_limit / 2); ++k) {
                /* Memory operation with potential aliasing */
                static_arr[k % 512] = reg_var + global_arr[k % 1024];
                
                /* Complex expression with multiple uses */
                reg_var = (reg_var * 3 + k) >> 1;
                reg_var ^= pure_func(k, i);
                
                /* Branch with prediction hint */
                if (__builtin_expect((reg_var & 0xF) == 0, 1)) {
                    reg_var += non_inline_func(k, i);
                }
            }
            loop_acc = reg_var;
        }
        
        total += loop_acc;
        
        /* Another memory barrier */
        memory_barrier();
    }
    
    return total;
}

/* Warm-up function */
void warmup_computation(void) {
    volatile int warmup_ctrl = 1;
    int dummy_arr[16] = {0};
    
    for (int w = 0; w < 1; ++w) {  /* Execute once */
        int warmup_acc = 0;
        for (int x = 0; x < 16; ++x) {
            for (int y = 0; y < 8; ++y) {
                warmup_acc += dummy_arr[x] * y + warmup_ctrl;
                dummy_arr[x] = warmup_acc & 0xFF;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    /* Initialize with pseudo-random values */
    unsigned int seed = 42;
    for (int i = 0; i < 1024; ++i) {
        global_arr[i] = (seed = seed * 1103515245 + 12345) & 0x7FFF;
    }
    for (int i = 0; i < 2048; ++i) {
        global_short_arr[i] = (seed = seed * 1103515245 + 12345) & 0xFFFF;
    }
    for (int i = 0; i < 512; ++i) {
        static_arr[i] = (seed = seed * 1103515245 + 12345);
    }
    
    /* Volatile control variable */
    volatile int control_var = (argc > 1) ? atoi(argv[1]) : 10;
    if (control_var < 5) control_var = 5;
    if (control_var > 100) control_var = 100;
    
    /* Warm up */
    warmup_computation();
    
    /* Main computation with nested loops */
    int outer_lim = control_var;
    int inner_lim = control_var * 2;
    
    unsigned long long result = complex_loops(outer_lim, inner_lim, &control_var);
    
    /* Additional checksum computation */
    int checksum = compute_checksum(global_arr, 256);
    result += checksum;
    
    printf("Result: %llu\n", result);
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
