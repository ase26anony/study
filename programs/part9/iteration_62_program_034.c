/* modulo-sched-coverage.c
 * Designed to trigger GCC's modulo scheduler debug prints for PSG moves.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -dP -march=armv7-a modulo-sched-coverage.c -o modulo-sched-coverage
 * Or for x86: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -funroll-loops -fdump-rtl-sms -dP -march=x86-64 modulo-sched-coverage.c -o modulo-sched-coverage
 */

#include <stdint.h>

#define ARRAY_SIZE 1024
#define VOLATILE_THRESHOLD 1000000

/* Prevent optimization of critical values */
static volatile int loop_counter = ARRAY_SIZE;
static volatile int outer_bound = 5;
static volatile int break_condition = VOLATILE_THRESHOLD;

/* Noinline function to prevent DCE */
__attribute__((noinline)) 
static int consume_array(volatile int *arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
        __asm__ volatile ("" : : "r"(arr[i]) : "r0", "r1", "r2", "r3");
    }
    return sum;
}

/* Simple PRNG without external dependencies */
static uint32_t lcg_state = 123456789;
static uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

int main(void) {
    /* Volatile arrays to prevent optimization and create memory dependencies */
    volatile int a[ARRAY_SIZE];
    volatile int b[ARRAY_SIZE];
    volatile int c[ARRAY_SIZE];
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        a[i] = (int)(lcg_rand() % 1000);
        b[i] = (int)(lcg_rand() % 1000);
        c[i] = (int)(lcg_rand() % 1000);
    }
    
    /* Create complex loop-carried dependencies and register pressure */
    volatile int temp1, temp2, temp3;
    volatile int accumulator = 0;
    
    /* Outer loop with volatile bound */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Count-down inner loop with volatile counter */
        volatile int i = loop_counter - 1;
        
        while (i > 0) {
            /* Chain of arithmetic operations with loop-carried dependencies */
            temp1 = b[i] * c[i] + 12345;
            temp2 = temp1 % 7919;  /* Prime number for modulo operation */
            
            /* Anti-dependency: read before write with volatile */
            temp3 = a[i-1] + accumulator;
            
            /* Complex computation with multiple dependencies */
            a[i] = (temp2 * temp3 + a[i]) / 2;
            
            /* Inline assembly to create register pressure */
            __asm__ volatile (
                "add %0, %0, %1\n\t"
                "mul %0, %0, %2\n\t"
                : "+r" (accumulator)
                : "r" (temp2), "r" (temp3)
                : "r0", "r1", "r2", "r3", "cc"
            );
            
            /* Conditional break based on volatile value */
            if (accumulator > break_condition) {
                /* Multiple exit points complicate scheduling */
                break;
            }
            
            /* Additional computation with modulo to create long latency */
            volatile int mod_result = accumulator % 9973;
            if (mod_result == 0) {
                /* Another potential exit point */
                if (i < loop_counter / 2) break;
            }
            
            /* More register pressure */
            __asm__ volatile (
                "umull r0, r1, %0, %1\n\t"
                "adds r0, r0, %2\n\t"
                "adc r1, r1, #0\n\t"
                : 
                : "r" (a[i]), "r" (b[i]), "r" (c[i])
                : "r0", "r1", "cc"
            );
            
            i--;
        }
        
        /* Modify break condition to affect loop behavior */
        break_condition += 500000;
    }
    
    /* Consume results to prevent dead code elimination */
    int checksum = consume_array((volatile int*)a, ARRAY_SIZE);
    
    /* Also consume other arrays */
    checksum += consume_array((volatile int*)b, ARRAY_SIZE / 4);
    checksum += consume_array((volatile int*)c, ARRAY_SIZE / 4);
    
    return checksum % 256;
}
