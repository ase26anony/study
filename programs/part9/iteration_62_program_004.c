/* modulo-sched-coverage.c
 * Designed to trigger uncovered lines in GCC's modulo-sched.cc
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -dP -march=armv7-a modulo-sched-coverage.c -o modulo-sched-coverage
 * Or for x86: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -funroll-loops -fdump-rtl-sms -dP -march=x86-64 modulo-sched-coverage.c -o modulo-sched-coverage
 */

#include <stdint.h>

#define ARRAY_SIZE 1024
#define OUTER_ITERATIONS 5
#define THRESHOLD 0x7FFFFFFF

/* Prevent dead code elimination */
__attribute__((noinline)) 
void consume_result(volatile int* arr, int size, volatile int* result) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
        /* Force memory access */
        __asm__ volatile ("" : : "r"(arr[i]) : "memory");
    }
    *result = sum;
}

/* Simple LCG to avoid external dependencies */
static inline uint32_t lcg_rand(uint32_t* state) {
    *state = *state * 1103515245 + 12345;
    return *state;
}

int main(void) {
    /* Volatile to prevent optimization and create anti-dependencies */
    volatile int a[ARRAY_SIZE];
    volatile int b[ARRAY_SIZE];
    volatile int c[ARRAY_SIZE];
    
    /* Volatile loop counters to prevent constant propagation */
    volatile int outer_bound = OUTER_ITERATIONS;
    volatile int inner_bound = ARRAY_SIZE;
    
    /* Seed for PRNG */
    uint32_t seed = 42;
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        a[i] = (int)(lcg_rand(&seed) % 1000);
        b[i] = (int)(lcg_rand(&seed) % 1000);
        c[i] = (int)(lcg_rand(&seed) % 1000) + 1; /* Ensure non-zero */
    }
    
    /* Critical nested loop structure */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Count-down loop with volatile bound */
        volatile int i = inner_bound - 1;
        
        while (i > 0) {
            /* Create complex loop-carried dependencies */
            volatile int temp1 = b[i] * c[i];
            volatile int temp2 = a[i-1];
            
            /* Inline assembly to consume registers and create pressure */
            __asm__ volatile (
                "add %0, %0, %1\n\t"
                "mul %0, %0, %2\n\t"
                : "+r" (temp1)
                : "r" (temp2), "r" (c[i])
                : "r0", "r1", "cc", "memory"
            );
            
            /* Chain of arithmetic operations with dependencies */
            a[i] = temp1 + (a[i] % c[i]) * 3;
            
            /* Additional operations to increase register pressure */
            volatile int temp3 = b[i-1] * 7;
            volatile int temp4 = a[i] ^ temp3;
            
            __asm__ volatile (
                "and %0, %0, %1\n\t"
                "orr %0, %0, %2\n\t"
                : "+r" (temp4)
                : "r" (temp1), "r" (temp3)
                : "r2", "r3", "cc", "memory"
            );
            
            b[i] = temp4 >> 2;
            
            /* Conditional break to create multiple exit points */
            volatile int check_val = a[i] + b[i];
            if (check_val > THRESHOLD) {
                /* Force anti-dependency */
                __asm__ volatile ("" : : "r"(check_val) : "memory");
                break;
            }
            
            /* Another conditional break possibility */
            if ((i & 0x3F) == 0) { /* Every 64 iterations */
                volatile int mod_check = c[i] % 17;
                if (mod_check == 0) {
                    __asm__ volatile ("" : : "r"(mod_check) : "memory");
                    break;
                }
            }
            
            /* More operations to increase II */
            volatile int temp5 = c[i] * a[i];
            volatile int temp6 = b[i] + temp5;
            
            __asm__ volatile (
                "sub %0, %0, %1\n\t"
                "rsb %0, %0, #0\n\t"
                : "+r" (temp6)
                : "r" (temp5)
                : "r4", "r5", "cc", "memory"
            );
            
            c[i] = (temp6 < 0) ? -temp6 : temp6;
            
            i--;
        }
        
        /* Modify array between outer iterations to prevent optimization */
        volatile int shuffle = outer * 37;
        for (int j = 0; j < 16; j++) {
            a[j] ^= shuffle;
            b[j] += shuffle;
            c[j] *= (shuffle % 31) + 1;
        }
    }
    
    /* Consume result to prevent elimination */
    volatile int final_result = 0;
    consume_result((int*)a, ARRAY_SIZE, &final_result);
    
    return final_result % 256;
}
