/* sel-sched-coverage.c
 * Program designed to trigger GCC's selective scheduler dump logic
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 -o test test.c
 * Or more aggressively: gcc -O3 -fsel-sched-pipelining-outer-loops -fdump-rtl-all -o test test.c
 */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable function to create call RTL patterns */
__attribute__((noinline)) 
int side_effect_func(int x, float y) {
    volatile static int counter = 0;
    counter += x + (int)y;
    return counter;
}

/* Another noinline function for conditional calls */
__attribute__((noinline))
float fp_side_effect(float a, float b) {
    volatile static float accum = 0.0f;
    accum = accum * 0.99f + a * b;
    return accum;
}

/* Simple LCG for pseudo-random data without external dependencies */
static uint32_t lcg_seed = 123456789;
static uint32_t lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

int main(void) {
    /* Arrays with different data types for diverse RTL patterns */
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
    
    /* Memory barrier to prevent pre-scheduling optimization */
    asm volatile("" ::: "memory");
    
    /* Outer loop with small iteration count */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with data dependencies */
        for (int i = 0; i < 256; i++) {
            /* Loop-carried dependency on int_acc */
            int temp_int = int_array[i];
            
            /* Mixed integer operations */
            int_acc = int_acc + temp_int * (i + 1);
            
            /* Conditional control flow for multi-basic-block scheduling */
            if (int_acc > 1000000) {
                /* Call side effect function in one branch */
                int_acc = side_effect_func(int_acc, float_acc);
                
                /* Floating point operations */
                float temp_float = float_array[i];
                float_acc = float_acc * 0.9f + temp_float * 2.0f;
            } else {
                /* Different operations in the else branch */
                float temp_float = float_array[(i + 1) % 256];
                float_acc = float_acc * 1.1f - temp_float;
                
                /* Another function call pattern */
                if (i % 32 == 0) {
                    float_acc = fp_side_effect(float_acc, temp_float);
                }
            }
            
            /* Additional arithmetic with type mixing */
            int mixed = (int)(float_acc * 10.0f) + int_acc % 100;
            
            /* Store to volatile to ensure instruction isn't eliminated */
            volatile int store_sink = mixed;
            (void)store_sink;  /* Suppress unused warning */
            
            /* Complex condition with short-circuit evaluation */
            if (i > 128 && int_acc < 500000 && float_acc > 50.0f) {
                int_acc = int_acc / 2;
                float_acc = float_acc * 0.5f;
            }
        }
        
        /* Loop interchange prevention barrier */
        asm volatile("" ::: "memory");
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
    
    /* Print checksum to ensure observable side effects */
    printf("Final checksum: int_acc = %d, float_acc = %f\n", 
           int_acc, float_acc);
    
    return 0;
}
