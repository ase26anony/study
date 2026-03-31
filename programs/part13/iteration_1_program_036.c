/* sel-sched-coverage.c
 * Program designed to trigger GCC's selective scheduler dump logic
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 -o test test.c
 * Or with: gcc -O3 -fsel-sched-pipelining-outer-loops -fdump-rtl-all -fdump-rtl-sched_details -o test test.c
 */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable function to create call RTL patterns */
__attribute__((noinline)) 
int side_effect_func(int x, float y) {
    static int counter = 0;
    counter++;
    return x + (int)y + counter;
}

/* Simple LCG for pseudo-random data without external dependencies */
static uint32_t lcg_seed = 123456789;
static inline uint32_t lcg_rand(void) {
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
    volatile int temp_result = 0;
    
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
            /* Loop-carried dependency for scheduling challenge */
            int_acc = int_acc + int_array[i] * (i + 1);
            
            /* Mixed integer and floating-point operations */
            float_acc = float_acc + float_array[i] * 1.5f;
            
            /* Store to volatile to prevent elimination */
            temp_result = int_array[i] ^ (int)float_array[i];
            
            /* Control flow for multi-basic-block scheduling */
            if (i % 3 == 0) {
                /* Integer arithmetic path */
                int_acc = int_acc - (int_array[i] / 2);
                /* Function call with side effects */
                int_acc = side_effect_func(int_acc, float_acc);
            } else if (i % 3 == 1) {
                /* Floating-point path */
                float_acc = float_acc * 1.1f;
                /* More complex arithmetic */
                int_acc = int_acc ^ (int)(float_acc * 100.0f);
            } else {
                /* Mixed operations path */
                float_acc = float_acc + (float)int_acc / 100.0f;
                int_acc = int_acc * 2 - (int)float_acc;
            }
            
            /* Additional data dependency chain */
            if (i > 0) {
                int_array[i] = int_array[i] + int_array[i-1];
                float_array[i] = float_array[i] + float_array[i-1];
            }
        }
        
        /* Cross-iteration dependency */
        int_acc = int_acc + outer * 1000;
        float_acc = float_acc + (float)outer * 0.5f;
        
        /* Memory barrier in loop body */
        asm volatile("" ::: "memory");
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
    
    /* Print checksum to ensure side effects */
    printf("Final checksum: int_acc = %d, float_acc = %f\n", 
           int_acc, (double)float_acc);
    
    /* Additional volatile store to ensure all operations are kept */
    volatile int final_check = int_acc + (int)float_acc;
    
    return final_check % 100;
}
