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
    static int counter = 0;
    counter++;
    /* Create side effects */
    asm volatile("" : "+r"(x) : : "memory");
    return x + (int)y + counter;
}

/* Another non-inlineable function for conditional calls */
__attribute__((noinline))
float fp_side_effect(float a, float b) {
    volatile static float accum = 0.0f;
    accum += a * b;
    return accum;
}

/* Simple PRNG without external dependencies */
static uint32_t lcg_seed = 123456789;
static uint32_t lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

int main(void) {
    /* Arrays with different data types */
    int int_array[256];
    float float_array[256];
    
    /* Volatile accumulators to prevent optimization */
    volatile int int_acc = 0;
    volatile float float_acc = 0.0f;
    volatile int temp_result;
    
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
            /* Loop-carried dependency on int_acc */
            int_acc = int_acc + int_array[i] * (i + 1);
            
            /* Mixed integer operations */
            temp_result = int_array[i] ^ (int_acc & 0xFF);
            
            /* Control flow for multi-basic-block scheduling */
            if (temp_result > 500) {
                /* Branch with floating-point operations */
                float_acc = float_acc + float_array[i] * 1.5f;
                
                /* Call non-inlineable function */
                int_acc = side_effect_func(int_acc, float_acc);
            } else {
                /* Alternative branch with different operations */
                float_acc = float_acc - float_array[i] * 0.5f;
                
                /* More complex integer arithmetic */
                int_acc = int_acc ^ (int_array[i] << 2);
            }
            
            /* Additional floating-point operation in all paths */
            float intermediate = float_array[i] * float_acc;
            
            /* Conditional function call based on loop index */
            if ((i % 3) == 0) {
                intermediate = fp_side_effect(intermediate, float_acc);
            }
            
            /* Memory barrier to prevent reordering */
            asm volatile("" ::: "memory");
            
            /* Store to volatile to ensure side effect */
            float_acc = intermediate * 0.99f;
            
            /* Nested if for additional basic blocks */
            if ((i % 7) == 0) {
                int_acc += outer * 100;
                asm volatile("" ::: "memory");
            }
        }
        
        /* Outer loop memory barrier */
        asm volatile("" ::: "memory");
        
        /* Cross-iteration dependency */
        int_acc = int_acc ^ (outer * 0xABCD);
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
    
    /* Print checksum to prevent dead code elimination */
    printf("Final checksum: int_acc = %d, float_acc = %f\n", 
           int_acc, float_acc);
    
    return 0;
}
