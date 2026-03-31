/* auto_inc_dec_test.c
 * Designed to trigger auto-increment/decrement RTL pattern matching
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to preserve RTL patterns */
__attribute__((noinline)) 
void use_result(volatile int* arr, int size) {
    volatile int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    /* Use sum to prevent dead code elimination */
    if (sum == 0xdeadbeef) {
        printf("Impossible\n");
    }
}

/* Another noinline function to create register pressure */
__attribute__((noinline, optimize("O0")))
void barrier_function(void) {
    __asm__ volatile ("" : : : "memory");
}

int main(void) {
    /* Volatile arrays to prevent optimization of memory accesses */
    volatile int src[100];
    volatile int dst[100];
    
    /* Initialize source array with sequential values */
    for (int i = 0; i < 100; i++) {
        src[i] = i * 3 + 1;
    }
    
    /* Initialize destination array */
    for (int i = 0; i < 100; i++) {
        dst[i] = 0;
    }
    
    /* Non-constant loop bound to prevent unrolling */
    int limit = 90;
    volatile int loop_counter = 0;
    
    /* Main processing loop designed to create (mem (plus (reg) (const_int 0))) patterns */
    for (int i = 0; i < limit; i++) {
        /* Pattern 1: Array access with constant index 0 - should create (mem (reg)) */
        int val1 = src[0];  /* This should generate: (set (reg) (mem (reg))) */
        
        /* Pattern 2: Another array access with constant index 0 */
        int val2 = dst[0];  /* Another (mem (reg)) pattern */
        
        /* Arithmetic operation to create register pressure */
        int result = val1 + val2 + 42;
        
        /* Pattern 3: Store with constant index 0 */
        dst[0] = result;    /* This should generate: (set (mem (reg)) (reg)) */
        
        /* Inline assembly to prevent reordering and create register dependencies */
        __asm__ volatile (
            ""
            : 
            : 
            : "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"
        );
        
        /* Pattern 4: Pointer dereference with zero offset */
        volatile int *p = &src[10];
        int val3 = *p;      /* Should create: (mem (plus (reg) (const_int 0))) */
        
        /* Modify and store back through pointer */
        *p = val3 + 1;
        
        /* Another barrier to separate memory operations */
        barrier_function();
        
        /* Scalar operations to create more register pressure */
        volatile int scalar = i;
        scalar = scalar * 2 + 1;
        dst[5] = scalar;
        
        loop_counter++;
    }
    
    /* Second loop with different access pattern */
    volatile int *src_ptr = &src[20];
    volatile int *dst_ptr = &dst[20];
    
    for (int i = 0; i < 50; i++) {
        /* Multiple memory references in sequence */
        int a = src_ptr[0];     /* (mem (reg)) */
        int b = src_ptr[1];     /* (mem (plus (reg) (const_int 4))) on 32-bit */
        
        /* Arithmetic with both values */
        int c = a + b;
        
        /* Store to different locations */
        dst_ptr[0] = c;         /* (set (mem (reg)) (reg)) */
        dst_ptr[1] = c * 2;
        
        /* Complex inline assembly with specific clobbers */
        __asm__ volatile (
            "nop\n\t"
            "nop"
            : 
            : 
            : "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "memory"
        );
        
        /* Access struct-like pattern */
        volatile int* arrs[3] = {&src[30], &src[40], &src[50]};
        for (int j = 0; j < 3; j++) {
            int temp = arrs[j][0];  /* Multiple (mem (reg)) patterns */
            arrs[j][0] = temp + j;
        }
    }
    
    /* Final checksum calculation */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= dst[i];
        checksum += src[i];
    }
    
    /* Use results to prevent elimination */
    use_result((volatile int*)dst, 100);
    use_result((volatile int*)src, 100);
    
    /* Return checksum to prevent tail-call optimization */
    if (checksum == 0) {
        return 0;
    }
    
    return checksum & 0xFF;
}
