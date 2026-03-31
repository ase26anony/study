/* sel-sched-coverage.c
 * Program designed to trigger GCC's selective scheduler dump logic
 * to cover lines 159-163 in sel-sched-dump.cc
 */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable function to create call RTL patterns */
__attribute__((noinline)) 
static void side_effect_func(int val, float fval) {
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
    /* Arrays with different data types to create diverse RTL */
    volatile int int_array[256];
    volatile float float_array[256];
    
    /* Volatile accumulators to prevent optimization */
    volatile int int_acc = 0;
    volatile float float_acc = 0.0f;
    volatile int temp_store;
    
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
            /* Create loop-carried dependency for integer */
            int_acc = int_acc + int_array[i] * (int_acc % 7 + 1);
            
            /* Conditional branch creating multiple basic blocks */
            if (int_acc > 1000000) {
                /* Reset if too large */
                int_acc = int_acc / 2;
                /* Floating point operation in one branch */
                float_acc = float_acc * 0.99f + float_array[i];
            } else {
                /* Different floating point operation in other branch */
                float_acc = float_acc * 1.01f - float_array[i];
            }
            
            /* Mixed integer/float operations */
            temp_store = (int)(float_acc * 100.0f) + int_acc;
            
            /* Call function with side effects periodically */
            if ((i & 15) == 0) {
                side_effect_func(temp_store, float_acc);
            }
            
            /* Additional arithmetic to create scheduling complexity */
            int_array[i] = int_array[i] ^ (temp_store & 0xFF);
            float_array[i] = float_array[i] + (float)(temp_store % 100) * 0.01f;
        }
        
        /* Cross-iteration dependency */
        float_acc = float_acc + (float)int_acc * 0.0001f;
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
    
    /* Print checksum to ensure side effects */
    printf("Final checksum: int_acc = %d, float_acc = %f\n", 
           int_acc, float_acc);
    
    return 0;
}
