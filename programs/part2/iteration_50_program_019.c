/* auto_inc_dec_test.c
 * Designed to trigger auto-increment/decrement RTL pattern matching
 * Specifically targets: (mem (plus (reg) (const_int 0))) pattern
 */

#include <stddef.h>

/* Prevent inlining to preserve RTL patterns */
__attribute__((noinline)) 
void consume_result(volatile int* arr, int size) {
    volatile int sink = 0;
    for (int i = 0; i < size; i++) {
        sink += arr[i];
    }
    /* Use sink to prevent elimination */
    asm volatile("" : : "r"(sink) : "memory");
}

int main(void) {
    /* Volatile arrays to prevent optimization of memory accesses */
    volatile int src[100];
    volatile int dst[100];
    
    /* Initialize source array with sequential values */
    for (int i = 0; i < 100; i++) {
        src[i] = i + 1;
    }
    
    /* Non-constant loop bound to prevent unrolling */
    int limit = 90;
    volatile int accumulator = 0;
    
    /* Main processing loop - designed to generate (mem (plus (reg) (const_int 0))) patterns */
    for (int i = 0; i < limit; i++) {
        /* Pattern 1: Array access with constant index 0 - should generate (mem (plus (reg) (const_int 0))) */
        int val1 = src[0];  /* Load from src + 0 */
        
        /* Arithmetic operation to create register pressure */
        val1 = val1 + 7;    /* Loop-invariant constant */
        
        /* Pattern 2: Store with constant index 0 */
        dst[0] = val1;      /* Store to dst + 0 */
        
        /* Memory barrier to prevent reordering/elimination */
        asm volatile("" : : : "memory");
        
        /* Pattern 3: Pointer dereference with zero offset */
        volatile int *p = &src[10];
        int val2 = *p;      /* Should generate (mem (reg)) or (mem (plus (reg) (const_int 0))) */
        
        /* More arithmetic to create register dependencies */
        accumulator += val2;
        
        /* Architecture-specific register clobbering for ARM */
        #ifdef __arm__
        asm volatile("" : : : "r0", "r1", "r2", "r3", "memory");
        #else
        asm volatile("" : : : "memory");
        #endif
        
        /* Additional memory operations to increase pattern opportunities */
        volatile int *q = &dst[20];
        *q = accumulator;
        
        /* Scalar operations mixed with array accesses */
        volatile int scalar = src[5];
        scalar = scalar * 2;
        dst[5] = scalar;
    }
    
    /* Second loop with different access patterns */
    for (int i = 1; i < limit - 10; i++) {
        /* Access multiple arrays with constant zero offsets */
        int a = src[0];
        int b = dst[0];
        int c = a + b;
        
        /* Store to different locations */
        dst[1] = c;
        src[1] = c * 2;
        
        /* More inline assembly for register pressure */
        asm volatile("" : : : "memory");
    }
    
    /* Force use of results to prevent dead code elimination */
    consume_result(dst, 100);
    consume_result(src, 100);
    
    /* Return checksum based on array contents */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= dst[i];
        checksum ^= src[i];
    }
    
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}
