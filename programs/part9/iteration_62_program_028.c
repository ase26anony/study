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
int consume_result(volatile int* arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
        __asm__ volatile ("" : : "r"(arr[i]) : "memory");
    }
    return sum;
}

/* Simple LCG to generate pseudo-random data without external dependencies */
static inline int lcg_rand(int* state) {
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
    
    /* Initialize arrays with pseudo-random data */
    int init_state = 42;
    for (i = 0; i < SIZE; i++) {
        a[i] = lcg_rand(&init_state) % 1000;
        b[i] = lcg_rand(&init_state) % 1000;
        c[i] = lcg_rand(&init_state) % 1000;
    }
    
    /* Complex nested loop designed to trigger aggressive modulo scheduling */
    for (j = 0; j < outer_bound; j++) {
        /* Count-down loop with volatile bound - affects scheduler heuristics */
        for (i = inner_bound - 1; i > 0; i--) {
            /* Chain of arithmetic operations with loop-carried dependencies */
            temp1 = b[i] * c[i];      /* Multiplication */
            temp2 = a[i-1] + temp1;   /* Addition with dependency on previous iteration */
            temp3 = temp2 % 997;      /* Modulo operation */
            a[i] = temp3 ^ b[i];      /* XOR operation */
            
            /* Inline assembly to create register pressure and anti-dependencies */
            __asm__ volatile (
                "add %0, %0, %1\n\t"
                "sub %1, %1, %0\n\t"
                : "+r"(temp1), "+r"(temp2)
                : 
                : "r0", "r1", "r2", "r3", "cc", "memory"
            );
            
            /* Multiple exit points based on volatile conditions */
            if (a[i] > THRESHOLD) {
                /* Early exit - creates complex control flow */
                break;
            }
            
            /* Additional arithmetic to increase II */
            c[i] = (c[i] * 3 + 1) & 0xFFF;
            
            /* More inline assembly for register pressure */
            __asm__ volatile (
                "mov r4, %0\n\t"
                "mul r5, r4, %1\n\t"
                : 
                : "r"(a[i]), "r"(c[i])
                : "r4", "r5", "memory"
            );
            
            /* Another conditional break point */
            if (j > 2 && i < inner_bound / 4) {
                if (b[i] < 0) break; /* Unlikely but prevents optimization */
            }
        }
        
        /* Cross-iteration dependency between outer and inner loops */
        b[0] = a[SIZE-1] ^ j;
        
        /* Additional computation to prevent loop invariant motion */
        for (int k = 0; k < 10; k++) {
            __asm__ volatile (
                "eor r6, r6, r7\n\t"
                "add r7, r7, #1\n\t"
                : 
                : 
                : "r6", "r7", "cc"
            );
        }
    }
    
    /* Force use of results to prevent dead code elimination */
    int result = consume_result((int*)a, SIZE);
    
    /* Return checksum */
    return result & 0xFF;
}
