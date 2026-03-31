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
    static int counter = 0;
    counter++;
    return (x ^ counter) + (int)(y * 100.0f);
}

/* Another noinline function for conditional calls */
__attribute__((noinline))
float fp_side_effect(float a, float b) {
    volatile static float accum = 0.0f;
    accum += a * b;
    return accum;
}

/* Simple LCG for pseudo-random data without external dependencies */
static uint32_t lcg_seed = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

int main(void) {
    /* Arrays with different data types for diverse RTL patterns */
    int int_array[256];
    float float_array[256];
    
    /* Volatile accumulators to prevent optimization */
    volatile int int_accum = 0;
    volatile float float_accum = 0.0f;
    
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
            /* Loop-carried dependency for scheduling challenge */
            int_accum += int_array[i] * int_accum;
            
            /* Mixed integer operations */
            int temp_int = int_array[i] ^ (i + outer);
            
            /* Floating-point operations */
            float temp_float = float_array[i] * float_accum + 1.0f;
            float_accum = temp_float - 0.5f;
            
            /* Control flow for multi-basic-block scheduling */
            if (temp_int % 3 == 0) {
                /* Conditional function call */
                int_accum += side_effect_func(temp_int, temp_float);
                
                /* More floating-point ops in conditional path */
                float_accum += fp_side_effect(float_accum, float_array[i]);
            } else if (temp_int % 7 == 0) {
                /* Alternative path with different operations */
                int_accum -= (temp_int >> 2);
                float_accum *= 0.99f;
            } else {
                /* Default path with arithmetic */
                int_accum ^= (temp_int * 2);
                float_accum = float_accum / 1.01f;
            }
            
            /* Additional data dependency chain */
            if (i > 0) {
                int_array[i] += int_array[i-1] % 17;
                float_array[i] += float_array[i-1] * 0.1f;
            }
            
            /* Volatile store to prevent dead code elimination */
            volatile int dummy = temp_int + (int)temp_float;
            (void)dummy; /* Suppress unused warning */
        }
        
        /* Cross-iteration dependency */
        if (outer > 0) {
            int_accum = (int_accum * 3) / 2;
            float_accum = float_accum * 1.5f;
        }
        
        /* Memory barrier between outer loop iterations */
        asm volatile("" ::: "memory");
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
    
    /* Print checksum to ensure observable side effect */
    printf("Final checksum: int=%d, float=%f\n", 
           int_accum & 0x7FFFFFFF,  /* Mask to avoid overflow in printf */
           float_accum);
    
    return (int_accum > 0) ? 0 : 1;
}
