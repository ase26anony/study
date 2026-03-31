/* auto-inc-dec-trigger.c
 * Designed to generate RTL pattern (mem (plus (reg) (const_int 0)))
 * for auto-increment/decrement optimization pass
 */

#include <stddef.h>
#include <stdio.h>

/* Prevent inlining to keep RTL patterns intact */
__attribute__((noinline)) 
static void consume_array(volatile int* arr, int size) {
    volatile int sink = 0;
    for (int i = 0; i < size; ++i) {
        sink += arr[i];
    }
    /* Use sink to prevent elimination */
    __asm__ volatile("" : : "r"(sink) : "memory");
}

int main(void) {
    /* Volatile arrays to prevent optimization of memory accesses */
    volatile int src[100];
    volatile int dst[100];
    
    /* Initialize source array with sequential values */
    for (int i = 0; i < 100; ++i) {
        src[i] = i * 3 + 1;
    }
    
    /* Non-constant loop bound to prevent unrolling */
    int limit = 90;
    volatile int accumulator = 0;
    
    /* Create multiple pointer-based accesses for distinct base registers */
    volatile int* p1 = &src[10];  /* Will generate (mem (reg)) pattern */
    volatile int* p2 = &dst[20];
    
    /* Main processing loop - designed to create (mem (plus (reg) (const_int 0))) patterns */
    for (int i = 0; i < limit; ++i) {
        /* Pattern 1: Array access with constant index 0 - should generate (mem (plus (reg) (const_int 0))) */
        int val1 = src[0];  /* Base: src, Offset: 0 */
        
        /* Pattern 2: Another array access with constant index 0 */
        int val2 = dst[0];  /* Base: dst, Offset: 0 */
        
        /* Arithmetic operation to create register pressure */
        int result = val1 + val2 + 7;
        
        /* Store back with constant index 0 */
        dst[0] = result;
        
        /* Pattern 3: Pointer dereference - generates (mem (reg)) */
        int ptr_val = *p1;
        
        /* Modify and store through pointer */
        *p2 = ptr_val * 2;
        
        /* Inline assembly to create register dependencies and prevent reordering */
        __asm__ volatile (
            "/* barrier */"
            : 
            : 
            : "memory", "r0", "r1", "r2", "r3"
        );
        
        /* Scalar operations to create (set (reg) (mem (reg))) patterns */
        volatile int temp = src[i % 10];
        temp = temp + 1;
        dst[i % 10] = temp;
        
        /* Another memory barrier */
        __asm__ volatile("" : : : "memory");
        
        accumulator += result + ptr_val;
    }
    
    /* Additional pointer-based accesses outside loop */
    volatile int* p3 = &src[50];
    volatile int final_val = *p3;
    dst[99] = final_val;
    
    /* Force consumption of arrays to prevent dead code elimination */
    consume_array(src, 100);
    consume_array(dst, 100);
    
    /* Return checksum based on array contents */
    int checksum = 0;
    for (int i = 0; i < 100; ++i) {
        checksum ^= src[i] ^ dst[i];
    }
    
    /* Use checksum to prevent elimination */
    __asm__ volatile("" : : "r"(checksum) : "memory");
    
    return checksum & 0xFF;
}
