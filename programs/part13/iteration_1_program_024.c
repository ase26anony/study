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
    volatile static int counter = 0;
    counter += x + (int)y;
    return counter & 0xFF;
}

/* Simple LCG for pseudo-random data without external dependencies */
static uint32_t lcg_seed = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

int main(void) {
    /* Arrays with different data types to create diverse RTL */
    int int_array[256];
    float float_array[256];
    
    /* Volatile accumulators to prevent optimization */
    volatile int int_acc = 0;
    volatile float float_acc = 0.0f;
    volatile int temp_store = 0;
    
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
            /* Loop-carried dependency for scheduling challenge */
            int_acc += int_array[i] * (int_acc & 0xF);
            
            /* Mixed integer and floating-point operations */
            float fval = float_array[i];
            float_acc = float_acc * 0.99f + fval * 1.01f;
            
            /* Store to volatile to prevent elimination */
            temp_store = int_acc + (int)float_acc;
            
            /* Control flow for multi-basic-block scheduling */
            if (i % 3 == 0) {
                /* Branch with computation */
                int_acc -= float_array[i] * 2;
                
                /* Function call with side effects */
                if (i % 7 == 0) {
                    int_acc ^= side_effect_func(int_acc, float_acc);
                }
            } else if (i % 5 == 0) {
                /* Alternative branch path */
                float_acc = float_acc / 1.5f;
                temp_store = (int)(float_acc * 100.0f);
            } else {
                /* Default path with more arithmetic */
                int delta = int_array[(i + 1) % 256] - int_array[i];
                int_acc += delta * delta;
            }
            
            /* Additional memory operations */
            if (i % 13 == 0) {
                int_array[i] = temp_store % 100;
            }
            
            /* Compiler barrier in loop body */
            asm volatile("" ::: "memory");
        }
        
        /* Cross-iteration dependency */
        float_acc = float_acc * 0.9f + (float)outer * 0.1f;
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
    
    /* Print checksum to ensure side effects */
    printf("Final checksum: int_acc = %d, float_acc = %f\n", 
           int_acc, (double)float_acc);
    
    return int_acc & 0xFF;
}
