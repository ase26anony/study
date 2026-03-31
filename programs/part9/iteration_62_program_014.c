/* modulo-sched-coverage.c
 * Designed to trigger GCC's modulo scheduler debug prints for PSG moves.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -dP -march=armv7-a modulo-sched-coverage.c -o modulo-sched-coverage
 * Or with: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -funroll-loops -fdump-rtl-all -march=x86-64 modulo-sched-coverage.c -o modulo-sched-coverage
 */

#include <stdint.h>

#define ARRAY_SIZE 1024
#define OUTER_ITERATIONS 5
#define THRESHOLD 1000000

/* Prevent dead code elimination */
__attribute__((noinline)) 
void consume_result(volatile int* arr, int size, volatile int* sink) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    *sink = sum;
}

/* Simple LCG to generate pseudo-random values without external dependencies */
static inline int lcg_rand(int* state) {
    *state = (*state * 1103515245 + 12345) & 0x7fffffff;
    return *state;
}

int main(void) {
    /* Volatile to prevent optimization and create anti-dependencies */
    volatile int a[ARRAY_SIZE];
    volatile int b[ARRAY_SIZE];
    volatile int c[ARRAY_SIZE];
    
    volatile int seed = 42;
    volatile int sink = 0;
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        a[i] = lcg_rand((int*)&seed) % 100;
        b[i] = lcg_rand((int*)&seed) % 100;
        c[i] = lcg_rand((int*)&seed) % 10 + 1; /* Avoid division by zero */
    }
    
    /* Volatile loop bounds to prevent constant propagation */
    volatile int outer_bound = OUTER_ITERATIONS;
    volatile int inner_bound = ARRAY_SIZE - 1;
    
    /* Complex nested loop designed to trigger aggressive modulo scheduling */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Count-down loop with volatile counter */
        volatile int i = inner_bound;
        
        while (i > 0) {
            /* Create loop-carried dependency chain */
            volatile int temp1 = a[i];
            volatile int temp2 = b[i];
            volatile int temp3 = c[i];
            
            /* Complex arithmetic with multiple operations */
            volatile int result = temp1 * temp3;
            result += temp2;
            result %= 997; /* Prime modulus to prevent simplification */
            
            /* Loop-carried dependency: use previous iteration's value */
            volatile int prev = a[i-1];
            result = result * prev;
            
            /* Inline assembly to create register pressure and anti-dependencies */
            __asm__ volatile (
                "mov %0, %0\n\t"  /* Fake operation on result */
                : "+r" (result)
                : 
                : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                  "r8", "r9", "r10", "r11", "r12"
            );
            
            /* Store with potential anti-dependency */
            a[i] = result;
            
            /* Additional operations to increase II */
            volatile int extra = b[i] * c[i];
            extra += a[i-1];
            extra &= 0xFF;
            
            /* Conditional break with volatile condition */
            volatile int break_cond = (result > THRESHOLD) || (extra < 0);
            if (break_cond) {
                /* Multiple exit points complicate scheduling */
                break;
            }
            
            /* Another conditional break point */
            volatile int alt_break = (i % 37 == 0) && (result < -THRESHOLD);
            if (alt_break) {
                break;
            }
            
            /* More arithmetic to create resource conflicts */
            volatile int tmp = c[i] * 13;
            tmp += b[i-1];
            tmp /= (c[i] | 1); /* Avoid division by zero */
            
            /* Another inline asm to consume registers */
            __asm__ volatile (
                "add %0, %0, #1\n\t"
                : "+r" (tmp)
                :
                : "cc", "memory"
            );
            
            b[i] = tmp;
            
            i--; /* Count down */
        }
        
        /* Mix up arrays between outer iterations */
        volatile int shuffle_temp = a[0];
        for (int j = 1; j < ARRAY_SIZE; j++) {
            a[j-1] = a[j] + b[j] * c[j];
        }
        a[ARRAY_SIZE-1] = shuffle_temp;
    }
    
    /* Consume result to prevent dead code elimination */
    consume_result((int*)a, ARRAY_SIZE, (int*)&sink);
    
    /* Return checksum */
    return sink & 0xFF;
}
