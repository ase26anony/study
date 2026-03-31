/* Coverage test for GCC selective scheduler dump functionality */
/* Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 -o test test.c */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable function to create call RTL patterns */
__attribute__((noinline)) 
static void side_effect_func(int val, float fval) {
    /* Use asm to prevent optimization */
    asm volatile("" : : "r"(val), "r"(fval) : "memory");
}

/* Simple LCG for pseudo-random data without external dependencies */
static uint32_t lcg_state = 123456789;
static uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

int main(void) {
    /* Arrays with different data types to create diverse RTL */
    volatile int int_array[256];
    volatile float float_array[256];
    
    /* Volatile accumulators to prevent optimization */
    volatile int int_acc = 0;
    volatile float float_acc = 0.0f;
    volatile int temp_store;
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < 256; i++) {
        int_array[i] = (int)(lcg_rand() % 1000);
        float_array[i] = (float)(lcg_rand() % 1000) * 0.001f;
    }
    
    /* Memory barrier to prevent pre-scheduling optimization */
    asm volatile("" ::: "memory");
    
    /* Outer loop with fixed iteration count */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with data dependencies */
        for (int i = 0; i < 256; i++) {
            /* Create loop-carried dependency */
            int_acc = int_acc + int_array[i] * (int_acc % 7 + 1);
            
            /* Control flow for multi-basic-block scheduling */
            if (i % 3 == 0) {
                /* Branch with floating-point operations */
                float_acc = float_acc * 1.01f + float_array[i];
                
                /* Call function with side effects */
                if (i % 7 == 0) {
                    side_effect_func(int_acc, float_acc);
                }
            } else if (i % 3 == 1) {
                /* Different branch with integer operations */
                temp_store = int_array[i] ^ (int_acc & 0xFF);
                int_acc = int_acc - temp_store;
            } else {
                /* Third branch with mixed operations */
                float temp_float = float_array[i] * 2.0f;
                float_acc = float_acc + temp_float;
                int_acc = int_acc + (int)temp_float;
            }
            
            /* Additional arithmetic to create scheduling complexity */
            int_acc = (int_acc * 3) / 2;
            float_acc = float_acc * 0.99f;
            
            /* Memory barrier to prevent reordering */
            asm volatile("" ::: "memory");
        }
        
        /* Cross-iteration dependency */
        int_array[outer] = int_acc % 1000;
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
    
    /* Print checksum to ensure side effects */
    printf("Final checksum: int_acc = %d, float_acc = %f\n", 
           int_acc, float_acc);
    
    return 0;
}
