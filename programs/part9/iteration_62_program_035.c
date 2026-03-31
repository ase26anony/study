/* modulo-sched-coverage.c
 * Designed to trigger GCC's modulo scheduler debug prints for PSG moves.
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -dP -march=armv7-a -c modulo-sched-coverage.c
 */

#include <stdint.h>

/* Prevent optimizations */
#define VOL volatile
#define NOINLINE __attribute__((noinline))

/* Simple PRNG to avoid external dependencies */
static uint32_t lcg_state = 123456789;
static uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Dummy function to prevent dead code elimination */
NOINLINE void consume_data(VOL int* arr, int size) {
    VOL int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
        __asm__ volatile ("" : : "r"(sum) : "r0", "r1");
    }
    __asm__ volatile ("" : : "r"(sum));
}

int main(void) {
    /* Volatile arrays to create memory dependencies */
    VOL int a[256];
    VOL int b[256];
    VOL int c[256];
    
    /* Volatile loop counters and bounds */
    VOL int outer_bound = 5;
    VOL int inner_bound = 100;
    VOL int threshold = 1000000;
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < 256; i++) {
        a[i] = (int)(lcg_rand() % 1000);
        b[i] = (int)(lcg_rand() % 1000);
        c[i] = (int)(lcg_rand() % 1000);
    }
    
    /* Complex nested loop designed to trigger aggressive modulo scheduling */
    for (VOL int outer = 0; outer < outer_bound; outer++) {
        VOL int start_idx = (lcg_rand() % 100) + 50;
        
        /* Count-down inner loop with multiple dependencies */
        for (VOL int i = inner_bound; i > 0; i--) {
            /* Create loop-carried dependency chain */
            VOL int idx = (start_idx + i) % 256;
            VOL int prev_idx = (idx == 0) ? 255 : idx - 1;
            
            /* Complex arithmetic with volatile intermediates */
            VOL int temp1 = b[idx] * c[idx];
            VOL int temp2 = a[prev_idx] + temp1;
            VOL int temp3 = temp2 % 7919;  /* Large prime for modulo */
            
            /* Inline assembly to create register pressure */
            __asm__ volatile (
                "add %0, %0, %1\n\t"
                "mul %1, %1, %2\n\t"
                : "+r"(temp1), "+r"(temp2)
                : "r"(temp3)
                : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                  "r8", "r9", "r10", "r11", "r12"
            );
            
            /* Anti-dependency through volatile */
            a[idx] = temp1 + temp2 + temp3;
            
            /* Multiple exit points based on volatile condition */
            VOL int check_val = a[idx] + b[idx];
            if (check_val > threshold) {
                /* Early exit creates control flow complexity */
                __asm__ volatile ("" : : "r"(check_val) : "r0", "r1");
                break;
            }
            
            /* Additional dependency chain */
            VOL int temp4 = c[idx] * 37;
            VOL int temp5 = a[idx] - temp4;
            
            /* More inline assembly for register pressure */
            __asm__ volatile (
                "and %0, %0, %1\n\t"
                "orr %1, %1, %2\n\t"
                : "+r"(temp4), "+r"(temp5)
                : "r"(i)
                : "r0", "r1", "r2", "r3"
            );
            
            b[idx] = temp5 ^ temp4;
            
            /* Second conditional exit */
            if (i % 13 == 0) {
                VOL int mod_check = b[idx] % 17;
                if (mod_check == 0) {
                    __asm__ volatile ("" : : "r"(mod_check));
                    i -= 3;  /* Skip iterations */
                }
            }
        }
        
        /* Cross-iteration dependency */
        VOL int rotate = a[0] % 32;
        for (VOL int j = 0; j < 256; j++) {
            VOL int new_idx = (j + rotate) % 256;
            c[new_idx] = (c[new_idx] + b[j]) % 997;
        }
    }
    
    /* Prevent dead code elimination */
    consume_data(a, 256);
    consume_data(b, 256);
    consume_data(c, 256);
    
    /* Return checksum */
    VOL int checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum = (checksum * 31 + a[i]) % 1000000007;
        checksum = (checksum * 31 + b[i]) % 1000000007;
        checksum = (checksum * 31 + c[i]) % 1000000007;
    }
    
    return checksum & 0xFF;
}
