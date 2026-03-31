/* modulo-sched-coverage.c
 * Designed to trigger GCC's modulo scheduling debug output for PSG move calculations
 * Compile with: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -fdump-rtl-sms -dP -march=armv7-a -o test test.c
 * Or for x86: gcc -O3 -fmodulo-sched -funroll-loops -fdump-rtl-sms -dP -march=x86-64 -o test test.c
 */

#include <stdint.h>

#define SIZE 1024
#define OUTER_ITER 5
#define THRESHOLD 0x7FFFFFFF

/* Prevent dead code elimination */
__attribute__((noinline)) 
int consume_result(volatile int* arr, int n) {
    volatile int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
        __asm__ volatile ("" : : "r"(sum) : "r0", "r1", "r2", "r3");
    }
    return sum;
}

/* Simple LCG to generate pseudo-random values without external dependencies */
static inline uint32_t lcg(uint32_t* state) {
    *state = *state * 1103515245 + 12345;
    return *state;
}

int main(void) {
    /* Volatile to prevent optimization and create anti-dependencies */
    volatile int array_a[SIZE];
    volatile int array_b[SIZE];
    volatile int array_c[SIZE];
    
    /* Volatile loop counters to prevent constant propagation */
    volatile int outer_count = OUTER_ITER;
    volatile int inner_bound = SIZE;
    
    /* Initialize arrays with pseudo-random values */
    uint32_t seed = 42;
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = (int)(lcg(&seed) % 1000);
        array_b[i] = (int)(lcg(&seed) % 1000);
        array_c[i] = (int)(lcg(&seed) % 100) + 1;
    }
    
    int result = 0;
    
    /* Outer loop with volatile bound */
    for (int outer = 0; outer < outer_count; outer++) {
        volatile int temp1, temp2, temp3;
        volatile int accumulator = array_a[0];
        
        /* Complex inner loop with loop-carried dependencies */
        /* Count-down loop to affect scheduler heuristics */
        for (int i = inner_bound - 1; i > 0; i--) {
            /* Multiple arithmetic operations with dependencies */
            temp1 = array_b[i] * array_c[i];
            __asm__ volatile ("" : : "r"(temp1) : "r0", "r1");
            
            temp2 = temp1 + accumulator;
            __asm__ volatile ("" : : "r"(temp2) : "r2", "r3");
            
            temp3 = temp2 % array_c[i];
            __asm__ volatile ("" : : "r"(temp3) : "r4", "r5");
            
            /* Loop-carried dependency */
            array_a[i] = temp3 + array_a[i-1];
            
            /* Additional operations to increase register pressure */
            volatile int extra1 = array_b[i-1] * 3;
            volatile int extra2 = array_c[i] * 7;
            volatile int extra3 = extra1 + extra2;
            
            __asm__ volatile ("" : : "r"(extra3) : "r6", "r7", "r8", "r9", "r10");
            
            /* Conditional break with volatile condition */
            volatile int break_cond = array_a[i];
            if (break_cond > THRESHOLD) {
                __asm__ volatile ("" : : "r"(break_cond) : "r11", "r12");
                break;
            }
            
            /* Update accumulator with complex expression */
            accumulator = (accumulator * 3 + array_a[i]) % 997;
            
            /* More inline assembly to consume registers */
            __asm__ volatile (""
                : 
                : "r"(accumulator), "r"(array_a[i]), "r"(array_b[i])
                : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
                  "r8", "r9", "r10", "r11", "r12", "r14");
        }
        
        /* Cross-iteration dependency in outer loop */
        array_a[0] = accumulator + outer;
    }
    
    /* Force use of results to prevent elimination */
    result = consume_result((volatile int*)array_a, 100);
    
    return result & 0xFF;  /* Return non-zero checksum */
}
