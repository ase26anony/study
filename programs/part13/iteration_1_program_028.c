/* sel-sched-coverage.c
 * Program designed to trigger GCC's selective scheduler dump logic
 * to cover lines 159-163 in sel-sched-dump.cc
 */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable function to create call RTL patterns */
__attribute__((noinline)) 
void side_effect_func(int val, float fval) {
    /* Use asm to prevent optimization */
    asm volatile("" : : "r"(val), "r"(*(int*)&fval) : "memory");
}

/* Simple LCG PRNG to avoid external dependencies */
static uint32_t lcg_state = 123456789;
static uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

int main(void) {
    /* Arrays with mixed data types for diverse RTL patterns */
    int int_array[256];
    float float_array[256];
    
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
            /* Loop-carried dependency for scheduling challenge */
            int_acc = int_acc + int_array[i] * (int_acc % 7 + 1);
            
            /* Mixed integer operations */
            int temp = int_array[i] * 3;
            temp = temp + (i % 3);
            
            /* Control flow for multi-basic-block scheduling */
            if (temp % 2 == 0) {
                /* Even path: floating-point operations */
                float_acc = float_acc * 1.01f + float_array[i];
                
                /* Store to volatile to prevent elimination */
                temp_store = temp;
                
                /* Function call with side effects */
                side_effect_func(temp, float_acc);
            } else {
                /* Odd path: different operations */
                float_acc = float_acc - float_array[i] * 0.5f;
                
                /* More complex integer math */
                int_acc = int_acc ^ (temp << 2);
                
                /* Another memory barrier */
                asm volatile("" ::: "memory");
            }
            
            /* Additional conditional for more basic blocks */
            if (i % 10 == 0) {
                /* Periodic operation with dependency */
                float_acc = float_acc + (float)int_array[i] * 0.1f;
                side_effect_func(i, float_acc);
            }
        }
        
        /* Inter-iteration memory barrier */
        asm volatile("" ::: "memory");
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
    
    /* Print checksum to ensure side effects */
    printf("Final checksum: int_acc = %d, float_acc = %f\n", 
           int_acc, float_acc);
    
    return 0;
}
