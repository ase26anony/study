/* Coverage test for GCC selective scheduler dump functionality */
/* Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 -o test_sel_sched test_sel_sched.c */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable function to create call RTL patterns */
__attribute__((noinline)) 
static void side_effect_func(int val, float fval) {
    /* Use asm to prevent optimization */
    asm volatile("" : : "r"(val), "r"(*(int*)&fval) : "memory");
}

/* Another noinline function for conditional calls */
__attribute__((noinline))
static int compute_mask(int x) {
    return (x & 0xFF) | ((x >> 8) & 0xFF00);
}

int main(void) {
    /* Initialize arrays with pseudo-random data */
    const int SIZE = 256;
    int int_array[SIZE];
    float float_array[SIZE];
    
    /* Simple LCG for deterministic pseudo-random values */
    uint32_t seed = 123456789;
    for (int i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        int_array[i] = (int)(seed % 1000);
        float_array[i] = (float)(seed % 1000) * 0.001f;
    }
    
    /* Volatile accumulators to prevent optimization */
    volatile int int_acc = 0;
    volatile float float_acc = 0.0f;
    volatile int temp_store = 0;
    
    /* Memory barrier before loop */
    asm volatile("" ::: "memory");
    
    /* Outer loop with small iteration count */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with data dependencies */
        for (int i = 0; i < SIZE; i++) {
            /* Create loop-carried dependency */
            int_acc = int_acc + int_array[i] * (int_acc % 100 + 1);
            
            /* Mixed integer operations */
            int mask = compute_mask(int_acc);
            temp_store = int_array[i] & mask;
            
            /* Floating-point operations */
            float_acc = float_acc + float_array[i] * (float_acc + 1.0f);
            
            /* Control flow creating multiple basic blocks */
            if (i % 3 == 0) {
                /* Branch 1: More integer ops */
                int_acc = int_acc ^ temp_store;
                side_effect_func(int_acc, float_acc);
            } else if (i % 3 == 1) {
                /* Branch 2: Mixed ops */
                float_acc = float_acc * 0.99f;
                int_acc = int_acc - (int)(float_acc * 10.0f);
            } else {
                /* Branch 3: Function call with both types */
                side_effect_func(temp_store, float_acc);
                int_acc = int_acc + outer * 7;
            }
            
            /* Additional arithmetic to increase scheduling complexity */
            if (i % 8 == 0) {
                float_acc = float_acc / 1.01f;
                asm volatile("" ::: "memory"); /* Compiler barrier */
            }
            
            /* Array access with stride pattern */
            if (i + 1 < SIZE) {
                int_array[i + 1] += int_array[i] >> 2;
            }
        }
        
        /* Cross-iteration dependency */
        float_acc = float_acc + (float)outer * 0.5f;
        asm volatile("" ::: "memory");
    }
    
    /* Memory barrier after loop */
    asm volatile("" ::: "memory");
    
    /* Final computation with observable side effect */
    int final_result = (int)(int_acc + float_acc * 100.0f);
    
    /* Print checksum to prevent dead code elimination */
    printf("Result checksum: %d\n", final_result);
    
    return final_result & 0xFF;
}
