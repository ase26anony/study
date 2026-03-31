/* sel-sched-coverage.c
 * Program designed to trigger GCC's selective scheduler dump logic
 * to cover lines 159-163 in sel-sched-dump.cc
 */

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

/* Simple LCG for pseudo-random data without external dependencies */
static uint32_t lcg_seed = 12345;
static uint32_t lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

int main(void) {
    /* Arrays with different data types to create diverse RTL */
    volatile int int_array[100];
    volatile float float_array[100];
    
    /* Volatile accumulators to prevent dead code elimination */
    volatile int int_acc = 0;
    volatile float float_acc = 0.0f;
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < 100; i++) {
        int_array[i] = (int)(lcg_rand() % 1000);
        float_array[i] = (float)(lcg_rand() % 1000) * 0.1f;
    }
    
    /* Memory barrier before loops */
    asm volatile("" ::: "memory");
    
    /* Outer loop with fixed iteration count */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with data dependencies */
        for (int i = 0; i < 100; i++) {
            /* Create loop-carried dependency */
            int_acc += int_array[i] * int_acc;
            
            /* Mix integer and floating-point operations */
            float temp_float = float_array[i];
            float_acc = float_acc * 0.99f + temp_float * 1.01f;
            
            /* Control flow for multi-basic-block scheduling */
            if (int_acc > 1000000) {
                /* Branch with different operations */
                int_acc = int_acc / 2;
                float_acc = float_acc * 0.5f;
                
                /* Call function with side effects */
                side_effect_func(int_acc, float_acc);
            } else if (int_acc < -1000000) {
                /* Another branch path */
                int_acc = int_acc * 2;
                float_acc = float_acc * 2.0f;
            } else {
                /* Default path with arithmetic mix */
                int_acc = int_acc + (i * outer);
                float_acc = float_acc + (i * outer * 0.1f);
                
                /* Memory operation */
                volatile int temp = int_array[(i + outer) % 100];
                int_acc += temp;
            }
            
            /* Additional arithmetic to create scheduling complexity */
            for (int j = 0; j < 3; j++) {
                /* Small inner loop for additional scheduling complexity */
                int_acc += j * outer;
                float_acc += j * outer * 0.01f;
            }
            
            /* Compiler barrier to prevent reordering */
            asm volatile("" ::: "memory");
        }
        
        /* Cross-iteration dependency */
        int_array[outer] = int_acc % 1000;
        float_array[outer] = float_acc;
    }
    
    /* Final output to prevent complete optimization */
    printf("Final checksum: int_acc = %d, float_acc = %f\n", 
           int_acc, float_acc);
    
    return 0;
}
