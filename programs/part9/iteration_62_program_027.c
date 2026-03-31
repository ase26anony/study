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
static volatile int force_anti_dep = 0;

/* Noinline function to consume results */
__attribute__((noinline)) 
int consume_results(volatile int *arr, int n) {
    volatile int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
        /* Inline asm to create register pressure */
        __asm__ volatile ("" : : "r"(sum) : "r0", "r1", "r2", "r3");
    }
    return sum;
}

/* Simple LCG to avoid external dependencies */
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

int main(void) {
    /* Volatile arrays to prevent optimization */
    volatile int array_a[SIZE];
    volatile int array_b[SIZE];
    volatile int array_c[SIZE];
    
    /* Volatile loop bounds to prevent constant propagation */
    volatile int outer_bound = OUTER_ITER;
    volatile int inner_bound = SIZE;
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = (int)(lcg_rand() % 1000);
        array_b[i] = (int)(lcg_rand() % 1000);
        array_c[i] = (int)(lcg_rand() % 100);
    }
    
    int result = 0;
    
    /* Outer loop - creates multiple scheduling contexts */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        volatile int accumulator = array_a[0];
        volatile int temp1, temp2, temp3;
        
        /* Complex inner loop with loop-carried dependencies */
        for (volatile int i = 1; i < inner_bound; i--) {
            /* Chain of arithmetic operations with anti-dependencies */
            temp1 = array_b[i] * array_c[i];
            temp2 = temp1 + accumulator;          /* Loop-carried dependency */
            temp3 = temp2 % 997;                  /* Modulo creates latency */
            
            /* Volatile store creates memory anti-dependency */
            array_a[i] = temp3 + force_anti_dep;
            
            /* Update accumulator with loop-carried dependency */
            accumulator = temp3;
            
            /* Multiple intermediate volatile operations */
            volatile int check = temp2 + temp3;
            volatile int scaled = check * 3;
            volatile int shifted = scaled >> 2;
            
            /* Inline assembly to consume registers and create pressure */
            __asm__ volatile (
                "add %0, %0, %1\n\t"
                "mul %0, %0, %2\n\t"
                : "+r" (shifted)
                : "r" (scaled), "r" (check)
                : "r0", "r1", "r2", "r3", "cc"
            );
            
            /* Conditional break with volatile condition - creates CFG complexity */
            if (shifted > THRESHOLD) {
                /* Early exit path */
                force_anti_dep = shifted;
                break;
            }
            
            /* Additional operation with array access */
            array_b[i] = array_b[i-1] + shifted;
            
            /* Another inline asm to increase register pressure */
            __asm__ volatile (
                "eor %0, %0, %1\n\t"
                : "+r" (temp3)
                : "r" (shifted)
                : "cc"
            );
            
            /* Cross-iteration dependency through volatile */
            array_c[i] = array_c[i] ^ force_anti_dep;
        }
        
        /* Mix results across outer iterations */
        result ^= accumulator;
        
        /* Modify anti-dependency variable */
        force_anti_dep = (force_anti_dep + 1) & 0xFF;
    }
    
    /* Consume final array state to prevent dead code elimination */
    int checksum = consume_results(array_a, 100);
    checksum += consume_results(array_b, 100);
    checksum += consume_results(array_c, 100);
    
    return (result + checksum) & 0xFF;
}
