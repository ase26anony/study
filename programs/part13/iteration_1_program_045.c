/* Coverage test for GCC selective scheduler dump functionality */
/* Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 -o test_sel_sched test_sel_sched.c */

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

/* Simple PRNG without external dependencies */
static uint32_t lcg_seed = 12345;
static uint32_t lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return (lcg_seed >> 16) & 0x7FFF;
}

int main(void) {
    /* Arrays with different types to create diverse RTL */
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
    
    /* Memory barrier before loops */
    asm volatile("" ::: "memory");
    
    /* Outer loop with small iteration count */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with data dependencies */
        for (int i = 0; i < 256; i++) {
            /* Create loop-carried dependency */
            int_acc = int_acc + int_array[i];
            
            /* Conditional branch for multi-basic-block scheduling */
            if (int_acc > 1000) {
                /* Branch taken path with float operations */
                float temp = float_array[i] * 2.5f;
                float_acc = float_acc + temp;
                
                /* Call function with side effects */
                if (i % 32 == 0) {
                    side_effect_func(int_acc, float_acc);
                }
            } else {
                /* Branch not taken path with different operations */
                float temp = float_array[i] / 2.0f;
                float_acc = float_acc - temp;
                
                /* More complex integer operation */
                int_acc = int_acc * 2 - int_array[i];
            }
            
            /* Additional mixed operations */
            if (i % 64 == 0) {
                /* Create memory pressure */
                volatile int temp = int_array[(i + 1) % 256];
                int_acc = int_acc ^ temp;
                
                /* Floating point operation with conversion */
                float_acc = float_acc + (float)temp * 0.1f;
            }
            
            /* Memory barrier in loop body */
            asm volatile("" ::: "memory");
        }
        
        /* Cross-iteration dependency */
        int_array[0] = int_acc % 100;
        float_array[0] = float_acc * 0.9f;
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
    
    /* Print checksum to prevent elimination */
    printf("Final checksum: int=%d, float=%.2f\n", int_acc, float_acc);
    
    return 0;
}
