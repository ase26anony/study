/* sel-sched-coverage.c
 * Program designed to trigger GCC's selective scheduler dump logic
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 sel-sched-coverage.c -o sel-sched-coverage
 * Or with: gcc -O3 -fsel-sched-pipelining-outer-loops -fdump-rtl-all sel-sched-coverage.c -o sel-sched-coverage
 */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable function to create call RTL patterns */
__attribute__((noinline)) 
int side_effect_func(int x, float y) {
    static int counter = 0;
    counter++;
    /* Create side effects */
    asm volatile("" : "+r"(x) : : "memory");
    return x + (int)y + counter;
}

/* Another noinline function for conditional calls */
__attribute__((noinline))
float fp_side_effect(float a, float b) {
    volatile static float accum = 0.0f;
    accum += a * b;
    return accum;
}

/* Simple PRNG without external dependencies */
static uint32_t prng_state = 123456789;

static uint32_t simple_rand(void) {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
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
        int_array[i] = (int)(simple_rand() % 1000);
        float_array[i] = (float)(simple_rand() % 1000) / 100.0f;
    }
    
    /* Memory barrier to prevent pre-scheduling optimization */
    asm volatile("" ::: "memory");
    
    /* Outer loop with small iteration count */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with data dependencies */
        for (int i = 0; i < 256; i++) {
            /* Loop-carried dependency on int_acc */
            int temp_int = int_array[i] * (int_acc + 1);
            
            /* Control flow to create multiple basic blocks */
            if (temp_int > 500) {
                /* Call non-inlineable function in one path */
                int_acc += side_effect_func(temp_int, float_array[i]);
                
                /* Floating-point operation */
                float temp_float = float_array[i] * (float_acc + 1.0f);
                float_acc += temp_float;
                
                /* Conditional function call */
                if (i % 3 == 0) {
                    float_acc = fp_side_effect(float_acc, float_array[i]);
                }
            } else {
                /* Different operations in the else path */
                int_acc += temp_int / 2;
                float_acc += float_array[i] / 2.0f;
                
                /* Mixed integer/float operations */
                int mixed_op = (int)(float_array[i] * 10.0f) + int_array[i];
                int_acc += mixed_op % 100;
            }
            
            /* Additional arithmetic to create scheduling complexity */
            for (int j = 0; j < 2; j++) {  /* Very small inner-inner loop */
                int_acc += (int_array[i] >> j) & 1;
                float_acc += (float)((int_array[i] >> j) & 1) * 0.1f;
            }
            
            /* Memory barrier to prevent reordering */
            asm volatile("" ::: "memory");
        }
        
        /* Cross-iteration dependency */
        int_acc = int_acc % 10000;
        float_acc = float_acc - (float)((int)float_acc);
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
    
    /* Print checksum to prevent dead code elimination */
    printf("Final checksum: int=%d, float=%.2f\n", int_acc, float_acc);
    
    return 0;
}
