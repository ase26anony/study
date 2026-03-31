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
    
    /* Memory barrier to prevent reordering */
    asm volatile("" ::: "memory");
}

/* Simple LCG PRNG to avoid external dependencies */
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
        float_array[i] = (float)(lcg_rand() % 1000) / 1000.0f;
    }
    
    /* Memory barrier before loops */
    asm volatile("" ::: "memory");
    
    /* Outer loop - creates scheduling regions */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with complex data dependencies */
        for (int i = 0; i < 256; i++) {
            /* Loop-carried dependency for integer */
            int temp_int = int_array[i];
            int_acc = int_acc * 13 + temp_int;
            
            /* Loop-carried dependency for float */
            float temp_float = float_array[i];
            float_acc = float_acc * 1.5f + temp_float;
            
            /* Mixed integer/float operations */
            volatile int mixed_op = (int)(float_acc) * temp_int;
            
            /* Control flow to create multiple basic blocks */
            if (i % 3 == 0) {
                /* Branch 1: More integer operations */
                int_acc += mixed_op / 7;
                float_acc += (float)mixed_op * 0.1f;
            } else if (i % 3 == 1) {
                /* Branch 2: Function call with side effects */
                side_effect_func(mixed_op, float_acc);
                
                /* Additional arithmetic */
                int_acc -= temp_int >> 2;
                float_acc -= temp_float * 0.25f;
            } else {
                /* Branch 3: Complex arithmetic chain */
                for (int j = 0; j < 2; j++) {
                    int_acc = (int_acc * 3 + temp_int) / 2;
                    float_acc = (float_acc + temp_float) * 0.75f;
                }
            }
            
            /* Memory barrier in loop body */
            asm volatile("" ::: "memory");
            
            /* Additional operation to create more RTL patterns */
            volatile float cross_dep = float_acc * (float)int_acc;
            if (cross_dep > 1000.0f) {
                int_acc = (int)(cross_dep / 100.0f);
            }
        }
        
        /* Inter-iteration dependency */
        int_acc = int_acc ^ (outer * 0x5A5A5A5A);
        float_acc = float_acc + (float)outer * 0.01f;
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
    
    /* Print checksum to prevent dead code elimination */
    printf("Final checksum: int=%d, float=%f\n", 
           (int)int_acc, (double)float_acc);
    
    return 0;
}
