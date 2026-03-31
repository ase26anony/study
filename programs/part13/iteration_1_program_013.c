/* Coverage test for GCC selective scheduler dump functionality */
/* Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 -o test test.c */

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

/* Simple LCG for pseudo-random data */
static uint32_t lcg_seed = 12345;
static uint32_t lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

int main(void) {
    /* Arrays with different data types */
    int int_array[256];
    float float_array[256];
    
    /* Volatile accumulators to prevent optimization */
    volatile int int_acc = 0;
    volatile float float_acc = 0.0f;
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < 256; i++) {
        int_array[i] = (int)(lcg_rand() % 1000);
        float_array[i] = (float)(lcg_rand() % 1000) / 100.0f;
    }
    
    /* Memory barrier before loops */
    asm volatile("" ::: "memory");
    
    /* Outer loop with fixed iteration count */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with data dependencies */
        for (int i = 0; i < 256; i++) {
            /* Create loop-carried dependency */
            int_acc = int_acc + int_array[i] * (int_acc % 7);
            
            /* Conditional branch for multi-basic-block scheduling */
            if (int_acc > 1000000) {
                /* Reset to avoid overflow */
                int_acc = int_acc / 2;
                float_acc = float_acc * 0.5f;
            } else {
                /* Different computation path */
                float_acc = float_acc + float_array[i] * (float_acc + 1.0f);
            }
            
            /* Mix integer and floating-point operations */
            volatile int temp_int = int_array[i] * 3;
            volatile float temp_float = float_array[i] * 2.5f;
            
            /* Function call with side effects */
            if (i % 32 == 0) {
                side_effect_func(temp_int, temp_float);
            }
            
            /* Additional arithmetic to create scheduling complexity */
            for (int j = 0; j < 2; j++) {  /* Small unrolled-like inner loop */
                int_acc = int_acc ^ (int_array[i] << j);
                float_acc = float_acc + (float)(j) * 0.1f;
            }
            
            /* Memory barrier in loop body */
            asm volatile("" ::: "memory");
        }
        
        /* Cross-iteration dependency */
        int_array[outer] = int_acc % 1000;
        float_array[outer] = float_acc;
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
    
    /* Print checksum to prevent dead code elimination */
    printf("Final checksum: int=%d, float=%.2f\n", int_acc, float_acc);
    
    return 0;
}
