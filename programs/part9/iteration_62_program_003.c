/* modulo-sched-coverage.c
 * Designed to trigger uncovered lines in GCC's modulo-sched.cc
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -dP -march=armv7-a modulo-sched-coverage.c -o modulo-sched-coverage
 * Or for x86: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -funroll-loops -fdump-rtl-all -march=x86-64 modulo-sched-coverage.c -o modulo-sched-coverage
 */

#include <stdint.h>

#define ARRAY_SIZE 1024

/* Prevent dead code elimination */
__attribute__((noinline)) 
int consume_result(volatile int* arr, int size) {
    volatile int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
        /* Force register pressure with inline asm */
        __asm__ volatile ("" : : "r"(arr[i]) : "r0", "r1", "r2", "r3");
    }
    return sum;
}

/* Simple LCG for pseudo-random values */
static inline uint32_t lcg(uint32_t* state) {
    *state = *state * 1103515245 + 12345;
    return *state;
}

int main(void) {
    /* Volatile to prevent optimization and create anti-dependencies */
    volatile int a[ARRAY_SIZE];
    volatile int b[ARRAY_SIZE];
    volatile int c[ARRAY_SIZE];
    
    /* Volatile iteration counters to create non-constant bounds */
    volatile int outer_iterations = 5;
    volatile int inner_iterations = ARRAY_SIZE - 1;
    
    /* Initialize with pseudo-random values */
    uint32_t seed = 42;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        a[i] = (int)(lcg(&seed) % 1000);
        b[i] = (int)(lcg(&seed) % 1000);
        c[i] = (int)(lcg(&seed) % 100) + 1; /* Ensure non-zero for modulo */
    }
    
    /* Create complex loop-carried dependencies and register pressure */
    for (volatile int outer = 0; outer < outer_iterations; outer++) {
        /* Count-down loop to affect scheduler heuristics */
        volatile int i = inner_iterations;
        
        while (i > 0) {
            /* Multiple volatile intermediates to create anti-dependencies */
            volatile int temp1 = b[i];
            volatile int temp2 = c[i];
            volatile int temp3 = a[i-1];
            
            /* Complex arithmetic chain with loop-carried dependency */
            volatile int product = temp1 * temp2;
            
            /* Inline assembly to consume registers and increase pressure */
            __asm__ volatile (
                "add %0, %0, %1\n\t"
                "mul %0, %0, %2\n\t"
                : "+r"(product)
                : "r"(temp3), "r"(i)
                : "r0", "r1", "r2", "r3", "cc"
            );
            
            /* Modulo operation to create longer latency */
            volatile int mod_result = product % (c[i] + 1);
            
            /* Chain of operations with data dependencies */
            a[i] = mod_result + a[i-1] * 3;
            
            /* Conditional break with volatile condition - creates multiple exit points */
            volatile int break_condition = a[i] - a[i-1];
            if (break_condition > 5000) {
                /* Additional inline asm to complicate control flow */
                __asm__ volatile ("" : : : "r0", "r1", "r2", "r3");
                break;
            }
            
            /* Another conditional break point */
            if (i % 7 == 0 && a[i] < -1000) {
                __asm__ volatile ("" : : : "r0", "r1");
                break;
            }
            
            /* Additional arithmetic to increase II */
            volatile int extra_calc = b[i] * a[i] / (c[i] | 1);
            b[i-1] += extra_calc;
            
            /* More inline asm for register pressure */
            __asm__ volatile (
                "mov r0, %0\n\t"
                "mov r1, %1\n\t"
                : 
                : "r"(a[i]), "r"(b[i])
                : "r0", "r1"
            );
            
            i--;
        }
        
        /* Cross-iteration dependency between outer and inner loops */
        if (outer > 0) {
            volatile int cross_dep = a[0] * outer;
            b[0] = cross_dep % 256;
        }
    }
    
    /* Force consumption of results to prevent elimination */
    int checksum = consume_result((volatile int*)a, ARRAY_SIZE);
    
    /* Also consume b and c to ensure all computations are needed */
    checksum += consume_result((volatile int*)b, ARRAY_SIZE);
    checksum += consume_result((volatile int*)c, ARRAY_SIZE / 4);
    
    return checksum & 0xFF; /* Return non-zero to indicate execution */
}
