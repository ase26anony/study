/* auto_inc_dec_trigger.c
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
    /* Prevent dead code elimination */
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
     * CRITICAL LOOP: Designed to generate (mem (plus (reg) (const_int 0))) patterns
     * Multiple memory references with constant index 0
     */
    for (int i = 0; i < limit; i++) {
        /* Pattern 1: Load from src[0] - should generate (mem (reg)) */
        int val1 = src[0];
        
        /* Pattern 2: Another load with constant index 0 */
        int val2 = src[0];
        
        /* Arithmetic operation to create register pressure */
        int result = val1 + val2 + 7;  /* Loop-invariant constant */
        
        /* Pattern 3: Store to dst[0] - should generate (set (mem (reg)) (reg)) */
        dst[0] = result;
        
        /* Inline assembly barrier with register clobbers
         * Creates artificial dependencies and prevents reordering
         */
        __asm__ volatile(
            ""
            : 
            : 
            : "memory", "r0", "r1", "r2", "r3"
        );
        
        /* Additional pointer-based access with zero offset */
        volatile int *p = &src[10];
        int val3 = *p;  /* Should generate (mem (reg)) where reg = &src[10] */
        
        /* Store to another array location */
        dst[5] = val3 + accumulator;
        
        /* Modify accumulator to create data dependencies */
        accumulator += (result & 0xFF);
        
        /* Another barrier with different clobbers */
        __asm__ volatile("" : : : "memory", "r4", "r5");
    }
    
    /* 
     * SECOND LOOP: Different pattern with multiple arrays
     * Creates more (mem (reg)) patterns for auto-inc-dec to analyze
     */
    volatile int temp[50];
    for (int i = 0; i < 50; i++) {
        /* Access multiple arrays with constant index 0 */
        int a = src[0];
        int b = dst[0];
        int c = temp[0];
        
        /* Chain of operations */
        temp[0] = a + b;
        dst[0] = c + temp[0];
        src[0] = dst[0] - a;
        
        /* Barrier to separate memory operations */
        __asm__ volatile("" : : : "memory");
    }
    
    /* 
     * POINTER ARITHMETIC SECTION
     * Creates base registers that might be incremented
     */
    volatile int *ptr1 = &src[20];
    volatile int *ptr2 = &dst[20];
    
    for (int i = 0; i < 30; i++) {
        /* Dereference pointers - generates (mem (reg)) */
        int x = *ptr1;
        int y = *ptr2;
        
        /* Store results */
        *ptr1 = x + y;
        *ptr2 = x - y;
        
        /* Scalar operations to mix with memory accesses */
        volatile int scalar = x * y;
        scalar = scalar >> 2;
        
        /* Barrier with specific ARM register clobbers */
        __asm__ volatile(
            ""
            :
            :
            : "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"
        );
    }
    
    /* Struct access to create different memory patterns */
    struct {
        volatile int a;
        volatile int b;
        volatile int c;
    } s = {0};
    
    s.a = src[0];
    s.b = dst[0];
    s.c = s.a + s.b;
    dst[0] = s.c;
    
    /* Final consumption to prevent elimination */
    consume_result((volatile int*)dst, 100);
    consume_result((volatile int*)src, 100);
    
    /* Return checksum */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= src[i];
        checksum ^= dst[i];
    }
    
    return checksum & 0xFF;
}
