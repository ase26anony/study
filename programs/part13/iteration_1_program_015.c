/* sel-sched-coverage.c
 * Program designed to trigger GCC selective scheduler dump logic
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 -o test test.c
 * Or more aggressively: gcc -O3 -fsel-sched-pipelining-outer-loops -fdump-rtl-all -fdump-rtl-sched_details -o test test.c
 */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable function to create call RTL patterns */
__attribute__((noinline, noipa))
void side_effect_func(int value, float fvalue) {
    /* Use asm to prevent optimization */
    asm volatile("" : : "r"(value), "r"(fvalue) : "memory");
}

/* Simple LCG PRNG to avoid external dependencies */
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
        float_array[i] = (float)(lcg_rand() % 1000) * 0.001f;
    }
    
    /* Memory barrier to prevent reordering before scheduling */
    asm volatile("" ::: "memory");
    
    /* Outer loop with fixed iteration count */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with data dependencies */
        for (int i = 0; i < 256; i++) {
            /* Control flow to create multiple basic blocks */
            if (i % 3 == 0) {
                /* Path 1: Integer-heavy operations */
                int temp = int_array[i];
                int_acc = int_acc + temp * (int_acc & 0xFF);  /* Loop-carried dependency */
                
                /* Mixed floating-point operation */
                float ftemp = float_array[i];
                float_acc = float_acc + ftemp * (float_acc * 0.5f);
                
                /* Function call with side effects */
                if (i % 7 == 0) {
                    side_effect_func(int_acc, float_acc);
                }
            } else if (i % 3 == 1) {
                /* Path 2: Different operation mix */
                int temp = int_array[i] * 2;
                int_acc = int_acc - temp + (i * outer);  /* Use outer loop variable */
                
                float ftemp = float_array[i] * 2.0f;
                float_acc = float_acc - ftemp + (i * 0.01f);
                
                /* Volatile store to prevent dead code elimination */
                volatile int dummy = temp;
                (void)dummy;
            } else {
                /* Path 3: More complex operations */
                int temp = int_array[i] + int_array[(i + 1) % 256];
                int_acc = int_acc ^ temp;  /* Bitwise operation for variety */
                
                float ftemp = float_array[i] + float_array[(i + 1) % 256];
                float_acc = float_acc * 0.99f + ftemp;
                
                /* Conditional function call */
                if (int_acc % 11 == 0) {
                    side_effect_func(i, ftemp);
                }
            }
            
            /* Additional arithmetic to increase scheduling complexity */
            if (i % 13 == 0) {
                int_acc = int_acc + (int_acc >> 3);
                float_acc = float_acc + 1.0f;
            }
            
            /* Compiler barrier to prevent optimization across iterations */
            asm volatile("" ::: "memory");
        }
        
        /* Cross-iteration dependency */
        int_acc = int_acc + outer * 1000;
        float_acc = float_acc + outer * 10.0f;
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
    
    /* Print checksum to ensure side effects */
    printf("Final checksum: int=%d, float=%f\n", int_acc, float_acc);
    
    return 0;
}
