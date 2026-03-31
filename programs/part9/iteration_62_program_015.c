/* modulo-sched-coverage.c
 * Program designed to trigger GCC's modulo scheduling debug output
 * for uncovered lines in modulo-sched.cc (lines 596-606)
 */

#include <stdint.h>

/* Prevent dead code elimination */
static volatile int sink;

/* Noinline function to consume results */
__attribute__((noinline))
static void consume_result(volatile int* arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    sink = sum;
}

/* Simple PRNG without external dependencies */
static uint32_t lcg_state = 123456789;
static uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

int main(void) {
    /* Volatile variables to prevent optimization and create anti-dependencies */
    volatile int n = 1000;
    volatile int threshold = 1000000;
    volatile int outer_bound = 5;
    
    /* Arrays with volatile accesses to create memory dependencies */
    volatile int a[1002] = {0};
    volatile int b[1002] = {0};
    volatile int c[1002] = {0};
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < 1002; i++) {
        a[i] = (int)(lcg_rand() % 100);
        b[i] = (int)(lcg_rand() % 100);
        c[i] = (int)(lcg_rand() % 100);
    }
    
    /* Outer loop to increase scheduling complexity */
    volatile int outer_iter = 0;
    while (outer_iter < outer_bound) {
        /* Complex inner loop with loop-carried dependencies */
        volatile int i = n;
        volatile int temp1, temp2, temp3;
        
        while (i > 0) {
            /* Chain of arithmetic operations with loop-carried dependency */
            temp1 = a[i] * b[i];          /* Multiplication */
            temp2 = temp1 + c[i];         /* Addition */
            temp3 = a[i-1] + temp2;       /* Loop-carried: depends on previous iteration */
            
            /* Modulo operation to create complex dependency */
            a[i] = temp3 % 997;
            
            /* Inline assembly to create register pressure and anti-dependencies */
            __asm__ volatile (
                "add %0, %0, %1\n\t"
                "sub %1, %1, %0\n\t"
                : "+r" (temp1), "+r" (temp2)
                :
                : "cc", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                  "r8", "r9", "r10", "r11", "r12", "r14"
            );
            
            /* Conditional break to create multiple exit points */
            if (temp3 > threshold) {
                /* Additional arithmetic to increase path complexity */
                b[i] = (b[i] * 3) / 2;
                break;
            }
            
            /* Another conditional with different computation */
            if (temp2 < 0) {
                c[i] = c[i] * 2 - 1;
                /* Continue instead of break to create different path */
            } else {
                /* More operations to increase instruction mix */
                volatile int temp4 = b[i] * c[i];
                volatile int temp5 = temp4 ^ a[i-1];
                a[i] = (a[i] + temp5) & 0xFF;
                
                /* More inline assembly for register pressure */
                __asm__ volatile (
                    "mov r0, %0\n\t"
                    "mov r1, %1\n\t"
                    "mul r2, r0, r1\n\t"
                    : 
                    : "r" (temp4), "r" (temp5)
                    : "r0", "r1", "r2", "r3", "cc"
                );
            }
            
            /* Additional loop-carried dependency chain */
            volatile int chain1 = b[i] * 7;
            volatile int chain2 = chain1 + a[i-1];
            volatile int chain3 = chain2 % 256;
            b[i-1] = chain3 ^ c[i];
            
            i--;
        }
        
        /* Post-loop computation to prevent tail optimization */
        volatile int post_sum = 0;
        for (int j = 0; j < 10; j++) {
            post_sum += a[j] + b[j] + c[j];
        }
        
        /* More inline assembly between outer loop iterations */
        __asm__ volatile (
            "eor r0, r0, r0\n\t"
            "add r0, r0, #1\n\t"
            : 
            : 
            : "r0", "cc"
        );
        
        outer_iter++;
    }
    
    /* Consume results to prevent dead code elimination */
    consume_result((volatile int*)a, 1000);
    consume_result((volatile int*)b, 1000);
    consume_result((volatile int*)c, 1000);
    
    /* Return checksum */
    int checksum = 0;
    for (int i = 0; i < 1000; i++) {
        checksum = (checksum * 31 + a[i]) & 0x7FFFFFFF;
    }
    
    return checksum & 0xFF;
}
