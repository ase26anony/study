/* modulo-sched-coverage.c
 * Program designed to trigger GCC's modulo scheduling debug output
 * for lines 596-606 in modulo-sched.cc
 */

#include <stdint.h>

/* Prevent dead code elimination */
__attribute__((noinline))
static void consume_result(volatile int* arr, int size, volatile int* sink) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
        __asm__ volatile ("" : : "r"(arr[i]) : "memory");
    }
    *sink = sum;
}

/* Simple LCG for pseudo-random values */
static inline uint32_t lcg(uint32_t* state) {
    *state = *state * 1103515245 + 12345;
    return *state;
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
    
    /* Initialize with pseudo-random values */
    uint32_t seed = 42;
    for (int i = 0; i < 1024; i++) {
        array_a[i] = (int)(lcg(&seed) % 100);
        array_b[i] = (int)(lcg(&seed) % 100);
        array_c[i] = (int)(lcg(&seed) % 100);
    }
    
    /* Complex nested loop designed to trigger aggressive modulo scheduling */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Count-down loop with volatile bound - affects scheduler heuristics */
        volatile int n = inner_bound;
        
        /* Multiple loop-carried dependencies with varying latencies */
        volatile int acc1 = array_a[0];
        volatile int acc2 = array_b[0];
        volatile int acc3 = array_c[0];
        
        /* Inline assembly to consume registers and increase pressure */
        __asm__ volatile (
            "mov %0, %0\n\t"
            "mov %1, %1\n\t"
            : "+r"(acc1), "+r"(acc2)
            : 
            : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9"
        );
        
        /* Main computation loop with complex data dependencies */
        for (volatile int i = n; i > 0; i--) {
            /* Chain of arithmetic operations creating loop-carried dependencies */
            int idx = i % 1024;
            int prev_idx = (i - 1) % 1024;
            
            /* Operation 1: Multiply with accumulation (creates RAW dependency) */
            volatile int temp1 = array_b[idx] * acc1;
            __asm__ volatile ("" : : "r"(temp1) : "r0", "r1");
            
            /* Operation 2: Modulo operation (different latency) */
            volatile int temp2 = temp1 % (array_c[idx] + 1);
            __asm__ volatile ("" : : "r"(temp2) : "r2", "r3");
            
            /* Operation 3: Addition with previous iteration (loop-carried) */
            volatile int temp3 = temp2 + array_a[prev_idx];
            __asm__ volatile ("" : : "r"(temp3) : "r4", "r5");
            
            /* Operation 4: Bitwise operations */
            volatile int temp4 = (temp3 ^ acc2) & 0xFFFF;
            __asm__ volatile ("" : : "r"(temp4) : "r6", "r7");
            
            /* Update accumulators with anti-dependencies */
            acc1 = temp4 + acc3;
            acc2 = temp3 - acc1;
            acc3 = temp2 * acc2;
            
            /* Store result with memory dependency */
            array_a[idx] = acc1;
            
            /* Conditional break with multiple exit points */
            volatile int check = acc1 + acc2 + acc3;
            if (check > threshold) {
                /* Early exit - creates control flow complexity */
                __asm__ volatile ("" : : "r"(check) : "r8", "r9");
                break;
            }
            
            /* Additional conditional with different computation */
            if ((i % 7) == 0) {
                volatile int extra = array_b[idx] * array_c[idx];
                array_b[idx] = extra + i;
                __asm__ volatile ("" : : "r"(extra) : "r10", "r11");
            }
            
            /* More register pressure */
            __asm__ volatile (
                "add %0, %0, %1\n\t"
                "sub %2, %2, %3\n\t"
                : "+r"(acc1), "+r"(acc2), "+r"(acc3)
                : "r"(i)
                : "r12", "r13", "r14", "r15"
            );
        }
        
        /* Cross-iteration dependency */
        array_b[outer % 1024] = acc1;
        array_c[outer % 1024] = acc2;
    }
    
    /* Force result consumption to prevent elimination */
    consume_result((int*)array_a, 1024, &sink);
    
    return sink & 0xFF;
}
