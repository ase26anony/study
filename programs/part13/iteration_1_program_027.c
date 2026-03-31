/* Coverage test for GCC selective scheduler dump functionality */
#include <stdio.h>
#include <stdint.h>

/* Non-inlineable function to create call RTL patterns */
__attribute__((noinline)) 
static void side_effect_func(int val, float fval) {
    /* Use asm to prevent optimization */
    asm volatile("" : : "r"(val), "r"(*(int*)&fval) : "memory");
}

/* Simple LCG for pseudo-random data without external dependencies */
static uint32_t lcg_state = 123456789;
static uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

int main(void) {
    /* Arrays with different types to create diverse RTL patterns */
    int int_array[256];
    float float_array[256];
    
    /* Volatile accumulators to prevent optimization */
    volatile int int_acc = 0;
    volatile float float_acc = 0.0f;
    volatile int temp_store;
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < 256; i++) {
        int_array[i] = (int)(lcg_rand() % 1000);
        float_array[i] = (float)(lcg_rand() % 1000) * 0.1f;
    }
    
    /* Memory barrier to prevent reordering before scheduling */
    asm volatile("" ::: "memory");
    
    /* Outer loop with small iteration count */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with data dependencies */
        for (int i = 0; i < 256; i++) {
            /* Create loop-carried dependency for integer operations */
            int_acc = int_acc + int_array[i] * (int_acc + 1);
            
            /* Create loop-carried dependency for float operations */
            float_acc = float_acc + float_array[i] * (float_acc + 1.0f);
            
            /* Store to volatile to prevent dead code elimination */
            temp_store = int_acc + (int)float_acc;
            
            /* Control flow to create multiple basic blocks */
            if (i % 3 == 0) {
                /* Branch with integer operations */
                int_acc = int_acc - (int_array[i] / 2);
            } else if (i % 3 == 1) {
                /* Branch with float operations */
                float_acc = float_acc * 0.9f;
            } else {
                /* Default branch with mixed operations */
                int_acc = int_acc ^ int_array[i];
                float_acc = float_acc + 0.5f;
            }
            
            /* Function call with side effects - creates call RTL */
            if (i % 16 == 0) {
                side_effect_func(int_acc, float_acc);
            }
            
            /* Additional arithmetic to increase instruction count */
            int_array[i] = int_array[i] + outer;
            float_array[i] = float_array[i] * (1.0f + outer * 0.01f);
        }
        
        /* Cross-iteration dependency */
        int_acc = int_acc ^ (outer * 7);
        float_acc = float_acc + (float)outer;
    }
    
    /* Memory barrier after the loop */
    asm volatile("" ::: "memory");
    
    /* Print checksum to ensure observable side effect */
    printf("Final checksum: %d (float: %f)\n", int_acc, float_acc);
    
    return 0;
}
