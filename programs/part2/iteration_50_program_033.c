/* auto_inc_dec_test.c
 * Designed to trigger auto-increment/decrement RTL pattern matching
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stddef.h>
#include <stdio.h>

/* Prevent inlining to keep RTL patterns simple */
__attribute__((noinline)) 
void consume_array(volatile int* arr, int size) {
    volatile int sink = 0;
    for (int i = 0; i < size; i++) {
        sink += arr[i];
    }
    /* Prevent optimization of sink */
    __asm__ volatile("" : : "r"(sink) : "memory");
}

int main(void) {
    /* Volatile arrays to prevent optimization of memory accesses */
    volatile int src[100];
    volatile int dst[100];
    
    /* Initialize source array with sequential values */
    for (int i = 0; i < 100; i++) {
        src[i] = i + 1;
    }
    
    /* Non-constant loop bound to prevent unrolling */
    int limit = 90;
    volatile int checksum = 0;
    
    /* Main processing loop - designed to create (mem (plus (reg) (const_int 0))) patterns */
    for (int i = 0; i < limit; i++) {
        /* Pattern 1: Load from src[0] - creates (mem (plus (reg) (const_int 0))) */
        int val1 = src[0];
        
        /* Pattern 2: Another memory access with constant index 0 */
        int val2 = src[1];  /* Different index to create separate base+offset */
        
        /* Arithmetic operation to create register pressure */
        int result = val1 + val2 + 42;  /* Loop-invariant constant */
        
        /* Pattern 3: Store to dst[0] - creates another (mem (plus (reg) (const_int 0))) */
        dst[0] = result;
        
        /* Inline assembly barrier to prevent reordering/elimination */
        __asm__ volatile("" : : : "memory", "r0", "r1", "r2", "r3");
        
        /* Pattern 4: Pointer-based access with zero offset */
        volatile int *p = &src[10];
        int val3 = *p;  /* Creates (mem (reg)) pattern */
        
        /* Pattern 5: Another pointer access */
        volatile int *q = &dst[20];
        *q = val3 + i;
        
        /* More register pressure */
        __asm__ volatile("" : : : "r4", "r5", "r6", "r7");
        
        /* Scalar operations to create load/store patterns */
        volatile int scalar = src[5];
        scalar += dst[5];
        dst[5] = scalar;
        
        checksum += result + val3;
    }
    
    /* Additional loop with different access patterns */
    for (int i = 0; i < 50; i++) {
        /* Array indexing with variable but predictable pattern */
        int idx = i & 3;  /* Creates 0-3 pattern */
        
        /* Multiple memory references in same statement */
        dst[idx] = src[idx] + src[idx + 1];
        
        /* Memory barrier */
        __asm__ volatile("" : : : "memory");
        
        /* Struct-like access pattern */
        volatile int* base1 = &src[30];
        volatile int* base2 = &dst[40];
        
        base1[0] = base2[0] + 1;
        base1[1] = base2[1] + 2;
        
        __asm__ volatile("" : : : "r8", "r9", "r10", "r11");
    }
    
    /* Prevent dead code elimination */
    consume_array(dst, 100);
    consume_array(src, 100);
    
    /* Return checksum to prevent optimization of entire computation */
    return checksum & 0xFF;
}
