/* auto_inc_dec_test.c
 * Designed to trigger the specific RTL pattern (mem (plus (reg) (const_int 0)))
 * and reach the uncovered lines in auto-inc-dec.cc
 */

#include <stddef.h>

/* Prevent inlining to keep RTL patterns intact */
__attribute__((noinline)) 
static void use_result(volatile int *arr, size_t n) {
    volatile int sink = 0;
    for (size_t i = 0; i < n; ++i) {
        sink += arr[i];
    }
    (void)sink; /* Prevent unused variable warning */
}

int main(void) {
    /* Volatile arrays to prevent optimization of memory accesses */
    volatile int src[100];
    volatile int dst[100];
    
    /* Initialize source array with sequential values */
    for (int i = 0; i < 100; ++i) {
        src[i] = i * 3 + 1;
    }
    
    /* Non-constant loop bound to prevent unrolling */
    int limit = 90;
    
    /* Critical processing loop - designed to generate (mem (plus (reg) (const_int 0))) patterns */
    for (int i = 0; i < limit; ++i) {
        /* Pattern 1: Array access with constant index 0 - should generate (mem (reg)) or (mem (plus (reg) (const_int 0))) */
        int val1 = src[0];
        
        /* Arithmetic operation to create register pressure */
        val1 = val1 + (i * 2);
        
        /* Pattern 2: Store with constant index 0 */
        dst[0] = val1;
        
        /* Memory barrier to prevent reordering/elimination */
        __asm__ volatile("" : : : "memory");
        
        /* Pattern 3: Separate pointer-based access */
        volatile int *p = &src[10];
        int val2 = *p;  /* Should generate (mem (reg)) where reg holds &src[10] */
        
        /* More arithmetic to create register dependencies */
        val2 = val2 - i;
        dst[10] = val2;
        
        /* Architecture-specific register clobber to increase register pressure */
        #ifdef __arm__
        __asm__ volatile("" : : : "r0", "r1", "r2", "r3", "memory");
        #elif __x86_64__
        __asm__ volatile("" : : : "rax", "rbx", "rcx", "rdx", "memory");
        #elif __powerpc__
        __asm__ volatile("" : : : "r0", "r3", "r4", "r5", "memory");
        #else
        __asm__ volatile("" : : : "memory");
        #endif
        
        /* Additional memory patterns with zero offset */
        volatile int *q = &dst[20];
        *q = src[5] + src[6];  /* Multiple loads with zero offset */
    }
    
    /* Second loop with different access patterns */
    for (int i = 0; i < limit; i += 2) {
        /* Struct-like access pattern */
        struct pair {
            volatile int a;
            volatile int b;
        };
        
        /* Simulate struct access through pointers */
        volatile int *ptr_a = &src[i];
        volatile int *ptr_b = &dst[i];
        
        int a_val = *ptr_a;      /* (mem (reg)) pattern */
        int b_val = *ptr_b;      /* Another (mem (reg)) pattern */
        
        /* Cross-store pattern */
        *ptr_a = b_val + 1;
        *ptr_b = a_val - 1;
        
        /* Memory barrier */
        __asm__ volatile("" : : : "memory");
    }
    
    /* Use results to prevent dead code elimination */
    use_result(dst, 100);
    use_result(src, 100);
    
    /* Calculate checksum */
    int checksum = 0;
    for (int i = 0; i < 100; ++i) {
        checksum ^= dst[i];
        checksum += src[i];
    }
    
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}
