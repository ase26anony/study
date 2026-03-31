/* auto_inc_dec_test.c
 * Designed to trigger auto-increment/decrement RTL pattern matching
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stddef.h>

/* Prevent inlining to preserve RTL patterns */
__attribute__((noinline)) void consume_result(volatile int* arr, int size) {
    volatile int sink = 0;
    for (int i = 0; i < size; i++) {
        sink += arr[i];
    }
    /* Use sink to prevent optimization */
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
    
    /* Main processing loop - designed to create (mem (plus (reg) (const_int 0))) patterns */
    for (int i = 0; i < limit; i++) {
        /* Pattern 1: Load from src[0] - creates (mem (plus (reg) (const_int 0))) */
        int val1 = src[0];
        
        /* Pattern 2: Pointer dereference with zero offset */
        volatile int *p = &src[10];
        int val2 = *p;  /* Creates (mem (reg)) pattern */
        
        /* Arithmetic operation to create register pressure */
        int result = val1 + val2 + 7;  /* Loop-invariant constant */
        
        /* Pattern 3: Store to dst[0] - creates (set (mem (plus (reg) (const_int 0))) (reg)) */
        dst[0] = result;
        
        /* Pattern 4: Additional memory access with different base */
        volatile int *q = &dst[20];
        *q = val1;
        
        /* Inline assembly barrier with register clobbers
         * Creates artificial dependencies and prevents reordering */
        __asm__ volatile(
            ""
            : 
            : 
            : "memory", "r0", "r1", "r2", "r3"
        );
        
        /* Scalar operations to create more (set (reg) ...) patterns */
        volatile int scalar = src[5];
        scalar += dst[5];
        dst[5] = scalar;
        
        /* Another barrier to separate memory operations */
        __asm__ volatile("" : : : "memory");
    }
    
    /* Second loop with different access pattern */
    volatile int *ptr1 = &src[30];
    volatile int *ptr2 = &dst[30];
    
    for (int j = 0; j < 50; j++) {
        /* Multiple memory references with zero offset */
        int a = *ptr1;      /* (mem (reg)) */
        int b = ptr1[0];    /* (mem (plus (reg) (const_int 0))) */
        
        /* Store with zero offset */
        ptr2[0] = a + b;
        
        /* Barrier with specific ARM register clobbers */
        __asm__ volatile(
            ""
            : 
            : 
            : "memory", "r4", "r5", "r6", "r7"
        );
    }
    
    /* Struct access to create different base registers */
    struct {
        volatile int a;
        volatile int b;
        volatile int c;
    } s = {0};
    
    volatile int *struct_ptr = &s.a;
    for (int k = 0; k < 20; k++) {
        /* Access struct members through pointer */
        int x = struct_ptr[0];  /* s.a */
        int y = struct_ptr[1];  /* s.b - different offset */
        struct_ptr[2] = x + y;  /* s.c */
        
        /* Barrier */
        __asm__ volatile("" : : : "memory");
    }
    
    /* Call dummy function to prevent dead code elimination */
    consume_result(dst, 100);
    
    /* Calculate checksum to ensure all operations matter */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= dst[i];
        checksum += src[i];
    }
    
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}
