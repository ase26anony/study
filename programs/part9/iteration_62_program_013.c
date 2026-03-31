/* modulo-sched-coverage.c
 * Designed to trigger uncovered lines in GCC's modulo-sched.cc
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -dP -march=armv7-a modulo-sched-coverage.c -o modulo-sched-coverage
 * Or for x86: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -funroll-loops -fdump-rtl-sms -dP -march=x86-64 modulo-sched-coverage.c -o modulo-sched-coverage
 */

#include <stdint.h>

#define ARRAY_SIZE 1024
#define OUTER_ITERATIONS 5
#define BREAK_THRESHOLD 0x7FFFFFFF

/* Prevent dead code elimination */
__attribute__((noinline)) 
static void consume_result(volatile int *arr, int size, volatile int *checksum) {
    for (int i = 0; i < size; i++) {
        *checksum ^= arr[i];
    }
}

/* Simple LCG to avoid external dependencies */
static inline uint32_t lcg(uint32_t *state) {
    *state = *state * 1103515245 + 12345;
    return *state;
}

int main(void) {
    /* Volatile to prevent optimization and create anti-dependencies */
    volatile int a[ARRAY_SIZE];
    volatile int b[ARRAY_SIZE];
    volatile int c[ARRAY_SIZE];
    
    /* Volatile iteration counters and bounds */
    volatile int outer_bound = OUTER_ITERATIONS;
    volatile int inner_bound = ARRAY_SIZE - 1;
    volatile int break_condition = 0;
    
    /* Initialize with pseudo-random values */
    uint32_t seed = 42;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        a[i] = (int)(lcg(&seed) % 1000);
        b[i] = (int)(lcg(&seed) % 1000);
        c[i] = (int)(lcg(&seed) % 100) + 1;  /* Non-zero for modulo */
    }
    
    /* Complex nested loop structure to force aggressive modulo scheduling */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Count-down loop with volatile counter */
        volatile int i = inner_bound;
        
        while (i > 0) {
            /* Create loop-carried dependency chain */
            volatile int temp1 = b[i] * c[i];
            volatile int temp2 = a[i-1];
            
            /* Complex arithmetic with multiple operations */
            volatile int result = temp1 + temp2;
            
            /* Modulo operation creates expensive operation */
            volatile int mod_result = result % c[i];
            
            /* Anti-dependency through volatile */
            a[i] = mod_result + a[i];
            
            /* Inline assembly to consume registers and increase pressure */
            __asm__ volatile (
                "mov %0, %0\n\t"
                "add %1, %1, %2"
                : "+r" (temp1), "+r" (temp2)
                : "r" (mod_result)
                : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                  "r8", "r9", "r10", "r11", "r12", "r14"
            );
            
            /* Multiple exit points with volatile conditions */
            if (break_condition) {
                break;
            }
            
            /* Another conditional break based on computed value */
            if (result > BREAK_THRESHOLD) {
                break_condition = 1;
                break;
            }
            
            /* Additional arithmetic to increase instruction mix */
            volatile int extra = (b[i] << 3) | (c[i] & 0xFF);
            a[i] ^= extra;
            
            /* Another inline assembly to force register allocation complexity */
            __asm__ volatile (
                "eor %0, %0, %1\n\t"
                "orr %2, %2, %0"
                : "+r" (extra), "+r" (result)
                : "r" (temp1)
                : "r0", "r1", "r2", "r3"
            );
            
            i--;  /* Count down */
        }
        
        /* Modify array between outer iterations to prevent optimization */
        for (int j = 0; j < ARRAY_SIZE; j++) {
            b[j] = (b[j] + outer) % 1000;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    volatile int final_checksum = 0;
    consume_result((int*)a, ARRAY_SIZE, &final_checksum);
    
    return final_checksum & 0xFF;
}
