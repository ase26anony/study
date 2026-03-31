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
void consume_result(volatile int *arr, int n, int *checksum) {
    volatile int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
        /* Inline asm to create register pressure */
        __asm__ volatile ("" : : "r"(arr[i]) : "r0", "r1", "r2", "r3");
    }
    *checksum = sum;
}

/* Simple LCG PRNG to avoid external dependencies */
static inline uint32_t lcg(uint32_t *state) {
    *state = *state * 1103515245 + 12345;
    return *state;
}

int main(void) {
    /* Volatile to prevent optimization and create anti-dependencies */
    volatile int array_a[SIZE];
    volatile int array_b[SIZE];
    volatile int array_c[SIZE];
    
    /* Volatile iteration counters to create non-constant bounds */
    volatile int outer_bound = OUTER_ITER;
    volatile int inner_bound = SIZE;
    
    uint32_t seed = 42;
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = (int)(lcg(&seed) % 1000);
        array_b[i] = (int)(lcg(&seed) % 1000);
        array_c[i] = (int)(lcg(&seed) % 100) + 1;
    }
    
    volatile int accumulator = 0;
    volatile int temp1, temp2, temp3;
    
    /* Outer loop with volatile bound */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Count-down inner loop to affect scheduler heuristics */
        volatile int i = inner_bound - 1;
        
        while (i > 0) {
            /* Complex chain of operations with loop-carried dependencies */
            temp1 = array_b[i] * array_c[i];
            
            /* Inline asm to consume registers and create pressure */
            __asm__ volatile (""
                : "=r"(temp1) 
                : "0"(temp1)
                : "r4", "r5", "r6", "r7", "r8", "r9", "r10");
            
            /* Loop-carried dependency: a[i] depends on a[i-1] */
            temp2 = array_a[i-1] + temp1;
            
            /* Another operation creating register pressure */
            temp3 = temp2 % (array_c[i] + 1);
            
            /* Volatile intermediate to prevent optimization */
            volatile int intermediate = temp3;
            
            /* Conditional break based on volatile computation */
            if (intermediate > THRESHOLD) {
                /* Multiple exit points to complicate CFG */
                break;
            }
            
            /* Store result with anti-dependency on previous read */
            array_a[i] = intermediate + accumulator;
            
            /* Update accumulator with cross-iteration dependency */
            accumulator = (accumulator * 3 + array_a[i]) % 997;
            
            /* Additional conditional break */
            if (accumulator < 0 && (i % 7 == 0)) {
                break;
            }
            
            i--;
            
            /* More inline asm for register pressure */
            __asm__ volatile (""
                : 
                : "r"(i), "r"(accumulator)
                : "r11", "r12", "r14", "r15");
        }
        
        /* Modify array_c for next outer iteration to create varying dependencies */
        for (int j = 0; j < SIZE; j += 8) {
            array_c[j] = (array_c[j] * 2) % 97;
        }
    }
    
    /* Consume result to prevent dead code elimination */
    int final_checksum = 0;
    consume_result((volatile int*)array_a, SIZE, &final_checksum);
    
    return final_checksum % 256;
}
