/* Coverage test for GCC selective scheduler dump functionality
 * Target: sel-sched-dump.cc lines 159-163
 */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable function to create call RTL patterns */
__attribute__((noinline)) 
void side_effect_function(int value, float fvalue) {
    /* Use volatile to prevent optimization */
    volatile static int counter = 0;
    volatile static float fcounter = 0.0f;
    
    counter += value;
    fcounter += fvalue;
    
    /* Memory barrier */
    asm volatile("" ::: "memory");
}

/* Simple PRNG without external dependencies */
static uint32_t lcg_seed = 123456789;
static uint32_t lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

int main(void) {
    /* Arrays with mixed data types */
    int int_array[256];
    float float_array[256];
    
    /* Volatile accumulators to prevent dead code elimination */
    volatile int int_acc = 0;
    volatile float float_acc = 0.0f;
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < 256; i++) {
        int_array[i] = (int)(lcg_rand() % 1000);
        float_array[i] = (float)(lcg_rand() % 1000) / 100.0f;
    }
    
    /* Memory barrier before loop */
    asm volatile("" ::: "memory");
    
    /* Outer loop with small iteration count */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with data dependencies */
        for (int i = 0; i < 256; i++) {
            /* Create loop-carried dependency */
            int_acc = int_acc + int_array[i] * (int_acc % 100 + 1);
            
            /* Mixed integer operations */
            int temp = int_array[i] * 3;
            temp = temp / 2 + 7;
            
            /* Floating-point operations */
            float_acc = float_acc + float_array[i] * (float_acc + 1.0f);
            
            /* Control flow for multi-basic-block scheduling */
            if (i % 3 == 0) {
                /* Branch 1: More complex operations */
                float_acc = float_acc * 1.1f - 0.5f;
                int_acc = int_acc ^ (temp << 2);
            } else if (i % 3 == 1) {
                /* Branch 2: Different operations */
                float_acc = float_acc / 1.5f + 2.0f;
                int_acc = int_acc | (temp >> 1);
            } else {
                /* Branch 3: Yet another path */
                float_acc = float_acc + float_array[i] * 0.8f;
                int_acc = int_acc & ~temp;
            }
            
            /* Volatile store to prevent optimization */
            volatile int store_temp = int_acc + (int)float_acc;
            
            /* Conditional function call */
            if (i % 16 == 0) {
                side_effect_function(int_acc, float_acc);
            }
            
            /* Memory barrier in loop body */
            asm volatile("" ::: "memory");
        }
        
        /* Cross-iteration dependency */
        int_acc = int_acc * 2 - outer;
        float_acc = float_acc * 0.9f + (float)outer;
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
    
    /* Print checksum to ensure side effects */
    printf("Final checksum: int=%d, float=%.2f\n", 
           (int)int_acc, (float)float_acc);
    
    return 0;
}
