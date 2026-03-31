/* auto_inc_dec_test.c
 * Designed to trigger auto-increment/decrement RTL pattern matching
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stddef.h>

/* Prevent inlining to keep RTL patterns intact */
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
     * Multiple memory references with constant index 0
     */
    for (int i = 0; i < limit; i++) {
        /* Pattern 1: Load from src[0] - creates (mem (reg)) */
        int val1 = src[0];
        
        /* Pattern 2: Load from src[1] with different base */
        int val2 = src[1];
        
        /* Arithmetic operation to create register pressure */
        int result = val1 + val2 + 7;  /* Loop-invariant constant */
        
        /* Pattern 3: Store to dst[0] - creates (set (mem (reg)) (reg)) */
        dst[0] = result;
        
        /* Pattern 4: Store to dst[1] with different base */
        dst[1] = result * 2;
        
        /* Inline assembly barrier with register clobbers
         * Creates artificial dependencies and prevents reordering
         */
        __asm__ volatile(
            ""
            : 
            : 
            : "memory", "r0", "r1", "r2", "r3"
        );
        
        /* Scalar operations to create more register pressure */
        accumulator += result;
        
        /* Additional memory access with pointer + 0 offset */
        volatile int *p = &src[10];
        int val3 = *p;  /* Creates (mem (reg)) pattern */
        
        /* Store to another location */
        dst[10] = val3 + accumulator;
        
        /* Another barrier to separate memory operations */
        __asm__ volatile("" : : : "memory");
    }
    
    /* 
     * Pointer-based access with zero offset
     * This should generate (mem (plus (reg) (const_int 0)))
     */
    volatile int *ptr1 = &src[20];
    volatile int *ptr2 = &dst[20];
    
    /* Chain of memory operations with the same base + 0 pattern */
    for (int j = 0; j < 10; j++) {
        /* Load with base + 0 */
        int load_val = *ptr1;
        
        /* Modify */
        load_val += j;
        
        /* Store with base + 0 */
        *ptr2 = load_val;
        
        /* Barrier to prevent fusion */
        __asm__ volatile("" : : : "memory", "r4", "r5");
    }
    
    /* Struct-like access pattern */
    struct pair {
        volatile int a;
        volatile int b;
    };
    
    struct pair pairs[10];
    for (int k = 0; k < 10; k++) {
        /* Access struct fields - creates multiple (mem (reg)) patterns */
        pairs[k].a = src[k];
        __asm__ volatile("" : : : "memory");
        pairs[k].b = dst[k];
        __asm__ volatile("" : : : "memory");
    }
    
    /* Final use of results to prevent dead code elimination */
    use_result(dst, 100);
    
    /* Create a checksum to ensure all operations matter */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= dst[i];
        checksum += src[i];
    }
    
    /* Use checksum to prevent optimization */
    volatile int final_result = checksum;
    
    return final_result & 0xFF;  /* Return non-zero to indicate success */
}
