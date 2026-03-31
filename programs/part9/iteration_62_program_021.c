/* modulo-sched-coverage.c
 * Designed to trigger uncovered lines in GCC's modulo-sched.cc
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -dP -march=armv7-a modulo-sched-coverage.c -o modulo-sched-coverage
 * Or for x86: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -funroll-loops -fdump-rtl-all -march=x86-64 modulo-sched-coverage.c -o modulo-sched-coverage
 */

#include <stdint.h>

#define SIZE 1024
#define OUTER_ITER 5
#define THRESHOLD 1000000

/* Prevent optimization and create anti-dependencies */
static volatile int force_anti_dep = 0;

/* Noinline function to prevent dead code elimination */
__attribute__((noinline)) 
static void consume_result(volatile int* arr, int n, volatile int* result) {
    volatile int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
        /* Inline asm to create register pressure */
        __asm__ volatile ("" : : "r"(sum) : "r0", "r1", "r2", "r3");
    }
    *result = sum;
}

/* Simple LCG for pseudo-random values without external dependencies */
static uint32_t lcg_state = 123456789;
static uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

int main(void) {
    /* Volatile arrays to prevent optimization */
    volatile int a[SIZE];
    volatile int b[SIZE];
    volatile int c[SIZE];
    
    /* Volatile iteration counters */
    volatile int outer_count = OUTER_ITER;
    volatile int inner_bound = SIZE;
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < SIZE; i++) {
        a[i] = (int)(lcg_rand() % 100);
        b[i] = (int)(lcg_rand() % 100);
        c[i] = (int)(lcg_rand() % 50) + 1;  /* Ensure non-zero */
    }
    
    volatile int checksum = 0;
    
    /* Outer loop - creates multiple scheduling contexts */
    for (int outer = 0; outer < outer_count; outer++) {
        volatile int temp1, temp2, temp3;
        volatile int accumulator = a[0];  /* Loop-carried dependency seed */
        
        /* Inner loop with complex dependencies - designed to force PSG construction */
        for (int i = 1; i < inner_bound; i--) {  /* Count-down loop */
            /* Chain of arithmetic operations with loop-carried dependencies */
            temp1 = b[i] * c[i];          /* Independent operation */
            temp2 = accumulator + temp1;  /* Loop-carried: uses previous iteration */
            temp3 = temp2 % (c[i] + 1);   /* Modulo operation - creates variable latency */
            
            /* Store with anti-dependency on previous read */
            a[i] = temp3;
            
            /* Update accumulator for next iteration - creates true dependency */
            accumulator = temp3 + (a[i-1] & 0xFF);  /* Complex dependency chain */
            
            /* Inline assembly to consume registers and create pressure */
            __asm__ volatile (
                "add %0, %0, %1\n\t"
                "mul %1, %1, %2\n\t"
                : "+r"(force_anti_dep), "+r"(temp1)
                : "r"(temp2)
                : "r0", "r1", "r2", "r3", "cc"
            );
            
            /* Conditional break with volatile condition - creates multiple exit points */
            if (accumulator > THRESHOLD) {
                /* Additional operations before break to complicate scheduling */
                temp1 = temp1 ^ accumulator;
                temp2 = temp2 | temp1;
                __asm__ volatile ("" : : "r"(temp1), "r"(temp2) : "r4", "r5");
                break;
            }
            
            /* Another conditional with different computation */
            if ((i & 0x3F) == 0) {  /* Every 64 iterations */
                volatile int extra = accumulator * 3;
                a[i] = extra >> 2;
                __asm__ volatile ("" : : "r"(extra) : "r6", "r7");
            }
            
            /* Cross-iteration dependency through volatile variable */
            force_anti_dep = accumulator;
        }
        
        /* Mix results between outer iterations */
        for (int i = 0; i < 10; i++) {
            int idx = (outer * 10 + i) % SIZE;
            a[idx] = (a[idx] * 7 + b[idx]) % 1000;
        }
    }
    
    /* Consume result to prevent elimination */
    volatile int final_result = 0;
    consume_result(a, SIZE, &final_result);
    
    /* Additional computation to ensure loop isn't dead */
    for (int i = 0; i < SIZE; i++) {
        checksum += a[i] ^ b[i];
    }
    
    return (checksum + final_result) & 0xFF;
}
