/* modulo-sched-coverage.c
 * Designed to trigger GCC's modulo scheduler debug prints for PSG moves.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -dP -march=armv7-a modulo-sched-coverage.c -o modulo-sched-coverage
 * Or for x86: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -funroll-loops -fdump-rtl-all -march=x86-64 modulo-sched-coverage.c -o modulo-sched-coverage
 */

#include <stdint.h>

#define SIZE 1024
#define OUTER_ITER 5

/* Prevent dead code elimination */
__attribute__((noinline)) 
int consume_result(volatile int* arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
        /* Force memory access */
        __asm__ volatile ("" : : "r"(arr[i]) : "memory");
    }
    return sum;
}

/* Simple LCG to generate pseudo-random values without external dependencies */
static inline uint32_t lcg(uint32_t* state) {
    *state = *state * 1103515245 + 12345;
    return *state;
}

int main(void) {
    /* Volatile to prevent optimization and create anti-dependencies */
    volatile int array_a[SIZE];
    volatile int array_b[SIZE];
    volatile int array_c[SIZE];
    
    /* Volatile iteration counters to create non-constant loop bounds */
    volatile int outer_count = OUTER_ITER;
    volatile int inner_bound = SIZE;
    
    /* Initialize arrays with pseudo-random values */
    uint32_t seed = 42;
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = (int)(lcg(&seed) % 1000);
        array_b[i] = (int)(lcg(&seed) % 1000);
        array_c[i] = (int)(lcg(&seed) % 100) + 1;  /* Avoid division by zero */
    }
    
    /* Critical nested loop structure designed to trigger complex modulo scheduling */
    for (volatile int outer = 0; outer < outer_count; outer++) {
        /* Count-down loop with volatile bound - affects scheduler heuristics */
        volatile int i = inner_bound - 1;
        
        /* Force register pressure with inline assembly clobbers */
        __asm__ volatile ("" : : : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7");
        
        while (i > 0) {
            /* Complex chain of arithmetic operations with loop-carried dependencies */
            volatile int temp1 = array_b[i] * array_c[i];
            volatile int temp2 = temp1 + array_a[i-1];  /* Loop-carried dependency */
            
            /* Modulo operation creates variable latency */
            volatile int temp3 = temp2 % (array_c[i] + 1);
            
            /* Multiple volatile intermediates create anti-dependencies */
            volatile int temp4 = temp3 * 7;
            volatile int temp5 = temp4 - array_b[i-1];
            
            /* Store with another loop-carried dependency */
            array_a[i] = temp5 + (array_a[i-2] & 0xFF);  /* Two-iteration dependency */
            
            /* Conditional break with volatile condition - creates multiple exit points */
            volatile int break_cond = array_a[i] - array_b[i];
            if (break_cond > 5000) {  /* Rare condition to preserve loop structure */
                __asm__ volatile ("" : : : "memory");
                break;
            }
            
            /* Additional arithmetic to increase ILP requirements */
            volatile int temp6 = array_b[i] + array_c[i];
            volatile int temp7 = temp6 * temp5;
            array_b[i] = temp7 >> 3;
            
            /* More register pressure */
            __asm__ volatile ("" : : : "r8", "r9", "r10", "r11", "r12");
            
            /* Another conditional with volatile */
            volatile int cond2 = array_c[i] % 17;
            if (cond2 == 0) {
                array_c[i] = array_c[i] * 2;
            }
            
            i--;
        }
        
        /* Modify inner bound slightly to prevent complete optimization */
        inner_bound = inner_bound - (outer & 1);
    }
    
    /* Consume results to prevent dead code elimination */
    int checksum = consume_result((volatile int*)array_a, 100);
    checksum += consume_result((volatile int*)array_b, 100);
    checksum += consume_result((volatile int*)array_c, 100);
    
    return checksum & 0xFF;  /* Return non-zero result */
}
