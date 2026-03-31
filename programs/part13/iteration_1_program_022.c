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
    return (x ^ counter) + (int)(y * 100.0f);
}

/* Another non-inlineable function for conditional calls */
__attribute__((noinline))
float fp_side_effect(float a, float b) {
    volatile float result = a * b - a / (b + 1.0f);
    return result;
}

/* Simple LCG for pseudo-random data without external dependencies */
static uint32_t lcg_state = 123456789;
static uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
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
        int_array[i] = (int)(lcg_rand() % 1000) - 500;
        float_array[i] = (float)(lcg_rand() % 1000) / 100.0f - 5.0f;
    }
    
    /* Memory barrier to prevent pre-scheduling optimization */
    asm volatile("" ::: "memory");
    
    /* Outer loop with small iteration count */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with data dependencies and control flow */
        for (int i = 0; i < 256; i++) {
            /* Create loop-carried dependency for scheduling challenge */
            int_acc = int_acc + int_array[i] * (int_acc % 100);
            
            /* Mixed integer operations */
            int temp_int = int_array[i] ^ (i * 7);
            temp_int = temp_int + (outer << 3);
            
            /* Floating-point operations with dependency */
            float_acc = float_acc + float_array[i] * (float_acc + 1.0f);
            
            /* More complex FP operations */
            float temp_float = float_array[i] * 2.5f - float_acc / 3.0f;
            
            /* Control flow to create multiple basic blocks */
            if (temp_int > 0) {
                /* Branch with arithmetic */
                temp_int = temp_int * 3 + 1;
                temp_float = temp_float * 1.5f;
                
                /* Conditional function call */
                if ((i % 7) == 0) {
                    volatile int call_result = side_effect_func(temp_int, temp_float);
                    int_acc += call_result % 17;
                }
            } else {
                /* Alternative branch with different operations */
                temp_int = (temp_int << 2) | 1;
                temp_float = temp_float / 2.0f + 1.0f;
                
                /* Different function call pattern */
                if ((i % 5) == 0) {
                    volatile float fp_result = fp_side_effect(temp_float, float_acc);
                    float_acc += fp_result * 0.1f;
                }
            }
            
            /* Additional nested if for more block complexity */
            if ((i % 13) == 0) {
                int_acc = int_acc ^ (temp_int & 0xFF);
                float_acc = float_acc - temp_float * 0.25f;
            }
            
            /* Store to volatile to ensure operations aren't eliminated */
            volatile int store_int = temp_int;
            volatile float store_float = temp_float;
            
            /* Compiler barrier to prevent reordering before scheduling */
            asm volatile("" ::: "memory");
        }
        
        /* Cross-iteration dependency */
        int_acc = int_acc * 2 - outer;
        float_acc = float_acc * 1.1f + (float)outer;
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
    
    /* Print checksum to ensure observable side effects */
    printf("Final checksum: int=%d, float=%.2f\n", 
           (int)(int_acc % 1000000), 
           float_acc);
    
    return 0;
}
