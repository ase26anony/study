/* auto_inc_trigger.c
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
    /* Use sink to prevent elimination */
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
         * Creates artificial dependencies and prevents reordering
         */
        __asm__ volatile("" 
                         : 
                         : 
                         : "memory", "r0", "r1", "r2", "r3");
        
        /* Pattern 4: Pointer-based access with zero offset */
        volatile int* p = &src[10];
        int val3 = *p;  /* Creates (mem (reg)) where reg holds &src[10] */
        
        /* Store to different array element */
        dst[5] = val3 + accumulator;
        
        /* Update accumulator to create loop-carried dependency */
        accumulator += 1;
        
        /* Another memory barrier */
        __asm__ volatile("" : : : "memory");
    }
    
    /* 
     * Additional pointer arithmetic patterns
     * Multiple independent base registers
     */
    volatile int* ptr1 = &src[20];
    volatile int* ptr2 = &dst[30];
    
    for (int j = 0; j < 10; j++) {
        /* Simple (mem (reg)) patterns */
        int x = *ptr1;      /* Load via pointer */
        *ptr2 = x + j;      /* Store via different pointer */
        
        /* Scalar operations mixed with memory accesses */
        volatile int temp = x;
        temp = temp * 2;
        *ptr2 = temp;
        
        /* Barrier to separate memory operations */
        __asm__ volatile("" : : : "memory", "r4", "r5");
    }
    
    /* Struct access pattern - creates base+offset */
    struct pair {
        volatile int a;
        volatile int b;
    } data;
    
    data.a = src[0];  /* Load into struct field */
    data.b = dst[0];  /* Load from different array */
    
    /* Mixed scalar and array operations */
    volatile int scalar = 42;
    for (int k = 0; k < 5; k++) {
        scalar += src[k];      /* Load with varying index */
        dst[k] = scalar;       /* Store with same index */
        
        /* Access with constant index 0 in the middle */
        int check = src[0];    /* Another (mem (reg)) pattern */
        if (check > 0) {
            dst[0] = check;    /* Store to dst[0] */
        }
    }
    
    /* Final consumption to prevent dead code elimination */
    consume_result(dst, 100);
    
    /* Return checksum based on array contents */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= dst[i];
    }
    
    return checksum & 0xFF;
}
