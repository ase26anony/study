/* modulo-sched-coverage.c
 * Designed to trigger GCC's modulo scheduler debug prints for PSG moves.
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -dP -march=armv7-a modulo-sched-coverage.c -o modulo-sched-coverage
 * Or for x86: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -funroll-loops -fdump-rtl-sms -dP modulo-sched-coverage.c -o modulo-sched-coverage
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
__attribute__((noinline)) 
static void consume_result(volatile int *arr, int n) {
    volatile int sink = 0;
    for (int i = 0; i < n; i++) {
        sink += arr[i];
    }
    force_volatile = sink;
}

int main(void) {
    /* Volatile arrays to prevent optimization */
    volatile int array_a[SIZE];
    volatile int array_b[SIZE];
    volatile int array_c[SIZE];
    
    /* Volatile loop counters and bounds */
    volatile int outer_bound = OUTER_ITER;
    volatile int inner_bound = SIZE;
    volatile int break_condition = THRESHOLD;
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = (int)(lcg_rand() % 1000);
        array_b[i] = (int)(lcg_rand() % 1000);
        array_c[i] = (int)(lcg_rand() % 1000);
    }
    
    int result = 0;
    
    /* Outer loop - creates pressure for nested modulo scheduling */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        volatile int accumulator = array_a[0];
        volatile int temp1, temp2, temp3;
        
        /* Complex inner loop designed to force modulo scheduling with PSG moves */
        for (volatile int i = inner_bound - 1; i > 0; i--) {
            /* Create loop-carried dependency chain */
            temp1 = array_b[i] * array_c[i];
            temp2 = temp1 + accumulator;  /* Depends on previous iteration */
            
            /* Introduce anti-dependencies with volatile accesses */
            temp3 = array_a[i] ^ temp2;
            accumulator = temp3 % 997;    /* Loop-carried dependency */
            
            /* Multiple operations to create resource pressure */
            array_a[i] = accumulator + array_b[i-1] * 3;
            array_b[i] = array_c[i] * 7 - accumulator;
            
            /* Inline assembly to create register pressure and anti-deps */
            __asm__ volatile (
                "mov %0, %0\n\t"
                "add %1, %1, #1\n\t"
                : "+r" (temp1), "+r" (temp2)
                : 
                : "r0", "r1", "r2", "r3", "memory"
            );
            
            /* Conditional break with volatile to create control flow complexity */
            if (accumulator > break_condition) {
                /* Multiple exit points affect distance calculations */
                result += 1;
                break;
            }
            
            /* Additional arithmetic to increase II */
            array_c[i] = (array_a[i] * array_b[i]) / (accumulator + 1);
            
            /* More inline assembly for register pressure */
            __asm__ volatile (
                "mul %0, %0, %1\n\t"
                : "+r" (temp3)
                : "r" (accumulator)
                : "cc", "memory"
            );
            
            /* Another conditional that might affect scheduling */
            if ((i & 0xF) == 0) {
                volatile int extra = array_b[i] | array_c[i];
                array_a[i] ^= extra;
            }
        }
        
        /* Cross-iteration dependency */
        array_a[0] = accumulator + outer;
        
        /* Additional computation to prevent loop invariant removal */
        for (volatile int j = 1; j < 10; j++) {
            array_b[j] = array_b[j] * (outer + j);
        }
    }
    
    /* Force use of results to prevent elimination */
    consume_result((volatile int*)array_a, 100);
    consume_result((volatile int*)array_b, 100);
    consume_result((volatile int*)array_c, 100);
    
    /* Return checksum */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= array_a[i];
        checksum ^= array_b[i];
        checksum ^= array_c[i];
    }
    
    return checksum + result;
}
