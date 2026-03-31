/* auto_inc_dec_test.c
 * Designed to trigger auto-increment/decrement RTL pattern matching
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stddef.h>

/* Prevent inlining to preserve RTL patterns */
__attribute__((noinline)) 
void use_result(volatile int* arr, int n) {
    volatile int dummy = 0;
    for (int i = 0; i < n; i++) {
        dummy += arr[i];
    }
    /* Prevent optimization */
    __asm__ volatile("" : : "r"(dummy) : "memory");
}

int main(void) {
    /* Volatile arrays to prevent optimization of memory accesses */
    volatile int src[100];
    volatile int dst[100];
    
    /* Initialize source array with sequential values */
    for (int i = 0; i < 100; i++) {
        src[i] = i * 3 + 1;
    }
    
    /* Non-constant loop bound to prevent unrolling */
    int limit = 90;
    volatile int accumulator = 0;
    
    /* 
     * Critical processing loop - designed to generate (mem (plus (reg) (const_int 0))) patterns
     * Multiple memory references with constant index 0
     */
    for (int i = 0; i < limit; i++) {
        /* Pattern 1: Load from src[0] - generates (mem (plus (reg) (const_int 0))) */
        int val1 = src[0];
        
        /* Pattern 2: Another load with constant offset 0 */
        int val2 = src[1];  /* Will be (mem (plus (reg) (const_int 4))) on most arches */
        
        /* Arithmetic operation to create register pressure */
        int result = val1 + val2 + 7;  /* Loop-invariant constant */
        
        /* Pattern 3: Store to dst[0] - generates (set (mem (plus (reg) (const_int 0))) (reg)) */
        dst[0] = result;
        
        /* Inline assembly barrier with register clobbers
         * Creates artificial dependencies and prevents reordering
         */
        __asm__ volatile(
            ""
            : 
            : "r"(val1), "r"(val2), "r"(result)
            : "memory"
#ifdef __arm__
            , "r0", "r1", "r2", "r3"
#endif
        );
        
        /* Pattern 4: Pointer-based access with zero offset */
        volatile int *p = &src[10];
        int val3 = *p;  /* Generates (mem (reg)) pattern */
        
        /* Store to another array location */
        dst[5] = val3 + accumulator;
        
        /* Update accumulator to create loop-carried dependency */
        accumulator += 1;
        
        /* Additional memory barrier */
        __asm__ volatile("" : : : "memory");
    }
    
    /* 
     * Second loop with different access patterns
     * Mixes scalar and array operations
     */
    volatile int temp = 0;
    for (int i = 0; i < 50; i++) {
        /* Load-modify-store pattern */
        temp = src[i];
        temp = temp * 2 + 1;
        dst[i + 20] = temp;
        
        /* Another zero-offset access */
        int val = src[0];
        dst[0] = val;
        
        /* Memory barrier with specific register clobbers for ARM */
        __asm__ volatile(
            ""
            : 
            : "r"(temp), "r"(val)
            : "memory"
#ifdef __arm__
            , "r4", "r5"
#endif
        );
    }
    
    /* 
     * Struct access pattern - can generate base+offset patterns
     */
    struct pair {
        volatile int a;
        volatile int b;
        volatile int c;
    };
    
    struct pair data[10];
    for (int i = 0; i < 10; i++) {
        data[i].a = src[i];
        data[i].b = src[i + 1];
        
        /* Access with zero offset from base */
        int base_val = data[0].a;
        data[0].c = base_val + 1;
        
        __asm__ volatile("" : : "r"(base_val) : "memory");
    }
    
    /* Prevent dead code elimination */
    use_result(dst, 100);
    
    /* Calculate checksum to ensure all operations matter */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= dst[i];
    }
    
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}
