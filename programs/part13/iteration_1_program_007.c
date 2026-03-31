/* sel-sched-coverage.c
 * Program designed to trigger GCC's selective scheduler dump logic
 * to cover lines 159-163 in sel-sched-dump.cc
 */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable function to create call RTL patterns */
__attribute__((noinline)) 
int side_effect_func(int x, float y) {
    static int counter = 0;
    counter++;
    return (x ^ counter) + (int)(y * 100.0f);
}

/* Simple LCG for pseudo-random data without external dependencies */
static uint32_t lcg_seed = 123456789;
static uint32_t lcg_rand(void) {
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
        int_array[i] = (int)(lcg_rand() % 1000) - 500;
        float_array[i] = (float)(lcg_rand() % 1000) / 100.0f - 5.0f;
    }
    
    /* Memory barrier to prevent pre-scheduling optimization */
    asm volatile("" ::: "memory");
    
    /* Outer loop with fixed iteration count */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with data dependencies */
        for (int i = 0; i < 256; i++) {
            /* Create loop-carried dependency */
            int_acc = int_acc + int_array[i] * (int_acc % 100 + 1);
            
            /* Mixed integer operations */
            int temp = int_array[i] ^ (i * 7);
            temp = temp + (outer << 3);
            
            /* Floating-point operations with dependency */
            float_acc = float_acc + float_array[i] * (float_acc + 1.0f);
            
            /* Conditional control flow for multi-basic-block scheduling */
            if (temp % 3 == 0) {
                /* Branch 1: More integer operations */
                int_acc = int_acc - (temp >> 2);
                float_acc = float_acc * 1.01f;
            } else if (temp % 3 == 1) {
                /* Branch 2: Different operations */
                int_acc = int_acc ^ temp;
                float_acc = float_acc / 1.01f;
            } else {
                /* Branch 3: Yet another path */
                int_acc = int_acc | (temp & 0xFF);
                float_acc = float_acc - 0.5f;
            }
            
            /* Store to volatile to prevent dead code elimination */
            temp_store = temp;
            
            /* Conditional function call with side effects */
            if (i % 32 == 0) {
                int result = side_effect_func(int_acc, float_acc);
                int_acc = int_acc + (result % 10);
            }
            
            /* Additional arithmetic mixing types */
            if (i % 64 == 0) {
                float_acc = float_acc + (float)(int_acc % 100) / 10.0f;
            }
        }
        
        /* Loop interchange opportunity for scheduler */
        for (int j = 0; j < 10; j++) {
            int_acc = int_acc + j * 7;
            float_acc = float_acc + (float)j * 0.1f;
        }
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
    
    /* Print checksum to ensure observable side effect */
    printf("Final checksum: int_acc = %d, float_acc = %f\n", 
           int_acc, float_acc);
    
    return 0;
}
