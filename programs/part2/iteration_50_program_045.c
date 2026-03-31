/* auto_inc_dec_test.c
 * Designed to trigger auto-increment/decrement RTL pattern matching
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stddef.h>

/* Prevent inlining to keep RTL patterns simple */
__attribute__((noinline)) 
void use_result(volatile int* arr, int size) {
    volatile int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    /* Prevent optimization */
    __asm__ volatile("" : : "r"(sum) : "memory");
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
    
    /* Main processing loop - designed to generate (mem (plus (reg) (const_int 0))) patterns */
    for (int i = 0; i < limit; i++) {
        /* Pattern 1: Load from src[0] - creates (mem (plus (reg) (const_int 0))) */
        int val1 = src[0];
        
        /* Pattern 2: Another memory access with constant index 0 */
        int val2 = src[0];
        
        /* Arithmetic operation to create register pressure */
        int result = val1 + val2 + 7;
        
        /* Pattern 3: Store to dst[0] - creates another (mem (plus (reg) (const_int 0))) */
        dst[0] = result;
        
        /* Inline assembly barrier with register clobbers
         * Creates artificial dependencies and prevents reordering */
        __asm__ volatile("" : : : "r0", "r1", "r2", "r3", "memory");
        
        /* Pattern 4: Pointer-based access with zero offset */
        volatile int *p = &src[10];
        int val3 = *p;  /* Creates (mem (reg)) pattern */
        
        /* Pattern 5: Another pointer access */
        volatile int *q = &dst[20];
        *q = val3 + accumulator;
        
        /* Update accumulator to create loop-carried dependency */
        accumulator += result;
        
        /* Additional memory barrier */
        __asm__ volatile("" : : : "memory");
    }
    
    /* Secondary loop with different access patterns */
    volatile int *ptr1 = &src[5];
    volatile int *ptr2 = &dst[15];
    
    for (int j = 0; j < 50; j++) {
        /* Multiple memory references with zero offset */
        int a = ptr1[0];
        int b = ptr2[0];
        
        /* Scalar operations mixed with array accesses */
        int temp = a * b;
        dst[j % 10] = temp;
        
        /* Create register pressure */
        __asm__ volatile("" : : : "r4", "r5", "r6", "r7", "memory");
        
        /* Access with different base registers */
        src[(j + 1) % 10] = temp + j;
    }
    
    /* Struct access to create different memory patterns */
    struct {
        volatile int a;
        volatile int b;
        volatile int c;
    } s = {0};
    
    s.a = src[0];
    s.b = dst[0];
    s.c = accumulator;
    
    /* Call noinline function to prevent dead code elimination */
    use_result((volatile int*)dst, 100);
    
    /* Return checksum based on array contents */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= dst[i];
    }
    
    return checksum;
}
