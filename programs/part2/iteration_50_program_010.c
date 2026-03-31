/* auto-inc-dec-trigger.c */
#include <stddef.h>

/* Prevent inlining to keep RTL patterns intact */
__attribute__((noinline)) 
void consume_result(volatile int* arr, int size) {
    volatile int sink = 0;
    for (int i = 0; i < size; i++) {
        sink += arr[i];
    }
    (void)sink;
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
    
    /* Main processing loop - designed to generate (mem (plus (reg) (const_int 0))) patterns */
    for (int i = 0; i < limit; i++) {
        /* Pattern 1: Array access with constant index 0 - should generate (mem (reg)) */
        int val1 = src[0];  /* This should become (mem (plus (regX) (const_int 0))) */
        
        /* Arithmetic operation to create register pressure */
        val1 = val1 + (i & 0xF);  /* Loop-variant addition */
        
        /* Pattern 2: Store with constant index 0 */
        dst[0] = val1;  /* Another (mem (plus (regY) (const_int 0))) */
        
        /* Inline assembly barrier with register clobbering */
        /* Creates artificial dependencies and prevents reordering */
        __asm__ volatile ("" : : : "memory", "r0", "r1", "r2", "r3");
        
        /* Pattern 3: Pointer-based access with zero offset */
        volatile int *p = &src[10];
        int val2 = *p;  /* Should generate (mem (reg)) pattern */
        
        /* More arithmetic to use the loaded value */
        accumulator += val2;
        
        /* Pattern 4: Struct-like access pattern */
        /* Access different array elements to create multiple base registers */
        int val3 = src[20];  /* Another base register */
        dst[20] = val3 + accumulator;
        
        /* Another barrier to separate memory operations */
        __asm__ volatile ("" : : : "memory");
        
        /* Pattern 5: Scalar load-modify-store sequence */
        volatile int scalar = src[30];
        scalar = scalar * 2 + 1;
        dst[30] = scalar;
        
        /* Mix in some conditional logic to prevent over-optimization */
        if (i & 1) {
            src[40] = dst[40];
        } else {
            dst[40] = src[40] + 1;
        }
    }
    
    /* Additional pointer-chasing pattern outside loop */
    volatile int *ptr1 = &src[50];
    volatile int *ptr2 = &dst[50];
    for (int j = 0; j < 10; j++) {
        /* These should generate simple (mem (reg)) patterns */
        int tmp = *ptr1;
        *ptr2 = tmp + j;
        
        /* Barrier to prevent fusion */
        __asm__ volatile ("" : : : "memory");
        
        /* Access with different offset to create variety */
        tmp = src[60 + j];
        dst[60 + j] = tmp * 2;
    }
    
    /* Final consumption to prevent dead code elimination */
    consume_result(dst, 100);
    
    /* Return checksum based on array state */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= dst[i];
    }
    
    return checksum & 0xFF;
}
