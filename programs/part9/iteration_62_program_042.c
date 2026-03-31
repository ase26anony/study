/* modulo-sched-coverage.c
 * Designed to trigger GCC's modulo scheduler debug prints for PSG moves.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -dP -march=armv7-a modulo-sched-coverage.c -o modulo-sched-coverage
 * Or for x86: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -funroll-loops -fdump-rtl-sms -dP modulo-sched-coverage.c -o modulo-sched-coverage
 */

#include <stdint.h>

#define SIZE 1024
#define OUTER_ITER 5
#define THRESHOLD 0x7FFFFFFF

/* Prevent dead code elimination */
__attribute__((noinline)) 
void consume_result(volatile int *arr, int n, int *checksum) {
    volatile int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
        /* Force register pressure with inline asm */
        __asm__ volatile ("" : : "r"(sum) : "r0", "r1", "r2", "r3");
    }
    *checksum = sum;
}

/* Simple LCG to avoid external dependencies */
static inline int pseudo_rand(int *state) {
    *state = (*state * 1103515245 + 12345) & 0x7FFFFFFF;
    return *state;
}

int main(void) {
    /* Volatile to prevent optimization and create anti-dependencies */
    volatile int a[SIZE];
    volatile int b[SIZE];
    volatile int c[SIZE];
    
    volatile int i, j;
    volatile int outer_bound = OUTER_ITER;
    volatile int inner_bound = SIZE;
    volatile int seed = 42;
    volatile int temp1, temp2, temp3;
    int checksum = 0;
    
    /* Initialize arrays with pseudo-random values */
    int init_state = 42;
    for (i = 0; i < SIZE; i++) {
        a[i] = pseudo_rand(&init_state) % 1000;
        b[i] = pseudo_rand(&init_state) % 1000;
        c[i] = pseudo_rand(&init_state) % 1000;
    }
    
    /* Complex nested loop designed to trigger aggressive modulo scheduling */
    for (j = 0; j < outer_bound; j++) {
        /* Count-down loop with volatile bound - affects scheduler heuristics */
        volatile int count = inner_bound;
        
        /* Multiple loop-carried dependencies with volatile intermediates */
        volatile int prev = a[0];
        
        for (i = 1; i < count; i--) {
            /* Chain of arithmetic operations creating data dependencies */
            temp1 = b[i] * c[i];
            temp2 = temp1 + prev;          /* Loop-carried: uses prev from previous iteration */
            temp3 = temp2 % 997;           /* Modulo operation - expensive */
            
            /* Anti-dependency: read after write with volatile */
            a[i] = temp3 + a[i-1];         /* Another loop-carried dependency */
            
            /* Inline assembly to consume registers and increase pressure */
            __asm__ volatile (
                "add %0, %0, %1\n\t"
                "mul %0, %0, %2"
                : "+r" (temp3)
                : "r" (temp1), "r" (temp2)
                : "r0", "r1", "r2", "r3", "cc"
            );
            
            /* Multiple exit points based on volatile conditions */
            if (temp3 > THRESHOLD) {
                /* Conditional break - creates complex CFG */
                break;
            }
            
            /* Additional operation with side effect */
            prev = temp3 + (i & 0xFF);
            
            /* More inline asm to prevent optimization */
            __asm__ volatile (
                ""
                : 
                : "r" (prev), "r" (temp1), "r" (temp2), "r" (temp3)
                : "r4", "r5", "r6", "r7"
            );
            
            /* Artificial dependency chain continuation */
            b[i] = (b[i] + prev) & 0xFFF;
            c[i] = (c[i] * 3 + i) % 1001;
            
            /* Another potential exit point */
            if (i < 10 && prev > (THRESHOLD >> 1)) {
                break;
            }
        }
        
        /* Cross-iteration dependency */
        if (j > 0) {
            a[0] = a[SIZE-1] + j;
        }
        
        /* More register pressure between outer loop iterations */
        __asm__ volatile (
            "mov r8, %0\n\t"
            "mov r9, %1"
            :
            : "r" (j), "r" (count)
            : "r8", "r9", "r10"
        );
    }
    
    /* Force result consumption to prevent dead code elimination */
    consume_result((volatile int *)a, SIZE > 100 ? 100 : SIZE, &checksum);
    
    return checksum & 0xFF;
}
