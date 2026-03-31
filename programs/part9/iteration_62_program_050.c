/* modulo-sched-coverage.c
 * Designed to trigger GCC's modulo scheduler debug prints for PSG moves.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -dP -march=armv7-a modulo-sched-coverage.c -o modulo-sched-coverage
 * Or for x86: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -funroll-loops -fdump-rtl-sms -dP -march=x86-64 modulo-sched-coverage.c -o modulo-sched-coverage
 */

#include <stdint.h>

#define SIZE 256
#define OUTER_ITER 5

/* Prevent dead code elimination */
__attribute__((noinline)) 
static void consume_result(volatile int* arr, int n, volatile int* sum) {
    for (int i = 0; i < n; i++) {
        *sum += arr[i];
    }
}

/* Simple LCG to generate pseudo-random values without external dependencies */
static inline int lcg_rand(int* state) {
    *state = (*state * 1103515245 + 12345) & 0x7fffffff;
    return *state;
}

int main(void) {
    /* Volatile to prevent optimization and create anti-dependencies */
    volatile int a[SIZE];
    volatile int b[SIZE];
    volatile int c[SIZE];
    
    /* Volatile loop bounds to prevent constant propagation */
    volatile int n = SIZE;
    volatile int outer_bound = OUTER_ITER;
    
    /* Initialize with pseudo-random values */
    int seed = 42;
    for (int i = 0; i < SIZE; i++) {
        a[i] = lcg_rand(&seed) % 100;
        b[i] = lcg_rand(&seed) % 100;
        c[i] = lcg_rand(&seed) % 100;
    }
    
    volatile int checksum = 0;
    
    /* Outer loop with volatile bound */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Complex inner loop with loop-carried dependencies */
        volatile int prev = a[0];
        
        /* Count-down loop to affect scheduler heuristics */
        for (volatile int i = n - 1; i > 0; i--) {
            /* Chain of arithmetic operations creating data dependencies */
            volatile int temp1 = b[i] * c[i];
            volatile int temp2 = temp1 + prev;        /* Loop-carried: uses prev from previous iteration */
            volatile int temp3 = temp2 % 7919;        /* Large prime to prevent simplification */
            
            /* Inline assembly to create register pressure and anti-dependencies */
            __asm__ volatile (
                "mov %0, %0\n\t"
                "add %1, %1, #1"
                : "+r" (temp3), "+r" (temp2)
                : 
                : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                  "r8", "r9", "r10", "r11", "r12"
            );
            
            /* Conditional break to create multiple exit points */
            volatile int should_break = (temp3 > 5000 && i < n/2);
            if (should_break) {
                /* Additional computation before break to create more scheduling constraints */
                volatile int break_temp = temp3 * 2;
                __asm__ volatile ("nop" : : : "memory");
                if (break_temp > 10000) {
                    break;
                }
            }
            
            /* More operations to increase resource usage */
            volatile int temp4 = temp3 ^ 0x55AA55AA;
            volatile int temp5 = (temp4 << 3) | (temp4 >> 29);
            
            /* Store with dependency chain */
            a[i] = temp5 + a[i-1];  /* Another loop-carried dependency */
            prev = temp3;           /* Update for next iteration */
            
            /* Additional volatile operations to prevent reordering */
            volatile int dummy = c[i] + outer;
            __asm__ volatile ("" : : "r" (dummy) : "memory");
        }
        
        /* Cross-iteration dependency in outer loop */
        a[0] = a[0] + outer;
        
        /* More inline assembly to consume registers */
        __asm__ volatile (
            "eor r0, r0, r0\n\t"
            "add r1, r1, #1\n\t"
            "mul r2, r1, r0"
            : : : "r0", "r1", "r2", "r3", "cc", "memory"
        );
    }
    
    /* Consume result to prevent elimination */
    volatile int final_sum = 0;
    consume_result((int*)a, SIZE, &final_sum);
    
    /* Additional computation to ensure all values are used */
    for (int i = 0; i < SIZE; i++) {
        checksum += b[i] + c[i];
    }
    
    checksum += final_sum;
    
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}
