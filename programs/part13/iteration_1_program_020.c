/* sel-sched-coverage.c
 * Program designed to trigger GCC's selective scheduler dump logic
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 -o test test.c
 * Or more aggressively: gcc -O3 -fsel-sched-pipelining-outer-loops -fdump-rtl-all -fdump-rtl-sched_details -o test test.c
 */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable function to create call RTL patterns */
__attribute__((noinline)) 
int side_effect_func(int x, float y) {
    volatile static int counter = 0;
    counter++;
    return (x + (int)y) ^ counter;
}

/* Another non-inlineable function for more RTL variety */
__attribute__((noinline))
float fp_side_effect(float a, float b) {
    volatile static float accum = 0.0f;
    accum += a * b;
    return accum;
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
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < 256; i++) {
        int_array[i] = (int)(lcg_rand() % 1000) - 500;
        float_array[i] = (float)(lcg_rand() % 1000) / 100.0f - 5.0f;
    }
    
    /* Memory barrier to prevent pre-scheduling optimizations */
    asm volatile("" ::: "memory");
    
    /* Outer loop with small iteration count */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with data dependencies */
        for (int i = 0; i < 256; i++) {
            /* Create loop-carried dependency */
            int_acc += int_array[i] * int_acc;
            
            /* Mixed integer operations */
            int temp_int = int_array[i] ^ (i + outer);
            
            /* Floating-point operations */
            float temp_float = float_array[i] * float_acc;
            float_acc += temp_float;
            
            /* Control flow for multi-basic-block scheduling */
            if (temp_int > 0) {
                /* Branch with arithmetic */
                float_acc = float_acc * 1.01f;
                
                /* Function call with side effects */
                int func_result = side_effect_func(temp_int, temp_float);
                int_acc ^= func_result;
            } else {
                /* Alternative branch with different operations */
                float_acc = float_acc * 0.99f;
                
                /* Different function call pattern */
                float fp_result = fp_side_effect(temp_float, float_acc);
                float_acc += fp_result;
            }
            
            /* More arithmetic mixing types */
            if (i % 3 == 0) {
                int_acc += (int)(float_acc * 10.0f);
            }
            
            /* Additional memory barrier in loop body */
            asm volatile("" ::: "memory");
        }
        
        /* Cross-iteration dependency */
        int_acc = int_acc ^ outer;
        float_acc = float_acc + (float)outer;
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
    
    /* Print result to ensure observable side effect */
    printf("Final checksum: int_acc = %d, float_acc = %f\n", int_acc, float_acc);
    
    return 0;
}
