/* modulo-sched-coverage.c
 * Designed to trigger GCC's modulo scheduler debug prints for PSG moves.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -dP -march=armv7-a modulo-sched-coverage.c -o modulo-sched-coverage
 * Or for x86: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -funroll-loops -fdump-rtl-all -march=x86-64 modulo-sched-coverage.c -o modulo-sched-coverage
 */

#include <stdint.h>

#define SIZE 1024
#define OUTER_ITER 5
#define THRESHOLD 1000000

/* Prevent optimization of critical variables */
static volatile int force_volatile = 0;

/* Dummy function to prevent dead code elimination */
__attribute__((noinline, used))
static int consume_array(volatile int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
        /* Force memory access */
        __asm__ volatile ("" : : "r" (arr[i]) : "memory");
    }
    return sum;
}

/* Simple PRNG without external dependencies */
static uint32_t lcg_seed = 12345;
static uint32_t lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

int main(void) {
    /* Volatile arrays to prevent optimization */
    volatile int a[SIZE];
    volatile int b[SIZE];
    volatile int c[SIZE];
    
    /* Volatile loop counters and bounds */
    volatile int outer_bound = OUTER_ITER + (force_volatile & 0);
    volatile int inner_bound = SIZE + (force_volatile & 0);
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < SIZE; i++) {
        a[i] = (int)(lcg_rand() % 1000);
        b[i] = (int)(lcg_rand() % 1000);
        c[i] = (int)(lcg_rand() % 100) + 1;  /* Avoid division by zero */
    }
    
    int result = 0;
    
    /* Outer loop to increase scheduling pressure */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Complex inner loop with loop-carried dependencies */
        volatile int i = inner_bound - 1;  /* Count-down for scheduling complexity */
        
        while (i > 0) {
            /* Create artificial register pressure with inline asm */
            register int r0 asm("r0") = a[i];
            register int r1 asm("r1") = b[i];
            register int r2 asm("r2") = c[i % 256];  /* Use modulo for non-uniform access */
            
            /* Complex chain of arithmetic operations with dependencies */
            int temp1 = r0 * r2;
            int temp2 = temp1 + r1;
            int temp3 = temp2 % (r2 + 1);  /* Avoid division by zero */
            
            /* Loop-carried dependency: a[i] depends on a[i-1] */
            int prev = a[i-1];
            int temp4 = temp3 + prev;
            
            /* More operations to increase latency */
            temp4 = temp4 * 7;
            temp4 = temp4 - (temp4 / 3);
            
            /* Volatile intermediate to prevent optimization */
            volatile int final_val = temp4;
            
            /* Store with anti-dependency on previous value */
            a[i] = final_val;
            
            /* Inline asm with register clobbers to increase pressure */
            __asm__ volatile (
                "add %0, %0, %1\n\t"
                "sub %1, %1, %2"
                : "+r" (r0), "+r" (r1)
                : "r" (r2)
                : "cc", "memory"
            );
            
            /* Conditional break based on computed value */
            if (final_val > THRESHOLD) {
                /* Force unpredictable exit */
                if (force_volatile) break;
            }
            
            /* Another conditional break for multiple exit points */
            if (i < (inner_bound / 4) && (final_val & 0xFF) == 0) {
                /* Force compiler to consider this path */
                __asm__ volatile ("" : : "r" (final_val) : "memory");
                if (force_volatile) break;
            }
            
            /* Additional operations to increase II */
            b[i] = (b[i] * 3 + a[i]) % 997;
            
            /* Count-down with volatile to prevent optimization */
            volatile int next_i = i - 1;
            i = next_i;
            
            /* More inline asm for register pressure */
            __asm__ volatile (
                "mov r3, %0\n\t"
                "mov r4, %1\n\t"
                : 
                : "r" (temp3), "r" (temp4)
                : "r3", "r4", "memory"
            );
        }
        
        /* Modify array c between outer iterations */
        for (int j = 0; j < 256; j++) {
            c[j] = (c[j] * 13 + outer) % 999;
        }
    }
    
    /* Consume results to prevent elimination */
    result = consume_array(a, 64);  /* Only check first 64 elements */
    
    /* Additional computation to ensure loop isn't dead */
    for (int i = 0; i < 32; i++) {
        result += b[i] - c[i % 256];
    }
    
    return result & 0xFF;  /* Return non-zero checksum */
}
