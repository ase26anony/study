/* modulo-sched-coverage.c
 * Designed to trigger GCC's modulo scheduler debug output for PSG move calculations
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -dP -march=armv7-a modulo-sched-coverage.c -o modulo-sched-coverage
 * Or for x86: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -funroll-loops -fdump-rtl-all -march=x86-64 modulo-sched-coverage.c -o modulo-sched-coverage
 */

#include <stdint.h>

#define ARRAY_SIZE 1024
#define OUTER_ITERATIONS 5
#define THRESHOLD 1000000

/* Prevent dead code elimination */
__attribute__((noinline)) 
static void consume_result(volatile int* arr, int size, volatile int* result) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
        /* Force memory access */
        __asm__ volatile ("" : : "r"(arr[i]) : "memory");
    }
    *result = sum;
}

/* Simple LCG to generate pseudo-random values without external dependencies */
static inline int lcg_rand(int* state) {
    *state = (*state * 1103515245 + 12345) & 0x7fffffff;
    return *state;
}

int main(void) {
    /* Volatile to prevent optimization and create anti-dependencies */
    volatile int a[ARRAY_SIZE];
    volatile int b[ARRAY_SIZE];
    volatile int c[ARRAY_SIZE];
    
    /* Volatile loop counters to prevent constant propagation */
    volatile int outer_iter = OUTER_ITERATIONS;
    volatile int n = ARRAY_SIZE;
    
    /* Initialize with pseudo-random values */
    int seed = 42;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        a[i] = lcg_rand(&seed) % 100;
        b[i] = lcg_rand(&seed) % 100;
        c[i] = lcg_rand(&seed) % 100;
    }
    
    /* Complex chain of operations to create loop-carried dependencies */
    volatile int temp1, temp2, temp3;
    volatile int accumulator = 0;
    
    /* Outer loop to increase scheduling pressure */
    for (volatile int outer = 0; outer < outer_iter; outer++) {
        
        /* Count-down inner loop - affects scheduler heuristics */
        for (volatile int i = n - 1; i > 0; i--) {
            
            /* Multiple volatile intermediates create anti-dependencies */
            temp1 = b[i] * c[i];
            
            /* Inline assembly to consume registers and increase pressure */
            __asm__ volatile (
                "add %0, %0, %1\n\t"
                : "+r" (temp1)
                : "r" (a[i-1])
                : "r0", "r1", "r2", "r3", "memory"
            );
            
            /* Loop-carried dependency chain */
            a[i] = temp1 + a[i-1] + (accumulator % 17);
            
            /* More arithmetic with volatile intermediates */
            temp2 = a[i] * 3;
            temp3 = temp2 - b[i];
            
            /* Another inline assembly with register clobbers */
            __asm__ volatile (
                "mul %0, %0, %0\n\t"
                : "+r" (temp3)
                :
                : "r4", "r5", "r6", "r7", "memory"
            );
            
            /* Update accumulator with complex operation */
            accumulator = (accumulator * 7 + temp3) % 1001;
            
            /* Conditional break with volatile condition - creates multiple exit points */
            if (accumulator > THRESHOLD) {
                /* Force register pressure before break */
                __asm__ volatile ("" : : "r"(temp1), "r"(temp2), "r"(temp3) : "memory");
                break;
            }
            
            /* Additional operation with modulo - creates resource contention */
            b[i] = (b[i] * 13 + i) % 97;
            
            /* More register pressure */
            __asm__ volatile (
                "and %0, %0, #255\n\t"
                : "+r" (c[i])
                :
                : "r8", "r9", "r10", "memory"
            );
        }
        
        /* Mix up data between outer iterations */
        for (volatile int j = 1; j < ARRAY_SIZE/4; j++) {
            a[j] = (a[j] + b[j*2] + c[j*3]) % 1000;
        }
    }
    
    /* Prevent dead code elimination */
    volatile int checksum = 0;
    consume_result((volatile int*)a, ARRAY_SIZE, &checksum);
    
    return checksum % 256;
}
