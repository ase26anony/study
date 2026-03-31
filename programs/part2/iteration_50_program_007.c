/* auto_inc_dec_test.c
 * Designed to trigger auto-increment/decrement RTL pattern matching
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stddef.h>

/* Prevent inlining to preserve RTL patterns */
__attribute__((noinline)) void use_result(volatile int *arr, size_t n) {
    volatile int sink = 0;
    for (size_t i = 0; i < n; i++) {
        sink += arr[i];
    }
    (void)sink; /* Prevent unused variable warning */
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
    
    /* Critical processing loop - designed to generate (mem (plus (reg) (const_int 0))) patterns */
    for (int i = 0; i < limit; i++) {
        /* Pattern 1: Load from src[0] - should generate (mem (plus (reg) (const_int 0))) */
        int val1 = src[0];
        
        /* Pattern 2: Another memory access with constant index 0 */
        int val2 = src[0];
        
        /* Arithmetic operation to create register pressure */
        int result = val1 + val2 + 42;
        
        /* Pattern 3: Store to dst[0] - another (mem (plus (reg) (const_int 0))) */
        dst[0] = result;
        
        /* Inline assembly barrier to prevent reordering/elimination */
        __asm__ volatile("" : : : "memory");
        
        /* Additional pointer-based access for more (mem (reg)) patterns */
        volatile int *p = &src[10];
        int val3 = *p;  /* Should generate (mem (reg)) pattern */
        
        /* More arithmetic to use the loaded value */
        dst[1] = val3 + i;
        
        /* Another barrier with register clobbers (ARM-specific) */
        __asm__ volatile("" : : : "r0", "r1", "r2", "r3", "memory");
        
        /* Scalar operations to create (set (reg) (mem (reg))) patterns */
        volatile int scalar = src[5];
        scalar = scalar * 2 + 1;
        dst[5] = scalar;
        
        /* Additional array accesses with different indices */
        dst[i % 10] = src[i % 10] + 1;
    }
    
    /* Second loop with different access pattern */
    for (int i = 0; i < 50; i++) {
        /* Multiple memory references in sequence */
        int a = src[i];
        int b = src[i + 1];
        int c = src[i + 2];
        
        /* Chain of operations */
        dst[i] = a + b;
        dst[i + 1] = b + c;
        dst[i + 2] = a + c;
        
        /* Memory barrier */
        __asm__ volatile("" : : : "memory");
    }
    
    /* Use a separate pointer variable to create base register */
    volatile int *src_ptr = src;
    volatile int *dst_ptr = dst;
    
    /* Loop with pointer arithmetic (but constant offset 0 access) */
    for (int i = 0; i < 30; i++) {
        /* Access via pointer with offset 0 */
        int val = *(src_ptr + 0);
        *(dst_ptr + 0) = val * 3;
        
        /* Increment pointers separately to create register increment patterns */
        src_ptr++;
        dst_ptr++;
        
        /* Barrier */
        __asm__ volatile("" : : : "r4", "r5", "memory");
    }
    
    /* Call function to use results, preventing dead code elimination */
    use_result(dst, 100);
    
    /* Calculate checksum to ensure all operations matter */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= dst[i];
    }
    
    return checksum;
}
