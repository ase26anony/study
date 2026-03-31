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
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < 256; i++) {
        int_array[i] = (int)(lcg_rand() % 1000);
        float_array[i] = (float)(lcg_rand() % 1000) * 0.001f;
    }
    
    /* Volatile accumulators to prevent dead code elimination */
    volatile int int_acc = 0;
    volatile float float_acc = 0.0f;
    volatile int temp_store;
    
    /* Outer loop with fixed iteration count */
    for (int outer = 0; outer < 5; outer++) {
        /* Memory barrier to prevent reordering before scheduling */
        asm volatile("" ::: "memory");
        
        /* Inner loop with data dependencies */
        for (int i = 0; i < 256; i++) {
            /* Loop-carried dependency for scheduling challenge */
            int_acc = int_acc + int_array[i] * (int_acc % 7 + 1);
            
            /* Mixed integer operations */
            temp_store = int_array[i] * 3 + (i & 0xF);
            
            /* Floating-point operations */
            float_acc = float_acc + float_array[i] * (float_acc * 0.1f + 1.0f);
            
            /* Control flow for multi-basic-block scheduling */
            if (i % 3 == 0) {
                /* Branch with arithmetic */
                int_acc -= temp_store / 2;
                float_acc *= 0.99f;
            } else if (i % 7 == 0) {
                /* Different branch path */
                int_acc += temp_store;
                float_acc /= 1.01f;
            } else {
                /* Default path */
                int_acc ^= temp_store;
            }
            
            /* Conditional function call with side effects */
            if (i % 13 == 0) {
                side_effect_func(int_acc, float_acc);
            }
            
            /* Additional memory barrier in loop body */
            asm volatile("" ::: "memory");
        }
        
        /* Cross-iteration dependency */
        int_acc = int_acc * 2 - outer;
        float_acc = float_acc * 1.5f + (float)outer;
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
    
    /* Print checksum to ensure side effects */
    printf("Final checksum: int=%d, float=%f\n", int_acc, float_acc);
    
    return 0;
}
