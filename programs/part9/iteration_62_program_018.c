/* modulo-sched-coverage.c
 * Designed to trigger GCC's modulo scheduler debug prints for PSG moves.
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -dP -march=armv7-a modulo-sched-coverage.c -o modulo-sched-test
 * Or for x86: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -funroll-loops -fdump-rtl-sms -dP -march=x86-64 modulo-sched-coverage.c -o modulo-sched-test
 */

#include <stdint.h>

#define ARRAY_SIZE 1024
#define OUTER_ITER 5

/* Prevent dead code elimination */
static void __attribute__((noinline)) consume_result(volatile int* arr, int size) {
    volatile int sink = 0;
    for (int i = 0; i < size; i++) {
        sink += arr[i];
    }
    __asm__ volatile ("" : : "r"(sink) : "memory");
}

/* Simple LCG to generate pseudo-random values without external dependencies */
static inline uint32_t lcg(uint32_t* state) {
    *state = *state * 1103515245 + 12345;
    return *state;
}

int main(void) {
    /* Volatile to prevent optimization and create anti-dependencies */
    volatile int a[ARRAY_SIZE];
    volatile int b[ARRAY_SIZE];
    volatile int c[ARRAY_SIZE];
    
    /* Volatile loop counters to prevent constant propagation */
    volatile int outer_bound = OUTER_ITER;
    volatile int inner_bound = ARRAY_SIZE - 1;
    
    /* Initialize with pseudo-random values */
    uint32_t seed = 42;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        a[i] = (int)(lcg(&seed) % 1000);
        b[i] = (int)(lcg(&seed) % 1000);
        c[i] = (int)(lcg(&seed) % 100) + 1;  /* Ensure non-zero for modulo */
    }
    
    /* Complex nested loop designed to trigger aggressive modulo scheduling */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Count-down loop with volatile bound */
        volatile int i = inner_bound;
        
        while (i > 0) {
            /* Create loop-carried dependencies with multiple operations */
            volatile int temp1 = b[i] * c[i];
            volatile int temp2 = a[i-1] + 12345;
            
            /* Inline assembly to consume registers and create pressure */
            __asm__ volatile (
                "add %0, %0, %1\n\t"
                "mul %0, %0, %2\n\t"
                : "+r" (temp1)
                : "r" (temp2), "r" (c[i])
                : "r0", "r1", "cc", "memory"
            );
            
            /* Complex chain of arithmetic with modulo operation */
            volatile int temp3 = temp1 % c[i];
            temp3 = temp3 * 7 + 3;
            
            /* More inline assembly with different clobbers */
            __asm__ volatile (
                "and %0, %0, #255\n\t"
                "orr %0, %0, %1\n\t"
                : "+r" (temp3)
                : "r" (a[i-1])
                : "r2", "r3", "cc"
            );
            
            /* Final assignment with loop-carried dependency */
            a[i] = temp3 + b[i] * a[i-1];
            
            /* Conditional break based on volatile computation */
            volatile int break_cond = a[i] - b[i];
            if (break_cond > 50000) {  /* Arbitrary large threshold */
                /* Another inline assembly to complicate control flow */
                __asm__ volatile ("nop" : : : "memory");
                break;
            }
            
            /* Additional conditional break with different condition */
            if (i % 17 == 0) {  /* Non-constant modulo */
                volatile int check = c[i] * outer;
                if (check < -1000) {  /* Unlikely but possible */
                    break;
                }
            }
            
            i--;
            
            /* Insert memory barrier to prevent reordering */
            __asm__ volatile ("" : : : "memory");
        }
        
        /* Modify array b for next outer iteration */
        for (int j = 1; j < ARRAY_SIZE; j++) {
            b[j] = (b[j] + a[j-1]) % 1000;
        }
    }
    
    /* Consume results to prevent dead code elimination */
    consume_result((volatile int*)a, ARRAY_SIZE);
    
    /* Return checksum */
    int checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum = (checksum * 31 + a[i]) & 0x7FFFFFFF;
    }
    
    return checksum % 256;
}
