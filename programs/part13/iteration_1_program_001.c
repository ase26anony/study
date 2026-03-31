/* sel-sched-coverage.c
 * Program designed to trigger GCC's selective scheduler dump logic
 * to cover lines 159-163 in sel-sched-dump.cc
 */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable function to create call RTL patterns */
__attribute__((noinline)) 
int side_effect_func(int x, float y) {
    volatile static int counter = 0;
    counter++;
    return (int)(x * y) + counter;
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
    volatile int temp_store;
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < 256; i++) {
        int_array[i] = (int)(lcg_rand() % 1000) - 500;
        float_array[i] = (float)(lcg_rand() % 1000) / 100.0f - 5.0f;
    }
    
    /* Memory barrier to prevent reordering before scheduling */
    asm volatile("" ::: "memory");
    
    /* Outer loop with small iteration count */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with data dependencies */
        for (int i = 0; i < 256; i++) {
            /* Control flow for multi-basic-block scheduling */
            if (i % 3 == 0) {
                /* Integer operations with loop-carried dependency */
                int_acc = int_acc + int_array[i] * (int_acc % 100 + 1);
                
                /* Store to volatile to prevent elimination */
                temp_store = int_acc;
                
                /* Function call with side effects */
                if (i % 7 == 0) {
                    int_acc = side_effect_func(int_acc, float_array[i]);
                }
            } 
            else if (i % 3 == 1) {
                /* Floating-point operations */
                float_acc = float_acc + float_array[i] * (float_acc + 1.0f);
                
                /* Mixed-type operations */
                int_acc += (int)(float_acc * 10.0f);
                
                /* Another volatile store */
                temp_store = (int)float_acc;
            }
            else {
                /* Complex mixed operations */
                float intermediate = float_array[i] * 2.5f;
                int_acc += int_array[i] + (int)intermediate;
                float_acc = float_acc * 0.99f + intermediate;
                
                /* Conditional function call */
                if (int_acc % 11 == 0) {
                    side_effect_func(int_array[i], intermediate);
                }
            }
            
            /* Additional data dependency chain */
            if (i > 0) {
                int_array[i] += int_array[i-1] / 2;
                float_array[i] += float_array[i-1] * 0.5f;
            }
        }
        
        /* Cross-iteration dependency */
        int_acc = int_acc % 1000;
        float_acc = float_acc - (int)float_acc;
    }
    
    /* Memory barrier after loop */
    asm volatile("" ::: "memory");
    
    /* Print checksum to ensure side effects */
    printf("Final checksum: %d (float: %.2f)\n", int_acc, float_acc);
    
    return int_acc != 0 ? 0 : 1;
}
