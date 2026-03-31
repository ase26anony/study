/* auto_inc_dec_test.c
 * Designed to trigger auto-increment/decrement RTL pattern matching
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to preserve RTL patterns */
__attribute__((noinline)) 
void consume_array(volatile int* arr, int size) {
    volatile int sink = 0;
    for (int i = 0; i < size; i++) {
        sink += arr[i];
    }
    /* Use sink to prevent elimination */
    asm volatile("" : : "r"(sink) : "memory");
}

/* Force register pressure and prevent optimization */
__attribute__((noinline))
void memory_barrier(void) {
    /* Clobber multiple registers to force spills */
    asm volatile("" : : : 
        "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7");
}

int main(void) {
    /* Volatile arrays to prevent optimization of memory ops */
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
        int val1 = src[0];  /* Constant index 0 */
        
        /* Pattern 2: Load from src via pointer with offset 0 */
        volatile int* p = &src[10];
        int val2 = *p;      /* Dereference: (mem (reg)) */
        
        /* Arithmetic operation (loop-invariant constant) */
        int result = val1 + val2 + 42;
        
        /* Pattern 3: Store to dst[0] - creates (set (mem (reg)) (reg)) */
        dst[0] = result;    /* Constant index 0 */
        
        /* Pattern 4: Another memory access with different base */
        volatile int* q = &dst[20];
        *q = result / 2;    /* Another (set (mem (reg)) (reg)) */
        
        /* Scalar operations to create register pressure */
        accumulator += result;
        
        /* 
         * Inline assembly barrier with register clobbers
         * Prevents reordering/elimination of memory ops
         */
        asm volatile("" : : : 
            "memory", "r0", "r1", "r2", "r3", "r4", "r5");
        
        /* Additional memory operations to create more patterns */
        src[5] = accumulator;  /* Store with different offset */
        int val3 = dst[5];     /* Load with different offset */
        
        /* More arithmetic to create data dependencies */
        accumulator ^= val3;
        
        /* Another barrier to separate memory operations */
        memory_barrier();
    }
    
    /* 
     * Additional pointer-based accesses outside loop
     * Creates standalone (mem (reg)) patterns
     */
    volatile int* r = &src[30];
    volatile int* s = &dst[40];
    
    for (int j = 0; j < 10; j++) {
        /* Simple load/store with zero offset via pointers */
        int temp = *r;      /* (mem (reg)) */
        *s = temp + j;      /* (set (mem (reg)) (reg)) */
        
        /* Barrier to prevent fusion */
        asm volatile("" : : : "memory");
    }
    
    /* Struct access to create different addressing patterns */
    struct {
        volatile int a;
        volatile int b;
        volatile int c;
    } data = {0};
    
    volatile struct simple_struct* ptr = &data;
    ptr->a = src[0];        /* Field access via pointer */
    ptr->b = dst[0];
    
    /* Prevent dead code elimination */
    consume_array((volatile int*)src, 100);
    consume_array((volatile int*)dst, 100);
    
    /* Return checksum based on array contents */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= src[i];
        checksum ^= dst[i];
    }
    
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}
