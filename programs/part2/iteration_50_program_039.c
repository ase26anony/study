/* auto_inc_dec_test.c
 * Designed to generate RTL pattern (mem (plus (reg) (const_int 0)))
 * for auto-inc-dec pass optimization
 */

#include <stddef.h>

/* Prevent inlining to preserve RTL patterns */
__attribute__((noinline)) 
static void use_result(volatile int *arr, size_t n) {
    volatile int sink = 0;
    for (size_t i = 0; i < n; i++) {
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
    
    /* Create pointer with zero offset pattern */
    volatile int *ptr0 = &src[10];
    
    /* Main processing loop - designed to generate (mem (plus (reg) (const_int 0))) */
    for (int i = 0; i < limit; i++) {
        /* Pattern 1: Array access with constant index 0 - should generate (mem (reg)) */
        int val1 = src[0];  /* Should become: (set (reg) (mem (reg))) */
        
        /* Arithmetic operation to create register pressure */
        val1 = val1 + (i * 2);
        
        /* Store with constant index 0 - should generate (set (mem (reg)) (reg)) */
        dst[0] = val1;
        
        /* Memory barrier to prevent reordering/elimination */
        __asm__ volatile("" : : : "memory");
        
        /* Pattern 2: Pointer dereference with zero offset */
        int val2 = *ptr0;  /* Should generate (mem (reg)) pattern */
        
        /* More arithmetic */
        val2 = val2 + src[0];  /* Another memory access */
        
        /* Store to different location */
        dst[1] = val2;
        
        /* Architecture-specific register clobber to create register pressure */
        #ifdef __arm__
        __asm__ volatile("" : : : "r0", "r1", "r2", "r3", "memory");
        #elif __x86_64__
        __asm__ volatile("" : : : "rax", "rbx", "rcx", "rdx", "memory");
        #elif __powerpc__
        __asm__ volatile("" : : : "r0", "r1", "r2", "r3", "memory");
        #else
        __asm__ volatile("" : : : "memory");
        #endif
        
        /* Pattern 3: Multiple array accesses with constant 0 indices */
        int val3 = src[0] + dst[0];  /* Two (mem (reg)) patterns */
        
        /* Store with offset */
        dst[i % 10] = val3;
        
        /* Accumulate to prevent dead code elimination */
        accumulator += val1 + val2 + val3;
    }
    
    /* Additional pointer-based patterns outside loop */
    volatile int *p1 = &src[20];
    volatile int *p2 = &dst[30];
    
    /* Series of memory operations that should generate the target RTL */
    for (int j = 0; j < 10; j++) {
        /* Direct pointer dereference - should be (mem (reg)) */
        int tmp = *p1;
        
        /* Arithmetic */
        tmp = tmp * 2 + j;
        
        /* Store via pointer - should be (set (mem (reg)) (reg)) */
        *p2 = tmp;
        
        /* Memory barrier */
        __asm__ volatile("" : : : "memory");
        
        /* Alternate between two pointers */
        if (j % 2 == 0) {
            p1 = &src[25];
        } else {
            p1 = &src[20];
        }
    }
    
    /* Struct access pattern */
    struct pair {
        volatile int a;
        volatile int b;
    } sp;
    
    sp.a = src[0];  /* Should generate (set (mem (reg)) (mem (reg))) */
    sp.b = dst[0];
    
    /* Final use of results to prevent optimization */
    use_result((volatile int *)dst, 100);
    use_result((volatile int *)src, 100);
    
    /* Return checksum */
    return accumulator & 0xFF;
}
