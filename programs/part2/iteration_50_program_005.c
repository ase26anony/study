/* auto_inc_dec_test.c
 * Designed to trigger auto-increment/decrement RTL pattern matching
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stddef.h>

/* Prevent inlining to keep RTL patterns simple */
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
    
    /* Critical processing loop - designed to generate (mem (plus (reg) (const_int 0))) patterns */
    for (int i = 0; i < limit; i++) {
        /* Pattern 1: Load from src[0] - should generate (mem (reg)) or (mem (plus (reg) (const_int 0))) */
        int val1 = src[0];
        
        /* Pattern 2: Another memory access with constant index 0 */
        int val2 = src[0];
        
        /* Arithmetic operation to create register pressure */
        int result = val1 + val2 + 7;  /* Loop-invariant constant */
        
        /* Pattern 3: Store to dst[0] - another (mem (reg)) pattern */
        dst[0] = result;
        
        /* Inline assembly barrier with register clobbers
         * Creates artificial dependencies and prevents reordering */
        __asm__ volatile("" 
                         : /* no outputs */
                         : /* no inputs */ 
                         : "r0", "r1", "r2", "r3", "memory");
        
        /* Pattern 4: Separate pointer-based access */
        volatile int *p = &src[10];
        int val3 = *p;  /* Should generate (mem (reg)) where reg = &src[10] */
        
        /* Mix scalar operations to create more patterns */
        accumulator += val3;
        
        /* Another barrier to separate memory operations */
        __asm__ volatile("" : : : "r4", "r5", "memory");
        
        /* Additional memory access with different base */
        dst[5] = accumulator;
    }
    
    /* Second loop with different access pattern */
    for (int i = 1; i < limit - 10; i++) {
        /* Access with offset 0 from different base pointers */
        int a = src[i];
        int b = dst[i];
        
        /* Create (set (reg) (mem (reg))) pattern */
        int sum = a + b;
        
        /* Store back with offset 0 */
        dst[i] = sum;
        
        /* Barrier to prevent optimization */
        __asm__ volatile("" : : : "r6", "r7", "memory");
    }
    
    /* Struct access to create different memory patterns */
    struct {
        volatile int a;
        volatile int b;
        volatile int c;
    } s = {0};
    
    /* Multiple struct field accesses */
    s.a = src[0];
    s.b = src[0];
    __asm__ volatile("" : : : "memory");
    s.c = s.a + s.b;
    
    /* Call dummy function to prevent dead code elimination */
    consume_result(dst, 100);
    
    /* Return checksum based on array contents */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= dst[i];
    }
    
    return checksum;
}
