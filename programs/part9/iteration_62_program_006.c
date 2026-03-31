/* modulo-sched-coverage.c
 * Designed to trigger GCC's modulo scheduler debug prints for PSG moves.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -dP -march=armv7-a modulo-sched-coverage.c -o modulo-sched-coverage
 * Or for x86: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -funroll-loops -fdump-rtl-sms -dP -march=x86-64 modulo-sched-coverage.c -o modulo-sched-coverage
 */

#include <stdint.h>

#define SIZE 256
#define OUTER_ITER 5

/* Prevent dead code elimination */
static void __attribute__((noinline)) consume_array(volatile int *arr, int n) {
    volatile int sink = 0;
    for (int i = 0; i < n; i++) {
        sink += arr[i];
    }
    __asm__ volatile ("" : : "r"(sink) : "memory");
}

/* Simple LCG for pseudo-random values */
static inline uint32_t lcg(uint32_t *state) {
    *state = *state * 1103515245 + 12345;
    return *state;
}

int main(void) {
    /* Volatile arrays to prevent optimization and create dependencies */
    volatile int array_a[SIZE];
    volatile int array_b[SIZE];
    volatile int array_c[SIZE];
    
    /* Volatile loop counters and bounds */
    volatile int outer_bound = OUTER_ITER;
    volatile int inner_bound = SIZE;
    volatile int threshold = 0x7FFFFFFF;
    
    /* Initialize with pseudo-random values */
    uint32_t seed = 42;
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = (int)(lcg(&seed) % 1000);
        array_b[i] = (int)(lcg(&seed) % 1000);
        array_c[i] = (int)(lcg(&seed) % 100) + 1;
    }
    
    /* Complex nested loop designed to trigger modulo scheduling */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Count-down inner loop with volatile counter */
        volatile int i = inner_bound - 1;
        
        while (i > 0) {
            /* Create loop-carried dependency chain */
            volatile int temp1 = array_a[i] * array_c[i];
            volatile int temp2 = array_b[i] + temp1;
            
            /* Anti-dependency: read before write with volatile barrier */
            volatile int prev = array_a[i-1];
            
            /* Complex arithmetic with multiple operations */
            array_a[i] = temp2 + prev * 3;
            
            /* Inline assembly to create register pressure */
            __asm__ volatile (
                "add %0, %0, %1\n\t"
                "mul %0, %0, %2\n\t"
                : "+r" (array_a[i])
                : "r" (array_b[i]), "r" (array_c[i])
                : "r0", "r1", "r2", "r3", "cc", "memory"
            );
            
            /* Additional operations to increase II */
            volatile int temp3 = array_a[i] % (array_c[i] + 1);
            array_b[i] = temp3 ^ array_a[i-1];
            
            /* Conditional break to create multiple exit points */
            if (array_a[i] > threshold) {
                /* Force unpredictable exit */
                volatile int should_break = (array_a[i] & 0xFF) == 0;
                if (should_break) {
                    break;
                }
            }
            
            /* Another inline assembly to consume registers */
            __asm__ volatile (
                "mov r4, %0\n\t"
                "mov r5, %1\n\t"
                "add r4, r4, r5\n\t"
                : 
                : "r" (array_a[i]), "r" (array_b[i])
                : "r4", "r5", "cc"
            );
            
            i--;
        }
        
        /* Cross-iteration dependency */
        if (outer > 0) {
            array_a[0] = array_a[SIZE-1] + outer;
        }
    }
    
    /* Prevent dead code elimination */
    consume_array(array_a, SIZE);
    consume_array(array_b, SIZE);
    
    /* Return checksum */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum ^= array_a[i];
        checksum ^= array_b[i];
    }
    
    return checksum & 0xFF;
}
