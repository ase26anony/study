/* sel-sched-coverage.c
 * Program designed to trigger GCC's selective scheduler dump logic
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 -o test test.c
 * Or with: gcc -O3 -fsel-sched-pipelining-outer-loops -fdump-rtl-all -o test test.c
 */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable function to create call RTL patterns */
__attribute__((noinline, noipa))
void side_effect_func(int val, float fval) {
    /* Use asm to prevent optimization */
    asm volatile("" : : "r"(val), "r"(fval) : "memory");
}

/* Another non-inlineable function for conditional calls */
__attribute__((noinline, noipa))
int compute_hash(int a, int b) {
    /* Simple non-linear operation */
    return (a ^ (b << 3)) + (a >> 2);
}

int main(void) {
    /* Initialize arrays with pseudo-random data */
    const int SIZE = 256;
    int int_array[SIZE];
    float float_array[SIZE];
    
    /* Simple LCG for deterministic pseudo-random values */
    uint32_t seed = 123456789;
    for (int i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        int_array[i] = (int)(seed % 1000);
        float_array[i] = (float)(seed % 1000) * 0.001f;
    }
    
    /* Volatile accumulators to prevent optimization */
    volatile int int_acc = 0;
    volatile float float_acc = 0.0f;
    volatile int temp_result = 0;
    
    /* Memory barrier to prevent reordering before scheduling */
    asm volatile("" ::: "memory");
    
    /* Outer loop with small iteration count */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with data dependencies */
        for (int i = 0; i < SIZE; i++) {
            /* Loop-carried dependency on int_acc */
            int load_val = int_array[i];
            
            /* Mixed integer operations */
            int mult_result = load_val * (int_acc + 1);
            int_acc = int_acc + mult_result;
            
            /* Floating-point operations */
            float fmult = float_array[i] * (float_acc + 1.0f);
            float_acc = float_acc + fmult;
            
            /* Store to volatile to prevent elimination */
            temp_result = mult_result;
            
            /* Control flow creating multiple basic blocks */
            if (i % 3 == 0) {
                /* Branch with function call */
                side_effect_func(mult_result, fmult);
                
                /* More complex arithmetic in this branch */
                int hash = compute_hash(mult_result, i);
                int_acc = int_acc ^ hash;
            } else if (i % 7 == 0) {
                /* Another branch with different operations */
                float_acc = float_acc * 0.99f;
                int_acc = int_acc - (load_val >> 2);
            } else {
                /* Default branch with memory barrier */
                asm volatile("" ::: "memory");
                int_acc = int_acc + (load_val % 17);
            }
            
            /* Additional dependency chain */
            if (i > 0) {
                int prev_val = int_array[i-1];
                float_acc = float_acc + (float)prev_val * 0.01f;
            }
        }
        
        /* Inter-iteration memory barrier */
        asm volatile("" ::: "memory");
        
        /* Cross-iteration dependency */
        int_acc = int_acc ^ (outer * 0x5A5A5A5A);
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
    
    /* Print checksum to ensure side effects */
    printf("Final checksum: int_acc = %d, float_acc = %f\n", 
           (int)int_acc, (double)float_acc);
    
    /* Additional volatile store to prevent dead code elimination */
    volatile int final_check = (int)int_acc;
    
    return final_check & 0xFF;
}

/* Additional function to increase scheduling complexity */
__attribute__((noinline))
void helper_function(volatile int* ptr, float* fptr) {
    /* Complex operations that scheduler must handle */
    for (int i = 0; i < 10; i++) {
        *ptr = *ptr * 3 + i;
        *fptr = *fptr * 1.5f + (float)i;
        asm volatile("" ::: "memory");
    }
}
