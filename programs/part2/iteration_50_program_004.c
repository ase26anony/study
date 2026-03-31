/* auto_inc_dec_test.c
 * Designed to trigger auto-increment/decrement RTL pattern generation
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stddef.h>

/* Prevent inlining to preserve RTL patterns */
__attribute__((noinline)) 
void use_result(volatile int* arr, int n) {
    volatile int sink = 0;
    for (int i = 0; i < n; i++) {
        sink += arr[i];
    }
    /* Prevent dead code elimination */
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
    
    /* 
     * Critical loop: Creates (mem (plus (reg) (const_int 0))) patterns
     * Multiple memory references with constant index 0
     */
    for (int i = 0; i < limit; i++) {
        /* Pattern 1: Load from src[0] - creates (mem (reg)) */
        int val1 = src[0];
        
        /* Pattern 2: Another load with constant index 0 */
        int val2 = src[0];
        
        /* Arithmetic operation to create register pressure */
        int result = val1 + val2 + 7;
        
        /* Pattern 3: Store to dst[0] - creates (set (mem (reg)) (reg)) */
        dst[0] = result;
        
        /* Inline assembly barrier with register clobbers */
        __asm__ volatile(
            ""
            : 
            : 
            : "memory", "r0", "r1", "r2", "r3"
        );
        
        /* Additional memory access pattern */
        accumulator += src[0];
        
        /* Pattern 4: Pointer-based access with zero offset */
        volatile int *p = &src[10];
        int val3 = *p;  /* Creates (mem (reg)) where reg holds &src[10] */
        
        /* Store with different base register */
        volatile int *q = &dst[20];
        *q = val3;
        
        /* Another barrier to separate memory operations */
        __asm__ volatile("" : : : "memory");
    }
    
    /* 
     * Second loop with array traversal - creates base+offset patterns
     * that might get simplified to base+0 then incremented
     */
    for (int i = 0; i < limit - 1; i++) {
        /* Access with variable index - may create (mem (plus (reg) (const_int))) */
        int a = src[i];
        int b = src[i + 1];
        dst[i] = a + b;
        
        /* Barrier to prevent reordering */
        __asm__ volatile("" : : : "r4", "r5", "memory");
    }
    
    /* 
     * Struct access pattern - creates multiple memory references
     * from the same base register
     */
    struct pair {
        volatile int first;
        volatile int second;
    };
    
    struct pair data[50];
    for (int i = 0; i < 50; i++) {
        data[i].first = i;
        data[i].second = i * 2;
    }
    
    /* Access struct fields - creates (mem (reg)) patterns */
    for (int i = 0; i < 25; i++) {
        int sum = data[i].first + data[i].second;
        data[i].first = sum;
        
        /* Barrier with specific register clobbers for ARM */
        __asm__ volatile(
            ""
            :
            :
            : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "memory"
        );
    }
    
    /* Mixed scalar and array operations */
    volatile int scalar = 42;
    for (int i = 0; i < 30; i++) {
        /* Load scalar - creates (set (reg) (mem (reg))) */
        int s = scalar;
        
        /* Load from array with constant index 0 */
        int arr_val = src[0];
        
        /* Operation mixing both */
        int mixed = s + arr_val;
        
        /* Store back to scalar */
        scalar = mixed;
        
        /* Store to array with constant index 0 */
        dst[0] = mixed;
        
        /* Comprehensive barrier */
        __asm__ volatile(
            ""
            :
            :
            : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
              "r8", "r9", "r10", "r11", "r12", "memory"
        );
    }
    
    /* Final use of results to prevent elimination */
    use_result((volatile int*)dst, 100);
    use_result((volatile int*)src, 100);
    
    /* Compute checksum */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= dst[i];
        checksum += src[i];
    }
    
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}
