/* auto_inc_dec_test.c
 * Designed to trigger auto-increment/decrement RTL pattern matching
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stddef.h>

/* Prevent inlining to preserve RTL patterns */
__attribute__((noinline)) 
void use_result(volatile int *arr, int size) {
    volatile int dummy = 0;
    for (int i = 0; i < size; i++) {
        dummy += arr[i];
    }
    /* Prevent optimization of the dummy variable */
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
     * Critical loop: Creates (mem (plus (reg) (const_int 0))) patterns
     * The auto-inc-dec pass looks for these to combine into auto-increment
     */
    for (int i = 0; i < limit; i++) {
        /* Pattern 1: Array access with constant index 0 - creates (mem (plus (reg) (const_int 0))) */
        int val1 = src[0];  /* Should generate: (set (reg) (mem (plus (reg) (const_int 0)))) */
        
        /* Arithmetic operation to create register pressure */
        val1 = val1 + 7;
        
        /* Pattern 2: Store with constant index 0 */
        dst[0] = val1;      /* Should generate: (set (mem (plus (reg) (const_int 0))) (reg)) */
        
        /* Memory barrier to prevent reordering/elimination */
        __asm__ volatile("" : : : "memory");
        
        /* Pattern 3: Pointer dereference with zero offset */
        volatile int *p = &src[10];
        int val2 = *p;      /* Should generate: (set (reg) (mem (reg))) */
        
        /* More arithmetic to create register dependencies */
        accumulator += val2;
        
        /* Architecture-specific register clobbering for ARM */
        __asm__ volatile("" : : : "r0", "r1", "r2", "r3", "memory");
        
        /* Pattern 4: Struct-like access through multiple arrays */
        int val3 = src[5];  /* Another (mem (plus (reg) (const_int 0))) pattern */
        dst[5] = val3 + accumulator;
        
        /* Additional memory barrier */
        __asm__ volatile("" : : : "memory");
    }
    
    /* 
     * Create additional memory access patterns outside the loop
     * to increase chances of hitting the target lines
     */
    volatile int *ptr1 = &src[20];
    volatile int *ptr2 = &dst[20];
    
    for (int j = 0; j < 10; j++) {
        /* Multiple memory references in sequence */
        int tmp = *ptr1;           /* (mem (reg)) */
        tmp = tmp * 2;
        *ptr2 = tmp;               /* (mem (reg)) */
        
        /* Scalar operations mixed with memory ops */
        volatile int scalar = accumulator;
        scalar = scalar + *ptr1;   /* Load with base register */
        *ptr2 = scalar;            /* Store with base register */
        
        /* Barrier with register clobbering */
        __asm__ volatile("" : : : "r4", "r5", "r6", "memory");
    }
    
    /* Prevent dead code elimination */
    use_result(dst, 100);
    
    /* Return checksum based on array contents */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= dst[i];
    }
    
    return checksum & 0xFF;
}
