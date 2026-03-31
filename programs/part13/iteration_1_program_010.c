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
    
    /* Memory barrier */
    asm volatile("" ::: "memory");
}

/* Simple LCG for pseudo-random data */
static uint32_t lcg_state = 123456789;
static uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

int main(void) {
    /* Arrays with mixed data types */
    int int_array[256];
    float float_array[256];
    
    /* Volatile accumulators to prevent optimization */
    volatile int int_acc = 0;
    volatile float float_acc = 0.0f;
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < 256; i++) {
        int_array[i] = (int)(lcg_rand() % 1000);
        float_array[i] = (float)(lcg_rand() % 1000) / 1000.0f;
    }
    
    /* Memory barrier before loops */
    asm volatile("" ::: "memory");
    
    /* Outer loop with fixed iteration count */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with data dependencies */
        for (int i = 0; i < 256; i++) {
            /* Create loop-carried dependency */
            int_acc = int_acc + int_array[i] * (int_acc % 100);
            
            /* Mixed integer operations */
            int temp = int_array[i];
            if (temp > 500) {
                temp = temp * 2 - 100;
            } else {
                temp = temp / 2 + 50;
            }
            
            /* Floating-point operations */
            float f_temp = float_array[i];
            float_acc = float_acc + f_temp * (float_acc + 1.0f);
            
            /* More complex FP operations with branching */
            if (float_acc > 1000.0f) {
                float_acc = float_acc * 0.5f;
            } else {
                float_acc = float_acc * 1.5f;
            }
            
            /* Volatile store to prevent elimination */
            volatile int store_var = temp;
            volatile float f_store_var = f_temp;
            
            /* Conditional function call */
            if ((i % 32) == 0) {
                side_effect_func(temp, f_temp);
            }
            
            /* Additional arithmetic to create more RTL patterns */
            int_acc = int_acc ^ (temp << 2);
            float_acc = float_acc + (float)(temp) * 0.01f;
            
            /* Memory barrier in loop body */
            asm volatile("" ::: "memory");
        }
        
        /* Cross-iteration dependency */
        int_acc = int_acc * 2 - outer;
        float_acc = float_acc - (float)outer * 0.1f;
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
    
    /* Print checksum to ensure side effects */
    printf("Final checksum: int=%d, float=%f\n", int_acc, float_acc);
    
    return 0;
}
