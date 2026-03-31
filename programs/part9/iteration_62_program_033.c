/* modulo-sched-coverage.c
 * Designed to trigger uncovered lines in GCC's modulo-sched.cc
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -dP -march=armv7-a modulo-sched-coverage.c -o modulo-sched-coverage
 * Or for x86: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -funroll-loops -fdump-rtl-all -march=x86-64 modulo-sched-coverage.c -o modulo-sched-coverage
 */

#include <stdint.h>

#define ARRAY_SIZE 1024

/* Prevent dead code elimination */
static void __attribute__((noinline)) consume_result(volatile int* arr, int n) {
    volatile int sink = 0;
    for (int i = 0; i < n; i++) {
        sink += arr[i];
    }
    __asm__ volatile ("" : : "r"(sink) : "memory");
}

/* Simple LCG to generate pseudo-random values without external dependencies */
static inline uint32_t lcg(uint32_t* state) {
    *state = *state * 1103515245 + 12345;
    return *state;
}

int main(void) {
    /* Volatile to prevent optimization and create anti-dependencies */
    volatile int a[ARRAY_SIZE];
    volatile int b[ARRAY_SIZE];
    volatile int c[ARRAY_SIZE];
    
    /* Volatile iteration counters to create non-constant loop bounds */
    volatile int outer_bound = 5;
    volatile int inner_bound = ARRAY_SIZE - 1;
    
    /* Initialize arrays with pseudo-random values */
    uint32_t seed = 42;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        a[i] = (int)(lcg(&seed) % 1000);
        b[i] = (int)(lcg(&seed) % 1000);
        c[i] = (int)(lcg(&seed) % 100) + 1;  /* Ensure non-zero for division */
    }
    
    /* Volatile threshold for conditional break */
    volatile int break_threshold = 5000000;
    
    /* Outer loop with volatile bound */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Count-down inner loop to affect scheduler heuristics */
        volatile int i = inner_bound;
        
        while (i > 0) {
            /* Create complex loop-carried dependencies */
            volatile int temp1 = b[i] * c[i];
            volatile int temp2 = a[i-1] + temp1;
            
            /* Inline assembly to consume registers and create pressure */
            __asm__ volatile (
                "add %0, %0, %1\n\t"
                "mul %0, %0, %2\n\t"
                : "+r" (temp2)
                : "r" (temp1), "r" (c[i])
                : "r0", "r1", "r2", "r3", "cc", "memory"
            );
            
            /* Modulo operation to create more complex dependencies */
            volatile int temp3 = temp2 % (c[i] + 1);
            
            /* Chain of arithmetic operations with anti-dependencies */
            a[i] = temp3 + (a[i-1] * 3) - (b[i] / 2);
            
            /* More inline assembly with register clobbers */
            __asm__ volatile (
                "and %0, %0, %1\n\t"
                "orr %0, %0, %2\n\t"
                : "+r" (a[i])
                : "r" (temp3), "r" (b[i])
                : "r4", "r5", "r6", "cc", "memory"
            );
            
            /* Conditional break with volatile condition - creates multiple exit points */
            volatile int check_val = a[i] + b[i];
            if (check_val > break_threshold) {
                /* Additional computation before break to create more scheduling complexity */
                volatile int pre_break = check_val * 2;
                __asm__ volatile ("" : : "r"(pre_break) : "memory");
                break;
            }
            
            /* Another conditional break possibility */
            if (i % 7 == 0) {
                volatile int mod_check = a[i] % 13;
                if (mod_check == 0) {
                    __asm__ volatile ("" : : "r"(mod_check) : "memory");
                    /* Don't break here - just create control flow complexity */
                }
            }
            
            /* Additional dependency chain */
            volatile int temp4 = a[i] * a[i-1];
            b[i] = (temp4 + c[i]) & 0xFFF;
            
            /* More register pressure */
            __asm__ volatile (
                "mov r7, %0\n\t"
                "mov r8, %1\n\t"
                "add r9, r7, r8\n\t"
                : 
                : "r" (temp4), "r" (c[i])
                : "r7", "r8", "r9", "cc", "memory"
            );
            
            i--;
        }
        
        /* Small computation between outer loop iterations */
        volatile int inter_loop = a[0] + b[0];
        __asm__ volatile ("" : : "r"(inter_loop) : "memory");
    }
    
    /* Final computation to prevent elimination */
    volatile int checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum = (checksum * 31 + a[i]) & 0x7FFFFFFF;
    }
    
    /* Consume result to prevent dead code elimination */
    consume_result((int*)a, ARRAY_SIZE);
    
    return checksum % 256;
}
