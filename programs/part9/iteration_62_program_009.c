/* modulo-sched-coverage.c
 * Designed to trigger GCC's modulo scheduler debug prints for PSG moves.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -dP -march=armv7-a -c modulo-sched-coverage.c
 */

#include <stdint.h>

#define SIZE 1024
#define THRESHOLD 1000000

/* Prevent optimization of critical variables */
static volatile int dummy_result = 0;

/* Noinline function to prevent DCE */
__attribute__((noinline)) 
static void consume_result(volatile int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    dummy_result = sum;
}

/* Simple PRNG without external dependencies */
static inline uint32_t lcg(uint32_t *state) {
    *state = *state * 1103515245 + 12345;
    return *state;
}

int main(void) {
    /* Volatile arrays to prevent optimization */
    volatile int a[SIZE];
    volatile int b[SIZE];
    volatile int c[SIZE];
    
    /* Volatile loop counters to prevent constant propagation */
    volatile int outer_bound = 5;
    volatile int inner_bound = SIZE;
    
    /* PRNG state */
    uint32_t seed = 42;
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < SIZE; i++) {
        a[i] = (int)(lcg(&seed) % 1000);
        b[i] = (int)(lcg(&seed) % 1000);
        c[i] = (int)(lcg(&seed) % 100) + 1;
    }
    
    /* Complex nested loop structure to force aggressive modulo scheduling */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Count-down loop with volatile bound */
        volatile int i = inner_bound;
        
        /* Force register pressure with inline assembly clobbers */
        register int r0 asm("r0");
        register int r1 asm("r1");
        register int r2 asm("r2");
        register int r3 asm("r3");
        
        while (i > 0) {
            /* Create loop-carried dependency chain */
            int idx = i - 1;
            
            /* Complex arithmetic with volatile intermediates */
            volatile int temp1 = b[idx] * c[idx];
            volatile int temp2;
            
            if (idx > 0) {
                /* Loop-carried dependency: a[i] depends on a[i-1] */
                temp2 = temp1 + a[idx - 1];
            } else {
                temp2 = temp1;
            }
            
            /* More arithmetic operations to increase II */
            volatile int temp3 = temp2 % (c[idx] + 1);
            volatile int temp4 = temp3 * 7;
            volatile int temp5 = temp4 - (temp2 >> 3);
            
            /* Store result with anti-dependency */
            a[idx] = temp5;
            
            /* Inline assembly to consume registers and create pressure */
            __asm__ volatile (
                "add %0, %1, %2\n\t"
                "sub %3, %0, %1"
                : "=r"(r0), "=r"(r1), "=r"(r2), "=r"(r3)
                : "0"(temp5), "1"(idx), "2"(c[idx]), "3"(temp1)
                : "cc"
            );
            
            /* Conditional break based on volatile computation */
            volatile int check_val = a[idx];
            if (check_val > THRESHOLD) {
                /* Multiple exit points complicate scheduling */
                break;
            }
            
            /* Another conditional break possibility */
            if (check_val < -THRESHOLD) {
                break;
            }
            
            /* Additional arithmetic to increase resource usage */
            volatile int temp6 = b[idx] + (a[idx] >> 2);
            volatile int temp7 = temp6 * temp6;
            
            /* More inline assembly with different clobbers */
            __asm__ volatile (
                "mul %0, %1, %2"
                : "=r"(r0)
                : "r"(temp7), "r"(c[idx])
                : "cc"
            );
            
            i--;
        }
        
        /* Modify array b for next outer iteration */
        for (int j = 0; j < SIZE; j += 8) {
            b[j] = a[j] + outer;
        }
    }
    
    /* Consume results to prevent dead code elimination */
    consume_result((volatile int*)a, SIZE);
    
    return dummy_result % 256;
}
