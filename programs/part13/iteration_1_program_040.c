/* sel-sched-coverage.c
 * Program designed to trigger GCC's selective scheduler dump logic
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 -o test test.c
 * Or more aggressively: gcc -O3 -fsel-sched-pipelining-outer-loops -fdump-rtl-all -fdump-rtl-sched_details -o test test.c
 */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable function to create call RTL patterns */
__attribute__((noinline, noipa))
void side_effect_func(int val, float fval) {
    /* Use volatile to prevent optimization */
    volatile static int counter = 0;
    counter += val;
    (void)fval; /* Use parameter to prevent removal */
}

/* Simple PRNG without external dependencies */
static uint32_t lcg_seed = 123456789;
static uint32_t lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

int main(void) {
    /* Arrays with different types to create diverse RTL */
    int int_array[256];
    float float_array[256];
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < 256; i++) {
        int_array[i] = (int)(lcg_rand() % 1000);
        float_array[i] = (float)(lcg_rand() % 1000) * 0.001f;
    }
    
    /* Volatile accumulators to prevent optimization */
    volatile int int_acc = 0;
    volatile float float_acc = 0.0f;
    
    /* Memory barrier to prevent reordering before scheduling */
    asm volatile("" ::: "memory");
    
    /* Outer loop - fixed small iteration count */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with data dependencies */
        for (int i = 0; i < 256; i++) {
            /* Create loop-carried dependency */
            int_acc = int_acc + int_array[i] * (int_acc % 100 + 1);
            
            /* Floating-point operation with dependency */
            float_acc = float_acc + float_array[i] * (float_acc * 0.5f + 1.0f);
            
            /* Control flow to create multiple basic blocks */
            if (int_acc % 3 == 0) {
                /* Branch 1: More integer operations */
                int temp = int_array[i] * 2;
                int_acc = int_acc - temp;
                
                /* Call function with side effects */
                side_effect_func(temp, float_acc);
            } else if (int_acc % 3 == 1) {
                /* Branch 2: Mixed operations */
                float temp_f = float_array[i] * 3.14f;
                float_acc = float_acc - temp_f;
                
                /* Memory barrier inside branch */
                asm volatile("" ::: "memory");
            } else {
                /* Branch 3: Complex expression */
                int_acc = int_acc ^ (int_array[i] << 2);
                float_acc = float_acc * 0.99f + float_array[i];
            }
            
            /* Additional operation to create scheduling complexity */
            if (i % 16 == 0) {
                int_acc = int_acc | 0x1;
                float_acc = float_acc + 0.001f;
            }
        }
        
        /* Cross-iteration dependency */
        int_acc = int_acc * 2 - outer;
        float_acc = float_acc * 1.5f;
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
    
    /* Print result to ensure side effects */
    printf("Final checksum: int_acc = %d, float_acc = %f\n", 
           int_acc, (double)float_acc);
    
    return 0;
}
