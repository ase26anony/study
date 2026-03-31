/* auto_inc_dec_test.c
 * Designed to trigger auto-increment/decrement RTL pattern matching
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stddef.h>

/* Prevent inlining to keep RTL patterns simple */
__attribute__((noinline)) 
void use_result(volatile int* arr, int size) {
    volatile int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    /* Prevent optimization */
    __asm__ volatile("" : : "r"(sum) : "memory");
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
     * Multiple memory references with constant index 0
     */
    for (int i = 0; i < limit; i++) {
        /* Pattern 1: Load from src[0] - creates (mem (reg)) */
        int val1 = src[0];
        
        /* Pattern 2: Load from src[1] with different base */
        int val2 = src[1];
        
        /* Arithmetic operation to create register pressure */
        int result = val1 + val2 + 7;
        
        /* Pattern 3: Store to dst[0] - creates (set (mem (reg)) (reg)) */
        dst[0] = result;
        
        /* Pattern 4: Store to dst[1] with different base */
        dst[1] = result * 2;
        
        /* Inline assembly barrier with register clobbering */
        __asm__ volatile(
            ""
            : 
            : "r"(val1), "r"(val2), "r"(result)
            : "memory", "r0", "r1", "r2", "r3"
        );
        
        /* Additional pointer-based access for more (mem (reg)) patterns */
        volatile int* p = &src[10];
        volatile int* q = &dst[20];
        
        /* Pattern 5: Pointer dereference *p */
        int val_p = *p;
        
        /* Pattern 6: Pointer dereference *q */
        *q = val_p + accumulator;
        
        /* Scalar operations to create register pressure */
        accumulator += (val1 & 0xFF) - (val2 & 0xFF);
        
        /* Another memory barrier */
        __asm__ volatile("" : : : "memory");
    }
    
    /* Second loop with different access patterns */
    volatile int* src_ptr = &src[5];
    volatile int* dst_ptr = &dst[5];
    
    for (int i = 0; i < 50; i++) {
        /* Pattern 7: Multiple loads with same base + 0 offset */
        int a = src_ptr[0];
        int b = src_ptr[1];
        
        /* Pattern 8: Multiple stores with same base + 0 offset */
        dst_ptr[0] = a + b;
        dst_ptr[1] = a - b;
        
        /* Complex enough to survive optimization but simple RTL */
        __asm__ volatile(
            "nop"
            : 
            : "r"(a), "r"(b)
            : "memory"
        );
        
        /* Modify pointers slightly to create different base registers */
        src_ptr = &src[(i % 10) + 5];
        dst_ptr = &dst[(i % 10) + 5];
    }
    
    /* Use results to prevent dead code elimination */
    use_result(dst, 100);
    
    /* Create checksum */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= dst[i];
    }
    
    return checksum & 0xFF;
}
