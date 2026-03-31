/* Coverage test for GCC selective scheduler dump functionality */
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
}

/* Simple LCG for pseudo-random data without external dependencies */
static uint32_t lcg_seed = 123456789;
static uint32_t lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

int main(void) {
    /* Arrays with mixed data types for diverse RTL patterns */
    volatile int int_array[256];
    volatile float float_array[256];
    
    /* Volatile accumulators to prevent dead code elimination */
    volatile int int_acc = 0;
    volatile float float_acc = 0.0f;
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < 256; i++) {
        int_array[i] = (int)(lcg_rand() % 1000) - 500;
        float_array[i] = (float)(lcg_rand() % 1000) / 10.0f - 50.0f;
    }
    
    /* Memory barrier to prevent reordering before scheduling */
    asm volatile("" ::: "memory");
    
    /* Outer loop with fixed iteration count */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with data dependencies for scheduling complexity */
        for (int i = 0; i < 256; i++) {
            /* Loop-carried dependency on int_acc */
            int temp_int = int_array[i];
            
            /* Mixed integer operations */
            int_acc = int_acc * 3 + temp_int;
            
            /* Control flow for multi-basic-block scheduling */
            if (int_acc > 1000) {
                /* Branch with different operations */
                int_acc = int_acc / 2;
                float_acc = float_acc * 0.5f;
            } else {
                /* Alternative branch path */
                int_acc = int_acc + temp_int;
                float_acc = float_acc + 1.0f;
            }
            
            /* Floating-point operations */
            float temp_float = float_array[i];
            float_acc = float_acc * 1.1f + temp_float;
            
            /* Additional data dependency chain */
            int dependent = int_acc;
            for (int j = 0; j < 3; j++) {
                dependent = dependent * 2 + 1;
            }
            
            /* Conditional function call with side effects */
            if ((i % 32) == 0) {
                side_effect_func(int_acc, float_acc);
            }
            
            /* Memory barrier to prevent optimization across iterations */
            asm volatile("" ::: "memory");
        }
        
        /* Cross-iteration dependency */
        int_acc = int_acc ^ (outer * 7);
        float_acc = float_acc + (float)outer * 0.25f;
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
    
    /* Print checksum to ensure observable side effects */
    printf("Final checksum: int=%d, float=%.2f\n", int_acc, float_acc);
    
    return 0;
}
