/* auto_inc_dec_test.c
 * Designed to trigger auto-increment/decrement RTL pattern matching
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stddef.h>

/* Prevent inlining to preserve RTL patterns */
__attribute__((noinline)) 
void consume_result(volatile int* arr, int size) {
    volatile int sink = 0;
    for (int i = 0; i < size; i++) {
        sink += arr[i];
    }
    /* Prevent optimization of sink */
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
    
    /* 
     * CRITICAL LOOP: Creates (mem (plus (reg) (const_int 0))) patterns
     * Multiple memory references with constant index 0
     */
    for (int i = 0; i < limit; i++) {
        /* Pattern 1: Load from src[0] - creates (mem (reg)) */
        int val1 = src[0];
        
        /* Pattern 2: Load from src[1] - another base register */
        int val2 = src[1];
        
        /* Arithmetic operation to create register pressure */
        int result = val1 + val2 + 7;  /* Loop-invariant constant */
        
        /* Pattern 3: Store to dst[0] - creates (set (mem (reg)) (reg)) */
        dst[0] = result;
        
        /* 
         * Inline assembly barrier with register clobbers
         * Creates artificial dependencies, prevents reordering
         */
        __asm__ volatile(
            ""
            : 
            : 
            : "memory", "r0", "r1", "r2", "r3"
        );
        
        /* 
         * Additional pointer-based access with zero offset
         * Creates (mem (plus (reg) (const_int 0))) pattern
         */
        volatile int* p = &src[10];
        int val3 = *p;  /* Dereference with no offset */
        
        /* Store to another array location */
        dst[5] = val3 + accumulator;
        
        /* Modify accumulator to create data dependencies */
        accumulator += (i & 1);
        
        /* Another barrier with different clobbers */
        __asm__ volatile("" : : : "r4", "r5", "r6", "r7", "memory");
    }
    
    /* 
     * Second loop with different access patterns
     * Mixes array and scalar operations
     */
    volatile int* src_ptr = &src[20];
    volatile int* dst_ptr = &dst[20];
    
    for (int i = 0; i < 50; i++) {
        /* Load via pointer with implicit offset 0 */
        int loaded = *src_ptr;
        
        /* Store via pointer with implicit offset 0 */
        *dst_ptr = loaded * 2;
        
        /* Scalar operation that gets stored back */
        volatile int temp = loaded + 1;
        dst[30] = temp;  /* Constant index access */
        
        /* Complex barrier to preserve all memory ops */
        __asm__ volatile(
            "nop\n\t"
            "nop"
            : 
            : 
            : "r8", "r9", "r10", "r11", "r12", "memory"
        );
        
        /* Access struct-like pattern using separate arrays */
        int a = src[40];  /* Base + 0 offset */
        int b = src[41];  /* Base + 4 offset (assuming 4-byte int) */
        dst[40] = a + b;
        dst[41] = a - b;
    }
    
    /* 
     * Loop with multiple independent memory references
     * Each creates separate (mem (reg)) patterns
     */
    for (int i = 0; i < 30; i++) {
        /* Four independent memory loads with constant indices */
        int v1 = src[60];
        int v2 = src[61];
        int v3 = src[62];
        int v4 = src[63];
        
        /* Computation creating register pressure */
        int sum = v1 + v2 + v3 + v4;
        
        /* Four independent stores */
        dst[60] = v1;
        dst[61] = v2;
        dst[62] = v3;
        dst[63] = sum;
        
        /* Barrier preventing optimization across iterations */
        __asm__ volatile("" : : : "r0", "r1", "r2", "r3", "r4", "memory");
    }
    
    /* Final consumption to prevent dead code elimination */
    consume_result(dst, 100);
    
    /* Create a checksum to return */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= dst[i];
    }
    
    return checksum & 0xFF;
}
