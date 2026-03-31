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
    /* Arrays with mixed data types */
    int int_array[256];
    float float_array[256];
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < 256; i++) {
        int_array[i] = (int)(lcg_rand() % 1000);
        float_array[i] = (float)(lcg_rand() % 1000) / 100.0f;
    }
    
    /* Volatile accumulators to prevent optimization */
    volatile int int_acc = 0;
    volatile float float_acc = 0.0f;
    volatile int temp_store = 0;
    
    /* Memory barrier before loop */
    asm volatile("" ::: "memory");
    
    /* Outer loop with small iteration count */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with data dependencies */
        for (int i = 0; i < 256; i++) {
            /* Loop-carried dependency for scheduling challenge */
            int_acc = int_acc + int_array[i] * (int_acc % 7 + 1);
            
            /* Mixed integer operations */
            int temp = int_array[i];
            if (temp > 500) {
                temp = temp * 2 - 100;
            } else {
                temp = temp / 2 + 50;
            }
            
            /* Store to volatile to prevent elimination */
            temp_store = temp;
            
            /* Floating-point operations */
            float fval = float_array[i];
            if (fval > 5.0f) {
                float_acc = float_acc + fval * 1.5f;
            } else {
                float_acc = float_acc + fval * 0.5f;
            }
            
            /* Cross-type operation */
            float_acc = float_acc + (float)(temp % 10) * 0.1f;
            
            /* Conditional function call with side effects */
            if ((i % 32) == 0) {
                side_effect_func(int_acc % 100, float_acc);
            }
            
            /* Memory barrier inside loop */
            asm volatile("" ::: "memory");
        }
        
        /* Additional operations between outer loop iterations */
        int_acc = int_acc ^ (outer * 0x5A5A);
        float_acc = float_acc * 0.99f;
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
    
    /* Print checksum to ensure side effects */
    printf("Final checksum: int_acc = %d, float_acc = %f\n", 
           int_acc, (double)float_acc);
    
    return 0;
}
