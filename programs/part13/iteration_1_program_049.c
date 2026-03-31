/* sel-sched-coverage.c
 * Program designed to trigger GCC's selective scheduler dump logic
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 sel-sched-coverage.c -o sel-sched-coverage
 * For more detailed dumps: gcc -O3 -fsel-sched-pipelining-outer-loops -fdump-rtl-all -fdump-rtl-sched_details sel-sched-coverage.c -o sel-sched-coverage
 */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable function to create call RTL patterns */
__attribute__((noinline, noipa))
int side_effect_func(int x, float y) {
    volatile int result = x + (int)y;
    asm volatile("" : "+r"(result) : : "memory");
    return result;
}

/* Another non-inlineable function for control flow */
__attribute__((noinline, noipa))
float conditional_transform(float a, int b) {
    volatile float temp = a;
    if (b & 1) {
        temp *= 2.0f;
    } else {
        temp /= 2.0f;
    }
    asm volatile("" : "+f"(temp) : : "memory");
    return temp;
}

/* Simple PRNG without external dependencies */
static uint32_t lcg_state = 123456789;
static uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

int main(void) {
    /* Initialize arrays with pseudo-random data */
    const int ARRAY_SIZE = 256;
    int int_array[ARRAY_SIZE];
    float float_array[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = (int)(lcg_rand() % 1000);
        float_array[i] = (float)(lcg_rand() % 1000) / 10.0f;
    }
    
    /* Volatile accumulators to prevent optimization */
    volatile int int_acc = 0;
    volatile float float_acc = 0.0f;
    volatile int temp_store = 0;
    
    /* Memory barrier before loop */
    asm volatile("" ::: "memory");
    
    /* Outer loop with fixed iteration count */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with data dependencies */
        for (int i = 0; i < ARRAY_SIZE; i++) {
            /* Integer operations with loop-carried dependency */
            int load_int = int_array[i];
            int_acc = int_acc + load_int * (int_acc % 100 + 1);
            
            /* Floating-point operations */
            float load_float = float_array[i];
            float_acc = float_acc + load_float * (float_acc + 1.0f);
            
            /* Mixed-type operations */
            temp_store = (int)(load_float * 100.0f) + load_int;
            
            /* Control flow creating multiple basic blocks */
            if (i % 3 == 0) {
                /* Call non-inlineable function */
                int func_result = side_effect_func(load_int, load_float);
                temp_store += func_result;
                
                /* More floating-point with conditional */
                float transformed = conditional_transform(load_float, load_int);
                float_acc += transformed;
            } else if (i % 3 == 1) {
                /* Different arithmetic mix */
                int_acc = int_acc ^ (load_int * 3);
                float_acc = float_acc - (load_float / 2.0f);
            } else {
                /* Yet another path */
                temp_store = temp_store * 2 - load_int;
                float_acc = float_acc * 1.5f;
            }
            
            /* Memory barrier to prevent reordering */
            asm volatile("" ::: "memory");
            
            /* Additional arithmetic to create more RTL patterns */
            if (i % 8 == 0) {
                volatile int special = int_array[(i + 1) % ARRAY_SIZE];
                int_acc = int_acc + special * special;
            }
        }
        
        /* Cross-iteration dependency */
        int_acc = int_acc ^ (outer * 0x5A5A5A5A);
        float_acc = float_acc + (float)outer * 0.1f;
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
    
    /* Print checksum to prevent dead code elimination */
    printf("Final checksum: int=%d, float=%f\n", int_acc, float_acc);
    
    return 0;
}
