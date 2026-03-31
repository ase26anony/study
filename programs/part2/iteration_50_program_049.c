/* auto_inc_dec_trigger.c
 * Program designed to generate RTL pattern (mem (plus (reg) (const_int 0)))
 * and trigger auto-increment/decrement optimization logic.
 */

#include <stddef.h>
#include <stdio.h>

/* Prevent inlining to preserve RTL patterns */
__attribute__((noinline)) 
static void consume_data(volatile int* arr, int size) {
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
    
    /* Create multiple pointer variables for distinct base registers */
    volatile int* src_ptr = &src[0];
    volatile int* dst_ptr = &dst[0];
    
    /* Additional pointer for another (mem (reg)) pattern */
    volatile int* p = &src[10];
    
    /* Main processing loop - designed to generate the target RTL pattern */
    for (int i = 0; i < limit; i++) {
        /* Pattern 1: (mem (plus (reg) (const_int 0))) via array indexing */
        int val1 = src[0];  /* Should generate: (mem (plus (reg:SI) (const_int 0))) */
        
        /* Pattern 2: Another memory access with constant 0 offset */
        int val2 = dst[0];  /* Another base register with offset 0 */
        
        /* Arithmetic operation to create register pressure */
        int result = val1 + val2 + 7;  /* Loop-invariant constant */
        
        /* Store back with constant 0 offset */
        dst[0] = result;
        
        /* Pattern 3: Pointer dereference (mem (reg)) */
        int val3 = *p;  /* Simple pointer dereference */
        
        /* Use inline assembly to create register dependencies and prevent reordering */
        __asm__ volatile(
            "nop\n\t"
            : 
            : 
            : "r0", "r1", "r2", "r3", "memory"
        );
        
        /* Mix scalar operations to generate (set (reg) (mem (reg))) patterns */
        volatile int temp = src_ptr[5];  /* Different offset to avoid pattern merging */
        temp = temp * 2;
        dst_ptr[5] = temp;
        
        /* Another memory barrier */
        __asm__ volatile("" : : : "memory");
        
        /* Accumulate to prevent dead code elimination */
        accumulator += result + val3;
        
        /* Additional pointer-based access with zero offset */
        volatile int* q = &src[20];
        int val4 = *q;
        accumulator += val4;
        
        /* Create artificial loop-carried dependency */
        __asm__ volatile(
            "add %0, %0, %1\n\t"
            : "+r" (accumulator)
            : "r" (i)
            : "cc"
        );
    }
    
    /* Second loop with different access pattern */
    for (int i = 0; i < 50; i++) {
        /* Access struct-like pattern using multiple arrays */
        int a = src[i];
        int b = src[i + 10];
        int c = src[i + 20];
        
        /* Store to different locations */
        dst[i] = a + b;
        dst[i + 30] = c;
        
        /* Memory barrier between operations */
        __asm__ volatile("" : : : "r4", "r5", "r6", "memory");
    }
    
    /* Call noinline function to consume results */
    consume_data(dst, 100);
    
    /* Calculate checksum to return */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= dst[i];  /* XOR checksum */
        checksum += src[i];   /* Add source values */
    }
    
    checksum += accumulator;
    
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}
