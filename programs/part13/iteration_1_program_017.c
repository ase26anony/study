/* sel-sched-coverage.c
 * Program designed to trigger GCC's selective scheduler dump functionality
 * to cover lines 159-163 in sel-sched-dump.cc
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
    
    /* Memory barrier to prevent reordering */
    asm volatile("" ::: "memory");
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
        float_array[i] = (float)(lcg_rand() % 1000) / 10.0f;
    }
    
    /* Memory barrier before loop */
    asm volatile("" ::: "memory");
    
    /* Outer loop with small iteration count */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with data dependencies */
        for (int i = 0; i < 256; i++) {
            /* Create loop-carried dependency */
            int_acc = int_acc + int_array[i] * (int_acc % 100);
            
            /* Mixed integer operations */
            int temp_int = int_array[i] * 3 + 7;
            
            /* Floating-point operations */
            float temp_float = float_array[i] * 1.5f + 0.5f;
            float_acc = float_acc + temp_float * (float_acc + 1.0f);
            
            /* Control flow for multi-basic-block scheduling */
            if (i % 3 == 0) {
                /* Branch with arithmetic */
                int_acc = int_acc - (temp_int / 2);
                float_acc = float_acc * 0.99f;
            } else if (i % 3 == 1) {
                /* Different branch with different operations */
                int_acc = int_acc ^ (temp_int & 0xFF);
                float_acc = float_acc + 2.5f;
            } else {
                /* Third branch with function call */
                side_effect_function(temp_int, temp_float);
            }
            
            /* Volatile store to prevent dead code elimination */
            volatile int store_var = temp_int;
            volatile float store_fvar = temp_float;
            
            /* Additional arithmetic to create scheduling complexity */
            for (int j = 0; j < 2; j++) {
                int_acc = int_acc + (store_var >> j);
                float_acc = float_acc + (store_fvar / (j + 1));
            }
            
            /* Memory barrier in loop body */
            asm volatile("" ::: "memory");
        }
        
        /* Cross-iteration dependency */
        int_acc = int_acc * 2 - outer;
        float_acc = float_acc * 0.9f + outer;
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
    
    /* Print result to prevent optimization */
    printf("Final checksum: int=%d, float=%.2f\n", int_acc, float_acc);
    
    return 0;
}
