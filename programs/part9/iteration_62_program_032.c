/* modulo-sched-coverage.c
 * Designed to trigger GCC's modulo scheduler debug prints for PSG moves.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -dP -march=armv7-a modulo-sched-coverage.c -o modulo-sched-coverage
 * Or for x86: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -funroll-loops -fdump-rtl-all -march=x86-64 modulo-sched-coverage.c -o modulo-sched-coverage
 */

#include <stdint.h>

#define SIZE 1024
#define OUTER_ITER 5
#define THRESHOLD 0x7FFFFFFF

/* Prevent optimization of critical variables */
static volatile int force_keep = 0;

/* Noinline function to consume results */
__attribute__((noinline)) 
static void consume_results(volatile int* arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    force_keep = sum;
}

/* Simple LCG for pseudo-random values */
static inline uint32_t lcg(uint32_t* state) {
    *state = *state * 1103515245 + 12345;
    return *state;
}

int main(void) {
    /* Volatile arrays to prevent optimization */
    volatile int a[SIZE];
    volatile int b[SIZE];
    volatile int c[SIZE];
    
    /* Volatile loop counters and bounds */
    volatile int outer_bound = OUTER_ITER;
    volatile int inner_bound = SIZE;
    volatile int break_condition = 0;
    
    uint32_t seed = 42;
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < SIZE; i++) {
        a[i] = (int)(lcg(&seed) % 1000);
        b[i] = (int)(lcg(&seed) % 1000);
        c[i] = (int)(lcg(&seed) % 1000);
    }
    
    /* Complex nested loop designed to trigger modulo scheduling */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Count-down inner loop with volatile counter */
        volatile int i = inner_bound - 1;
        
        while (i > 0) {
            /* Create loop-carried dependency chain */
            int temp1 = b[i] * c[i];
            
            /* Inline assembly to create register pressure and anti-dependencies */
            __asm__ volatile (
                "add %0, %0, %1\n\t"
                : "+r" (temp1)
                : "r" (a[i-1])
                : "r0", "r1", "cc"
            );
            
            /* Complex arithmetic with multiple operations */
            int temp2 = temp1 % 997;
            temp2 = temp2 * 13 + 7;
            
            /* Another inline asm to consume registers */
            __asm__ volatile (
                "orr %0, %0, #0xFF\n\t"
                : "+r" (temp2)
                :
                : "r2", "r3", "cc"
            );
            
            /* Store with dependency on previous iteration */
            a[i] = temp2 + (a[i-1] & 0x1F);
            
            /* Create anti-dependency through volatile access */
            int dummy = break_condition;
            (void)dummy;
            
            /* Multiple exit points based on volatile condition */
            if (a[i] > THRESHOLD) {
                /* Early exit path 1 */
                break_condition = 1;
                break;
            }
            
            if (i % 17 == 0 && a[i] < -THRESHOLD) {
                /* Early exit path 2 */
                break_condition = 2;
                break;
            }
            
            /* Additional arithmetic to increase II */
            c[i] = (b[i] * 3 + c[i-1]) % 1001;
            b[i] = (a[i] * 2 - b[i-1]) & 0xFFF;
            
            /* Count-down decrement with volatile access */
            i--;
            
            /* Artificial dependency on outer loop */
            if (outer % 2 == 0) {
                a[i+1] += outer;
            }
        }
        
        /* Cross-iteration dependency */
        if (outer > 0) {
            a[0] = a[SIZE-1] / 2;
        }
    }
    
    /* Consume results to prevent dead code elimination */
    consume_results((volatile int*)a, SIZE > 100 ? 100 : SIZE);
    
    /* Return checksum */
    int checksum = 0;
    for (int i = 0; i < (SIZE > 50 ? 50 : SIZE); i++) {
        checksum ^= a[i];
    }
    
    return checksum & 0xFF;
}
