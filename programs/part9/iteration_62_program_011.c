/* modulo-sched-coverage.c
 * Designed to trigger GCC's modulo scheduling debug output for uncovered lines
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -dP -march=armv7-a modulo-sched-coverage.c -o modulo-sched-coverage
 * Or for x86: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -funroll-loops -fdump-rtl-all -march=x86-64 modulo-sched-coverage.c -o modulo-sched-coverage
 */

#include <stdint.h>

#define SIZE 1024
#define OUTER_ITER 5
#define THRESHOLD 1000000

/* Prevent optimization of critical variables */
volatile int v_bound = SIZE;
volatile int v_outer = OUTER_ITER;
volatile int v_thresh = THRESHOLD;

/* Dummy function to prevent dead code elimination */
__attribute__((noinline)) 
void consume_result(volatile int* arr, int size, int* checksum) {
    for (int i = 0; i < size; i++) {
        *checksum ^= arr[i];
    }
}

/* Simple LCG PRNG to avoid external dependencies */
static inline uint32_t lcg(uint32_t* state) {
    *state = *state * 1103515245 + 12345;
    return *state;
}

int main(void) {
    /* Volatile arrays to create memory dependencies */
    volatile int a[SIZE];
    volatile int b[SIZE];
    volatile int c[SIZE];
    
    /* Initialize with pseudo-random values */
    uint32_t seed = 42;
    for (int i = 0; i < SIZE; i++) {
        a[i] = lcg(&seed) % 100;
        b[i] = lcg(&seed) % 100;
        c[i] = lcg(&seed) % 100;
    }
    
    /* Volatile iteration counters */
    volatile int outer_count = v_outer;
    volatile int break_flag = 0;
    
    /* Complex nested loop designed to trigger modulo scheduling */
    while (outer_count-- > 0) {
        /* Count-down loop with volatile bound - affects scheduler heuristics */
        volatile int n = v_bound;
        int i = n;
        
        /* Multiple volatile intermediates to create anti-dependencies */
        volatile int temp1, temp2, temp3;
        volatile int accum = a[0];  /* Loop-carried dependency */
        
        /* Inline assembly to create register pressure */
        __asm__ volatile (
            "mov %0, %1\n\t"
            : "=r" (temp1)
            : "r" (accum)
            : /* No clobbers here, but will add more below */
        );
        
        while (i > 0) {
            /* Complex chain of arithmetic operations with loop-carried dependency */
            temp1 = b[i-1] * 17;      /* Multiplication creates longer latency */
            temp2 = temp1 + c[i-1];   /* Dependent operation */
            temp3 = temp2 % 7919;     /* Prime modulo - expensive operation */
            
            /* Loop-carried dependency: a[i] depends on a[i-1] */
            accum = accum + temp3;
            a[i-1] = accum;
            
            /* Inline assembly with register clobbers to increase pressure */
            __asm__ volatile (
                "add %0, %0, %1\n\t"
                : "+r" (accum)
                : "r" (temp3)
                : "r0", "r1", "r2", "r3"  /* Explicit clobbers for ARM */
            );
            
            /* Additional arithmetic to increase ILP opportunities */
            b[i-1] = (b[i-1] * 3 + a[i-1]) % 1023;
            c[i-1] = (c[i-1] * 5 + b[i-1]) % 1023;
            
            /* Conditional break with volatile condition - creates multiple exits */
            if (accum > v_thresh) {
                break_flag = 1;
                /* Additional inline assembly on break path */
                __asm__ volatile (
                    "mov %0, #1\n\t"
                    : "=r" (break_flag)
                    :
                    : "r0"
                );
                break;  /* Multiple exit points */
            }
            
            /* Another conditional break based on array value */
            if (i < n/2 && a[i-1] < 0) {
                break;
            }
            
            i--;
            
            /* More inline assembly to consume registers */
            __asm__ volatile (
                "cmp %0, #0\n\t"
                "bgt 1f\n\t"
                "mov %1, %0\n\t"
                "1:\n\t"
                : 
                : "r" (i), "r" (temp1)
                : "cc", "r4", "r5"
            );
        }
        
        /* Reset for next outer iteration */
        if (break_flag) {
            accum = 0;
            break_flag = 0;
        }
    }
    
    /* Compute checksum to prevent elimination and return meaningful value */
    int checksum = 0;
    consume_result(a, SIZE, &checksum);
    consume_result(b, SIZE, &checksum);
    consume_result(c, SIZE, &checksum);
    
    return checksum & 0xFF;
}
