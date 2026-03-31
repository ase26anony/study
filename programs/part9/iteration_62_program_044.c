/* modulo-sched-coverage.c
 * Designed to trigger GCC's modulo scheduler debug output for PSG moves.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -dP -march=armv7-a modulo-sched-coverage.c -o modulo-sched-coverage
 * Or for x86: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -funroll-loops -fdump-rtl-all -march=x86-64 modulo-sched-coverage.c -o modulo-sched-coverage
 */

#include <stdint.h>

/* Prevent dead code elimination */
__attribute__((noinline)) 
static void consume_result(volatile int* arr, int size, volatile int* sink) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
        /* Force memory dependency */
        __asm__ volatile ("" : "+r" (sum) : : "memory");
    }
    *sink = sum;
}

/* Simple LCG PRNG to avoid external dependencies */
static inline uint32_t lcg(uint32_t* state) {
    *state = *state * 1103515245 + 12345;
    return *state;
}

int main(void) {
    /* Volatile to prevent optimization and create anti-dependencies */
    volatile int N = 1000;
    volatile int outer_bound = 5;
    volatile int threshold = 0x7FFFFFFF;
    
    /* Arrays with volatile accesses to create memory dependencies */
    volatile int a[1000];
    volatile int b[1000];
    volatile int c[1000];
    
    /* Initialize with pseudo-random values */
    uint32_t seed = 42;
    for (volatile int i = 0; i < N; i++) {
        a[i] = (int)(lcg(&seed) % 100);
        b[i] = (int)(lcg(&seed) % 100);
        c[i] = (int)(lcg(&seed) % 100) + 1; /* Avoid division by zero */
    }
    
    volatile int sink = 0;
    
    /* Outer loop with volatile bound */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Complex inner loop with loop-carried dependencies */
        /* Count-down loop to affect scheduler heuristics */
        volatile int i = N - 1;
        
        while (i > 0) {
            /* Chain of arithmetic operations with volatile intermediates */
            volatile int temp1 = b[i] * 17;
            volatile int temp2 = c[i] * 3;
            
            /* Loop-carried dependency: a[i] depends on a[i-1] */
            volatile int prev = a[i-1];
            
            /* Complex computation with multiple operations */
            volatile int result = temp1 + temp2;
            result = result * prev;
            result = result % c[i];  /* Creates dependency on c[i] */
            result = result + (temp1 >> 3);
            
            /* Anti-dependency through volatile */
            a[i] = result + a[i];
            
            /* Inline assembly to create register pressure and clobbers */
            __asm__ volatile (
                "add %0, %0, %1\n\t"
                "mul %0, %0, %2\n\t"
                : "+r" (result)
                : "r" (temp2), "r" (prev)
                : "r0", "r1", "cc", "memory"
            );
            
            /* Conditional break with volatile condition */
            volatile int check = result;
            if (check > threshold) {
                /* Multiple exit points affect control flow */
                break;
            }
            
            /* Another conditional break possibility */
            if (i % 7 == 0 && check < -threshold) {
                break;
            }
            
            /* Additional computation to increase II */
            volatile int extra = b[i] * c[i];
            extra = extra / (c[i] + 1);
            a[i] = a[i] + extra;
            
            /* More inline assembly for register pressure */
            __asm__ volatile (
                "mov r2, %0\n\t"
                "add r2, r2, #1\n\t"
                "mov %0, r2\n\t"
                : "+r" (extra)
                :
                : "r2", "cc"
            );
            
            i--;
        }
        
        /* Modify array b based on results from inner loop */
        for (volatile int j = 1; j < 10; j++) {
            b[j] = a[j] * b[j-1] + c[j];
        }
    }
    
    /* Consume results to prevent elimination */
    consume_result((int*)a, 100, &sink);
    
    /* Return checksum */
    return sink & 0xFF;
}
