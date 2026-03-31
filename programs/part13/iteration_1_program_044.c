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
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < 256; i++) {
        int_array[i] = (int)(lcg_rand() % 1000);
        float_array[i] = (float)(lcg_rand() % 1000) / 1000.0f;
    }
    
    /* Volatile accumulators to prevent optimization */
    volatile int int_acc = 0;
    volatile float float_acc = 0.0f;
    volatile int temp_store = 0;
    
    /* Memory barrier before loops */
    asm volatile("" ::: "memory");
    
    /* Outer loop with small iteration count */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with data dependencies */
        for (int i = 0; i < 256; i++) {
            /* Integer operations with loop-carried dependency */
            int_acc = int_acc + int_array[i] * (int_acc % 7 + 1);
            
            /* Floating-point operations */
            float_acc = float_acc + float_array[i] * (float_acc * 0.1f + 1.0f);
            
            /* Store intermediate result */
            temp_store = int_acc + (int)float_acc;
            
            /* Control flow creating multiple basic blocks */
            if (i % 3 == 0) {
                /* Branch 1: More integer operations */
                int_acc = int_acc ^ (int_array[i] << 2);
                float_acc = float_acc * 1.01f;
            } else if (i % 3 == 1) {
                /* Branch 2: Different operations */
                int_acc = int_acc - (int_array[i] / 3);
                float_acc = float_acc - float_array[i];
            } else {
                /* Branch 3: Mixed operations */
                int_acc = int_acc | (int_array[i] & 0xFF);
                float_acc = float_acc / (float_array[i] + 1.0f);
            }
            
            /* Function call with side effects - creates call RTL */
            if (i % 8 == 0) {
                side_effect_func(int_acc, float_acc);
            }
            
            /* Additional arithmetic mixing types */
            temp_store = (int)(float_acc * 100.0f) + int_acc;
            
            /* Memory barrier in loop body */
            asm volatile("" ::: "memory");
        }
        
        /* Cross-iteration dependency */
        int_acc = int_acc + outer * 17;
        float_acc = float_acc + (float)outer * 0.5f;
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
    
    /* Print checksum to prevent dead code elimination */
    printf("Final checksum: int=%d, float_as_int=%d\n", 
           int_acc, (int)(float_acc * 1000.0f));
    
    return 0;
}
