/* Coverage test for GCC selective scheduler dump functionality
 * Specifically targets lines 159-163 in sel-sched-dump.cc:
 *   switch_dump (stderr);
 *   dump_insn_rtx_1 (insn, debug_insn_rtx_flags);
 *   sel_print ("\n");
 *   restore_dump ();
 */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable function to create call RTL patterns */
__attribute__((noinline, noipa))
void side_effect_function(int val, float fval) {
    /* Use asm to prevent optimization */
    asm volatile("" : : "r"(val), "r"(*(int*)&fval) : "memory");
}

/* Simple LCG for pseudo-random data without external dependencies */
static uint32_t lcg_state = 123456789;
static uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

int main(void) {
    /* Arrays with mixed data types to create diverse RTL */
    volatile int int_array[256];
    volatile float float_array[256];
    
    /* Volatile accumulators to prevent optimization */
    volatile int int_acc = 0;
    volatile float float_acc = 0.0f;
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < 256; i++) {
        int_array[i] = (int)(lcg_rand() % 1000) - 500;
        float_array[i] = (float)(lcg_rand() % 1000) / 10.0f - 50.0f;
    }
    
    /* Memory barrier to prevent reordering before scheduling */
    asm volatile("" ::: "memory");
    
    /* Outer loop with small iteration count */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with data dependencies */
        for (int i = 0; i < 256; i++) {
            /* Create loop-carried dependency for scheduling challenge */
            int temp_int = int_acc;
            float temp_float = float_acc;
            
            /* Mixed integer operations */
            int load_int = int_array[i];
            int mult_int = load_int * temp_int;
            int_acc = mult_int + (i * 17);
            
            /* Mixed floating-point operations */
            float load_float = float_array[i];
            float mult_float = load_float * temp_float;
            float_acc = mult_float + (float)(i * 0.1f);
            
            /* Control flow for multi-basic-block scheduling */
            if (i % 3 == 0) {
                /* Branch with arithmetic */
                int_acc += (load_int % 7);
                float_acc += 1.5f;
            } else if (i % 5 == 0) {
                /* Different branch with different operations */
                int_acc -= (load_int / 3);
                float_acc *= 0.9f;
            } else {
                /* Default path with more operations */
                int_acc ^= (load_int << 2);
                float_acc = float_acc / 1.1f;
            }
            
            /* Conditional function call with side effects */
            if (i % 8 == 0) {
                side_effect_function(int_acc, float_acc);
            }
            
            /* Additional arithmetic to create more RTL instructions */
            volatile int store_me = int_acc * 3 + outer;
            volatile float store_me_too = float_acc * 2.0f - (float)outer;
            
            /* Use the stored values to prevent dead code elimination */
            asm volatile("" : : "r"(store_me), "r"(*(int*)&store_me_too) : "memory");
        }
        
        /* Loop interchange opportunity for scheduler */
        for (int j = 0; j < 10; j++) {
            int_acc += j * outer;
            float_acc += (float)(j * outer) * 0.01f;
        }
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
    
    /* Print checksum to ensure observable side effect */
    printf("Final checksum: int=%d, float_as_int=%d\n", 
           int_acc, *(int*)&float_acc);
    
    return 0;
}
