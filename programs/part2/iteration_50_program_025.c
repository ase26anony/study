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
    (void)sink; /* Prevent unused variable warning */
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
    
    /* Critical processing loop - designed to generate (mem (plus (reg) (const_int 0))) patterns */
    for (int i = 0; i < limit; i++) {
        /* Pattern 1: Load from src[0] - creates (mem (plus (reg) (const_int 0))) */
        int val1 = src[0];
        
        /* Pattern 2: Load from src[1] - creates another similar pattern */
        int val2 = src[1];
        
        /* Arithmetic operations to create register pressure */
        int sum = val1 + val2;
        sum += 42; /* Loop-invariant constant */
        
        /* Pattern 3: Store to dst[0] - creates (set (mem (plus (reg) (const_int 0))) (reg)) */
        dst[0] = sum;
        
        /* Pattern 4: Store to dst[1] */
        dst[1] = sum * 2;
        
        /* Memory barrier to prevent reordering/elimination */
        __asm__ volatile("" : : : "memory");
        
        /* Additional pointer-based access for more (mem (reg)) patterns */
        volatile int *p = &src[10];
        int val3 = *p;  /* Creates (mem (reg)) pattern */
        
        /* More arithmetic to create register dependencies */
        val3 = val3 * 3 + 7;
        
        /* Store back through pointer */
        *p = val3;
        
        /* Architecture-specific register clobbering for ARM */
        #ifdef __arm__
        __asm__ volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7");
        #endif
        
        /* Scalar operations to generate load/store patterns */
        volatile int scalar = src[5];
        scalar += i;  /* Loop-variant modification */
        dst[5] = scalar;
        
        /* Another memory barrier */
        __asm__ volatile("" : : : "memory");
    }
    
    /* Second loop with different access patterns */
    volatile int *src_ptr = &src[20];
    volatile int *dst_ptr = &dst[20];
    
    for (int i = 0; i < 50; i++) {
        /* Pointer dereference with zero offset (through pointer arithmetic) */
        int val = *(src_ptr + 0);  /* Should create (mem (reg)) */
        
        /* Modify and store */
        val = val * 2 - 5;
        *(dst_ptr + 0) = val;  /* Another (mem (reg)) pattern */
        
        /* Inline assembly to create artificial dependencies */
        __asm__ volatile(
            "nop\n\t"
            "nop\n\t"
            : 
            : 
            : "memory"
        );
        
        /* Access with constant index 0 through different pointer */
        volatile int *q = &src[30];
        int temp = q[0];  /* Array indexing with constant 0 */
        dst[30] = temp + 1;
    }
    
    /* Struct access to create different memory patterns */
    struct pair {
        volatile int a;
        volatile int b;
    };
    
    struct pair pairs[10];
    for (int i = 0; i < 10; i++) {
        pairs[i].a = src[i];
        pairs[i].b = dst[i];
        
        /* Access struct fields with constant offsets */
        int a_val = pairs[0].a;  /* Should create (mem (plus (reg) (const_int 0))) */
        int b_val = pairs[0].b;  /* Another similar pattern */
        
        pairs[0].a = a_val + b_val;
        
        __asm__ volatile("" : : : "memory");
    }
    
    /* Final consumption to prevent dead code elimination */
    consume_result(dst, 100);
    
    /* Calculate checksum to ensure all operations matter */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= dst[i];
    }
    
    return checksum & 0xFF; /* Return non-zero to indicate execution */
}
