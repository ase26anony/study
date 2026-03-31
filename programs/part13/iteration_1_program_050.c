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
    static int counter = 0;
    counter++;
    /* Create side effects */
    asm volatile("" : "+r"(x) : : "memory");
    return x + (int)y + counter;
}

/* Another noinline function for conditional calls */
__attribute__((noinline))
float fp_side_effect(float a, float b) {
    volatile float result = a * b;
    /* Memory barrier to prevent optimization */
    asm volatile("" ::: "memory");
    return result;
}

/* Simple LCG for pseudo-random data without external dependencies */
static uint32_t lcg_seed = 123456789;
static uint32_t lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

int main(void) {
    /* Arrays with different data types to create diverse RTL */
    volatile int int_array[256];
    volatile float float_array[256];
    
    /* Volatile accumulators to prevent optimization */
    volatile int int_acc = 0;
    volatile float float_acc = 0.0f;
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < 256; i++) {
        int_array[i] = (int)(lcg_rand() % 1000) - 500;
        float_array[i] = (float)(lcg_rand() % 1000) / 100.0f - 5.0f;
    }
    
    /* Memory barrier before loops */
    asm volatile("" ::: "memory");
    
    /* Outer loop with small iteration count */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with data dependencies */
        for (int i = 0; i < 256; i++) {
            /* Create loop-carried dependency */
            int_acc = int_acc + int_array[i] * (int_acc % 100 + 1);
            
            /* Mixed integer operations */
            int temp_int = int_array[i] * 3;
            temp_int = temp_int / 2 + 1;
            
            /* Floating-point operations */
            float temp_float = float_array[i] * float_acc;
            float_acc = float_acc + temp_float * 0.5f;
            
            /* Conditional call based on control flow */
            if (i % 3 == 0) {
                /* Call non-inlineable function */
                int_acc += side_effect_func(temp_int, temp_float);
            } else if (i % 7 == 0) {
                /* Different control flow path */
                float_acc = fp_side_effect(float_acc, float_array[i]);
                int_acc -= 1;
            } else {
                /* Another path with arithmetic mix */
                int_acc = int_acc ^ (temp_int & 0xFF);
                float_acc = float_acc - float_array[i] / 2.0f;
            }
            
            /* Additional operations to increase scheduling complexity */
            if (int_acc > 1000000) {
                int_acc = int_acc / 2;
            }
            
            if (float_acc > 1000.0f || float_acc < -1000.0f) {
                float_acc = float_acc * 0.9f;
            }
            
            /* Memory barrier in loop body */
            asm volatile("" ::: "memory");
        }
        
        /* Cross-iteration dependency */
        int_acc = int_acc * 2 - outer;
        float_acc = float_acc * 0.99f;
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
    
    /* Print checksum to prevent dead code elimination */
    printf("Final int accumulator: %d\n", int_acc);
    printf("Final float accumulator: %f\n", float_acc);
    
    /* Additional computation to ensure scheduler has work */
    volatile int final_check = 0;
    for (int i = 0; i < 100; i++) {
        final_check += int_array[i % 256] * i;
    }
    printf("Final check: %d\n", final_check);
    
    return 0;
}
