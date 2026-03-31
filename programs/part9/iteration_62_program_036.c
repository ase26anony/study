/* modulo-sched-coverage.c
 * Designed to trigger GCC's modulo scheduler debug output for PSG move calculations.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -dP -march=armv7-a modulo-sched-coverage.c -o modulo-sched-coverage
 */

#include <stdint.h>

#define SIZE 1024
#define THRESHOLD 1000000

/* Prevent optimization of critical variables */
static volatile int force_volatile = 0;

/* Noinline function to prevent dead code elimination */
__attribute__((noinline)) 
static void consume_result(volatile int *arr, int size) {
    volatile int sink = 0;
    for (int i = 0; i < size; i++) {
        sink += arr[i];
    }
    force_volatile = sink;
}

/* Simple LCG to generate pseudo-random values without external dependencies */
static inline int pseudo_rand(int *seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return *seed;
}

int main(void) {
    /* Volatile arrays to prevent optimization */
    volatile int a[SIZE];
    volatile int b[SIZE];
    volatile int c[SIZE];
    
    /* Volatile loop counters to prevent constant propagation */
    volatile int outer_bound = 5;
    volatile int inner_bound = SIZE;
    volatile int i, j;
    
    int seed = 42;
    
    /* Initialize arrays with pseudo-random values */
    for (i = 0; i < SIZE; i++) {
        a[i] = pseudo_rand(&seed) % 1000;
        b[i] = pseudo_rand(&seed) % 1000;
        c[i] = pseudo_rand(&seed) % 1000;
    }
    
    /* Complex nested loop designed to trigger aggressive modulo scheduling */
    for (j = 0; j < outer_bound; j++) {
        /* Count-down loop with volatile bound */
        volatile int count = inner_bound;
        
        /* Multiple volatile intermediates to create register pressure */
        volatile int temp1, temp2, temp3;
        volatile int accumulator = a[0];
        
        /* Inline assembly to clobber registers and increase pressure */
        __asm__ volatile (
            "mov r0, %0\n\t"
            "mov r1, %1\n\t"
            : 
            : "r" (j), "r" (count)
            : "r0", "r1", "memory"
        );
        
        /* Inner loop with complex data dependencies */
        for (i = count; i > 0; i--) {
            /* Chain of arithmetic operations with loop-carried dependency */
            temp1 = b[i] * c[i];
            temp2 = temp1 + accumulator;          /* RAW dependency on accumulator */
            temp3 = temp2 % 7919;                 /* Prime modulo to prevent optimization */
            
            /* Anti-dependency: read before write with volatile */
            accumulator = a[i] + temp3;           /* WAW and WAR dependencies */
            a[i] = accumulator;
            
            /* Additional inline assembly to consume registers */
            __asm__ volatile (
                "add r2, %0, %1\n\t"
                "sub r3, %2, %0\n\t"
                : 
                : "r" (temp1), "r" (temp2), "r" (temp3)
                : "r2", "r3"
            );
            
            /* Conditional break to create multiple exit points */
            if (accumulator > THRESHOLD) {
                /* Force complex control flow */
                volatile int should_break = (accumulator & 0xFF) > 200;
                if (should_break) {
                    break;
                }
            }
            
            /* Additional arithmetic to increase II */
            temp1 = temp1 ^ temp2;
            temp2 = temp2 * 3;
            temp3 = temp3 / 2;
            
            /* More inline assembly with clobbers */
            __asm__ volatile (
                "mul r4, %0, %1\n\t"
                : 
                : "r" (temp1), "r" (temp2)
                : "r4", "memory"
            );
        }
        
        /* Cross-iteration dependency through array */
        if (j > 0) {
            a[0] = a[SIZE-1] + j;
        }
    }
    
    /* Consume results to prevent dead code elimination */
    consume_result(a, SIZE);
    
    /* Return checksum */
    int checksum = 0;
    for (i = 0; i < SIZE; i++) {
        checksum ^= a[i];
    }
    
    return checksum & 0xFF;
}
