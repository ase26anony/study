/* sel-sched-coverage.c
 * Program designed to trigger GCC's selective scheduler dump logic
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 -o test test.c
 * Or with: gcc -O3 -fsel-sched-pipelining-outer-loops -fdump-rtl-all -dP -o test test.c
 */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable function to create call RTL patterns */
__attribute__((noinline, noipa))
int side_effect_func(int x, float y) {
    static int counter = 0;
    counter++;
    return (x + counter) * (int)(y * 2.0f);
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
    
    /* Memory barrier to prevent pre-scheduling optimization */
    asm volatile("" ::: "memory");
    
    /* Outer loop with fixed iteration count */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with data dependencies */
        for (int i = 0; i < 256; i++) {
            /* Loop-carried dependency on int_acc */
            int temp_int = int_array[i] * (int_acc + 1);
            
            /* Conditional control flow for multi-basic-block scheduling */
            if (temp_int > 0) {
                /* Integer arithmetic path */
                int_acc = int_acc + temp_int;
                
                /* Mixed floating-point operation */
                float temp_float = float_array[i] * (float_acc + 1.0f);
                float_acc = float_acc + temp_float;
                
                /* Volatile store to prevent elimination */
                volatile int store_var = temp_int;
                (void)store_var; /* Use variable to avoid unused warning */
                
                /* Function call with side effects - creates call RTL */
                if (i % 32 == 0) {
                    int func_result = side_effect_func(temp_int, temp_float);
                    int_acc = int_acc + (func_result % 100);
                }
            } else {
                /* Alternative path with different operations */
                int_acc = int_acc - (temp_int % 100);
                float_acc = float_acc - (float_array[i] * 0.5f);
                
                /* Another function call pattern */
                if (i % 64 == 0) {
                    side_effect_func(-temp_int, float_array[i]);
                }
            }
            
            /* Additional arithmetic to create more RTL instructions */
            if (i % 8 == 0) {
                /* Complex expression with multiple operations */
                int complex_op = (int_array[i] * 3) / (int_acc + 1) + 7;
                float complex_float = (float_array[i] * 2.0f) / (float_acc + 1.0f);
                
                /* Memory barrier to prevent reordering */
                asm volatile("" ::: "memory");
                
                int_acc = int_acc ^ complex_op; /* Bitwise operation for variety */
                float_acc = float_acc * (1.0f + complex_float * 0.01f);
            }
        }
        
        /* Loop-carried dependency across outer iterations */
        int_acc = int_acc * 2 - outer;
        float_acc = float_acc * 1.5f + (float)outer;
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
    
    /* Print checksum to ensure side effects are observable */
    printf("Final checksum: int_acc = %d, float_acc = %f\n", 
           (int)int_acc, (float)float_acc);
    
    return 0;
}
