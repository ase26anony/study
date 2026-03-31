/* modulo-sched-coverage.c
 * Designed to trigger GCC's modulo scheduler debug prints for PSG moves.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -dP -march=armv7-a modulo-sched-coverage.c -o modulo-sched-coverage
 * Or for x86: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -funroll-loops -fdump-rtl-all -march=x86-64 modulo-sched-coverage.c -o modulo-sched-coverage
 */

#include <stdint.h>

#define SIZE 1024
#define OUTER_ITER 5
#define THRESHOLD 0x7FFFFFFF

/* Prevent dead code elimination */
__attribute__((noinline))
static int consume_result(volatile int *arr, int n) {
    volatile int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
        __asm__ volatile ("" : : "r"(sum) : "memory");
    }
    return sum;
}

/* Simple LCG to avoid external dependencies */
static uint32_t lcg_state = 123456789;
static uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

int main(void) {
    /* Volatile to prevent optimization and create anti-dependencies */
    volatile int array_a[SIZE];
    volatile int array_b[SIZE];
    volatile int array_c[SIZE];
    
    volatile int i, j;
    volatile int outer_bound = OUTER_ITER;
    volatile int inner_bound = SIZE;
    
    /* Initialize with pseudo-random values */
    for (i = 0; i < SIZE; i++) {
        array_a[i] = (int)(lcg_rand() % 1000);
        array_b[i] = (int)(lcg_rand() % 1000);
        array_c[i] = (int)(lcg_rand() % 1000);
    }
    
    /* Complex nested loop designed to trigger aggressive modulo scheduling */
    for (j = 0; j < outer_bound; j++) {
        volatile int temp1, temp2, temp3;
        volatile int accumulator = array_a[0];
        
        /* Count-down loop with volatile bounds - affects scheduler heuristics */
        for (i = inner_bound - 1; i > 0; i--) {
            /* Create loop-carried dependency chain */
            temp1 = array_b[i] * 17;
            temp2 = array_c[i] * 23;
            
            /* Complex arithmetic with multiple dependencies */
            accumulator = accumulator + temp1 - temp2;
            
            /* Modulo operation creates variable latency */
            temp3 = accumulator % 7919;
            
            /* Array access with anti-dependency */
            array_a[i] = temp3 + array_a[i-1] * 3;
            
            /* Inline assembly to create register pressure and clobbers */
            __asm__ volatile (
                "add %0, %0, %1\n\t"
                "sub %1, %1, %0"
                : "+r" (temp1), "+r" (temp2)
                :
                : "cc", "r0", "r1", "r2", "r3"
            );
            
            /* Conditional break with multiple exit points */
            if (accumulator > THRESHOLD) {
                /* Force early exit - creates control flow complexity */
                break;
            }
            
            /* Additional volatile operation to prevent optimization */
            volatile int check = array_b[i] & 0xFF;
            if (check == 0) {
                /* Another potential exit point */
                accumulator = accumulator / 2;
            }
            
            /* More arithmetic to increase II */
            array_c[i] = (array_a[i] * array_b[i]) % 9973;
            
            /* Another inline asm with different clobbers */
            __asm__ volatile (
                "mul %0, %0, %1\n\t"
                : "+r" (temp3)
                : "r" (accumulator)
                : "cc", "r4", "r5"
            );
        }
        
        /* Cross-iteration dependency */
        array_a[0] = accumulator % 10007;
        
        /* Additional computation between outer loop iterations */
        volatile int cross_temp = 0;
        for (int k = 0; k < 10; k++) {
            cross_temp += array_b[k] * array_c[k];
            __asm__ volatile ("" : : "r"(cross_temp) : "memory");
        }
    }
    
    /* Force result consumption to prevent dead code elimination */
    int result = consume_result((volatile int*)array_a, 100);
    
    /* Return checksum */
    return result & 0xFF;
}
