/* auto-inc-dec-trigger.c
 * Program designed to trigger auto-increment/decrement RTL pattern matching
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stddef.h>
#include <stdio.h>

/* Prevent inlining to preserve RTL patterns */
__attribute__((noinline)) void consume_array(volatile int* arr, int size) {
    volatile int sink = 0;
    for (int i = 0; i < size; i++) {
        sink += arr[i];
    }
    /* Use sink to prevent optimization */
    __asm__ volatile("" : : "r"(sink) : "memory");
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
    
    /* Create multiple pointer variables to generate distinct base registers */
    volatile int* src_ptr = &src[0];
    volatile int* dst_ptr = &dst[0];
    
    /* Additional pointer for more (mem (reg)) patterns */
    volatile int* p = &src[10];
    
    /* Main processing loop - designed to generate (mem (plus (reg) (const_int 0))) patterns */
    for (int i = 0; i < limit; i++) {
        /* Pattern 1: Load from src[0] - should generate (mem (reg)) or (mem (plus (reg) (const_int 0))) */
        int val1 = src[0];  /* Constant index 0 */
        
        /* Pattern 2: Load via pointer with zero offset */
        int val2 = *p;      /* Equivalent to p[0] */
        
        /* Arithmetic operation to create register pressure */
        int result = val1 + val2 + 7;  /* Loop-invariant constant */
        
        /* Pattern 3: Store to dst[0] - another (mem (reg)) pattern */
        dst[0] = result;    /* Constant index 0 */
        
        /* Scalar operation to generate (set (reg) (mem (reg))) pattern */
        accumulator += result;
        
        /* Inline assembly barrier with register clobbers
         * Creates artificial dependencies and prevents reordering */
        __asm__ volatile(
            ""
            : /* no outputs */
            : /* no inputs */
            : "memory", "r0", "r1", "r2", "r3"
        );
        
        /* Additional memory operations to increase pattern density */
        volatile int temp = src[5];  /* Another constant index access */
        dst[5] = temp + accumulator;
        
        /* More pointer-based accesses */
        volatile int* q = &dst[15];
        *q = accumulator;
        
        /* Another barrier to separate memory operations */
        __asm__ volatile("" : : : "memory");
    }
    
    /* Second loop with different access patterns */
    for (int i = 1; i < limit - 10; i++) {
        /* Array indexing with variable but simple pattern */
        int idx = i & 1;  /* Results in 0 or 1 */
        
        /* These should generate (mem (plus (reg) (const_int 0))) when idx=0 */
        int a = src[idx];
        dst[idx] = a * 2;
        
        /* More complex but still simple addressing */
        volatile int* r = &src[i % 4];
        volatile int* s = &dst[i % 4];
        *s = *r + i;
        
        /* Barrier with specific register clobbers for ARM */
        __asm__ volatile(
            ""
            : 
            : 
            : "r4", "r5", "r6", "r7", "memory"
        );
    }
    
    /* Use the results to prevent dead code elimination */
    consume_array(dst, 100);
    consume_array(src, 100);
    
    /* Calculate checksum */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= dst[i];
        checksum += src[i];
    }
    
    /* Use checksum to prevent optimization */
    __asm__ volatile("" : : "r"(checksum) : "memory");
    
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}
