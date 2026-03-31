/* modulo-sched-coverage.c
 * Designed to trigger GCC's modulo scheduler debug prints for PSG moves.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -dP -march=armv7-a modulo-sched-coverage.c -o modulo-sched-coverage
 * Or for x86: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -funroll-loops -fdump-rtl-sms -dP modulo-sched-coverage.c -o modulo-sched-coverage
 */

#include <stdint.h>

#define SIZE 256
#define OUTER_ITER 5
#define THRESHOLD 0x7FFFFFFF

/* Prevent dead code elimination */
__attribute__((noinline)) 
static void consume_result(volatile int *arr, int n) {
    volatile int sink = 0;
    for (int i = 0; i < n; i++) {
        sink += arr[i];
    }
    __asm__ volatile ("" : : "r"(sink) : "memory");
}

/* Simple LCG to generate pseudo-random values without external dependencies */
static inline int lcg_rand(int *state) {
    *state = (*state * 1103515245 + 12345) & 0x7FFFFFFF;
    return *state;
}

int main(void) {
    /* Volatile arrays to prevent optimization and create memory dependencies */
    volatile int a[SIZE], b[SIZE], c[SIZE];
    volatile int seed = 42;
    volatile int outer_bound = OUTER_ITER;
    volatile int inner_bound = SIZE;
    volatile int threshold = THRESHOLD;
    
    /* Initialize arrays with pseudo-random values */
    int init_state = 42;
    for (int i = 0; i < SIZE; i++) {
        a[i] = lcg_rand(&init_state) % 1000;
        b[i] = lcg_rand(&init_state) % 1000;
        c[i] = lcg_rand(&init_state) % 1000;
    }
    
    /* Complex nested loop designed to trigger aggressive modulo scheduling */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Count-down loop with volatile bound to affect scheduler heuristics */
        volatile int i = inner_bound - 1;
        
        while (i > 0) {
            /* Chain of arithmetic operations with loop-carried dependencies */
            volatile int temp1 = b[i] * c[i];
            volatile int temp2 = a[i-1] + temp1;
            volatile int temp3 = temp2 % 997;  /* Prime modulus to prevent optimization */
            
            /* Anti-dependency: read before write with volatile */
            volatile int prev = a[i];
            a[i] = temp3 + prev;
            
            /* Inline assembly to create register pressure and clobber specific registers */
            __asm__ volatile (
                "add %0, %0, %1\n\t"
                "mul %1, %1, %2\n\t"
                : "+r" (temp1), "+r" (temp2)
                : "r" (temp3)
                : "r0", "r1", "r2", "r3", "cc", "memory"
            );
            
            /* Multiple exit points based on volatile conditions */
            volatile int check1 = a[i] - b[i];
            if (check1 > threshold) {
                /* Early exit path 1 */
                break;
            }
            
            volatile int check2 = c[i] * outer;
            if (check2 < -threshold) {
                /* Early exit path 2 (unlikely but present for CFG complexity) */
                break;
            }
            
            /* Additional computation to increase II */
            volatile int temp4 = a[i] * a[i-1];
            volatile int temp5 = temp4 / (c[i] + 1);
            b[i] = (b[i] + temp5) & 0xFFF;
            
            /* Another inline asm to consume registers */
            __asm__ volatile (
                "eor %0, %0, %1\n\t"
                : "+r" (temp4)
                : "r" (temp5)
                : "r4", "r5", "cc"
            );
            
            i--;
        }
        
        /* Cross-iteration dependency to force longer recurrence cycles */
        if (outer > 0) {
            volatile int cross_temp = a[0] * outer;
            b[0] = (b[0] + cross_temp) % 1000;
        }
    }
    
    /* Final computation to prevent dead code elimination */
    volatile int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum = (checksum * 31 + a[i]) & 0x7FFFFFFF;
        checksum = (checksum * 31 + b[i]) & 0x7FFFFFFF;
    }
    
    consume_result((int*)a, SIZE);
    consume_result((int*)b, SIZE);
    
    return checksum & 0xFF;
}
