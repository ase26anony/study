/* auto_inc_dec_test.c
 * Designed to trigger auto-increment/decrement RTL pattern matching
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stddef.h>
#include <stdio.h>

/* Prevent inlining to keep RTL patterns simple */
__attribute__((noinline)) 
void consume_result(volatile int* arr, int size) {
    volatile int sink = 0;
    for (int i = 0; i < size; i++) {
        sink += arr[i];
    }
    /* Use sink to prevent optimization */
    __asm__ volatile("" : : "r"(sink) : "memory");
}

int main(void) {
    /* Volatile arrays to prevent optimization of memory operations */
    volatile int src[100];
    volatile int dst[100];
    
    /* Initialize source array with sequential values */
    for (int i = 0; i < 100; i++) {
        src[i] = i * 3 + 1;
    }
    
    /* Non-constant loop bound to prevent unrolling */
    int limit = 90;
    volatile int checksum = 0;
    
    /* Create multiple pointer variables to generate distinct base registers */
    volatile int* p_src = &src[0];
    volatile int* p_dst = &dst[0];
    volatile int* p_alt = &src[10];
    
    /* Main processing loop - designed to generate (mem (plus (reg) (const_int 0))) patterns */
    for (int i = 0; i < limit; i++) {
        /* Pattern 1: Load from src[0] - should generate (mem (reg)) or (mem (plus (reg) (const_int 0))) */
        int val1 = src[0];  /* Constant index 0 */
        
        /* Pattern 2: Load via pointer with zero offset */
        int val2 = *p_src;  /* Equivalent to p_src[0] */
        
        /* Arithmetic operation to create register pressure */
        val1 = val1 + val2 + 7;  /* Loop-invariant constant */
        
        /* Pattern 3: Store to dst[0] */
        dst[0] = val1;  /* Constant index 0 */
        
        /* Pattern 4: Store via pointer with zero offset */
        *p_dst = val1;  /* Equivalent to p_dst[0] */
        
        /* Pattern 5: Separate pointer access to create another (mem (reg)) pattern */
        int val3 = *p_alt;  /* This should generate another base register */
        checksum += val3;
        
        /* Inline assembly barrier with register clobbers
         * Creates artificial dependencies and prevents reordering
         */
        __asm__ volatile(
            ""
            : 
            : 
            : "memory", "r0", "r1", "r2", "r3"
        );
        
        /* Scalar operations to generate (set (reg) (mem (reg))) patterns */
        volatile int scalar = src[5];  /* Another memory access pattern */
        scalar = scalar * 2 + 1;
        dst[5] = scalar;
        
        /* Additional memory barrier */
        __asm__ volatile("" : : : "memory");
        
        /* Mix array and scalar operations */
        checksum += src[1] - dst[1];
    }
    
    /* Second loop with different access pattern */
    volatile int* ptr1 = &src[20];
    volatile int* ptr2 = &dst[20];
    
    for (int i = 0; i < 50; i++) {
        /* Multiple memory references in sequence */
        int a = ptr1[0];  /* (mem (plus (reg) (const_int 0))) */
        int b = ptr2[0];  /* Another base register */
        
        /* Arithmetic creates register dependencies */
        int c = a + b + i;
        
        /* Store operations */
        ptr1[0] = c;
        ptr2[0] = c * 2;
        
        /* Memory barrier with specific register clobbers for ARM */
        __asm__ volatile(
            ""
            : 
            : 
            : "memory", "r4", "r5", "r6", "r7"
        );
    }
    
    /* Force compiler to keep all computations */
    consume_result(dst, 100);
    consume_result(src, 100);
    
    /* Return checksum to prevent dead code elimination */
    return checksum & 0xFF;
}
