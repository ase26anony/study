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
void consume_result(volatile int *arr, int n, int *checksum) {
    volatile int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
        /* Force register pressure with inline asm */
        __asm__ volatile ("" : : "r"(sum) : "r0", "r1", "r2", "r3");
    }
    *checksum = sum;
}

int main(void) {
    /* Volatile to prevent optimization and create anti-dependencies */
    volatile int a[SIZE], b[SIZE], c[SIZE];
    volatile int i, j, k;
    volatile int outer_bound = OUTER_ITER;
    volatile int inner_bound = SIZE;
    volatile int temp1, temp2, temp3;
    volatile int condition;
    int checksum = 0;
    
    /* Simple LCG for pseudo-random initialization */
    uint32_t seed = 123456789;
    for (i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        a[i] = (seed >> 16) & 0x7FFF;
        b[i] = (seed >> 8) & 0x7FFF;
        c[i] = seed & 0xFF;
    }
    
    /* Complex nested loop with multiple dependencies */
    for (k = 0; k < outer_bound; k++) {
        /* Count-down loop to affect scheduler heuristics */
        for (i = inner_bound - 1; i > 0; i--) {
            /* Chain of arithmetic operations with loop-carried dependencies */
            temp1 = a[i] * 17 + 42;
            temp2 = b[i] * temp1 - c[i];
            
            /* Critical: loop-carried dependency on a[i-1] */
            a[i] = temp2 + a[i-1] * 3;
            
            /* Additional operations to increase II */
            temp3 = (a[i] * b[i]) % 257;
            b[i] = temp3 ^ c[i];
            
            /* Volatile intermediate with inline asm to create register pressure */
            condition = a[i] + b[i] + c[i];
            __asm__ volatile (
                "add %0, %0, %1\n\t"
                "mul %0, %0, %2\n\t"
                : "+r"(condition) 
                : "r"(temp3), "r"(i)
                : "r0", "r1", "r2", "r3", "cc"
            );
            
            /* Multiple exit points - affects distance calculations */
            if (condition > THRESHOLD) {
                /* Early break creates control flow complexity */
                break;
            }
            
            /* Another potential exit point */
            if (i % 13 == 0 && condition < 0) {
                /* Force anti-dependency through volatile */
                volatile int anti_dep = a[i];
                __asm__ volatile ("" : : "r"(anti_dep) : "r4", "r5");
                if (anti_dep > 1000000) break;
            }
            
            /* More operations to increase resource conflicts */
            c[i] = (c[i] * 7 + a[i]) % 1023;
            
            /* Additional inline asm for register pressure */
            __asm__ volatile (
                "eor %0, %0, %1\n\t"
                "orr %0, %0, %2\n\t"
                : "+r"(c[i])
                : "r"(b[i]), "r"(i)
                : "r6", "r7", "cc"
            );
        }
        
        /* Cross-iteration dependency between outer loop iterations */
        if (k > 0) {
            volatile int cross_dep = a[0] + b[0];
            a[0] = cross_dep * k;
            __asm__ volatile ("" : : "r"(cross_dep) : "r8", "r9");
        }
    }
    
    /* Consume result to prevent elimination */
    consume_result((volatile int*)a, SIZE > 100 ? 100 : SIZE, &checksum);
    
    return checksum & 0xFF;
}
