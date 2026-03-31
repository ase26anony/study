/* sel-sched-coverage.c
 * Designed to trigger GCC's selective scheduler dump logic
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 -o test test.c
 * Or more aggressively: gcc -O3 -fsel-sched-pipelining-outer-loops -fdump-rtl-all -o test test.c
 */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable function to create call RTL patterns */
__attribute__((noinline)) 
void side_effect_function(int val, float fval) {
    /* Use asm to prevent optimization */
    asm volatile("" : : "r"(val), "r"(fval) : "memory");
}

/* Simple LCG for pseudo-random data without external dependencies */
static uint32_t lcg_state = 123456789;
static uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

int main(void) {
    /* Arrays with different data types for diverse RTL patterns */
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
    
    /* Memory barrier to prevent reordering before scheduling */
    asm volatile("" ::: "memory");
    
    /* Outer loop with small iteration count */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with data dependencies */
        for (int i = 0; i < 256; i++) {
            /* Create loop-carried dependency for scheduling challenge */
            int_acc = int_acc + int_array[i] * (int_acc % 100 + 1);
            
            /* Mixed integer operations */
            int temp = int_array[i] * 3;
            temp = temp / 2 + 1;
            
            /* Floating-point operations */
            float ftemp = float_array[i] * 2.5f;
            float_acc = float_acc + ftemp * (float)(i + 1);
            
            /* Store to volatile to prevent elimination */
            temp_store = temp;
            
            /* Control flow for multi-basic-block scheduling */
            if (i % 3 == 0) {
                /* Branch with operations */
                int_acc = int_acc - temp;
                float_acc = float_acc * 0.99f;
                
                /* Function call with side effects */
                side_effect_function(temp, ftemp);
            } else if (i % 7 == 0) {
                /* Another branch with different operations */
                int_acc = int_acc | (temp & 0xFF);
                float_acc = float_acc + 1.0f;
            } else {
                /* Default path with more operations */
                int_acc = int_acc ^ (temp << 2);
                float_acc = float_acc - 0.5f;
            }
            
            /* Additional arithmetic to create more RTL patterns */
            if (i % 2 == 0) {
                int_acc = int_acc + (int)(float_acc * 10.0f);
            }
        }
        
        /* Memory barrier between outer loop iterations */
        asm volatile("" ::: "memory");
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
    
    /* Print checksum to ensure side effects are observable */
    printf("Final checksum: %d (float: %f)\n", int_acc, float_acc);
    
    return 0;
}
