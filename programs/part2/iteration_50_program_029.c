/* auto_inc_dec_test.c
 * Designed to trigger auto-increment/decrement RTL pattern matching
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stddef.h>

/* Prevent inlining to preserve RTL patterns */
__attribute__((noinline)) 
void use_result(volatile int* arr, int size) {
    volatile int dummy = 0;
    for (int i = 0; i < size; i++) {
        dummy += arr[i];
    }
    /* Prevent optimization */
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
    
    /* 
     * Critical loop: Creates (mem (plus (reg) (const_int 0))) patterns
     * The auto-inc-dec pass looks for these to combine into auto-increment
     */
    for (int i = 0; i < limit; i++) {
        /* Pattern 1: Load from src[0] - creates (mem (reg)) with zero offset */
        int val1 = src[0];  /* Should generate: (set (reg) (mem (plus (reg) (const_int 0)))) */
        
        /* Arithmetic operation to create register pressure */
        val1 = val1 + (i & 0xF);  /* Loop-variant constant */
        
        /* Pattern 2: Store to dst[0] - creates (mem (reg)) pattern */
        dst[0] = val1;  /* Should generate: (set (mem (plus (reg) (const_int 0))) (reg)) */
        
        /* Memory barrier to prevent reordering/elimination */
        __asm__ volatile("" : : : "memory");
        
        /* Pattern 3: Separate pointer access - creates another (mem (reg)) */
        volatile int *p = &src[10];
        int val2 = *p;  /* Direct pointer dereference: (mem (reg)) */
        
        /* Use val2 to prevent dead code elimination */
        accumulator += val2;
        
        /* Architecture-specific register clobbering for ARM */
        #ifdef __arm__
        __asm__ volatile("" : : : "r0", "r1", "r2", "r3", "memory");
        #elif __x86_64__
        __asm__ volatile("" : : : "rax", "rbx", "rcx", "rdx", "memory");
        #elif __powerpc__
        __asm__ volatile("" : : : "r0", "r1", "r2", "r3", "memory");
        #else
        __asm__ volatile("" : : : "memory");
        #endif
        
        /* Additional memory operations to create more patterns */
        dst[5] = src[5] + accumulator;
        __asm__ volatile("" : : : "memory");
        
        /* Scalar operations mixed with array accesses */
        volatile int scalar = src[20];
        scalar = scalar * 2 - 1;
        dst[20] = scalar;
    }
    
    /* 
     * Additional pointer-based loop with constant zero offset
     * This should generate the exact pattern: (mem (plus (reg) (const_int 0)))
     */
    volatile int *src_ptr = &src[30];
    volatile int *dst_ptr = &dst[30];
    
    for (int i = 0; i < 20; i++) {
        /* Multiple memory references with zero offset */
        int temp = src_ptr[0];  /* (mem (plus (reg) (const_int 0))) */
        temp = temp + dst_ptr[0];  /* Another (mem (plus (reg) (const_int 0))) */
        dst_ptr[0] = temp;  /* Store with same pattern */
        
        /* Barrier with register clobbering */
        __asm__ volatile("" : : : "memory");
        
        /* Pointer arithmetic outside the memory access */
        src_ptr = src_ptr + 1;
        dst_ptr = dst_ptr + 1;
    }
    
    /* Struct access to create different base registers */
    struct pair {
        volatile int a;
        volatile int b;
    };
    
    struct pair pairs[10];
    for (int i = 0; i < 10; i++) {
        pairs[i].a = src[i * 2];      /* Load with base + offset */
        pairs[i].b = pairs[i].a * 3;  /* Store to different field */
        __asm__ volatile("" : : : "memory");
    }
    
    /* Final use of results to prevent dead code elimination */
    use_result(dst, 100);
    
    /* Return checksum based on array contents */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= dst[i];
    }
    
    return checksum & 0xFF;
}
