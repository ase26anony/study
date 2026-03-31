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
static void consume_result(volatile int *arr, int n) {
    volatile int sink = 0;
    for (int i = 0; i < n; i += 64) {
        sink += arr[i];
    }
    (void)sink;
}

/* Simple LCG PRNG to avoid external dependencies */
static inline uint32_t lcg(uint32_t *state) {
    *state = *state * 1103515245 + 12345;
    return *state;
}

int main(void) {
    /* Volatile to prevent optimization and create anti-dependencies */
    volatile int a[SIZE];
    volatile int b[SIZE];
    volatile int c[SIZE];
    
    /* Volatile iteration counters to prevent constant propagation */
    volatile int outer_bound = OUTER_ITER;
    volatile int inner_bound = SIZE - 1;
    
    /* Initialize with pseudo-random values */
    uint32_t seed = 42;
    for (int i = 0; i < SIZE; i++) {
        a[i] = (int)(lcg(&seed) % 1000);
        b[i] = (int)(lcg(&seed) % 1000);
        c[i] = (int)(lcg(&seed) % 100) + 1;
    }
    
    /* Complex nested loop designed to trigger aggressive modulo scheduling */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Count-down loop with volatile bound */
        volatile int i = inner_bound;
        
        while (i > 0) {
            /* Create loop-carried dependency chain */
            volatile int temp1 = b[i] * c[i];
            volatile int temp2 = a[i-1];
            
            /* Inline assembly to create register pressure and anti-dependencies */
            __asm__ volatile (
                "add %0, %0, %1\n\t"
                "mul %0, %0, %2\n\t"
                : "+r" (temp1)
                : "r" (temp2), "r" (c[i])
                : "r0", "r1", "cc", "memory"
            );
            
            /* Complex arithmetic with multiple dependencies */
            a[i] = temp1 + (a[i] & 0xFF) - (b[i] % 7);
            
            /* Additional arithmetic to increase II */
            volatile int temp3 = a[i] * 3;
            volatile int temp4 = b[i] * 2;
            
            __asm__ volatile (
                "and %0, %0, %1\n\t"
                "orr %0, %0, %2\n\t"
                : "+r" (temp3)
                : "r" (temp4), "r" (0xFFFF)
                : "r2", "r3", "cc"
            );
            
            b[i] = (temp3 >> 4) | (temp4 << 4);
            
            /* Conditional break to create multiple exit points */
            volatile int check = a[i];
            if (check > THRESHOLD) {
                /* Force compiler to consider this path */
                __asm__ volatile ("nop" ::: "memory");
                break;
            }
            
            /* Another conditional break based on computation */
            volatile int mod_check = a[i] % 13;
            if (mod_check == 0 && i < inner_bound / 2) {
                /* Create control flow complexity */
                c[i] = c[i] * 2;
                if (c[i] > 1000) {
                    break;
                }
            }
            
            /* Additional dependency chain */
            volatile int temp5 = a[i] + b[i];
            __asm__ volatile (
                "eor %0, %0, %1\n\t"
                : "+r" (temp5)
                : "r" (c[i])
                : "r4", "cc"
            );
            
            c[i-1] = (c[i-1] + temp5) & 0xFFF;
            
            i--;
        }
        
        /* Cross-iteration dependency to prevent loop unrolling from simplifying */
        if (outer > 0) {
            a[0] = a[SIZE-1] ^ b[0];
        }
    }
    
    /* Consume results to prevent dead code elimination */
    consume_result((int*)a, SIZE);
    consume_result((int*)b, SIZE);
    consume_result((int*)c, SIZE);
    
    /* Return checksum */
    int checksum = 0;
    for (int i = 0; i < SIZE; i += 16) {
        checksum ^= a[i] ^ b[i] ^ c[i];
    }
    
    return checksum & 0xFF;
}
