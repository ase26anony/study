/* auto_inc_trigger.c
 * Designed to trigger auto-increment/decrement RTL pattern matching
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep RTL patterns simple */
__attribute__((noinline)) 
void consume_result(volatile int* arr, int size) {
    volatile int dummy = 0;
    for (int i = 0; i < size; i++) {
        dummy += arr[i];
    }
    /* Use dummy to prevent elimination */
    __asm__ volatile("" : : "r"(dummy) : "memory");
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
    volatile int accumulator = 0;
    
    /* Main processing loop - designed to create (mem (plus (reg) (const_int 0))) patterns */
    for (int i = 0; i < limit; i++) {
        /* Pattern 1: Array access with constant index 0 - creates (mem (reg)) */
        int val1 = src[0];  /* Should generate: (set (reg) (mem (plus (reg) (const_int 0)))) */
        
        /* Arithmetic operation to create register pressure */
        val1 = val1 + 7;  /* Loop-invariant constant */
        
        /* Pattern 2: Store with constant index 0 */
        dst[0] = val1;    /* Should generate: (set (mem (plus (reg) (const_int 0))) (reg)) */
        
        /* Memory barrier to prevent reordering/elimination */
        __asm__ volatile("" : : : "memory");
        
        /* Pattern 3: Separate pointer-based access */
        volatile int *p = &src[10];
        int val2 = *p;    /* Another (mem (reg)) pattern */
        
        /* Use val2 to prevent dead code elimination */
        accumulator += val2;
        
        /* Architecture-specific register clobbering to create register pressure */
        #ifdef __arm__
        __asm__ volatile("" : : : "r0", "r1", "r2", "r3", "memory");
        #elif __powerpc__
        __asm__ volatile("" : : : "r3", "r4", "r5", "r6", "memory");
        #else
        __asm__ volatile("" : : : "rax", "rbx", "rcx", "rdx", "memory");
        #endif
        
        /* Additional memory operations to increase pattern instances */
        src[5] = src[5] + 1;  /* Load-modify-store pattern */
        dst[5] = src[5] * 2;
        
        /* Another barrier */
        __asm__ volatile("" : : : "memory");
    }
    
    /* Second loop with different access patterns */
    volatile int *src_ptr = (volatile int*)src;
    volatile int *dst_ptr = (volatile int*)dst;
    
    for (int i = 0; i < 50; i++) {
        /* Multiple independent memory references */
        int a = src_ptr[0];  /* (mem (plus (reg) (const_int 0))) */
        int b = src_ptr[1];
        int c = src_ptr[2];
        
        /* Scalar operations mixed with array ops */
        int sum = a + b + c;
        dst_ptr[0] = sum;    /* Another (mem (plus (reg) (const_int 0))) */
        
        /* Create artificial pointer arithmetic that might be optimized */
        src_ptr = (volatile int*)((char*)src_ptr + sizeof(int));
        dst_ptr = (volatile int*)((char*)dst_ptr + sizeof(int));
        
        /* Barrier with register clobbering */
        __asm__ volatile("" : : : "memory");
    }
    
    /* Force consumption of results to prevent elimination */
    consume_result(dst, 100);
    
    /* Calculate checksum */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= dst[i];  /* Simple checksum */
    }
    
    /* Use checksum to prevent dead code elimination */
    __asm__ volatile("" : : "r"(checksum) : "memory");
    
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}
