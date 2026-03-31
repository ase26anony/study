/* sel-sched-coverage.c
 * Program designed to trigger GCC's selective scheduler dump logic
 * to cover lines 159-163 in sel-sched-dump.cc
 */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable function to create call RTL patterns */
__attribute__((noinline)) 
void side_effect_func(int val, float fval) {
    /* Use volatile to prevent optimization */
    volatile static int counter = 0;
    volatile static float fcounter = 0.0f;
    
    counter += val;
    fcounter += fval;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" ::: "memory");
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
        int_array[i] = (int)(lcg_rand() % 1000);
        float_array[i] = (float)(lcg_rand() % 1000) * 0.001f;
    }
    
    /* Memory barrier before loops */
    asm volatile("" ::: "memory");
    
    /* Outer loop with fixed iteration count */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with data dependencies */
        for (int i = 0; i < 256; i++) {
            /* Create loop-carried dependency */
            int_acc = int_acc + int_array[i];
            
            /* Mixed integer operations */
            int temp_int = int_acc * (i + 1);
            
            /* Floating-point operations */
            float temp_float = float_acc + float_array[i];
            float_acc = temp_float * 1.01f;
            
            /* Control flow for multi-basic-block scheduling */
            if (i % 3 == 0) {
                /* Branch with arithmetic */
                temp_int = temp_int / 2;
                temp_float = temp_float * 0.5f;
                
                /* Function call with side effects */
                side_effect_func(temp_int, temp_float);
            } else if (i % 7 == 0) {
                /* Another branch with different operations */
                temp_int = temp_int | 0xFF;
                temp_float = temp_float + 1.0f;
            } else {
                /* Default path with more operations */
                temp_int = temp_int ^ (i << 3);
                temp_float = temp_float - 0.1f;
            }
            
            /* Store to volatile to prevent dead code elimination */
            volatile int store_int = temp_int;
            volatile float store_float = temp_float;
            
            /* More arithmetic with dependencies */
            int_acc = int_acc ^ store_int;
            float_acc = float_acc + store_float;
            
            /* Memory barrier in loop body */
            asm volatile("" ::: "memory");
        }
        
        /* Additional operations between outer loop iterations */
        int_acc = int_acc * 3;
        float_acc = float_acc * 0.99f;
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
    
    /* Print checksum to ensure side effects */
    printf("Final checksum: int=%d, float=%f\n", int_acc, float_acc);
    
    return 0;
}
