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

/* Simple LCG to generate pseudo-random values without external dependencies */
static inline int lcg_rand(int* state) {
    *state = (*state * 1103515245 + 12345) & 0x7FFFFFFF;
    return *state;
}

int main(void) {
    /* Volatile to prevent optimization and create anti-dependencies */
    volatile int a[SIZE];
    volatile int b[SIZE];
    volatile int c[SIZE];
    
    /* Volatile iteration counters to prevent constant propagation */
    volatile int outer_bound = OUTER_ITER;
    volatile int inner_bound = SIZE;
    
    /* Initialize with pseudo-random values */
    int seed = 42;
    for (int i = 0; i < SIZE; i++) {
        a[i] = lcg_rand(&seed) % 1000;
        b[i] = lcg_rand(&seed) % 1000;
        c[i] = lcg_rand(&seed) % 1000;
    }
    
    /* Complex nested loop designed to trigger aggressive modulo scheduling */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Count-down loop with volatile bound - affects scheduler heuristics */
        volatile int i = inner_bound - 1;
        
        while (i > 0) {
            /* Create loop-carried dependency chain */
            volatile int temp1 = a[i] * c[i];
            volatile int temp2 = b[i] + temp1;
            
            /* Inline assembly to consume registers and increase pressure */
            __asm__ volatile (
                "add %0, %0, %1\n\t"
                "mul %1, %1, %2\n\t"
                : "+r"(temp1), "+r"(temp2)
                : "r"(c[i])
                : "r0", "r1", "r2", "r3", "cc", "memory"
            );
            
            /* Complex arithmetic with multiple dependencies */
            a[i] = temp2 * 3 + a[i-1] * 7;
            
            /* Additional volatile operations to create anti-dependencies */
            volatile int check = a[i] - b[i];
            
            /* Conditional break with multiple exit points */
            if (check > THRESHOLD) {
                __asm__ volatile ("" : : "r"(check) : "memory");
                break;
            }
            
            /* More operations to increase II */
            volatile int mod_op = a[i] % 17;
            b[i] = mod_op * c[i] + b[i-1];
            
            /* Another conditional break possibility */
            if (i % 13 == 0 && a[i] > 1000000) {
                volatile int dummy = a[i] + b[i];
                __asm__ volatile ("" : : "r"(dummy) : "memory");
                if (dummy > THRESHOLD / 2) break;
            }
            
            /* Additional inline assembly to complicate scheduling */
            __asm__ volatile (
                "eor %0, %0, %1\n\t"
                "orr %1, %1, %0\n\t"
                : "+r"(a[i]), "+r"(b[i])
                :
                : "r0", "r1", "cc", "memory"
            );
            
            i--;
        }
        
        /* Modify array c in outer loop to create cross-iteration dependencies */
        for (int j = 1; j < SIZE; j++) {
            c[j] = (c[j] + a[j-1]) & 0xFFF;
        }
    }
    
    /* Consume result to prevent elimination */
    int result = consume_result((volatile int*)a, SIZE);
    
    /* Return checksum */
    return result & 0xFF;
}
