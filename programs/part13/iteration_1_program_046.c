/* Coverage test for GCC selective scheduler dump functionality */
#include <stdio.h>
#include <stdint.h>

/* Non-inline function to create call RTL patterns */
__attribute__((noinline)) 
static void side_effect_func(int val, float fval) {
    /* Use asm to prevent complete optimization */
    asm volatile("" : : "r"(val), "r"(*(int*)&fval) : "memory");
}

/* Simple LCG for pseudo-random data without external dependencies */
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
        float_array[i] = (float)(lcg_rand() % 1000) * 0.001f;
    }
    
    /* Memory barrier before loop */
    asm volatile("" ::: "memory");
    
    /* Outer loop with small iteration count */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with data dependencies */
        for (int i = 0; i < 256; i++) {
            /* Create loop-carried dependency */
            int_acc = int_acc + int_array[i] * (int_acc % 7);
            
            /* Control flow for multi-basic-block scheduling */
            if (int_acc > 1000000) {
                /* Branch with different operations */
                float_acc = float_acc * 1.01f + float_array[i];
            } else {
                /* Alternative branch */
                float_acc = float_acc * 0.99f - float_array[i];
            }
            
            /* Mixed integer/float operations */
            volatile int temp = int_array[i] * 3;
            volatile float ftemp = float_array[i] * 2.5f;
            
            /* Conditional function call with side effects */
            if ((i % 32) == 0) {
                side_effect_func(temp, ftemp);
            }
            
            /* Additional arithmetic to create scheduling complexity */
            int_acc = int_acc ^ (temp << 3);
            float_acc = float_acc + ftemp * 0.1f;
            
            /* Memory barrier inside loop */
            asm volatile("" ::: "memory");
        }
        
        /* Cross-iteration dependency */
        int_array[outer] = int_acc % 1000;
    }
    
    /* Memory barrier after loop */
    asm volatile("" ::: "memory");
    
    /* Print checksum to prevent dead code elimination */
    printf("Final checksum: int=%d, float=%f\n", int_acc, float_acc);
    
    return 0;
}
