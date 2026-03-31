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
static uint32_t lcg_seed = 12345;
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
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < 256; i++) {
        int_array[i] = (int)(lcg_rand() % 1000);
        float_array[i] = (float)(lcg_rand() % 1000) / 100.0f;
    }
    
    /* Memory barrier to prevent reordering before scheduling */
    asm volatile("" ::: "memory");
    
    /* Outer loop with small iteration count */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with data dependencies */
        for (int i = 0; i < 256; i++) {
            /* Create loop-carried dependency */
            int_acc = int_acc + int_array[i] * (int_acc % 7);
            
            /* Mixed integer operations */
            int temp_int = int_array[i] ^ (i * 3);
            
            /* Floating-point operations */
            float_acc = float_acc + float_array[i] * (float_acc + 1.0f);
            
            /* Control flow for multi-basic-block scheduling */
            if (temp_int > 500) {
                /* Branch with arithmetic */
                int_acc = int_acc - (temp_int / 2);
                float_acc = float_acc * 0.99f;
                
                /* Function call with side effects */
                int result = side_effect_func(temp_int, float_acc);
                int_acc = int_acc ^ result;
            } else {
                /* Alternative path with different operations */
                int_acc = int_acc | (temp_int << 2);
                float_acc = float_acc / 1.01f;
                
                /* Another function call */
                if (i % 3 == 0) {
                    int result = side_effect_func(-temp_int, float_acc);
                    int_acc = int_acc + result;
                }
            }
            
            /* Additional arithmetic to create scheduling complexity */
            for (int j = 0; j < 3; j++) {
                /* Small inner loop for additional scheduling complexity */
                volatile int inner_temp = int_array[(i + j) % 256];
                int_acc = int_acc + (inner_temp << j);
            }
        }
        
        /* Cross-iteration dependency */
        float_acc = float_acc + (float)outer * 0.5f;
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
    
    /* Print checksum to prevent dead code elimination */
    printf("Final checksum: int=%d, float=%.2f\n", int_acc, float_acc);
    
    return 0;
}
