/* modulo-sched-coverage.c
 * Designed to trigger GCC's modulo scheduler debug prints for PSG moves.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -dP -march=armv7-a modulo-sched-coverage.c -o modulo-sched-coverage
 * Or for x86: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -funroll-loops -fdump-rtl-sms -dP modulo-sched-coverage.c -o modulo-sched-coverage
 */

#include <stdint.h>

#define SIZE 1024
#define OUTER_ITER 5
#define THRESHOLD 0x7FFFFFFF

/* Prevent optimization of critical variables */
static volatile int use_result = 0;

/* Noinline function to consume results */
__attribute__((noinline)) 
static void consume_results(volatile int *arr, int n) {
    volatile int sink = 0;
    for (int i = 0; i < n; i++) {
        sink += arr[i];
    }
    use_result = sink;
}

/* Simple PRNG without external dependencies */
static uint32_t lcg_state = 123456789;
static uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

int main(void) {
    /* Volatile arrays to prevent optimization and create memory dependencies */
    volatile int array_a[SIZE];
    volatile int array_b[SIZE];
    volatile int array_c[SIZE];
    
    /* Volatile loop counters and bounds */
    volatile int outer_bound = OUTER_ITER;
    volatile int inner_bound = SIZE;
    volatile int threshold = THRESHOLD;
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = (int)(lcg_rand() % 1000);
        array_b[i] = (int)(lcg_rand() % 1000);
        array_c[i] = (int)(lcg_rand() % 100) + 1; /* Non-zero for modulo */
    }
    
    int result = 0;
    
    /* Outer loop with volatile bound */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Complex inner loop with multiple loop-carried dependencies */
        volatile int prev = array_a[0];
        volatile int acc = 0;
        volatile int temp1, temp2, temp3;
        
        /* Count-down loop with volatile bound - creates scheduling complexity */
        for (volatile int i = inner_bound - 1; i > 0; i--) {
            /* Chain of arithmetic operations with anti-dependencies */
            temp1 = array_b[i] * array_c[i];      /* Multiplication */
            temp2 = temp1 + prev;                 /* Addition with previous result */
            temp3 = temp2 % array_c[i];           /* Modulo operation */
            
            /* Loop-carried dependency: current depends on previous */
            array_a[i] = temp3 + array_a[i-1];
            
            /* Volatile intermediate to prevent optimization */
            prev = array_a[i];
            
            /* Artificial register pressure with inline assembly */
            __asm__ volatile (
                "mov %0, %0\n\t"
                "add %1, %1, %2"
                : "+r" (temp1), "+r" (temp2)
                : "r" (temp3)
                : /* Explicit clobber list to increase register pressure */
                "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                "r8", "r9", "r10", "r11", "r12"
            );
            
            /* Multiple exit points based on volatile condition */
            if (array_a[i] > threshold) {
                /* Early exit - creates control flow complexity */
                result = 1;
                break;
            }
            
            /* Additional computation to increase II */
            acc += array_b[i] * 3;
            array_b[i] = (array_b[i] ^ acc) & 0xFF;
            
            /* Another inline asm to create resource conflicts */
            __asm__ volatile (
                "mul %0, %1, %2"
                : "=r" (temp3)
                : "r" (array_c[i]), "r" (i)
                : "memory"
            );
        }
        
        /* Cross-iteration dependency through array_c */
        for (volatile int j = 1; j < SIZE - 1; j++) {
            array_c[j] = (array_c[j-1] + array_c[j+1]) / 2;
        }
    }
    
    /* Consume results to prevent dead code elimination */
    consume_results((volatile int*)array_a, SIZE > 100 ? 100 : SIZE);
    consume_results((volatile int*)array_b, SIZE > 100 ? 100 : SIZE);
    consume_results((volatile int*)array_c, SIZE > 100 ? 100 : SIZE);
    
    /* Return checksum */
    return result + (array_a[0] & 0xFF) + (array_b[0] & 0xFF);
}
