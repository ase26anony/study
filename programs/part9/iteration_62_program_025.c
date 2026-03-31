/* modulo-sched-coverage.c
 * Designed to trigger uncovered lines in GCC's modulo-sched.cc
 * Specifically targets lines 596-606 in the PSG move verification logic
 */

#include <stdint.h>

/* Prevent dead code elimination */
__attribute__((noinline)) 
void consume_result(volatile int* arr, int size, volatile int* sink) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
        __asm__ volatile ("" : : "r"(arr[i]) : "memory");
    }
    *sink = sum;
}

/* Simple LCG for pseudo-random values without external dependencies */
static inline int lcg_rand(int* state) {
    *state = (*state * 1103515245 + 12345) & 0x7fffffff;
    return *state;
}

int main(void) {
    /* Volatile to prevent optimization and create anti-dependencies */
    volatile int N = 1000;
    volatile int outer_bound = 5;
    volatile int threshold = 0x70000000;
    volatile int sink = 0;
    
    /* Arrays with volatile accesses to create memory dependencies */
    volatile int a[1002] = {0};
    volatile int b[1002] = {0};
    volatile int c[1002] = {0};
    
    /* Initialize with pseudo-random values */
    int seed = 42;
    for (int i = 0; i < 1002; i++) {
        a[i] = lcg_rand(&seed) % 100;
        b[i] = lcg_rand(&seed) % 100;
        c[i] = lcg_rand(&seed) % 100;
    }
    
    /* Complex nested loop structure to force aggressive modulo scheduling */
    volatile int i, j;
    volatile int temp1, temp2, temp3;
    
    for (j = outer_bound; j > 0; j--) {  /* Count-down outer loop */
        
        /* Multiple volatile intermediates for register pressure */
        volatile int accum = a[0];
        volatile int mod_base = 97;
        
        /* Inner loop with loop-carried dependencies and complex operations */
        for (i = N; i > 0; i--) {
            /* Chain of arithmetic operations with dependencies */
            temp1 = b[i] * c[i];
            temp2 = temp1 + accum;           /* Loop-carried dependency */
            temp3 = temp2 % mod_base;
            a[i] = temp3 + a[i-1];           /* Another loop-carried dependency */
            
            /* Inline assembly to consume registers and create pressure */
            __asm__ volatile (
                "add %0, %0, %1\n\t"
                "mul %0, %0, %2\n\t"
                : "+r" (temp3)
                : "r" (temp1), "r" (temp2)
                : "r0", "r1", "r2", "r3", "cc", "memory"
            );
            
            /* Multiple exit points to affect control flow */
            if (temp3 > threshold) {
                /* Early exit based on volatile computation */
                break;
            }
            
            /* Additional operations to increase II */
            accum = (accum * 3 + a[i]) % 256;
            
            /* More inline assembly with different clobbers */
            __asm__ volatile (
                "eor %0, %0, %1\n\t"
                : "+r" (accum)
                : "r" (i)
                : "r4", "r5", "cc"
            );
            
            /* Second conditional break for more complex CFG */
            if (accum < 0 && i < N/2) {
                break;
            }
            
            /* Cross-iteration dependency through array */
            b[i-1] = (b[i-1] + a[i]) % 100;
        }
        
        /* Modify loop variables to affect next iteration */
        N = (N > 100) ? N - 50 : 1000;
        threshold = threshold + 0x01000000;
    }
    
    /* Force result consumption to prevent elimination */
    consume_result((volatile int*)a, 1000, &sink);
    
    return sink & 0xFF;  /* Return checksum */
}
