/* sel-sched-coverage.c
 * Program designed to trigger GCC's selective scheduler dump logic
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
    /* Create side effects that prevent optimization */
    asm volatile("" : "+r"(x) : : "memory");
    return x + (int)y + counter;
}

/* Another non-inlineable function for conditional calls */
__attribute__((noinline))
float fp_side_effect(float a, float b) {
    static float accum = 0.0f;
    accum += a * b;
    /* Memory barrier to prevent reordering */
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
    /* Arrays with different data types */
    int int_array[256];
    float float_array[256];
    
    /* Volatile accumulators to prevent dead code elimination */
    volatile int int_acc = 0;
    volatile float float_acc = 0.0f;
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < 256; i++) {
        int_array[i] = (int)(lcg_rand() % 1000);
        float_array[i] = (float)(lcg_rand() % 1000) / 100.0f;
    }
    
    /* Memory barrier before loops */
    asm volatile("" ::: "memory");
    
    /* Outer loop with small iteration count */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with data dependencies */
        for (int i = 0; i < 256; i++) {
            /* Create loop-carried dependency */
            int_acc = int_acc + int_array[i] * (int_acc % 100 + 1);
            
            /* Mix integer and floating-point operations */
            float temp_float = float_array[i] * (float_acc + 1.0f);
            float_acc = float_acc + temp_float;
            
            /* Volatile temporary to prevent optimization */
            volatile int temp = int_array[i] + (int)float_array[i];
            
            /* Control flow for multi-basic-block scheduling */
            if (i % 3 == 0) {
                /* Call non-inlineable function */
                int_acc = side_effect_func(int_acc, float_acc);
                
                /* More floating-point operations */
                float_acc = fp_side_effect(float_acc, float_array[i]);
            } else if (i % 7 == 0) {
                /* Different code path */
                int_acc = int_acc - (int)(float_acc * 2.0f);
                float_acc = float_acc / 2.0f;
            } else {
                /* Default path with arithmetic mix */
                int_acc = int_acc ^ (int_array[i] << 2);
                float_acc = float_acc * 1.01f;
            }
            
            /* Additional data dependency chain */
            if (i > 0) {
                int_array[i] = int_array[i] + int_array[i-1] / 2;
                float_array[i] = float_array[i] + float_array[i-1] * 0.5f;
            }
            
            /* Memory barrier in loop body */
            asm volatile("" ::: "memory");
        }
        
        /* Cross-iteration dependency */
        int_acc = int_acc + outer * 1000;
        float_acc = float_acc + (float)outer * 10.0f;
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
    
    /* Print checksum to ensure side effects */
    printf("Final checksum: int_acc = %d, float_acc = %f\n", 
           (int)int_acc, (float)float_acc);
    
    /* Additional volatile store to prevent dead store elimination */
    volatile int final_result = (int)int_acc + (int)float_acc;
    
    return final_result % 256;
}
