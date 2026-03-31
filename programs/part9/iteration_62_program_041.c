/* modulo-sched-coverage.c
 * Designed to trigger GCC's modulo scheduler debug prints for PSG moves.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -dP -march=armv7-a modulo-sched-coverage.c -o modulo-sched-coverage
 * Or for x86: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -funroll-loops -fdump-rtl-sms -dP -march=x86-64 modulo-sched-coverage.c -o modulo-sched-coverage
 */

#include <stdint.h>

#define ARRAY_SIZE 1024
#define OUTER_ITER 5
#define BREAK_THRESHOLD 0x7FFFFFFF

/* Prevent dead code elimination */
__attribute__((noinline)) 
static void consume_result(volatile int* arr, int size, volatile int* sink) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
        /* Add inline asm to prevent optimization */
        __asm__ volatile ("" : "+r" (sum) : : "r0", "r1", "r2", "r3");
    }
    *sink = sum;
}

/* Simple LCG to generate pseudo-random values without external dependencies */
static inline int lcg(int* state) {
    *state = (*state * 1103515245 + 12345) & 0x7FFFFFFF;
    return *state;
}

int main(void) {
    /* Volatile to prevent optimization and create anti-dependencies */
    volatile int a[ARRAY_SIZE];
    volatile int b[ARRAY_SIZE];
    volatile int c[ARRAY_SIZE];
    volatile int sink = 0;
    
    /* Volatile iteration counters to prevent constant propagation */
    volatile int outer_bound = OUTER_ITER;
    volatile int inner_bound = ARRAY_SIZE;
    
    /* Initialize with pseudo-random values */
    int seed = 42;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        a[i] = lcg(&seed) % 1000;
        b[i] = lcg(&seed) % 1000;
        c[i] = lcg(&seed) % 1000;
    }
    
    /* Complex nested loop designed to trigger aggressive modulo scheduling */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Count-down loop with volatile counter to affect scheduler heuristics */
        volatile int i = inner_bound - 1;
        
        while (i > 0) {
            /* Create loop-carried dependencies with multiple operations */
            volatile int temp1 = b[i] * c[i];
            volatile int temp2 = a[i-1] + temp1;
            
            /* Inline assembly to consume registers and increase pressure */
            __asm__ volatile (
                "add %0, %0, %1\n\t"
                "mul %0, %0, %2\n\t"
                : "+r" (temp2)
                : "r" (temp1), "r" (c[i])
                : "r0", "r1", "r2", "r3", "cc"
            );
            
            /* Complex chain of arithmetic operations */
            a[i] = (temp2 * 3 + a[i-1] * 7) % 997;
            
            /* Additional volatile operations to create anti-dependencies */
            volatile int check = a[i] - b[i];
            __asm__ volatile ("" : "+r" (check) : : "r4", "r5");
            
            /* Conditional break with multiple exit points */
            if (check > BREAK_THRESHOLD) {
                /* First potential exit */
                break;
            }
            
            /* Another conditional break based on volatile computation */
            volatile int alt_check = a[i] + c[i];
            if (alt_check < 0) {
                /* Second potential exit */
                break;
            }
            
            /* More operations to increase II */
            b[i] = (b[i] * 13 + i) % 991;
            c[i] = (c[i] * 17 + a[i]) % 983;
            
            /* Register-consuming inline assembly */
            __asm__ volatile (
                "mov r6, %0\n\t"
                "mov r7, %1\n\t"
                "add r6, r6, r7\n\t"
                : 
                : "r" (b[i]), "r" (c[i])
                : "r6", "r7"
            );
            
            i--;
        }
        
        /* Additional operations between outer loop iterations */
        volatile int inter_iter = a[0] + b[0];
        __asm__ volatile ("" : "+r" (inter_iter) : : "r8", "r9");
    }
    
    /* Consume results to prevent dead code elimination */
    consume_result((volatile int*)a, ARRAY_SIZE, &sink);
    
    /* Return checksum */
    return sink & 0xFF;
}
