/* auto_inc_dec_trigger.c
 * Designed to trigger auto-increment/decrement RTL pattern matching
 * Specifically targets: (mem (plus (reg) (const_int 0))) pattern
 */

#include <stddef.h>
#include <stdio.h>

/* Prevent inlining to preserve RTL patterns */
__attribute__((noinline)) 
void consume_result(volatile int* arr, int size) {
    volatile int sink = 0;
    for (int i = 0; i < size; i++) {
        sink += arr[i];
    }
    /* Use sink to prevent elimination */
    __asm__ volatile("" : : "r"(sink) : "memory");
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
    volatile int checksum = 0;
    
    /* Critical processing loop - designed to generate (mem (plus (reg) (const_int 0))) patterns */
    for (int i = 0; i < limit; i++) {
        /* Pattern 1: Array access with constant index 0 - should generate (mem (plus (reg) (const_int 0))) */
        int val1 = src[0];  /* Load from src with offset 0 */
        
        /* Arithmetic operation to create register pressure */
        val1 = val1 + 7;    /* Add loop-invariant constant */
        
        /* Pattern 2: Store to dst with constant index 0 */
        dst[0] = val1;      /* Store to dst with offset 0 */
        
        /* Memory barrier to prevent reordering/elimination */
        __asm__ volatile("" : : : "memory");
        
        /* Pattern 3: Separate pointer-based access */
        volatile int *p = &src[10];
        int val2 = *p;      /* Dereference pointer - should generate (mem (reg)) */
        
        /* More arithmetic to create register dependencies */
        val2 = val2 * 2;
        
        /* Pattern 4: Store to different location */
        dst[5] = val2;
        
        /* Inline assembly with register clobbers for ARM */
        __asm__ volatile("" : : : "r0", "r1", "r2", "r3", "memory");
        
        /* Mix scalar operations to create (set (reg) (mem (reg))) patterns */
        volatile int scalar = src[20];
        scalar = scalar + i;
        dst[20] = scalar;
        
        /* Additional memory barrier */
        __asm__ volatile("" : : : "memory");
        
        checksum += val1 + val2 + scalar;
    }
    
    /* Second loop with different access patterns */
    for (int i = 0; i < 50; i++) {
        /* Access multiple arrays to create distinct base registers */
        int a = src[i];
        int b = dst[i];
        
        /* Create register pressure */
        __asm__ volatile("" : : : "r4", "r5", "r6", "r7");
        
        /* Store results with offset 0 patterns */
        dst[i + 25] = a + b;
        src[i + 25] = a - b;
        
        /* Memory barrier */
        __asm__ volatile("" : : : "memory");
    }
    
    /* Use results to prevent dead code elimination */
    consume_result(dst, 100);
    consume_result(src, 100);
    
    /* Return checksum based on array contents */
    int final_checksum = 0;
    for (int i = 0; i < 100; i++) {
        final_checksum += src[i] + dst[i];
    }
    
    return final_checksum % 256;
}
