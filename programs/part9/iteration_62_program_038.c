/* modulo-sched-coverage.c
 * Designed to trigger GCC's modulo scheduler debug prints for PSG moves.
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -dP -march=armv7-a -c modulo-sched-coverage.c
 * Or for x86: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -funroll-loops -fdump-rtl-sms -dP -march=x86-64 -c modulo-sched-coverage.c
 */

#include <stdint.h>

#define SIZE 256
#define THRESHOLD 1000000

/* Prevent dead code elimination */
__attribute__((noinline)) 
int consume_result(volatile int* arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
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
    /* Volatile to prevent optimization and create anti-dependencies */
    volatile int a[SIZE];
    volatile int b[SIZE];
    volatile int c[SIZE];
    
    /* Volatile loop bounds to prevent constant propagation */
    volatile int outer_bound = 5;
    volatile int inner_bound = SIZE;
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < SIZE; i++) {
        a[i] = (int)(lcg_rand() % 1000);
        b[i] = (int)(lcg_rand() % 1000);
        c[i] = (int)(lcg_rand() % 100);
    }
    
    /* Complex nested loop designed to trigger aggressive modulo scheduling */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Count-down loop with volatile counter */
        volatile int i = inner_bound - 1;
        
        while (i > 0) {
            /* Chain of arithmetic operations with loop-carried dependencies */
            volatile int temp1 = b[i] * c[i];
            volatile int temp2 = a[i-1] + temp1;
            
            /* Inline assembly to create register pressure and anti-dependencies */
            __asm__ volatile (
                "add %0, %0, %1\n\t"
                "mul %1, %1, %2\n\t"
                : "+r" (temp2), "+r" (temp1)
                : "r" (c[i])
                : "r0", "r1", "cc"
            );
            
            /* Complex calculation with modulo to create long latency chains */
            volatile int temp3 = (temp2 * 37) % 7919;
            volatile int temp4 = (temp1 * 73) % 7919;
            
            /* More inline assembly with different clobbers */
            __asm__ volatile (
                "eor %0, %0, %1\n\t"
                "orr %1, %1, %0\n\t"
                : "+r" (temp3), "+r" (temp4)
                :
                : "r2", "r3", "cc"
            );
            
            /* Final assignment with loop-carried dependency */
            a[i] = temp3 + temp4 + a[i-1];
            
            /* Conditional break to create multiple exit points */
            volatile int check = a[i];
            if (check > THRESHOLD) {
                /* Additional inline assembly before break */
                __asm__ volatile (
                    "mov r4, %0\n\t"
                    "mov r5, %1\n\t"
                    :
                    : "r" (check), "r" (i)
                    : "r4", "r5"
                );
                break;
            }
            
            /* Another conditional break based on volatile calculation */
            volatile int mod_check = (i * 17) % 13;
            if (mod_check == 0 && i > inner_bound/2) {
                /* Force register spillage */
                volatile int spill1 = a[i] * b[i];
                volatile int spill2 = c[i] * spill1;
                volatile int spill3 = spill2 % 97;
                __asm__ volatile ("" : : "r" (spill1), "r" (spill2), "r" (spill3));
                if (spill3 > 50) break;
            }
            
            i--;
        }
        
        /* Modify array b for next outer iteration to create cross-iteration dependencies */
        for (volatile int j = 1; j < SIZE; j++) {
            b[j] = (b[j] + a[j-1]) % 1000;
        }
    }
    
    /* Prevent dead code elimination */
    int result = consume_result((volatile int*)a, SIZE);
    
    return result % 256;
}
