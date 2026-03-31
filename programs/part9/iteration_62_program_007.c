/* modulo-sched-coverage.c
 * Designed to trigger GCC's modulo scheduler debug prints for PSG moves.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -dP -march=armv7-a -c modulo-sched-coverage.c
 * Or for x86: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -funroll-loops -fdump-rtl-all -march=x86-64 -c modulo-sched-coverage.c
 */

#include <stdint.h>

/* Prevent optimization of critical variables */
#define VOLATILE volatile
#define NOINLINE __attribute__((noinline))

/* Simple LCG for pseudo-random values without external dependencies */
static inline uint32_t lcg(uint32_t *state) {
    *state = *state * 1103515245 + 12345;
    return *state;
}

/* Dummy function to consume results and prevent dead code elimination */
NOINLINE int64_t consume_results(VOLATILE int *arr, int size) {
    int64_t sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
        /* Force memory barrier */
        __asm__ volatile("" : : "r"(arr[i]) : "memory");
    }
    return sum;
}

int main(void) {
    /* Volatile iteration counters to prevent constant propagation */
    VOLATILE int outer_iter = 5;
    VOLATILE int inner_size = 1000;
    VOLATILE int threshold = 0x7FFFFFFF;
    
    /* Volatile arrays with data dependencies */
    VOLATILE int array_a[1002] = {0};  /* Extra elements for i-1 access */
    VOLATILE int array_b[1002] = {0};
    VOLATILE int array_c[1002] = {0};
    
    /* Initialize arrays with pseudo-random values */
    uint32_t seed = 42;
    for (int i = 0; i < 1002; i++) {
        array_a[i] = (int)(lcg(&seed) % 1000);
        array_b[i] = (int)(lcg(&seed) % 1000);
        array_c[i] = (int)(lcg(&seed) % 100) + 1;  /* Non-zero */
    }
    
    /* Complex nested loop designed to trigger modulo scheduling */
    for (VOLATILE int outer = 0; outer < outer_iter; outer++) {
        /* Count-down loop with volatile bound - affects scheduler heuristics */
        VOLATILE int i = inner_size;
        
        while (i > 0) {
            /* Volatile intermediates to create anti-dependencies */
            VOLATILE int idx = i;
            VOLATILE int temp1, temp2, temp3;
            
            /* Chain of arithmetic operations with loop-carried dependencies */
            temp1 = array_b[idx] * array_c[idx];      /* Multiply */
            temp2 = temp1 + array_a[idx - 1];         /* Add with previous iteration */
            temp3 = temp2 % (array_c[idx] + 1);       /* Modulo operation */
            
            /* Inline assembly to consume registers and create pressure */
            __asm__ volatile (
                "add %0, %0, %1\n\t"
                "mul %0, %0, %2"
                : "+r" (temp3)
                : "r" (temp1), "r" (temp2)
                : "r0", "r1", "r2", "r3", "cc"
            );
            
            array_a[idx] = temp3;
            
            /* Additional volatile operation to increase register pressure */
            VOLATILE int check = array_a[idx] * 3;
            
            /* Conditional break with multiple exit points */
            if (check > threshold) {
                /* Force complex control flow */
                __asm__ volatile("" : : "r"(check) : "memory");
                break;
            }
            
            /* Another conditional break possibility */
            if ((array_a[idx] & 0xFF) == 0) {
                VOLATILE int dummy = array_b[idx];
                __asm__ volatile("" : : "r"(dummy) : "memory");
                if (dummy < 100) break;
            }
            
            /* Secondary computation with anti-dependency */
            VOLATILE int aux = array_b[idx] + array_c[idx];
            array_b[idx] = aux ^ array_a[idx];
            
            /* More inline assembly to stress register allocator */
            __asm__ volatile (
                "eor %0, %0, %1\n\t"
                "add %0, %0, #1"
                : "+r" (aux)
                : "r" (idx)
                : "r4", "r5", "cc"
            );
            
            array_c[idx] = aux;
            
            i--;
        }
        
        /* Modify arrays between outer iterations to prevent optimization */
        VOLATILE int shift = outer * 7;
        for (int j = 1; j <= 10; j++) {
            array_a[j] += shift;
            array_b[j] ^= shift;
        }
    }
    
    /* Consume results to prevent dead code elimination */
    int64_t result = consume_results(array_a, 1000);
    
    /* Return checksum */
    return (int)(result & 0x7FFFFFFF);
}
