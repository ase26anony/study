/* modulo-sched-coverage.c
 * Program designed to trigger GCC's modulo scheduler debug output
 * for uncovered lines in modulo-sched.cc (lines 596-606)
 */

#include <stdint.h>

/* Prevent optimization of critical variables */
#define VOLATILE volatile
#define NOINLINE __attribute__((noinline))

/* Simple LCG for pseudo-random values without external dependencies */
static uint32_t lcg_seed = 123456789;
static uint32_t lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

/* Dummy function to prevent dead code elimination */
NOINLINE void consume_data(VOLATILE int* arr, int size) {
    VOLATILE int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
        /* Inline asm to create register pressure */
        __asm__ volatile ("" : : "r" (arr[i]) : "r0", "r1", "r2", "r3");
    }
    __asm__ volatile ("" : : "r" (sum));
}

int main(void) {
    /* Volatile variables to prevent optimization */
    VOLATILE int outer_iterations = 5;
    VOLATILE int array_size = 1000;
    VOLATILE int threshold = 1000000;
    
    /* Volatile arrays to create memory dependencies */
    VOLATILE int a[1000];
    VOLATILE int b[1000];
    VOLATILE int c[1000];
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < array_size; i++) {
        a[i] = (int)(lcg_rand() % 100);
        b[i] = (int)(lcg_rand() % 100);
        c[i] = (int)(lcg_rand() % 100);
    }
    
    /* Outer loop to increase scheduling complexity */
    for (VOLATILE int outer = 0; outer < outer_iterations; outer++) {
        VOLATILE int n = array_size;
        VOLATILE int accumulator = 0;
        
        /* Inner loop with count-down structure and loop-carried dependencies */
        for (VOLATILE int i = n - 1; i > 0; i--) {
            /* Complex chain of arithmetic operations with volatile intermediates */
            VOLATILE int temp1 = b[i] * c[i];
            VOLATILE int temp2 = a[i - 1] * 7;
            VOLATILE int temp3 = temp1 + temp2;
            
            /* Loop-carried dependency: current depends on previous */
            a[i] = temp3 % 997 + a[i - 1];
            
            /* Additional operations to increase register pressure */
            VOLATILE int temp4 = b[i] * 13;
            VOLATILE int temp5 = c[i] * 17;
            VOLATILE int temp6 = temp4 + temp5;
            
            /* Anti-dependency through volatile */
            b[i] = temp6 % 991 + b[i - 1];
            
            /* Inline assembly to consume registers and prevent optimization */
            __asm__ volatile (
                "add %0, %0, %1\n\t"
                "mul %2, %2, %3\n\t"
                : "+r" (accumulator), "+r" (temp3)
                : "r" (temp6), "r" (temp1)
                : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                  "r8", "r9", "r10", "r11", "r12", "r14"
            );
            
            /* Conditional break with volatile condition - creates multiple exits */
            VOLATILE int check_val = accumulator + a[i];
            if (check_val > threshold) {
                /* Force early exit to complicate control flow */
                __asm__ volatile ("" : : "r" (check_val) : "memory");
                break;
            }
            
            /* Additional dependency chain */
            VOLATILE int temp7 = c[i] * 19;
            VOLATILE int temp8 = a[i] * 23;
            c[i] = (temp7 + temp8) % 983 + c[i - 1];
            
            /* More inline asm for register pressure */
            __asm__ volatile (
                "orr %0, %0, %1\n\t"
                "eor %2, %2, %3\n\t"
                : "+r" (accumulator), "+r" (temp7)
                : "r" (temp8), "r" (check_val)
                : "r0", "r1", "r2", "r3", "r4", "r5"
            );
        }
        
        /* Cross-iteration dependency */
        a[0] = b[array_size - 1] + c[array_size - 1];
        
        /* Additional computation between outer loop iterations */
        VOLATILE int inter_outer = 0;
        for (VOLATILE int j = 0; j < 10; j++) {
            inter_outer += a[j] * b[j];
            __asm__ volatile ("" : : "r" (inter_outer) : "r0", "r1");
        }
    }
    
    /* Consume results to prevent elimination */
    consume_data(a, array_size);
    consume_data(b, array_size);
    consume_data(c, array_size);
    
    /* Return checksum */
    VOLATILE int checksum = 0;
    for (int i = 0; i < array_size; i++) {
        checksum = (checksum * 31 + a[i]) % 1000000007;
    }
    
    return checksum & 0xFF;  /* Return lower byte as exit code */
}
