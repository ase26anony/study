/* auto_inc_dec_test.c
 * Designed to trigger auto-increment/decrement RTL pattern matching
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stddef.h>

/* Prevent inlining to preserve RTL patterns */
__attribute__((noinline)) 
void use_result(volatile int* arr, int size) {
    volatile int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    /* Prevent dead code elimination */
    __asm__ volatile("" : : "r"(sum) : "memory");
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
    volatile int accumulator = 0;
    
    /* 
     * Critical processing loop - designed to create (mem (plus (reg) (const_int 0))) patterns
     * The auto-inc-dec pass looks for these patterns to combine into auto-increment forms
     */
    for (int i = 0; i < limit; i++) {
        /* Pattern 1: Array access with constant index 0 - creates (mem (plus (reg) (const_int 0))) */
        int val1 = src[0];  /* Should generate: (set (reg) (mem (plus (reg) (const_int 0)))) */
        
        /* Pattern 2: Another array access with constant index 0 */
        int val2 = dst[0];  /* Another (mem (plus (reg) (const_int 0))) pattern */
        
        /* Arithmetic operation to create register pressure */
        int result = val1 + val2 + 7;  /* Loop-invariant constant addition */
        
        /* Pattern 3: Store with constant index 0 */
        dst[0] = result;    /* (set (mem (plus (reg) (const_int 0))) (reg)) */
        
        /* 
         * Inline assembly barrier with register clobbers
         * Creates artificial dependencies and prevents reordering
         */
        __asm__ volatile(
            ""
            : 
            : 
            : "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"
        );
        
        /* Pattern 4: Pointer-based access - creates another (mem (reg)) pattern */
        volatile int *p = &src[10];
        int val3 = *p;      /* Should generate: (set (reg) (mem (reg))) */
        
        /* Mix scalar operations to create more patterns */
        accumulator += val3;
        
        /* Another barrier to separate memory operations */
        __asm__ volatile("" : : : "memory");
        
        /* Pattern 5: Struct-like access pattern */
        volatile int *q = &dst[5];
        *q = accumulator;   /* (set (mem (reg)) (reg)) */
    }
    
    /* 
     * Additional loop with multiple memory references
     * Increases chances of auto-inc-dec pattern matching
     */
    volatile int *sptr = &src[20];
    volatile int *dptr = &dst[20];
    
    for (int j = 0; j < 50; j++) {
        /* Multiple independent memory references */
        int a = sptr[0];    /* (mem (plus (reg) (const_int 0))) */
        int b = sptr[1];    /* Different offset - may trigger different pattern */
        
        __asm__ volatile("" : : : "memory");
        
        dptr[0] = a + b;    /* Store operation */
        dptr[1] = a - b;    /* Another store with different offset */
        
        /* Barrier with specific register clobbers for ARM */
        __asm__ volatile(
            ""
            : 
            : 
            : "memory", "r8", "r9", "r10", "r11", "r12"
        );
    }
    
    /* Call noinline function to prevent dead code elimination */
    use_result(dst, 100);
    
    /* Calculate checksum to ensure all operations have effect */
    int checksum = 0;
    for (int k = 0; k < 100; k++) {
        checksum ^= dst[k];  /* XOR checksum */
    }
    
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}
