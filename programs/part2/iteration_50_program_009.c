/* auto_inc_dec_test.c
 * Designed to trigger auto-increment/decrement RTL pattern matching
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stddef.h>

/* Prevent inlining to preserve RTL patterns */
__attribute__((noinline)) 
void consume_result(volatile int* arr, int size) {
    volatile int dummy = 0;
    for (int i = 0; i < size; i++) {
        dummy += arr[i];
    }
    /* Use dummy to prevent optimization */
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
    
    /* Create pointer with zero offset pattern */
    volatile int* ptr1 = &src[10];
    volatile int* ptr2 = &dst[20];
    
    /* Main processing loop - designed to create (mem (plus (reg) (const_int 0))) patterns */
    for (int i = 0; i < limit; i++) {
        /* Pattern 1: Array access with constant index 0 - should create (mem (reg)) */
        int val1 = src[0];  /* This should generate (mem (plus (reg) (const_int 0))) */
        
        /* Pattern 2: Pointer dereference - should create (mem (reg)) */
        int val2 = *ptr1;   /* Another (mem (reg)) pattern */
        
        /* Arithmetic operation to create register pressure */
        int result = val1 + val2 + 7;  /* Loop-invariant constant */
        
        /* Pattern 3: Store with constant index 0 */
        dst[0] = result;    /* (set (mem (reg)) (reg)) pattern */
        
        /* Pattern 4: Store through pointer */
        *ptr2 = result;     /* Another store pattern */
        
        /* Inline assembly to create register dependencies and prevent reordering */
        __asm__ volatile(
            ""
            : 
            : "r"(val1), "r"(val2), "r"(result)
            : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "memory"
        );
        
        /* Additional memory barrier */
        __asm__ volatile("" : : : "memory");
        
        /* Mix in scalar operations */
        accumulator += result;
        
        /* Create artificial dependencies to prevent elimination */
        src[0] = accumulator & 0xFF;
    }
    
    /* Second loop with different pattern */
    volatile int* p = &src[5];
    volatile int* q = &dst[15];
    
    for (int j = 0; j < 50; j++) {
        /* Multiple memory references in sequence */
        int a = *p;          /* (mem (reg)) */
        int b = src[1];      /* (mem (plus (reg) (const_int 4))) on 32-bit */
        int c = dst[2];      /* (mem (plus (reg) (const_int 8))) */
        
        /* Complex enough to survive optimization but simple enough for pattern matching */
        int tmp = a * b - c;
        
        *q = tmp;           /* Store pattern */
        dst[3] = tmp;       /* Another store */
        
        /* More inline assembly for register pressure */
        __asm__ volatile(
            "nop"
            : 
            : "r"(a), "r"(b), "r"(c), "r"(tmp)
            : "memory"
        );
        
        /* Scalar operation to create (set (reg) ...) patterns */
        accumulator ^= tmp;
    }
    
    /* Force compiler to keep all computations */
    consume_result(dst, 100);
    consume_result(src, 100);
    
    /* Return checksum to prevent dead code elimination */
    return (accumulator & 0xFF) + dst[0] + src[0];
}
