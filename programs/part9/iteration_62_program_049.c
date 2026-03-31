/* modulo-sched-coverage.c
 * Designed to trigger GCC's modulo scheduler debug prints for PSG moves.
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -dP -march=armv7-a -c modulo-sched-coverage.c
 */

#include <stdint.h>

#define SIZE 256
#define DUMMY_USE(x) __asm__ volatile("" : : "r"(x))

/* Prevent inlining to ensure loop structure is preserved */
__attribute__((noinline)) 
static void consume_array(volatile int *arr, int n) {
    volatile int sink = 0;
    for (int i = 0; i < n; i++) {
        sink += arr[i];
    }
    DUMMY_USE(sink);
}

/* Simple LCG to generate pseudo-random values without external dependencies */
static inline int lcg_rand(int *state) {
    *state = (*state * 1103515245 + 12345) & 0x7fffffff;
    return *state;
}

int main(void) {
    /* Volatile arrays to prevent optimization and create memory dependencies */
    volatile int a[SIZE], b[SIZE], c[SIZE];
    volatile int seed = 42;
    volatile int outer_bound = 5;
    volatile int inner_bound = SIZE - 1;
    volatile int threshold = 0x70000000;
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < SIZE; i++) {
        a[i] = lcg_rand((int*)&seed) % 100;
        b[i] = lcg_rand((int*)&seed) % 100;
        c[i] = lcg_rand((int*)&seed) % 100;
    }
    
    /* Complex nested loop structure to force aggressive modulo scheduling */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Count-down loop with volatile bound to affect scheduler heuristics */
        volatile int i = inner_bound;
        
        while (i > 0) {
            /* Chain of arithmetic operations with loop-carried dependencies */
            volatile int temp1 = b[i] * c[i];
            volatile int temp2 = a[i-1] + temp1;
            volatile int temp3 = temp2 % 997;  /* Prime modulo to prevent simplification */
            
            /* Multiple volatile intermediate computations */
            volatile int temp4 = temp3 ^ (temp1 >> 3);
            volatile int temp5 = temp4 * 13;
            
            /* Loop-carried dependency with anti-dependency */
            a[i] = temp5 + a[i-1];
            
            /* Inline assembly to create register pressure and clobber specific registers */
            __asm__ volatile (
                "add %0, %0, %1\n\t"
                "mul %0, %0, %2\n\t"
                : "+r" (temp5)
                : "r" (temp4), "r" (17)
                : "r0", "r1", "cc"
            );
            
            /* Conditional break with volatile condition to create multiple exit points */
            volatile int check = temp5;
            if (check > threshold) {
                /* Early exit - affects control flow graph */
                break;
            }
            
            /* Additional computation with another volatile */
            volatile int temp6 = a[i] * 3;
            b[i] = temp6 - c[i];
            
            /* More inline assembly for register pressure */
            __asm__ volatile (
                "orr %0, %0, %1\n\t"
                : "+r" (temp6)
                : "r" (0x5555)
                : "r2", "r3", "cc"
            );
            
            /* Another conditional that might trigger */
            if (temp6 & 0x1000) {
                c[i] = temp6 >> 4;
            }
            
            i--;
        }
        
        /* Modify bounds slightly each outer iteration */
        inner_bound = (inner_bound * 7) % (SIZE - 10) + 5;
    }
    
    /* Force use of results to prevent dead code elimination */
    consume_array((int*)a, SIZE);
    consume_array((int*)b, SIZE);
    consume_array((int*)c, SIZE);
    
    /* Return checksum to ensure all computations matter */
    volatile int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum ^= a[i] ^ b[i] ^ c[i];
    }
    
    return checksum & 0xFF;
}
