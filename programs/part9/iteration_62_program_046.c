/* modulo-sched-coverage.c
 * Designed to trigger GCC's modulo scheduler debug prints for PSG moves.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -dP -march=armv7-a -c modulo-sched-coverage.c
 * Or with: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -funroll-loops -fdump-rtl-all -march=x86-64 -c modulo-sched-coverage.c
 */

#include <stdint.h>

#define SIZE 1024
#define UNROLL_FACTOR 4

/* Prevent dead code elimination */
static volatile int sink;

/* Noinline function to consume results */
__attribute__((noinline)) 
void consume_results(volatile int* arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    sink = sum;
}

/* Simple PRNG without external dependencies */
static inline uint32_t lcg(uint32_t* state) {
    *state = *state * 1103515245 + 12345;
    return *state;
}

int main(void) {
    /* Volatile to prevent optimization and create anti-dependencies */
    volatile int a[SIZE];
    volatile int b[SIZE];
    volatile int c[SIZE];
    
    /* Volatile loop counters to prevent constant propagation */
    volatile int outer_bound = 5;
    volatile int inner_bound = SIZE;
    
    /* Initialize with pseudo-random values */
    uint32_t seed = 42;
    for (int i = 0; i < SIZE; i++) {
        a[i] = (int)(lcg(&seed) % 100);
        b[i] = (int)(lcg(&seed) % 100);
        c[i] = (int)(lcg(&seed) % 100);
    }
    
    /* Complex nested loop to trigger aggressive modulo scheduling */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Count-down loop with volatile bound - affects scheduler heuristics */
        volatile int i = inner_bound - 1;
        
        while (i > 0) {
            /* Create loop-carried dependency chain */
            volatile int temp1 = a[i] * c[i];
            volatile int temp2 = b[i] + 17;
            volatile int temp3 = temp1 % (temp2 + 1);
            
            /* Critical: loop-carried dependency with distance 1 */
            a[i] = temp3 + a[i-1] * 3;
            
            /* Additional arithmetic to increase register pressure */
            volatile int temp4 = b[i] * 7;
            volatile int temp5 = c[i] * 11;
            volatile int temp6 = temp4 + temp5;
            
            /* Inline assembly to consume registers and create pressure */
            __asm__ volatile (
                "add %0, %0, %1\n\t"
                "mul %0, %0, %2\n\t"
                : "+r" (temp6)
                : "r" (temp3), "r" (temp1)
                : "r0", "r1", "cc"
            );
            
            b[i] = temp6 ^ a[i];
            
            /* Multiple exit points - affects control flow */
            if (a[i] > 1000000) {
                /* Unlikely but possible exit */
                break;
            }
            
            /* Another conditional exit */
            if (i % 13 == 0 && b[i] < -1000000) {
                break;
            }
            
            /* Additional operations to increase II */
            volatile int temp7 = c[i] * a[i];
            volatile int temp8 = b[i] * 19;
            c[i] = (temp7 + temp8) % 97;
            
            /* More inline assembly with different clobbers */
            __asm__ volatile (
                "mov r2, %0\n\t"
                "add r2, r2, #1\n\t"
                "mov %0, r2\n\t"
                : "+r" (c[i])
                :
                : "r2", "cc"
            );
            
            /* Unrolled operations to increase scheduling complexity */
            for (int j = 0; j < UNROLL_FACTOR; j++) {
                volatile int unroll_temp = a[i] + j;
                a[i] = unroll_temp * (b[i] + j);
            }
            
            i--;
        }
        
        /* Cross-iteration dependency between outer loop iterations */
        if (outer > 0) {
            a[0] = a[SIZE-1] + outer;
        }
    }
    
    /* Consume results to prevent elimination */
    consume_results((volatile int*)a, SIZE);
    
    /* Return checksum */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum = (checksum * 31 + a[i]) % 1000000007;
    }
    
    return checksum;
}
