/* sel-sched-coverage.c
 * Program designed to trigger GCC's selective scheduler dump logic
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 -o test test.c
 * Or with: gcc -O3 -fsel-sched-pipelining-outer-loops -fdump-rtl-all -fdump-rtl-sched_details -o test test.c
 */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable function to create call RTL patterns */
__attribute__((noinline, noipa))
void side_effect_func(int val, float fval) {
    /* Use volatile to prevent optimization */
    volatile static int counter = 0;
    volatile static float fcounter = 0.0f;
    
    counter += val;
    fcounter += fval;
    
    /* Memory barrier */
    asm volatile("" ::: "memory");
}

/* Simple LCG for pseudo-random data */
static uint32_t lcg_seed = 123456789;
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
    
    /* Outer loop - fixed iteration count */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with data dependencies */
        for (int i = 0; i < 256; i++) {
            /* Create loop-carried dependency */
            int_acc = int_acc + int_array[i] * (int_acc % 100);
            
            /* Mix integer and floating-point operations */
            float temp_float = float_array[i];
            float_acc = float_acc + temp_float * (float_acc + 1.0f);
            
            /* Control flow for multi-basic-block scheduling */
            if (i % 3 == 0) {
                /* Branch 1: More complex operations */
                int_acc = int_acc - (int_array[i] / 2);
                float_acc = float_acc * 0.99f;
                
                /* Call non-inlineable function */
                side_effect_func(int_array[i], float_array[i]);
            } else if (i % 3 == 1) {
                /* Branch 2: Different operations */
                int_acc = int_acc ^ (int_array[i] << 2);
                float_acc = float_acc / 1.01f;
                
                /* Volatile store to prevent elimination */
                volatile int temp = int_acc;
                (void)temp;
            } else {
                /* Branch 3: Yet another path */
                int_acc = int_acc | (int_array[i] & 0xFF);
                float_acc = float_acc + 2.5f;
                
                /* Memory barrier in one branch */
                asm volatile("" ::: "memory");
            }
            
            /* Additional arithmetic mixing types */
            if (i % 8 == 0) {
                /* Type conversion operations */
                float_acc = float_acc + (float)int_acc * 0.001f;
                int_acc = int_acc + (int)(float_acc * 10.0f);
            }
        }
        
        /* Outer loop dependency */
        int_acc = int_acc * 2 - outer;
        float_acc = float_acc * 1.5f + (float)outer;
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
    
    /* Print checksum to ensure side effects */
    printf("Final checksum: int=%d, float=%.2f\n", int_acc, float_acc);
    
    return 0;
}
