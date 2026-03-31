/* modulo-sched-coverage.c
 * Designed to trigger GCC's modulo scheduler debug output for PSG move calculations.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -dP -march=armv7-a modulo-sched-coverage.c -o modulo-sched-coverage
 * Or for x86: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -funroll-loops -fdump-rtl-sms -dP modulo-sched-coverage.c -o modulo-sched-coverage
 */

#include <stdint.h>

#define SIZE 256
#define OUTER_ITER 5

/* Prevent dead code elimination */
static void __attribute__((noinline)) consume(int *arr, int n) {
    volatile int sink = 0;
    for (int i = 0; i < n; i++) {
        sink += arr[i];
    }
    (void)sink;
}

/* Simple LCG for pseudo-random values */
static inline int lcg(int *state) {
    *state = (*state * 1103515245 + 12345) & 0x7fffffff;
    return *state;
}

int main(void) {
    /* Volatile arrays to prevent optimization and create dependencies */
    volatile int a[SIZE];
    volatile int b[SIZE];
    volatile int c[SIZE];
    
    /* Volatile iteration counters to create non-constant bounds */
    volatile int n = SIZE;
    volatile int outer_bound = OUTER_ITER;
    
    /* Initialize with pseudo-random values */
    int seed = 42;
    for (int i = 0; i < SIZE; i++) {
        a[i] = lcg(&seed) % 100;
        b[i] = lcg(&seed) % 100;
        c[i] = lcg(&seed) % 100;
    }
    
    /* Complex nested loops to trigger aggressive modulo scheduling */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Count-down loop with volatile counter */
        volatile int i = n;
        
        /* Multiple volatile intermediates to create register pressure */
        volatile int acc1, acc2, acc3;
        volatile int temp1, temp2;
        
        /* Start with some initial values */
        acc1 = a[0];
        acc2 = b[0];
        acc3 = c[0];
        
        while (i > 0) {
            /* Complex chain of arithmetic operations with loop-carried dependencies */
            temp1 = b[i] * acc3 + 17;  /* RAW dependency on acc3 */
            
            /* Inline assembly to consume registers and create pressure */
            __asm__ volatile (
                "mov %0, %0\n\t"
                "add %1, %1, #1"
                : "+r" (temp1), "+r" (acc2)
                :
                : "r0", "r1", "cc"
            );
            
            temp2 = temp1 % 7919;  /* Large prime for modulo operation */
            
            /* Anti-dependency: acc1 is read before being written */
            acc3 = acc1 * temp2 + c[i];
            
            /* More inline assembly with different clobbers */
            __asm__ volatile (
                "eor %0, %0, %1\n\t"
                : "+r" (acc3)
                : "r" (temp2)
                : "r2", "cc"
            );
            
            /* Loop-carried dependency: a[i] depends on a[i-1] */
            a[i] = b[i] * acc3 + a[i-1];
            
            /* Additional computation creating cross-iteration dependencies */
            acc1 = a[i] + b[i] * 3;
            acc2 = acc1 - temp1;
            
            /* Conditional break with volatile condition - creates multiple exit points */
            volatile int break_cond = (acc3 > 1000000) || (temp2 < 0);
            if (break_cond) {
                /* Force compiler to consider this path */
                __asm__ volatile ("nop" ::: "memory");
                break;
            }
            
            /* Another conditional break possibility */
            if (i % 13 == 0) {
                volatile int maybe_break = outer * i;
                if (maybe_break > 50) {
                    break;
                }
            }
            
            /* More register pressure */
            __asm__ volatile (
                "mul %0, %1, %2\n\t"
                : "=r" (temp1)
                : "r" (acc2), "r" (acc3)
                : "r3", "cc"
            );
            
            i--;  /* Count down */
        }
        
        /* Cross-iteration dependency in outer loop */
        if (outer > 0) {
            a[0] = a[SIZE-1] + outer;
        }
    }
    
    /* Prevent dead code elimination */
    consume((int *)a, SIZE);
    consume((int *)b, SIZE);
    consume((int *)c, SIZE);
    
    /* Return checksum */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum ^= a[i];
    }
    return checksum;
}
