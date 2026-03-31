/* sel-sched-coverage.c
 * Designed to trigger GCC's selective scheduler dump logic
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 -o test test.c
 * Or more aggressively: gcc -O3 -fsel-sched-pipelining-outer-loops -fdump-rtl-all -fdump-rtl-sched_details -o test test.c
 */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable function to create call RTL patterns */
__attribute__((noinline, noipa))
static void side_effect_func(int val, float fval) {
    /* Use volatile to prevent optimization */
    volatile int sink = val;
    volatile float fsink = fval;
    (void)sink;
    (void)fsink;
}

/* Another non-inlineable function for control flow */
__attribute__((noinline, noipa))
static int conditional_helper(int x) {
    volatile int v = x;
    return v * 3 + 1;
}

/* Simple PRNG without external dependencies */
static uint32_t lcg_state = 123456789;
static uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

int main(void) {
    /* Arrays with mixed data types */
    int int_array[256];
    float float_array[256];
    
    /* Volatile accumulators to prevent optimization */
    volatile int int_acc = 0;
    volatile float float_acc = 0.0f;
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < 256; i++) {
        int_array[i] = (int)(lcg_rand() % 1000);
        float_array[i] = (float)(lcg_rand() % 1000) / 10.0f;
    }
    
    /* Memory barrier to prevent reordering before scheduling */
    asm volatile("" ::: "memory");
    
    /* Outer loop with small iteration count */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with data dependencies */
        for (int i = 0; i < 256; i++) {
            /* Loop-carried dependency on int_acc */
            int temp_int = int_array[i] * (int_acc + 1);
            
            /* Mix integer and floating-point operations */
            float temp_float = float_array[i] * (float_acc + 1.0f);
            
            /* Control flow creating multiple basic blocks */
            if (temp_int > 500) {
                /* Branch with more complex operations */
                int_acc += temp_int / 2;
                float_acc += temp_float * 0.5f;
                
                /* Call non-inlineable function */
                side_effect_func(temp_int, temp_float);
            } else {
                /* Alternative branch with different operations */
                int_acc += temp_int * 2;
                float_acc += temp_float * 2.0f;
                
                /* Another function call with conditional */
                if (i % 3 == 0) {
                    int helper_val = conditional_helper(temp_int);
                    int_acc += helper_val % 7;
                }
            }
            
            /* Additional floating-point operation */
            float_acc = float_acc * 0.99f + 0.01f;
            
            /* Memory barrier to prevent optimization within loop */
            asm volatile("" ::: "memory");
        }
        
        /* Cross-iteration dependency */
        int_acc = (int_acc * 13 + 17) % 1000;
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
    
    /* Print checksum to ensure side effects */
    printf("Final checksum: int_acc = %d, float_acc = %f\n", 
           int_acc, (double)float_acc);
    
    return 0;
}
