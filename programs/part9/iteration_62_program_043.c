/* modulo-sched-coverage.c
 * Designed to trigger GCC's modulo scheduler debug prints for PSG moves.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -dP -march=armv7-a modulo-sched-coverage.c -o modulo-sched-coverage
 * Or for x86: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -funroll-loops -fdump-rtl-sms -dP modulo-sched-coverage.c -o modulo-sched-coverage
 */

#include <stdint.h>

#define SIZE 1024
#define OUTER_ITER 5
#define THRESHOLD 0x7FFFFFFF

/* Prevent dead code elimination */
__attribute__((noinline)) 
void consume_result(volatile int *arr, int n, int *checksum) {
    volatile int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
        /* Force register pressure with inline asm */
        __asm__ volatile ("" : : "r"(sum) : "r0", "r1", "r2", "r3");
    }
    *checksum = sum;
}

/* Simple LCG to avoid external dependencies */
static inline int pseudo_rand(int *state) {
    *state = (*state * 1103515245 + 12345) & 0x7FFFFFFF;
    return *state;
}

int main(void) {
    /* Volatile to prevent optimization and create anti-dependencies */
    volatile int a[SIZE], b[SIZE], c[SIZE];
    volatile int i, j, n = SIZE;
    volatile int outer_bound = OUTER_ITER;
    volatile int temp1, temp2, temp3;
    int seed = 42;
    int checksum = 0;
    
    /* Initialize arrays with pseudo-random values */
    for (i = 0; i < SIZE; i++) {
        a[i] = pseudo_rand(&seed) % 1000;
        b[i] = pseudo_rand(&seed) % 1000;
        c[i] = pseudo_rand(&seed) % 1000;
    }
    
    /* Complex nested loop structure to force aggressive modulo scheduling */
    for (j = 0; j < outer_bound; j++) {
        /* Count-down loop with volatile bound - affects scheduler heuristics */
        volatile int count = n;
        
        /* Multiple loop-carried dependencies with volatile intermediates */
        for (i = count - 1; i > 0; i--) {
            /* Chain of arithmetic operations creating data dependencies */
            temp1 = b[i] * c[i];      /* Multiplication */
            temp2 = a[i-1] + temp1;   /* Loop-carried: a[i-1] */
            temp3 = temp2 % 997;      /* Modulo operation */
            a[i] = temp3 + j;         /* Store with outer loop dependency */
            
            /* Inline assembly to consume registers and create pressure */
            __asm__ volatile (
                "add %0, %0, %1\n\t"
                "mul %1, %1, %2\n\t"
                : "+r" (temp1), "+r" (temp2)
                : "r" (temp3)
                : "r0", "r1", "r2", "r3", "cc"
            );
            
            /* Conditional break with multiple exit points */
            volatile int break_cond = (a[i] > THRESHOLD);
            if (break_cond) {
                /* Additional computation before break */
                a[i] = a[i] / 2;
                break;
            }
            
            /* Another potential exit point */
            volatile int break_cond2 = (i < n/4 && j > 2);
            if (break_cond2) {
                a[i] = a[i] * 3;
                break;
            }
            
            /* Additional dependency chain */
            b[i] = (b[i] + a[i-1]) * 3;
            c[i] = (c[i] + b[i-1]) % 991;
            
            /* More inline asm for register pressure */
            __asm__ volatile (
                "eor %0, %0, %1\n\t"
                "orr %1, %1, %2\n\t"
                : "+r" (temp3), "+r" (temp1)
                : "r" (a[i])
                : "r4", "r5", "cc"
            );
        }
        
        /* Cross-iteration dependency */
        if (j > 0) {
            volatile int prev = a[0];
            a[0] = (prev + a[n-1]) / 2;
        }
    }
    
    /* Consume result to prevent dead code elimination */
    consume_result((volatile int *)a, SIZE, &checksum);
    
    return checksum & 0xFF;
}
