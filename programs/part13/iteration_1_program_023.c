/* Coverage test for GCC selective scheduler dump functionality
 * Specifically targets lines 159-163 in sel-sched-dump.cc
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 -o test test.c
 * Or more aggressively: gcc -O3 -fsel-sched-pipelining-outer-loops -fdump-rtl-all -fdump-rtl-sched_details -o test test.c
 */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable function to create call RTL patterns */
__attribute__((noinline)) 
static int side_effect_func(int x, float y) {
    volatile static int counter = 0;
    counter++;
    return (int)(x * y) + counter;
}

/* Another non-inlineable function for conditional calls */
__attribute__((noinline))
static float fp_side_effect(float a, float b) {
    volatile static float accumulator = 0.0f;
    accumulator += a * b;
    return accumulator;
}

/* Simple LCG for pseudo-random data without external dependencies */
static uint32_t lcg_seed = 123456789;
static uint32_t lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

int main(void) {
    /* Arrays with different data types for diverse RTL patterns */
    int int_array[256];
    float float_array[256];
    
    /* Volatile accumulators to prevent optimization */
    volatile int int_acc = 0;
    volatile float float_acc = 0.0f;
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < 256; i++) {
        int_array[i] = (int)(lcg_rand() % 1000) - 500;
        float_array[i] = (float)(lcg_rand() % 1000) / 100.0f - 5.0f;
    }
    
    /* Memory barrier to prevent pre-scheduling optimization */
    asm volatile("" ::: "memory");
    
    /* Outer loop with small iteration count */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with data dependencies and control flow */
        for (int i = 0; i < 256; i++) {
            /* Create loop-carried dependency */
            int temp_int = int_array[i];
            float temp_float = float_array[i];
            
            /* Mixed integer operations */
            int_acc = int_acc + temp_int * (i + 1);
            
            /* Conditional with multiple basic blocks */
            if (temp_int > 0) {
                /* Integer arithmetic path */
                int_acc = int_acc - (temp_int % 17);
                
                /* Call non-inlineable function conditionally */
                if ((i % 3) == 0) {
                    int_acc += side_effect_func(temp_int, temp_float);
                }
            } else {
                /* Floating-point arithmetic path */
                float_acc = float_acc + temp_float * 2.5f;
                
                /* More complex FP operations */
                if ((i % 4) == 0) {
                    float_acc = float_acc - fp_side_effect(temp_float, float_acc);
                }
            }
            
            /* Cross-type operation to create diverse RTL */
            if ((i % 5) == 0) {
                int_acc += (int)(float_acc * 10.0f);
            }
            
            /* Additional arithmetic to increase scheduling complexity */
            volatile int intermediate = int_acc * 3 - i;
            volatile float fp_intermediate = float_acc / (temp_float + 1.0f);
            
            /* Use intermediate values to prevent dead code elimination */
            if (intermediate > 1000000) {
                intermediate = intermediate % 1000;
            }
            
            /* Memory barrier inside loop to constrain scheduling */
            asm volatile("" ::: "memory");
        }
        
        /* Loop interchange opportunity for scheduler */
        for (int j = 0; j < 10; j++) {
            float_acc = float_acc * 0.99f;
            int_acc = int_acc - j;
        }
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
    
    /* Print checksum to ensure side effects are observable */
    printf("Final checksum: int=%d, float=%f\n", int_acc, float_acc);
    
    return 0;
}
