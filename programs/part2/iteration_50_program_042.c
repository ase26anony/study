/* auto_inc_dec_test.c
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
        /* Pattern 1: Array access with constant index 0 - should generate (mem (plus (reg) (const_int 0))) */
        int val1 = src[0];  /* This should create: (set (reg) (mem (plus (reg) (const_int 0)))) */
        
        /* Arithmetic operation to create register pressure */
        val1 = val1 + 42;   /* Loop-invariant constant */
        
        /* Pattern 2: Store with constant index 0 */
        dst[0] = val1;      /* This should create: (set (mem (plus (reg) (const_int 0))) (reg)) */
        
        /* Memory barrier to prevent reordering/elimination */
        __asm__ volatile("" : : : "memory");
        
        /* Pattern 3: Separate pointer-based access with zero offset */
        volatile int *p = &src[10];
        int val2 = *p;      /* Should generate: (set (reg) (mem (reg))) */
        
        /* More arithmetic to create register dependencies */
        val2 = val2 * 2;
        
        /* Pattern 4: Another store through pointer */
        volatile int *q = &dst[20];
        *q = val2;          /* Should generate: (set (mem (reg)) (reg)) */
        
        /* Inline assembly with register clobbers for ARM */
        __asm__ volatile("" : : : "r0", "r1", "r2", "r3", "memory");
        
        /* Additional array accesses to create multiple base registers */
        src[5] = dst[5] + 1;
        dst[15] = src[15] - 1;
        
        /* Another memory barrier */
        __asm__ volatile("" : : : "memory");
    }
    
    /* Second loop with different access patterns */
    volatile int *ptr1 = &src[0];
    volatile int *ptr2 = &dst[0];
    
    for (int i = 0; i < 50; i++) {
        /* Pattern: Multiple memory references with the same base */
        int a = *ptr1;      /* (mem (reg)) */
        int b = *(ptr1 + 1); /* (mem (plus (reg) (const_int 4))) */
        
        /* Create register pressure */
        int c = a + b;
        
        /* Store to different base */
        *ptr2 = c;          /* (set (mem (reg)) (reg)) */
        *(ptr2 + 1) = c * 2;
        
        /* Memory barrier */
        __asm__ volatile("" : : : "memory");
        
        /* Scalar operations mixed with array ops */
        volatile int scalar = 0;
        scalar = src[i];    /* (set (reg) (mem (plus (reg) (const_int 0)))) */
        scalar += 5;
        dst[i] = scalar;    /* (set (mem (plus (reg) (const_int 0))) (reg)) */
        
        /* Register clobber for x86 */
        __asm__ volatile("" : : : "rax", "rbx", "rcx", "rdx", "memory");
    }
    
    /* Prevent dead code elimination */
    consume_result(dst, 100);
    
    /* Calculate checksum */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= dst[i];
    }
    
    return checksum;
}
