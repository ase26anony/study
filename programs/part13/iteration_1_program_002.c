/* sel-sched-coverage.c
 * Program designed to trigger GCC's selective scheduler dump logic
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 -o test test.c
 * Or more aggressively: gcc -O3 -fsel-sched-pipelining-outer-loops -fdump-rtl-all -fdump-rtl-sched_details -o test test.c
 */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable function to create call RTL patterns */
__attribute__((noinline)) 
int side_effect_func(int x, float y) {
    volatile static int counter = 0;
    counter += x + (int)y;
    return counter;
}

/* Another non-inlineable function for conditional calls */
__attribute__((noinline))
float fp_side_effect(float a, float b) {
    volatile static float accum = 0.0f;
    accum += a * b;
    return accum;
}

/* Simple PRNG without external dependencies */
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
        int_array[i] = (int)(lcg_rand() % 1000);
        float_array[i] = (float)(lcg_rand() % 1000) / 100.0f;
    }
    
    /* Memory barrier to prevent pre-scheduling optimization */
    asm volatile("" ::: "memory");
    
    /* Outer loop with small iteration count */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with data dependencies */
        for (int i = 0; i < 256; i++) {
            /* Loop-carried dependency for scheduling challenge */
            int_acc += int_array[i] * int_acc;
            
            /* Mixed integer operations */
            int temp_int = int_array[i] * 3 + 7;
            
            /* Floating-point operations */
            float temp_float = float_array[i] * 2.5f + 1.0f;
            float_acc += temp_float;
            
            /* Control flow for multi-basic-block scheduling */
            if (i % 3 == 0) {
                /* Conditional function call */
                int_acc += side_effect_func(temp_int, temp_float);
                
                /* More floating-point operations in branch */
                float_acc = fp_side_effect(float_acc, float_array[i]);
            } else if (i % 5 == 0) {
                /* Different operations in else-if branch */
                float_acc = float_acc * 0.9f - float_array[i];
                int_acc -= temp_int / 2;
            } else {
                /* Default path with arithmetic mix */
                int_acc = int_acc ^ (temp_int << 2);
                float_acc = float_acc + (float_array[i] / 3.0f);
            }
            
            /* Volatile store to prevent elimination */
            volatile int store_sink = temp_int;
            volatile float float_sink = temp_float;
            (void)store_sink;  /* Suppress unused warning */
            (void)float_sink;
            
            /* Memory barrier in loop body */
            asm volatile("" ::: "memory");
        }
        
        /* Cross-iteration dependency */
        int_array[outer] = int_acc % 1000;
        float_array[outer] = float_acc / 100.0f;
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
    
    /* Print checksum to ensure side effects */
    printf("Final checksum: int=%d, float=%f\n", int_acc, float_acc);
    
    return 0;
}
