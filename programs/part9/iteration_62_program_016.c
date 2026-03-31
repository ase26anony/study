/* modulo-sched-coverage.c
 * Designed to trigger GCC's modulo scheduler debug prints for PSG moves.
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -dP -march=armv7-a modulo-sched-coverage.c -o modulo-sched-test
 * Or for x86: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -funroll-loops -fdump-rtl-all -march=x86-64 modulo-sched-coverage.c -o modulo-sched-test
 */

#include <stdint.h>

#define SIZE 1024
#define OUTER_ITER 5
#define THRESHOLD 0x7FFFFFFF

/* Prevent dead code elimination */
__attribute__((noinline)) 
void consume_result(volatile int *arr, int n, volatile int *sum) {
    for (int i = 0; i < n; i++) {
        *sum += arr[i];
    }
}

/* Simple LCG to generate pseudo-random values without external dependencies */
static inline int lcg_rand(volatile int *state) {
    *state = (*state * 1103515245 + 12345) & 0x7FFFFFFF;
    return *state;
}

int main(void) {
    /* Volatile to prevent optimization and create anti-dependencies */
    volatile int a[SIZE], b[SIZE], c[SIZE];
    volatile int seed = 42;
    volatile int outer_bound = OUTER_ITER;
    volatile int inner_bound = SIZE;
    volatile int break_condition = 0;
    volatile int temp1, temp2, temp3;
    volatile int checksum = 0;
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < SIZE; i++) {
        a[i] = lcg_rand(&seed) % 1000;
        b[i] = lcg_rand(&seed) % 1000;
        c[i] = lcg_rand(&seed) % 1000;
    }
    
    /* Complex nested loop designed to trigger aggressive modulo scheduling */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Count-down loop with volatile bound - affects scheduler heuristics */
        volatile int i = inner_bound - 1;
        
        while (i > 0) {
            /* Chain of arithmetic operations with loop-carried dependencies */
            temp1 = b[i] * c[i];      /* Multiplication */
            temp2 = a[i-1] + temp1;   /* Addition with previous iteration dependency */
            
            /* Inline assembly to create register pressure and anti-dependencies */
            __asm__ volatile (
                "mov %0, %1\n\t"
                "add %0, %0, #1\n\t"
                : "=r" (temp3)
                : "r" (temp2)
                : "r0", "r1", "r2", "r3"  /* Explicit clobber list for ARM */
            );
            
            a[i] = temp3 % 7919;      /* Modulo operation - prime number for complexity */
            
            /* Multiple exit points with volatile condition */
            if (a[i] > THRESHOLD) {
                break_condition = 1;
            }
            
            /* Another arithmetic chain with anti-dependencies */
            temp1 = b[i-1] * a[i];    /* Cross-iteration dependency */
            temp2 = c[i] + temp1;
            
            /* More inline assembly for register pressure */
            __asm__ volatile (
                "eor %0, %0, %1\n\t"
                : "+r" (temp2)
                : "r" (i)
                : "r4", "r5", "r6", "r7"  /* More clobbered registers */
            );
            
            b[i] = temp2 % 65537;
            
            /* Second conditional break point */
            if (break_condition && (i % 7 == 0)) {
                break;
            }
            
            /* Third arithmetic chain */
            temp1 = a[i] * b[i];
            temp2 = temp1 + c[i];
            
            /* Complex expression with multiple operations */
            c[i] = (temp2 * 3 + 1) % 1021;
            
            i--;  /* Count-down decrement */
            
            /* Third potential exit point */
            if (i < inner_bound / 4) {
                volatile int early_exit = outer * i;
                if (early_exit > 1000) {
                    break;
                }
            }
        }
        
        /* Reset break condition for next outer iteration */
        break_condition = 0;
        
        /* Additional computation between outer iterations */
        for (int j = 0; j < 10; j++) {
            volatile int mix = a[j] ^ b[SIZE-1-j];
            c[j] = (c[j] + mix) % 255;
        }
    }
    
    /* Consume result to prevent dead code elimination */
    consume_result((int*)a, SIZE, &checksum);
    consume_result((int*)b, SIZE / 2, &checksum);
    consume_result((int*)c, SIZE / 4, &checksum);
    
    return checksum & 0xFF;
}
