/* sel-sched-coverage.c
 * Designed to trigger GCC selective scheduler dump logic
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 -o test test.c
 * Or more aggressively: gcc -O3 -fsel-sched-pipelining-outer-loops -fdump-rtl-all -fdump-rtl-sched_details -o test test.c
 */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable function to create call RTL patterns */
__attribute__((noinline)) 
int side_effect_func(int x, float y) {
    static int counter = 0;
    counter++;
    /* Use asm to prevent optimization */
    asm volatile("" : "+r"(x) : : "memory");
    return x + (int)y + counter;
}

/* Another noinline function for diversity */
__attribute__((noinline))
float fp_side_effect(float a, float b) {
    volatile static float accum = 0.0f;
    accum += a * b;
    /* Memory barrier */
    asm volatile("" ::: "memory");
    return accum;
}

/* Simple PRNG without external dependencies */
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
        float_array[i] = (float)(lcg_rand() % 1000) * 0.01f;
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
            /* Loop-carried dependency on int_acc */
            int_acc = int_acc + int_array[i] * (int_acc % 100 + 1);
            
            /* Mixed integer operations */
            int temp = int_array[i] * 3;
            temp = temp / 2 + 7;
            
            /* Control flow for multi-basic-block scheduling */
            if (i % 3 == 0) {
                /* Branch 1: Integer-heavy path */
                temp_store = temp * 2 - int_array[(i + 1) % 256];
                
                /* Call non-inlineable function */
                if (i % 7 == 0) {
                    temp_store = side_effect_func(temp_store, float_array[i]);
                }
            } else if (i % 3 == 1) {
                /* Branch 2: Float-heavy path */
                float_acc = float_acc + float_array[i] * (float_acc + 1.0f);
                
                /* Mixed float operations */
                float ftemp = float_array[i] * 2.5f;
                ftemp = ftemp / 1.7f + 0.3f;
                
                /* Store to volatile to prevent elimination */
                *(volatile float*)&float_acc = ftemp;
                
                /* Call float side effect function */
                if (i % 5 == 0) {
                    float_acc = fp_side_effect(float_acc, float_array[i]);
                }
            } else {
                /* Branch 3: Mixed operations */
                int_acc = int_acc - int_array[i];
                float_acc = float_acc - float_array[i];
                
                /* Complex expression with both types */
                temp_store = (int)(float_acc * 10.0f) + int_acc % 50;
            }
            
            /* Additional arithmetic to create more RTL patterns */
            int_acc = int_acc ^ (temp_store << 3);
            float_acc = float_acc * 0.99f + 0.01f;
            
            /* Memory barrier in loop body */
            asm volatile("" ::: "memory");
        }
        
        /* Loop interchange opportunity */
        for (int j = 0; j < 10; j++) {
            int_acc = int_acc + j * 7;
            float_acc = float_acc + (float)j * 0.1f;
        }
    }
    
    /* Final computation to ensure side effects */
    int final_result = int_acc + (int)float_acc;
    
    /* Print result to prevent dead code elimination */
    printf("Final checksum: %d\n", final_result);
    
    /* Additional volatile store */
    *(volatile int*)&int_acc = final_result;
    
    return final_result % 256;
}
