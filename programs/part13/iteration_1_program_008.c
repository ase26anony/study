/* sel-sched-coverage.c
 * Program designed to trigger GCC's selective scheduler dump logic
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 sel-sched-coverage.c -o sel-sched-coverage
 * Or more aggressively: gcc -O3 -fsel-sched-pipelining-outer-loops -fdump-rtl-all sel-sched-coverage.c -o sel-sched-coverage
 */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable function to create call RTL patterns */
__attribute__((noinline)) 
static int side_effect_function(int x, float y) {
    volatile static int counter = 0;
    counter += x + (int)y;
    return counter;
}

/* Another non-inlineable function for control flow */
__attribute__((noinline))
static float process_float(float a, float b, int flag) {
    volatile static float accumulator = 0.0f;
    if (flag) {
        accumulator += a * b;
    } else {
        accumulator += a / (b + 1.0f);
    }
    return accumulator;
}

/* Simple PRNG without external dependencies */
static uint32_t lcg_pseudo_rand(uint32_t *state) {
    *state = *state * 1103515245 + 12345;
    return *state;
}

int main(void) {
    /* Initialize arrays with pseudo-random data */
    int int_array[256];
    float float_array[256];
    
    uint32_t seed = 42;
    for (int i = 0; i < 256; i++) {
        int_array[i] = (int)(lcg_pseudo_rand(&seed) % 1000);
        float_array[i] = (float)(lcg_pseudo_rand(&seed) % 1000) / 10.0f;
    }
    
    /* Volatile accumulators to prevent optimization */
    volatile int int_acc = 0;
    volatile float float_acc = 0.0f;
    volatile int temp_storage = 0;
    
    /* Memory barrier to prevent pre-scheduling optimization */
    asm volatile("" ::: "memory");
    
    /* Outer loop with small iteration count */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with data dependencies */
        for (int i = 0; i < 256; i++) {
            /* Loop-carried dependency on int_acc */
            int_acc += int_array[i] * (int_acc % 100 + 1);
            
            /* Mixed integer operations */
            temp_storage = int_array[i] * 3 + (i % 7);
            
            /* Control flow creating multiple basic blocks */
            if (temp_storage > 500) {
                /* Integer arithmetic path */
                int_acc -= temp_storage / 2;
                
                /* Function call with side effects */
                int dummy = side_effect_function(int_acc, float_array[i]);
                (void)dummy; /* Suppress unused warning */
            } else {
                /* Floating-point arithmetic path */
                float_acc += float_array[i] * (float_acc + 1.0f);
                
                /* More complex floating-point operations */
                float_acc = process_float(float_acc, float_array[i], i % 2);
            }
            
            /* Cross-type operation */
            if (i % 3 == 0) {
                float_acc += (float)int_array[i];
            }
            
            /* Another memory barrier to create scheduling boundaries */
            asm volatile("" ::: "memory");
        }
        
        /* Additional operations between outer loop iterations */
        int_acc = (int_acc * 3) % 10000;
        float_acc = float_acc * 0.9f;
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
    
    /* Print checksum to ensure side effects are observable */
    printf("Final checksum: int_acc = %d, float_acc = %f\n", 
           int_acc, float_acc);
    
    return 0;
}
