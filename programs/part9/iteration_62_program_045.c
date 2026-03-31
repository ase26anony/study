/* modulo-sched-coverage.c
 * Designed to trigger GCC's modulo scheduler debug output for PSG moves.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -dP -march=armv7-a modulo-sched-coverage.c -o modulo-sched-coverage
 * Or for x86: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -funroll-loops -fdump-rtl-all -march=x86-64 modulo-sched-coverage.c -o modulo-sched-coverage
 */

#include <stdint.h>

/* Prevent dead code elimination */
__attribute__((noinline)) 
static void consume_result(volatile int* arr, int size, volatile int* sink) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
        /* Force memory dependency */
        __asm__ volatile ("" : : "r"(arr[i]) : "memory");
    }
    *sink = sum;
}

/* Simple LCG to generate pseudo-random values without external dependencies */
static inline int pseudo_rand(int* seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return *seed;
}

int main(void) {
    /* Volatile to prevent optimization and create anti-dependencies */
    volatile int array_a[256];
    volatile int array_b[256];
    volatile int array_c[256];
    
    volatile int loop_bound = 256;  /* Non-constant bound */
    volatile int outer_iter = 5;    /* Force multiple outer iterations */
    volatile int threshold = 1000000000;
    volatile int sink = 0;          /* Result sink */
    
    int seed = 42;
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < 256; i++) {
        array_a[i] = pseudo_rand(&seed) % 1000;
        array_b[i] = pseudo_rand(&seed) % 1000;
        array_c[i] = pseudo_rand(&seed) % 1000;
    }
    
    /* Complex nested loop structure to trigger aggressive modulo scheduling */
    for (volatile int outer = 0; outer < outer_iter; outer++) {
        /* Count-down loop with volatile counter */
        volatile int i = loop_bound - 1;
        
        while (i > 0) {
            /* Create loop-carried dependency chain */
            volatile int temp1 = array_b[i] * array_c[i];
            volatile int temp2 = array_a[i-1] + temp1;
            
            /* Inline assembly to create register pressure and anti-dependencies */
            __asm__ volatile (
                "add %0, %0, %1\n\t"
                "mul %0, %0, %2\n\t"
                : "+r"(temp2)
                : "r"(array_c[i]), "r"(i)
                : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "memory"
            );
            
            /* Complex arithmetic with modulo to create long latency operations */
            volatile int temp3 = (temp2 * 17) % 7919;  /* Prime modulo */
            volatile int temp4 = (temp3 + array_b[i]) % 9973;  /* Another prime */
            
            /* More inline assembly for additional register pressure */
            __asm__ volatile (
                "eor %0, %0, %1\n\t"
                "ror %0, %0, #3\n\t"
                : "+r"(temp4)
                : "r"(array_a[i])
                : "r0", "r1", "r2", "r3", "cc", "memory"
            );
            
            array_a[i] = temp4 + array_a[i-1];
            
            /* Conditional break with volatile condition - creates multiple exit points */
            volatile int break_cond = array_a[i];
            if (break_cond > threshold) {
                /* Early exit path */
                __asm__ volatile ("" : : "r"(break_cond) : "memory");
                break;
            }
            
            /* Another conditional break possibility */
            if (i % 37 == 0) {  /* Prime-based condition */
                volatile int mod_check = array_b[i] % array_c[i];
                if (mod_check == 0) {
                    __asm__ volatile ("" : : "r"(mod_check) : "memory");
                    /* Don't break here - just create the control flow edge */
                }
            }
            
            /* Additional arithmetic to increase computational density */
            volatile int temp5 = array_b[i] * i;
            volatile int temp6 = array_c[i] * (loop_bound - i);
            array_b[i] = (temp5 + temp6) % 65537;
            
            /* More inline assembly with clobbers */
            __asm__ volatile (
                "umull r0, r1, %0, %1\n\t"
                "add r0, r0, r1\n\t"
                "mov %0, r0\n\t"
                : "+r"(array_c[i])
                : "r"(i)
                : "r0", "r1", "cc", "memory"
            );
            
            i--;  /* Count down */
        }
        
        /* Modify loop bound slightly for next outer iteration */
        loop_bound = (loop_bound * 3) % 250 + 6;
    }
    
    /* Consume results to prevent elimination */
    consume_result((int*)array_a, 256, (int*)&sink);
    consume_result((int*)array_b, 256, (int*)&sink);
    consume_result((int*)array_c, 256, (int*)&sink);
    
    /* Return checksum */
    return sink & 0xFF;
}
