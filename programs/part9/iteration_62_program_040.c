/* modulo-sched-coverage.c
 * Designed to trigger uncovered lines in GCC's modulo-sched.cc
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -dP -march=armv7-a modulo-sched-coverage.c -o modulo-sched-coverage
 * Or for x86: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -funroll-loops -fdump-rtl-all -march=x86-64 modulo-sched-coverage.c -o modulo-sched-coverage
 */

#include <stdint.h>

#define ARRAY_SIZE 1024
#define OUTER_ITERATIONS 5
#define THRESHOLD 1000000

/* Force anti-dependencies and register pressure */
volatile int a[ARRAY_SIZE];
volatile int b[ARRAY_SIZE];
volatile int c[ARRAY_SIZE];
volatile int d[ARRAY_SIZE];

/* Prevent optimization of loop bounds */
volatile int volatile_bound = ARRAY_SIZE;
volatile int volatile_outer = OUTER_ITERATIONS;

/* Simple LCG for pseudo-random values without external dependencies */
static inline uint32_t lcg(uint32_t* state) {
    *state = *state * 1103515245 + 12345;
    return *state;
}

/* Dummy function to prevent dead code elimination */
__attribute__((noinline, used))
static void consume_array(volatile int* arr, int size) {
    volatile int sink = 0;
    for (int i = 0; i < size; i++) {
        sink += arr[i];
    }
    __asm__ volatile ("" : : "r"(sink) : "memory");
}

int main(void) {
    uint32_t seed = 42;
    volatile int checksum = 0;
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        a[i] = lcg(&seed) % 1000;
        b[i] = lcg(&seed) % 1000;
        c[i] = lcg(&seed) % 1000;
        d[i] = lcg(&seed) % 1000;
    }
    
    /* Create complex loop-carried dependencies and register pressure */
    for (volatile int outer = 0; outer < volatile_outer; outer++) {
        volatile int temp1, temp2, temp3, temp4;
        volatile int accumulator = 0;
        
        /* Count-down loop with volatile bound - affects scheduler heuristics */
        for (volatile int i = volatile_bound - 1; i > 0; i--) {
            /* Chain of arithmetic operations with loop-carried dependencies */
            temp1 = a[i] * b[i] + c[i];
            temp2 = temp1 % 997;  /* Prime modulo to prevent optimization */
            
            /* Loop-carried dependency: current depends on previous */
            temp3 = temp2 + a[i-1] * 3;
            
            /* More complex dependency chain */
            temp4 = (temp3 * 7) % 991 + d[i];
            
            /* Store with anti-dependency */
            a[i] = temp4 + accumulator;
            
            /* Update accumulator with loop-carried dependency */
            accumulator = accumulator * 13 + temp4;
            
            /* Inline assembly to create register pressure and prevent optimization */
            __asm__ volatile (
                "add %0, %0, %1\n\t"
                "mul %0, %0, %2\n\t"
                : "+r" (accumulator)
                : "r" (temp4), "r" (17)
                : "r0", "r1", "cc", "memory"
            );
            
            /* Conditional break with volatile condition - creates multiple exit points */
            volatile int break_condition = temp4;
            if (break_condition > THRESHOLD) {
                /* Additional inline assembly to complicate control flow */
                __asm__ volatile (
                    "cmp %0, #0\n\t"
                    "bne 1f\n\t"
                    "mov %0, #1\n\t"
                    "1:\n\t"
                    : "+r" (break_condition)
                    :
                    : "cc", "memory"
                );
                if (break_condition > THRESHOLD * 2) {
                    break;
                }
            }
            
            /* Another inline assembly to consume registers */
            __asm__ volatile (
                "eor %0, %0, %1\n\t"
                "ror %0, %0, #3\n\t"
                : "+r" (temp3)
                : "r" (temp1)
                : "cc"
            );
            
            /* Cross-iteration dependency through array */
            b[i] = b[i-1] + temp3;
            
            /* More arithmetic with volatile intermediates */
            volatile int cross_temp = c[i] * d[i-1];
            c[i] = cross_temp % 977 + a[i];
        }
        
        /* Additional computation between outer loop iterations */
        volatile int inter_iteration = 0;
        for (int j = 0; j < 10; j++) {
            inter_iteration = inter_iteration * 31 + a[j];
            __asm__ volatile ("" : : "r"(inter_iteration) : "memory");
        }
    }
    
    /* Compute checksum to prevent optimization */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum = checksum * 31 + a[i];
        checksum = checksum * 17 + b[i];
        checksum = checksum * 13 + c[i];
        checksum = checksum * 7 + d[i];
    }
    
    /* Consume arrays to prevent dead code elimination */
    consume_array(a, ARRAY_SIZE);
    consume_array(b, ARRAY_SIZE);
    consume_array(c, ARRAY_SIZE);
    consume_array(d, ARRAY_SIZE);
    
    return checksum & 0xFF;
}
