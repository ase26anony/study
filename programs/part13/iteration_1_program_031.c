/* sel-sched-coverage.c
 * Program designed to trigger GCC's selective scheduler dump logic
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 -o sel-sched-test sel-sched-coverage.c
 * Or more aggressively: gcc -O3 -fsel-sched-pipelining-outer-loops -fdump-rtl-all -fdump-rtl-sched_details -o sel-sched-test sel-sched-coverage.c
 */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable function to create call RTL patterns */
__attribute__((noinline)) 
int side_effect_func(int x, float y) {
    volatile static int counter = 0;
    counter++;
    return (int)(x * y) + counter;
}

/* Another non-inlineable function for control flow */
__attribute__((noinline))
float conditional_transform(float val, int threshold) {
    asm volatile("" ::: "memory");  /* Compiler barrier */
    if (val > threshold) {
        return val * 0.5f;
    } else {
        return val * 2.0f;
    }
}

/* Simple LCG for pseudo-random data without external dependencies */
static uint32_t lcg_state = 123456789;
static uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

int main(void) {
    /* Arrays with different data types for diverse RTL patterns */
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
    
    /* Memory barrier before main computation */
    asm volatile("" ::: "memory");
    
    /* Outer loop - fixed iteration count */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with data dependencies */
        for (int i = 0; i < 256; i++) {
            /* Loop-carried dependency for scheduling challenge */
            int temp_int = int_array[i];
            float temp_float = float_array[i];
            
            /* Mixed integer operations */
            int_acc = int_acc + temp_int * (i + 1);
            
            /* Mixed floating-point operations */
            float_acc = float_acc + temp_float * (float)(i % 10 + 1);
            
            /* Control flow creating multiple basic blocks */
            if (i % 3 == 0) {
                /* Branch 1: More complex operations */
                int_acc = int_acc - (temp_int / 2);
                float_acc = conditional_transform(float_acc, 100);
            } else if (i % 3 == 1) {
                /* Branch 2: Different operations */
                int_acc = int_acc ^ (temp_int << 2);
                float_acc = float_acc * 1.1f;
            } else {
                /* Branch 3: Function call with side effects */
                int func_result = side_effect_func(temp_int, temp_float);
                int_acc = int_acc + func_result;
            }
            
            /* Cross-type operation */
            if (i % 8 == 0) {
                float_acc = float_acc + (float)int_acc * 0.01f;
            }
            
            /* Memory barrier to prevent reordering */
            asm volatile("" ::: "memory");
        }
        
        /* Additional operations between outer loop iterations */
        int_acc = int_acc * 2 - 1;
        float_acc = float_acc * 0.99f;
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
    
    /* Print checksum to ensure side effects */
    printf("Final integer accumulator: %d\n", int_acc);
    printf("Final float accumulator: %f\n", float_acc);
    
    /* Additional volatile store to prevent dead code elimination */
    volatile int final_check = int_acc + (int)float_acc;
    
    return final_check % 100;
}
