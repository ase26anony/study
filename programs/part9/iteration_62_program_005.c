/* modulo-sched-coverage.c
 * Designed to trigger GCC's modulo scheduler debug prints for PSG moves.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -dP -march=armv7-a modulo-sched-coverage.c -o modulo-sched-coverage
 * Or for x86: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -funroll-loops -fdump-rtl-sms -dP -march=x86-64 modulo-sched-coverage.c -o modulo-sched-coverage
 */

#include <stdint.h>

#define SIZE 1024
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
    volatile int a[SIZE];
    volatile int b[SIZE];
    volatile int c[SIZE];
    
    /* Volatile loop counters and bounds */
    volatile int outer_bound = OUTER_ITER;
    volatile int inner_bound = SIZE;
    volatile int seed = 42;
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < SIZE; i++) {
        a[i] = lcg_rand(&seed) % 1000;
        b[i] = lcg_rand(&seed) % 1000;
        c[i] = lcg_rand(&seed) % 1000;
    }
    
    /* Complex nested loop designed to trigger aggressive modulo scheduling */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Count-down loop to affect scheduler heuristics */
        volatile int i = inner_bound;
        
        while (i > 0) {
            /* Create loop-carried dependency chain */
            volatile int idx = i - 1;
            volatile int prev_idx = (idx > 0) ? idx - 1 : 0;
            
            /* Complex arithmetic operations with volatile intermediates */
            volatile int temp1 = b[idx] * c[idx];
            volatile int temp2 = a[prev_idx];
            volatile int temp3 = temp1 + temp2;
            
            /* Inline assembly to create register pressure and anti-dependencies */
            __asm__ volatile (
                "add %0, %0, %1\n\t"
                "mul %0, %0, %2\n\t"
                : "+r" (temp3)
                : "r" (temp1), "r" (temp2)
                : "r0", "r1", "r2", "r3", "cc", "memory"
            );
            
            /* Modulo operation to create longer latency operations */
            volatile int temp4 = temp3 % 997;
            
            /* More inline assembly with register clobbers */
            __asm__ volatile (
                "and %0, %0, #0xFFF\n\t"
                : "+r" (temp4)
                :
                : "r0", "r1", "cc"
            );
            
            /* Store with anti-dependency */
            a[idx] = temp4 + (a[idx] & 1);
            
            /* Conditional break to create multiple exit points */
            volatile int break_cond = temp4;
            if (break_cond > THRESHOLD) {
                __asm__ volatile ("" : : : "memory");
                break;
            }
            
            /* Additional dependency chain */
            volatile int temp5 = b[idx] + c[idx];
            volatile int temp6 = temp5 * a[idx];
            
            /* More inline assembly for register pressure */
            __asm__ volatile (
                "eor %0, %0, %1\n\t"
                "ror %0, %0, #3\n\t"
                : "+r" (temp6)
                : "r" (temp5)
                : "r0", "r1", "cc"
            );
            
            b[idx] = temp6 ^ 0x5555;
            
            /* Another conditional break possibility */
            volatile int break_cond2 = b[idx];
            if (break_cond2 < 0) {
                __asm__ volatile ("" : : : "memory");
                break;
            }
            
            i--;
            
            /* Additional computation to increase loop body size */
            volatile int temp7 = c[idx] * 3;
            volatile int temp8 = temp7 / 2;
            c[idx] = temp8 + (idx % 7);
        }
        
        /* Small computation between outer loop iterations */
        volatile int outer_temp = outer * 7;
        __asm__ volatile (
            "add %0, %0, #1\n\t"
            "mul %0, %0, %0\n\t"
            : "+r" (outer_temp)
            :
            : "r0", "r1", "cc"
        );
    }
    
    /* Consume results to prevent elimination */
    consume_result((int*)a, SIZE > 100 ? 100 : SIZE);
    
    /* Return checksum */
    int checksum = 0;
    for (int i = 0; i < (SIZE > 50 ? 50 : SIZE); i++) {
        checksum ^= a[i];
        checksum = (checksum << 1) | (checksum >> 31);
    }
    
    return checksum & 0xFF;
}
