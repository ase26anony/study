/* sel-sched-coverage.c
 * Program designed to trigger GCC's selective scheduler dump logic
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 -o test test.c
 * Or more aggressively: gcc -O3 -fsel-sched-pipelining-outer-loops -fdump-rtl-all -o test test.c
 */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable function to create call RTL patterns */
__attribute__((noinline)) 
int side_effect_func(int x, float y) {
    static int counter = 0;
    counter++;
    return (x ^ counter) + (int)(y * 100.0f);
}

/* Another noinline function for conditional calls */
__attribute__((noinline))
float fp_side_effect(float a, float b) {
    volatile static float accum = 0.0f;
    accum += a * b;
    return accum;
}

/* Simple LCG for pseudo-random data without external dependencies */
static uint32_t lcg_seed = 123456789;
static uint32_t lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

int main(void) {
    /* Arrays with mixed data types for diverse RTL patterns */
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
        /* Inner loop with data dependencies and control flow */
        for (int i = 0; i < 256; i++) {
            /* Loop-carried dependency to challenge scheduler */
            int_acc = int_acc + int_array[i] * (int_acc % 7);
            
            /* Mixed integer operations */
            int temp = int_array[i] ^ (i * outer);
            
            /* Conditional block for multi-basic-block scheduling */
            if (temp % 3 == 0) {
                /* Integer arithmetic */
                int_acc += temp * 2;
                
                /* Floating-point operations */
                float fval = float_array[i] * 1.5f;
                float_acc = float_acc + fval;
                
                /* Function call with side effects */
                int dummy = side_effect_func(temp, fval);
                int_acc ^= dummy;
            } else if (temp % 5 == 0) {
                /* Different execution path */
                float_acc = float_acc - float_array[i];
                
                /* More floating-point operations */
                float_acc = fp_side_effect(float_acc, float_array[i]);
                
                /* Integer multiplication */
                int_acc = int_acc * (temp & 0xFF);
            } else {
                /* Default path with arithmetic mix */
                int_acc = int_acc | (int_array[i] << 2);
                float_acc = float_acc * 0.99f + float_array[i];
            }
            
            /* Additional data dependency chain */
            if (i > 0) {
                int_array[i] += int_array[i-1] / 2;
                float_array[i] += float_array[i-1] * 0.5f;
            }
            
            /* Memory barrier in loop body */
            asm volatile("" ::: "memory");
        }
        
        /* Cross-iteration dependency */
        int_acc = int_acc ^ outer;
        float_acc = float_acc + (float)outer;
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
    
    /* Print checksum to prevent dead code elimination */
    printf("Final checksum: int=%d, float=%.2f\n", int_acc, float_acc);
    
    return 0;
}
