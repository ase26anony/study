/* auto-inc-dec-trigger.c */
#include <stddef.h>

/* Prevent inlining to preserve RTL patterns */
__attribute__((noinline)) void consume_result(volatile int *arr, int size) {
    volatile int sink = 0;
    for (int i = 0; i < size; i++) {
        sink += arr[i];
    }
    (void)sink;
}

int main(void) {
    /* Volatile arrays to prevent optimization */
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
    volatile int *ptr0 = &src[10];
    
    /* Main processing loop - designed to generate (mem (plus (reg) (const_int 0))) */
    for (int i = 0; i < limit; i++) {
        /* Pattern 1: Array access with constant index 0 - should generate (mem (reg)) */
        int val1 = src[0];  /* (mem (plus (reg) (const_int 0))) */
        
        /* Pattern 2: Pointer dereference with zero offset */
        int val2 = *ptr0;   /* Another (mem (plus (reg) (const_int 0))) */
        
        /* Arithmetic operation to create register pressure */
        int result = val1 + val2 + 7;
        
        /* Pattern 3: Store with constant index 0 */
        dst[0] = result;    /* (set (mem (plus (reg) (const_int 0))) (reg)) */
        
        /* Inline assembly barrier to prevent reordering/elimination */
        __asm__ volatile("" : : : "memory");
        
        /* Additional memory operations with different bases */
        volatile int *ptr1 = &src[20];
        volatile int *ptr2 = &dst[30];
        
        /* More (mem (reg)) patterns */
        int val3 = *ptr1;
        *ptr2 = val3 * 2;
        
        /* Architecture-specific register clobber for ARM */
        #ifdef __arm__
        __asm__ volatile("" : : : "r0", "r1", "r2", "r3");
        #endif
        
        /* Scalar operations to create (set (reg) (mem (reg))) patterns */
        volatile int temp = src[5];
        temp = temp + accumulator;
        dst[5] = temp;
        accumulator = temp & 0xFF;
        
        /* Another barrier */
        __asm__ volatile("" : : : "memory");
    }
    
    /* Second loop with pointer arithmetic - may generate auto-increment */
    volatile int *src_ptr = &src[0];
    volatile int *dst_ptr = &dst[0];
    
    for (int i = 0; i < 50; i++) {
        /* Multiple memory references in sequence */
        int a = *src_ptr;           /* (mem (reg)) */
        __asm__ volatile("" : : : "memory");
        int b = *(src_ptr + 1);     /* (mem (plus (reg) (const_int 4))) on 32-bit */
        __asm__ volatile("" : : : "memory");
        
        *dst_ptr = a + b;           /* (set (mem (reg)) (reg)) */
        __asm__ volatile("" : : : "memory");
        
        /* Manual pointer increment to create register increment pattern */
        src_ptr = src_ptr + 1;
        dst_ptr = dst_ptr + 1;
    }
    
    /* Prevent dead code elimination */
    consume_result(dst, 100);
    
    /* Calculate checksum */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= dst[i];
    }
    
    return checksum & 0xFF;
}
