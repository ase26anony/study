/* modulo-sched-coverage.c
 * Program designed to trigger GCC's modulo scheduling debug prints
 * for uncovered lines in modulo-sched.cc (lines 596-606)
 */

#include <stdint.h>

/* Prevent dead code elimination */
__attribute__((noinline))
static void consume_result(volatile int* arr, int size, volatile int* sink) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    *sink = sum;
}

/* Simple PRNG without external dependencies */
static inline uint32_t lcg_parkmiller(uint32_t* state) {
    return *state = (uint64_t)*state * 48271 % 0x7fffffff;
}

int main(void) {
    /* Volatile variables to prevent optimization and create anti-dependencies */
    volatile int outer_bound = 5;
    volatile int inner_bound = 1000;
    volatile int threshold = 0x7FFFFFFF;
    volatile int sink = 0;
    
    /* Arrays with volatile accesses to create memory dependencies */
    volatile int array_a[1024];
    volatile int array_b[1024];
    volatile int array_c[1024];
    
    /* Initialize arrays with pseudo-random values */
    uint32_t seed = 123456789;
    for (int i = 0; i < 1024; i++) {
        array_a[i] = (int)(lcg_parkmiller(&seed) % 1000);
        array_b[i] = (int)(lcg_parkmiller(&seed) % 1000);
        array_c[i] = (int)(lcg_parkmiller(&seed) % 1000);
    }
    
    /* Complex nested loop structure to force modulo scheduling */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Count-down loop with volatile bound - affects scheduler heuristics */
        volatile int i = inner_bound;
        
        while (i > 0) {
            /* Create loop-carried dependency chain */
            int idx = i - 1;
            
            /* Complex arithmetic operations with volatile intermediates */
            volatile int temp1 = array_b[idx] * array_c[idx];
            volatile int temp2 = array_a[idx] + temp1;
            
            /* Loop-carried dependency: current depends on previous */
            volatile int prev = (idx > 0) ? array_a[idx - 1] : 0;
            volatile int temp3 = temp2 * prev;
            
            /* Modulo operation creates variable latency */
            volatile int temp4 = temp3 % 997;
            
            /* Multiple operations with anti-dependencies */
            volatile int temp5 = temp4 + array_b[idx];
            volatile int temp6 = temp5 - array_c[idx];
            
            /* Inline assembly to create register pressure and clobbers */
            __asm__ volatile (
                "add %0, %0, %1\n\t"
                "sub %0, %0, %2"
                : "+r" (temp6)
                : "r" (array_a[idx]), "r" (prev)
                : "r0", "r1", "cc"
            );
            
            /* Additional inline assembly for more register pressure */
            __asm__ volatile (
                "mov %0, %0\n\t"
                "orr %0, %0, #0"
                : "+r" (temp6)
                :
                : "cc"
            );
            
            /* Store with potential overflow check */
            array_a[idx] = temp6;
            
            /* Conditional break with volatile condition - creates multiple exits */
            volatile int check_val = temp6;
            if (check_val > threshold) {
                /* Force early exit path */
                break;
            }
            
            /* Another conditional break based on complex condition */
            volatile int mod_check = temp6 % 13;
            if (mod_check == 0 && (i % 7) == 0) {
                /* Different exit path */
                break;
            }
            
            /* Decrement with volatile to prevent optimization */
            i--;
        }
        
        /* Additional computation between outer loop iterations */
        volatile int inter_iter = array_a[0] * array_b[0];
        __asm__ volatile ("" : : "r" (inter_iter) : "r2", "r3");
    }
    
    /* Consume result to prevent dead code elimination */
    consume_result((int*)array_a, 1024, &sink);
    
    /* Return checksum */
    return sink & 0xFF;
}
