/* modulo-sched-coverage.c
 * Designed to trigger GCC's modulo scheduler debug prints for PSG moves.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -dP -march=armv7-a -c modulo-sched-coverage.c
 * Or for x86: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -funroll-loops -fdump-rtl-sms -dP -march=x86-64 -c modulo-sched-coverage.c
 */

#include <stdint.h>

#define SIZE 256
#define THRESHOLD 1000000

/* Prevent dead code elimination */
__attribute__((noinline)) 
int consume_result(volatile int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Simple LCG to generate pseudo-random values without external dependencies */
static inline int lcg_rand(int *state) {
    *state = (*state * 1103515245 + 12345) & 0x7fffffff;
    return *state;
}

int main(void) {
    /* Volatile to prevent optimization and create anti-dependencies */
    volatile int a[SIZE], b[SIZE], c[SIZE];
    volatile int seed = 42;
    volatile int outer_bound = 5;  /* Non-constant outer bound */
    volatile int inner_bound = SIZE;
    volatile int temp1, temp2, temp3;
    volatile int break_condition = 0;
    
    /* Initialize arrays with pseudo-random values */
    int init_state = 42;
    for (int i = 0; i < SIZE; i++) {
        a[i] = lcg_rand(&init_state) % 1000;
        b[i] = lcg_rand(&init_state) % 1000;
        c[i] = lcg_rand(&init_state) % 1000;
    }
    
    /* Complex nested loop designed to trigger aggressive modulo scheduling */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Count-down loop with volatile bound - affects scheduler heuristics */
        volatile int i = inner_bound - 1;
        
        while (i > 0) {
            /* Chain of arithmetic operations with loop-carried dependencies */
            /* a[i] = b[i] * c[i] + a[i-1] % 997 */
            temp1 = b[i];
            temp2 = c[i];
            
            /* Inline assembly to create register pressure and anti-dependencies */
            __asm__ volatile (
                "mov %0, %0\n\t"  /* Dummy operation using the register */
                : "+r" (temp1), "+r" (temp2)
                : 
                : "r0", "r1", "r2", "r3"  /* Clobber multiple registers */
            );
            
            /* Complex computation with multiple dependencies */
            temp3 = temp1 * temp2;
            temp3 = temp3 + a[i-1];  /* Loop-carried dependency */
            temp3 = temp3 % 997;     /* Modulo operation adds latency */
            
            /* Another inline asm to break optimization patterns */
            __asm__ volatile (
                "add %0, %0, #1\n\t"
                : "+r" (temp3)
                :
                : "cc"
            );
            
            a[i] = temp3;
            
            /* Create multiple exit points with volatile conditions */
            if (break_condition) {
                /* Early exit path 1 */
                break;
            }
            
            /* Additional computation to increase II */
            volatile int check = a[i] * b[i];
            if (check > THRESHOLD) {
                /* Early exit path 2 */
                break_condition = 1;
            }
            
            /* More operations to create resource conflicts */
            b[i] = (b[i] * 3 + 1) % 1000;
            c[i] = (c[i] * 7 + 3) % 1000;
            
            /* Another inline asm for register pressure */
            __asm__ volatile (
                "eor %0, %0, %1\n\t"
                : "+r" (b[i])
                : "r" (c[i])
                : "cc"
            );
            
            i--;  /* Count down */
        }
        
        /* Modify bounds to create varying iteration patterns */
        if (outer % 2 == 0) {
            inner_bound = SIZE / 2;
        } else {
            inner_bound = SIZE;
        }
    }
    
    /* Force result consumption to prevent dead code elimination */
    int result = consume_result((int*)a, SIZE);
    
    /* Return checksum to ensure all computations matter */
    return result % 256;
}
