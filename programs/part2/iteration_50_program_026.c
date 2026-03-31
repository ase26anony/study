/* auto_inc_dec_test.c
 * Designed to trigger auto-increment/decrement RTL pattern matching
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stddef.h>

/* Prevent inlining to preserve RTL patterns */
__attribute__((noinline)) 
void use_result(volatile int *arr, int size) {
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
     * Critical processing loop - designed to generate:
     * (mem (plus (reg) (const_int 0))) patterns
     */
    for (int i = 0; i < limit; i++) {
        /* Pattern 1: Load from src[0] - creates (mem (reg)) with offset 0 */
        int val1 = src[0];
        
        /* Pattern 2: Another memory access with constant index 0 */
        int val2 = src[0];
        
        /* Arithmetic operation to create register pressure */
        int result = val1 + val2 + 7;
        
        /* Pattern 3: Store to dst[0] - creates (set (mem (reg)) (reg)) */
        dst[0] = result;
        
        /* Memory barrier to prevent reordering/elimination */
        __asm__ volatile("" : : : "memory");
        
        /* Additional scalar operations to create more patterns */
        accumulator += result;
        
        /* Pattern 4: Pointer-based access with zero offset */
        volatile int *p = &src[10];
        int val3 = *p;  /* Creates (mem (reg)) where reg holds &src[10] */
        
        /* Store back to create store pattern */
        *p = val3 + 1;
        
        /* Architecture-specific register clobbering */
        #ifdef __arm__
        __asm__ volatile("" : : : "r0", "r1", "r2", "r3", "memory");
        #elif __powerpc__
        __asm__ volatile("" : : : "r0", "r1", "r2", "r3", "memory");
        #else
        __asm__ volatile("" : : : "rax", "rbx", "rcx", "rdx", "memory");
        #endif
        
        /* Create another memory access pattern with different base */
        volatile int *q = &dst[20];
        *q = accumulator;
    }
    
    /* 
     * Additional loop with multiple array accesses 
     * to increase pattern matching opportunities
     */
    for (int i = 1; i < limit - 1; i++) {
        /* Multiple memory references with constant indices */
        int a = src[i - 1];    /* (mem (plus (reg) (const_int -4))) */
        int b = src[i];        /* (mem (plus (reg) (const_int 0))) */
        int c = src[i + 1];    /* (mem (plus (reg) (const_int 4))) */
        
        /* Complex enough to survive optimization */
        dst[i] = (a * b) / (c != 0 ? c : 1);
        
        /* Memory barrier between operations */
        __asm__ volatile("" : : : "memory");
    }
    
    /* Struct access to create different memory patterns */
    struct pair {
        volatile int first;
        volatile int second;
    };
    
    struct pair data[50];
    for (int i = 0; i < 50; i++) {
        data[i].first = src[i];
        data[i].second = dst[i];
        
        /* Access struct members with zero offset from base */
        int f = data[i].first;   /* (mem (reg)) */
        int s = data[i].second;  /* (mem (plus (reg) (const_int 4))) */
        
        /* Store back to create store patterns */
        data[i].first = f + s;
    }
    
    /* Final function call to prevent dead code elimination */
    use_result(dst, 100);
    
    /* Return checksum based on array contents */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= dst[i];
    }
    
    return checksum & 0xFF;
}
