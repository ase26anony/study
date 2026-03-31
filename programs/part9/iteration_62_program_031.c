/* modulo-sched-coverage.c
 * Designed to trigger GCC's modulo scheduler debug output for PSG moves.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -dP -march=armv7-a modulo-sched-coverage.c -o modulo-sched-coverage
 * Or for x86: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -funroll-loops -fdump-rtl-all -march=x86-64 modulo-sched-coverage.c -o modulo-sched-coverage
 */

#include <stdint.h>

/* Prevent dead code elimination */
__attribute__((noinline)) 
static void consume_result(volatile int *arr, int n) {
    volatile int sink = 0;
    for (int i = 0; i < n; i++) {
        sink += arr[i];
    }
    __asm__ volatile ("" : : "r"(sink) : "memory");
}

/* Main test function with complex loop structure */
int main(void) {
    /* Volatile to prevent optimization and create anti-dependencies */
    volatile int N = 1000;
    volatile int outer_bound = 5;
    volatile int threshold = 1000000;
    
    /* Arrays with volatile accesses to create memory dependencies */
    volatile int a[1002], b[1002], c[1002];
    
    /* Simple LCG for pseudo-random initialization */
    uint32_t seed = 123456789;
    for (int i = 0; i < 1002; i++) {
        seed = seed * 1103515245 + 12345;
        a[i] = (seed >> 16) & 0x7FFF;
        b[i] = (seed >> 8) & 0xFF;
        c[i] = seed & 0xFF;
    }
    
    /* Complex nested loop structure to trigger aggressive modulo scheduling */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        volatile int n = N;
        volatile int accumulator = a[0];
        
        /* Count-down loop with volatile bound - affects scheduler heuristics */
        for (volatile int i = n; i > 0; i--) {
            /* Chain of arithmetic operations with loop-carried dependencies */
            volatile int temp1 = b[i] * c[i];
            volatile int temp2 = temp1 + accumulator;  /* RAW dependency */
            volatile int temp3 = temp2 % 97;           /* Creates modulo operation */
            
            /* Anti-dependency: a[i] read before write in next statement */
            volatile int temp4 = a[i] + temp3;
            
            /* Loop-carried dependency: a[i] depends on a[i-1] via accumulator */
            a[i] = temp4;
            accumulator = a[i];  /* Feed into next iteration */
            
            /* Inline assembly to create register pressure and clobbers */
            __asm__ volatile (
                "add %0, %0, %1\n\t"
                "mul %0, %0, %2\n\t"
                : "+r" (temp4)
                : "r" (temp3), "r" (i)
                : "r0", "r1", "cc", "memory"
            );
            
            /* Multiple exit points based on volatile condition */
            if (temp4 > threshold) {
                /* Early exit creates control flow complexity */
                volatile int check = b[i] * 2;
                if (check > threshold / 2) {
                    break;
                }
            }
            
            /* Additional arithmetic to increase II */
            volatile int temp5 = c[i] * a[i-1];
            volatile int temp6 = temp5 ^ b[i];  /* XOR operation */
            
            /* More inline assembly for register pressure */
            __asm__ volatile (
                "eor %0, %0, %1\n\t"
                : "+r" (temp6)
                : "r" (i)
                : "r2", "cc"
            );
            
            /* Another conditional break point */
            if (temp6 < 0 && (i % 7 == 0)) {
                volatile int mod_check = i % 11;
                if (mod_check == 0) {
                    break;
                }
            }
            
            /* Cross-iteration dependency through array b */
            b[i] = b[i-1] + c[i] + (i & 0xF);
        }
        
        /* Modify loop bound for next outer iteration */
        N = N - 50;
    }
    
    /* Force result consumption to prevent elimination */
    consume_result((int*)a, 100);
    consume_result((int*)b, 100);
    consume_result((int*)c, 100);
    
    /* Return checksum to ensure all computation matters */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum = (checksum * 31 + a[i]) & 0xFFFF;
    }
    
    return checksum;
}
