/* sel-sched-coverage.c
 * Program designed to trigger GCC's selective scheduler dump logic
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 -o test test.c
 * Or with: gcc -O3 -fsel-sched-pipelining-outer-loops -fdump-rtl-all -dP -o test test.c
 */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable function to create call RTL patterns */
__attribute__((noinline, noipa))
void side_effect_func(int val, float fval) {
    /* Use volatile to prevent optimization */
    volatile int sink = val;
    volatile float fsink = fval;
    (void)sink;
    (void)fsink;
}

/* Another non-inlineable function for conditional calls */
__attribute__((noinline, noipa))
int compute_mod(int x, int y) {
    volatile int result = x % (y + 1);
    return result;
}

/* Simple PRNG without external dependencies */
static uint32_t lcg_state = 123456789;
static uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
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
        int_array[i] = (int)(lcg_rand() % 1000);
        float_array[i] = (float)(lcg_rand() % 1000) / 100.0f;
    }
    
    /* Memory barrier to prevent pre-scheduling optimization */
    asm volatile("" ::: "memory");
    
    /* Outer loop with small iteration count */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with data dependencies */
        for (int i = 0; i < 256; i++) {
            /* Create loop-carried dependency */
            int_acc = int_acc + int_array[i] * (int_acc % 7);
            
            /* Mixed integer operations */
            int temp = int_array[i] * 3;
            temp = temp - (i << 2);
            
            /* Floating-point operations with dependency */
            float_acc = float_acc + float_array[i] * (float_acc + 1.0f);
            
            /* Control flow for multi-basic-block scheduling */
            if (i % 3 == 0) {
                /* Branch 1: More integer operations */
                int_acc = int_acc ^ (temp & 0xFF);
                float_acc = float_acc * 1.01f;
                
                /* Function call with side effects */
                side_effect_func(int_acc, float_acc);
            } else if (i % 3 == 1) {
                /* Branch 2: Different operations */
                int_acc = int_acc | (int_array[i] << 3);
                float_acc = float_acc - float_array[i] * 0.5f;
                
                /* Another function call pattern */
                if (int_acc % 5 == 0) {
                    int mod_result = compute_mod(int_acc, i);
                    int_acc = int_acc + mod_result;
                }
            } else {
                /* Branch 3: Yet another pattern */
                int_acc = int_acc & ~(0xFF << (i % 8));
                float_acc = float_acc / (float_array[i] + 1.0f);
                
                /* Memory operation */
                volatile int mem_sink = int_array[i];
                (void)mem_sink;
            }
            
            /* Additional arithmetic to create more RTL patterns */
            if (i % 8 == 0) {
                float_acc = float_acc + (float)(int_acc) * 0.001f;
            }
            
            /* Compiler barrier to prevent reordering */
            asm volatile("" ::: "memory");
        }
        
        /* Cross-iteration dependency */
        int_acc = int_acc * 2 - outer;
        float_acc = float_acc * 0.99f + (float)outer;
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
    
    /* Print checksum to prevent dead code elimination */
    printf("Final checksum: int=%d, float=%.2f\n", int_acc, float_acc);
    
    return 0;
}
