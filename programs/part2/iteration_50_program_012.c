/* auto_inc_dec_trigger.c
 * Designed to generate RTL pattern (mem (plus (reg) (const_int 0)))
 * for auto-increment/decrement pass coverage
 */

#include <stddef.h>

/* Prevent inlining to preserve RTL patterns */
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
    
    /* Create multiple pointer-based memory access patterns */
    volatile int* p1 = &src[10];
    volatile int* p2 = &dst[20];
    
    /* Main processing loop - designed to generate (mem (plus (reg) (const_int 0))) patterns */
    for (int i = 0; i < limit; i++) {
        /* Pattern 1: Array access with constant index 0 - should generate (mem (reg)) */
        int val1 = src[0];  /* (mem (plus (reg:SI src_base) (const_int 0))) */
        
        /* Pattern 2: Another array access with constant index 0 */
        int val2 = dst[0];  /* Another (mem (plus (reg:SI dst_base) (const_int 0))) */
        
        /* Arithmetic operation to create register pressure */
        int result = val1 + val2 + 7;  /* Loop-invariant constant */
        
        /* Store back with constant index 0 */
        dst[0] = result;    /* (set (mem (plus (reg:SI dst_base) (const_int 0))) (reg:SI result)) */
        
        /* Pattern 3: Pointer dereference - should generate (mem (reg)) */
        int ptr_val = *p1;  /* (mem (reg:SI p1)) */
        
        /* Modify and store through pointer */
        *p2 = ptr_val * 2;
        
        /* Inline assembly barrier with register clobbers
         * Creates artificial dependencies and prevents reordering
         */
        __asm__ volatile(
            ""
            : /* no outputs */
            : /* no inputs */
            : "memory", "r0", "r1", "r2", "r3"
        );
        
        /* Scalar operations to create more (set (reg) (mem (reg))) patterns */
        volatile int temp = accumulator;
        temp += src[i % 10];  /* Varying index to prevent optimization */
        accumulator = temp;
        
        /* Another memory barrier */
        __asm__ volatile("" : : : "memory");
        
        /* Additional array accesses with constant 0 offset */
        src[5] = src[0] + 1;  /* Load from src[0], store to src[5] */
        
        /* Create register pressure by using multiple scalars */
        volatile int a = src[1];
        volatile int b = src[2];
        volatile int c = src[3];
        a = b + c;
        src[1] = a;
    }
    
    /* Second loop with different access pattern */
    volatile int* src_ptr = &src[0];
    volatile int* dst_ptr = &dst[0];
    
    for (int i = 0; i < 50; i++) {
        /* Direct pointer access - should generate (mem (reg)) */
        int val = *src_ptr;
        
        /* Simple arithmetic */
        val = val + (i & 0xF);  /* Prevent constant propagation */
        
        /* Store through pointer */
        *dst_ptr = val;
        
        /* Inline assembly to prevent optimization */
        __asm__ volatile(
            "nop"
            : /* no outputs */
            : /* no inputs */
            : "memory"
        );
    }
    
    /* Force consumption of results to prevent dead code elimination */
    consume_result(dst, 100);
    
    /* Calculate checksum */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= dst[i];  /* XOR checksum */
        checksum += src[i];  /* Add source values */
    }
    
    return checksum & 0xFF;  /* Return non-zero result */
}
