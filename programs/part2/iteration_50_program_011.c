/* auto_inc_dec_test.c
 * Designed to trigger auto-increment/decrement RTL pattern matching
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stddef.h>

/* Prevent inlining to keep RTL patterns simple */
__attribute__((noinline)) 
void use_result(volatile int* arr, int size) {
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
        src[i] = i + 1;
    }
    
    /* Non-constant loop bound to prevent full unrolling */
    int limit = 90;
    volatile int accumulator = 0;
    
    /* 
     * Critical processing loop designed to create (mem (plus (reg) (const_int 0))) patterns
     * Multiple memory references with constant index 0
     */
    for (int i = 0; i < limit; i++) {
        /* Pattern 1: Load from src[0] - creates (mem (plus (reg) (const_int 0))) */
        int val1 = src[0];
        
        /* Pattern 2: Load from src[1] - another similar pattern */
        int val2 = src[1];
        
        /* Arithmetic operation mixing values */
        int result = val1 + val2 + 42;  /* Loop-invariant constant */
        
        /* Pattern 3: Store to dst[0] - creates (set (mem (plus (reg) (const_int 0))) (reg)) */
        dst[0] = result;
        
        /* Pattern 4: Store to dst[1] */
        dst[1] = result * 2;
        
        /* Inline assembly barrier with register clobbers
         * Creates artificial register pressure and prevents reordering
         */
        __asm__ volatile(
            ""
            : 
            : 
            : "memory", "r0", "r1", "r2", "r3"
        );
        
        /* Scalar operations to create register pressure */
        accumulator += result;
        
        /* Additional pointer-based access with zero offset */
        volatile int* p = &src[10];
        int val3 = *p;  /* Creates (mem (reg)) pattern */
        
        /* Store back to create (set (mem (reg)) (reg)) */
        *p = val3 + 1;
        
        /* Another barrier */
        __asm__ volatile("" : : : "memory");
    }
    
    /* 
     * Second loop with different access patterns
     * Uses pointer arithmetic that could be optimized to auto-increment
     */
    volatile int* src_ptr = &src[20];
    volatile int* dst_ptr = &dst[20];
    
    for (int i = 0; i < 50; i++) {
        /* Sequential accesses that could be combined */
        int a = src_ptr[0];
        int b = src_ptr[1];
        
        dst_ptr[0] = a + b;
        dst_ptr[1] = a - b;
        
        /* Barrier with specific register clobbers for ARM */
        __asm__ volatile(
            "nop"
            :
            :
            : "r4", "r5", "r6", "r7", "memory"
        );
    }
    
    /* Struct access to create different base registers */
    struct pair {
        volatile int a;
        volatile int b;
    };
    
    struct pair pairs[10];
    for (int i = 0; i < 10; i++) {
        /* These create multiple (mem (reg)) patterns with the same base */
        pairs[i].a = src[i];
        pairs[i].b = dst[i];
        
        __asm__ volatile("" : : : "memory");
    }
    
    /* Prevent dead code elimination of arrays */
    use_result(dst, 100);
    
    /* Return checksum based on array contents */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= dst[i];
    }
    
    return checksum;
}
