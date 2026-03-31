/* modulo-sched-coverage.c
 * Designed to trigger GCC's modulo scheduler debug prints for PSG moves.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -dP -march=armv7-a modulo-sched-coverage.c -o modulo-sched-coverage
 * Or for x86: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -funroll-loops -fdump-rtl-sms -dP -march=x86-64 modulo-sched-coverage.c -o modulo-sched-coverage
 */

#include <stdint.h>

#define SIZE 1024
#define OUTER_ITER 5
#define THRESHOLD 0x7FFFFFFF

/* Prevent optimization of critical variables */
static volatile int force_volatile = 0;

/* Simple LCG PRNG to avoid external dependencies */
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Dummy function to prevent dead code elimination */
__attribute__((noinline, noipa))
static void consume_result(volatile int *arr, int n) {
    volatile int sink = 0;
    for (int i = 0; i < n; i++) {
        sink += arr[i];
    }
    force_volatile = sink;
}

int main(void) {
    /* Volatile arrays to prevent optimization and create memory dependencies */
    volatile int array_a[SIZE];
    volatile int array_b[SIZE];
    volatile int array_c[SIZE];
    
    /* Volatile loop counters to prevent constant propagation */
    volatile int outer_count = OUTER_ITER;
    volatile int inner_bound = SIZE;
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = (int)(lcg_rand() % 1000);
        array_b[i] = (int)(lcg_rand() % 1000);
        array_c[i] = (int)(lcg_rand() % 1000);
    }
    
    int result_checksum = 0;
    
    /* Outer loop to increase scheduling pressure */
    for (int outer = 0; outer < outer_count; outer++) {
        /* Volatile iteration variable for count-down loop */
        volatile int i = inner_bound - 1;
        
        /* Complex inner loop with loop-carried dependencies */
        while (i > 0) {
            /* Create anti-dependencies with volatile intermediates */
            volatile int temp1 = array_b[i];
            volatile int temp2 = array_c[i];
            
            /* Chain of arithmetic operations with loop-carried dependency */
            int computed = temp1 * 37 + temp2;
            
            /* Inline assembly to consume registers and create pressure */
            __asm__ volatile (
                "add %0, %0, %1\n\t"
                "mul %0, %0, %2\n\t"
                : "+r" (computed)
                : "r" (array_a[i-1]), "r" (17)
                : "r0", "r1", "cc"
            );
            
            /* Complex calculation with modulo to increase latency */
            computed = (computed * 1103515245) % 2147483647;
            
            /* Store with loop-carried dependency */
            array_a[i] = computed + array_a[i-1] + (outer * 7919);
            
            /* Conditional break based on volatile condition */
            volatile int break_cond = array_a[i];
            if (break_cond > THRESHOLD) {
                /* Additional inline assembly to complicate control flow */
                __asm__ volatile (
                    "cmp %0, #0\n\t"
                    "bne 1f\n\t"
                    "mov %0, #1\n\t"
                    "1:\n\t"
                    : "+r" (break_cond)
                    :
                    : "cc"
                );
                if (break_cond & 1) {
                    break;
                }
            }
            
            /* More arithmetic to increase resource usage */
            volatile int extra_op = array_b[i] * array_c[i];
            __asm__ volatile ("" : : "r" (extra_op));
            
            /* Another loop-carried dependency chain */
            array_b[i] = array_a[i] * 3 - array_b[i-1];
            
            /* Count-down with volatile decrement */
            i--;
        }
        
        /* Mix arrays between outer iterations */
        for (int j = 1; j < SIZE; j++) {
            volatile int mix = array_a[j] ^ array_b[j-1];
            array_c[j] = (mix * 13) + array_c[j-1];
        }
    }
    
    /* Compute checksum to use results */
    for (int i = 0; i < SIZE; i++) {
        result_checksum += array_a[i] + array_b[i] + array_c[i];
    }
    
    /* Prevent dead code elimination */
    consume_result(array_a, SIZE > 100 ? 100 : SIZE);
    
    return result_checksum & 0xFF;
}
