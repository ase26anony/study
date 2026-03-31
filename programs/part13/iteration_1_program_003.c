/* sel-sched-coverage.c
 * Program designed to trigger GCC's selective scheduler dump logic
 * Specifically targets uncovered lines in sel-sched-dump.cc
 */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable function to create call RTL patterns */
__attribute__((noinline)) 
void side_effect_func(int value, float fvalue) {
    /* Use volatile to prevent optimization */
    volatile static int counter = 0;
    volatile static float fcounter = 0.0f;
    
    counter += value;
    fcounter += fvalue;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" ::: "memory");
}

/* Simple LCG for pseudo-random data without external dependencies */
static uint32_t lcg_seed = 123456789;
static uint32_t lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

int main(void) {
    /* Arrays with different types to create diverse RTL */
    volatile int int_array[256];
    volatile float float_array[256];
    
    /* Volatile accumulators to prevent dead code elimination */
    volatile int int_acc = 0;
    volatile float float_acc = 0.0f;
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < 256; i++) {
        int_array[i] = (int)(lcg_rand() % 1000);
        float_array[i] = (float)(lcg_rand() % 1000) * 0.001f;
    }
    
    /* Memory barrier before loop */
    asm volatile("" ::: "memory");
    
    /* Outer loop with small iteration count */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with data dependencies */
        for (int i = 0; i < 256; i++) {
            /* Create loop-carried dependency */
            int_acc = int_acc + int_array[i] * (i + 1);
            
            /* Control flow to create multiple basic blocks */
            if (int_acc > 1000000) {
                /* Branch with different operations */
                float_acc = float_acc * 1.01f + float_array[i];
                
                /* Call non-inlineable function */
                side_effect_func(int_array[i], float_array[i]);
            } else {
                /* Alternative path with different operations */
                float_acc = float_acc + float_array[i] * 0.99f;
                
                /* More complex integer operation */
                int temp = int_array[i] * 3;
                int_acc = int_acc - temp / 2;
                
                /* Conditional call */
                if (i % 3 == 0) {
                    side_effect_func(temp, float_acc);
                }
            }
            
            /* Additional operations to increase scheduling complexity */
            volatile int temp_var = int_array[i] * int_acc;
            volatile float temp_fvar = float_array[i] * float_acc;
            
            /* Mix integer and floating-point operations */
            if (i % 2 == 0) {
                int_acc = int_acc ^ (temp_var & 0xFF);
                float_acc = float_acc + temp_fvar * 0.5f;
            } else {
                int_acc = int_acc | (temp_var >> 2);
                float_acc = float_acc - temp_fvar * 0.25f;
            }
            
            /* Memory barrier to prevent optimization */
            asm volatile("" ::: "memory");
        }
        
        /* Cross-iteration dependency */
        int_array[0] = int_acc % 1000;
        float_array[0] = float_acc * 0.001f;
    }
    
    /* Final output to prevent dead code elimination */
    printf("Final checksum: int_acc = %d, float_acc = %f\n", 
           (int)int_acc, (double)float_acc);
    
    return 0;
}
