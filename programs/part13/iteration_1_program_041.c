/* sel-sched-coverage.c
 * Program designed to trigger GCC's selective scheduler dump functionality
 * to cover lines 159-163 in sel-sched-dump.cc
 */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable function to create call RTL patterns */
__attribute__((noinline)) 
static void side_effect_func(int val, float fval) {
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
    /* Arrays with different types to create diverse RTL */
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
    
    /* Outer loop with fixed iteration count */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with data dependencies */
        for (int i = 0; i < 256; i++) {
            /* Create loop-carried dependency */
            int_acc = int_acc + int_array[i];
            
            /* Mix integer and floating-point operations */
            float temp_float = float_acc * 1.01f;
            float_acc = temp_float + float_array[i];
            
            /* Control flow for multi-basic-block scheduling */
            if (int_acc > 1000) {
                /* Branch with different operations */
                int_acc = int_acc / 2;
                float_acc = float_acc * 0.5f;
                
                /* Call non-inlineable function */
                side_effect_func(int_acc, float_acc);
            } else if (int_acc < -1000) {
                /* Another branch with different operations */
                int_acc = int_acc * 2;
                float_acc = float_acc + 10.0f;
            } else {
                /* Default path with arithmetic mix */
                int_acc = int_acc * 3 - 7;
                float_acc = float_acc - 1.5f;
                
                /* Complex expression to create more RTL */
                volatile int temp = (int_array[i] * int_acc) / (outer + 1);
                int_acc += temp;
            }
            
            /* Additional floating-point operation */
            float_acc = float_acc + (float)int_acc * 0.001f;
            
            /* Memory barrier to prevent reordering/elimination */
            asm volatile("" ::: "memory");
        }
        
        /* Cross-iteration dependency */
        int_acc = int_acc ^ (outer * 0x5A5A5A5A);
    }
    
    /* Final computation to ensure side effects */
    volatile int final_result = int_acc + (int)float_acc;
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d (float: %f)\n", final_result, float_acc);
    
    return 0;
}
