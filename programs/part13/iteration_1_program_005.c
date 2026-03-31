/* sel-sched-coverage.c
 * Program designed to trigger GCC's selective scheduler dump functionality
 * to cover lines 159-163 in sel-sched-dump.cc
 */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable function to create call RTL patterns */
__attribute__((noinline)) 
void side_effect_func(int val, float fval) {
    /* Use volatile to prevent optimization */
    volatile static int counter = 0;
    volatile static float accumulator = 0.0f;
    
    counter += val;
    accumulator += fval;
    
    /* Memory barrier */
    asm volatile("" ::: "memory");
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
        float_array[i] = (float)(lcg_rand() % 1000) / 10.0f;
    }
    
    /* Memory barrier before loops */
    asm volatile("" ::: "memory");
    
    /* Outer loop with fixed iteration count */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with data dependencies */
        for (int i = 0; i < 256; i++) {
            /* Create loop-carried dependency */
            int_acc += int_array[i] * int_acc;
            
            /* Control flow for multi-basic-block scheduling */
            if (int_acc > 1000000) {
                /* Branch with different operations */
                float_acc = float_acc * 0.99f;
                int_acc = int_acc / 2;
            } else {
                /* Alternative branch with mixed operations */
                float_acc = float_acc + float_array[i];
                
                /* Complex expression with multiple operations */
                int temp = int_array[i] * 3 + outer * 7;
                int_acc = int_acc ^ temp;
            }
            
            /* Additional floating-point operation */
            float_acc = float_acc * 1.01f + (float)i * 0.001f;
            
            /* Conditional function call */
            if ((i % 32) == 0) {
                side_effect_func(int_acc, float_acc);
            }
            
            /* Memory barrier inside loop */
            asm volatile("" ::: "memory");
            
            /* More mixed operations */
            int_array[i] = int_array[i] + outer;
            float_array[i] = float_array[i] * (1.0f + (float)outer * 0.1f);
            
            /* Additional dependency chain */
            if (i > 0) {
                int_acc = int_acc + int_array[i-1];
                float_acc = float_acc - float_array[i-1];
            }
        }
        
        /* Inter-loop operations */
        volatile int temp = int_acc;
        volatile float ftemp = float_acc;
        
        /* Cross-type operations */
        int_acc = int_acc + (int)float_acc;
        float_acc = float_acc + (float)int_acc;
        
        /* Another memory barrier */
        asm volatile("" ::: "memory");
    }
    
    /* Final computation to ensure side effects */
    volatile int final_result = int_acc + (int)float_acc;
    
    /* Print result to prevent dead code elimination */
    printf("Final checksum: %d\n", final_result);
    
    return final_result > 0 ? 0 : 1;
}
