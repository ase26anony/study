/* auto_inc_dec_test.c
 * Designed to trigger auto-increment/decrement RTL pattern matching
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stddef.h>
#include <stdio.h>

/* Prevent inlining to preserve RTL patterns */
__attribute__((noinline)) 
void consume_data(volatile int* arr, int size) {
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
    
    /* Critical processing loop - designed to generate (mem (plus (reg) (const_int 0))) patterns */
    for (int i = 0; i < limit; i++) {
        /* Pattern 1: Array access with constant index 0 - should generate (mem (plus (reg) (const_int 0))) */
        int val1 = src[0];  /* Load from src[0] */
        
        /* Pattern 2: Another array access with constant index 0 */
        int val2 = dst[0];  /* Load from dst[0] */
        
        /* Arithmetic operation to create register pressure */
        int result = val1 + val2 + 7;  /* Add loop-invariant constant */
        
        /* Pattern 3: Store with constant index 0 */
        dst[0] = result;  /* Store to dst[0] */
        
        /* Inline assembly barrier to prevent reordering/optimization */
        __asm__ volatile("" : : : "memory", "r0", "r1", "r2", "r3");
        
        /* Pattern 4: Pointer-based access with zero offset */
        volatile int *p = &src[10];
        int val3 = *p;  /* Dereference pointer - should be (mem (reg)) */
        
        /* Use val3 to prevent dead code elimination */
        accumulator += val3;
        
        /* Another barrier with register clobbers */
        __asm__ volatile("" : : : "r4", "r5", "r6", "r7");
        
        /* Pattern 5: Struct-like access pattern */
        volatile int *base_ptr = &src[20];
        int val4 = base_ptr[0];  /* Another (mem (plus (reg) (const_int 0))) */
        base_ptr[0] = val4 + accumulator;
        
        /* Scalar operation to create (set (reg) (mem (reg))) pattern */
        volatile int scalar = src[30];
        scalar = scalar * 2 + 1;
        src[30] = scalar;
    }
    
    /* Additional loop with multiple memory references */
    volatile int *src_ptr = &src[0];
    volatile int *dst_ptr = &dst[0];
    
    for (int i = 0; i < 50; i++) {
        /* Multiple independent memory accesses */
        int a = src_ptr[0];
        int b = src_ptr[1];
        int c = dst_ptr[0];
        
        /* Create artificial dependencies */
        __asm__ volatile("" : : "r"(a), "r"(b), "r"(c) : "memory");
        
        dst_ptr[0] = a + b - c;
        dst_ptr[1] = b * 2;
        
        /* Barrier between memory operations */
        __asm__ volatile("" : : : "r8", "r9", "r10", "r11", "r12");
    }
    
    /* Call dummy function to prevent dead code elimination */
    consume_data(dst, 100);
    
    /* Calculate checksum to ensure all operations matter */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= dst[i];
        checksum += src[i];
    }
    
    /* Use checksum to prevent optimization */
    __asm__ volatile("" : : "r"(checksum) : "memory");
    
    return checksum & 0xFF;  /* Return non-deterministic value */
}
