/* Program to trigger auto-inc-dec RTL pattern (mem (plus (reg) (const_int 0))) */
#include <stddef.h>

/* Prevent inlining to keep RTL patterns distinct */
__attribute__((noinline)) void consume_result(volatile int* arr, int size) {
    volatile int sink = 0;
    for (int i = 0; i < size; ++i) {
        sink += arr[i];
    }
    (void)sink;
}

int main(void) {
    /* Volatile arrays to prevent optimization of memory operations */
    volatile int src[100];
    volatile int dst[100];
    
    /* Initialize source array with sequential values */
    for (int i = 0; i < 100; ++i) {
        src[i] = i * 3 + 1;
    }
    
    /* Non-constant loop bound to prevent unrolling */
    int limit = 90;
    volatile int accumulator = 0;
    
    /* Create multiple pointer-based accesses for distinct base registers */
    volatile int* p1 = &src[10];
    volatile int* p2 = &dst[20];
    
    /* Main processing loop with multiple memory reference patterns */
    for (int i = 0; i < limit; ++i) {
        /* Pattern 1: Array access with constant index 0 - should generate (mem (plus (reg) (const_int 0))) */
        int val1 = src[0];  /* Load from src[0] */
        
        /* Pattern 2: Pointer dereference with zero offset */
        int val2 = *p1;     /* Equivalent to p1[0] */
        
        /* Arithmetic operation (loop-invariant constant) */
        int result = val1 + val2 + 7;
        
        /* Pattern 3: Store to dst[0] */
        dst[0] = result;
        
        /* Pattern 4: Store through another pointer with zero offset */
        *p2 = result * 2;
        
        /* Inline assembly barrier with register clobbers to create register pressure */
        /* For ARM: */
        __asm__ volatile ("" : : : "memory", "r0", "r1", "r2", "r3");
        /* For x86: */
        /* __asm__ volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx"); */
        
        /* Scalar operations to generate (set (reg) (mem (reg))) patterns */
        volatile int temp = src[i % 50];  /* Variable index to prevent over-optimization */
        accumulator += temp;
        
        /* Another memory barrier to separate operations */
        __asm__ volatile ("" : : : "memory");
        
        /* Store back to create (set (mem (reg)) (reg)) pattern */
        dst[i % 50] = accumulator;
    }
    
    /* Additional pointer-based accesses outside loop */
    volatile int* p3 = &src[30];
    volatile int* p4 = &dst[40];
    int val3 = *p3;      /* Another (mem (reg)) pattern */
    *p4 = val3 * 3;
    
    /* Mix array and pointer accesses in a second loop */
    for (int i = 1; i < 50; ++i) {
        /* Access with base + 0 offset using different syntax */
        int a = *(src + 0);      /* Should be same as src[0] */
        int b = *(dst + 0);      /* Should be same as dst[0] */
        
        /* Store with offset 0 */
        *(dst + i) = a + b + i;
        
        /* Memory barrier */
        __asm__ volatile ("" : : : "memory");
    }
    
    /* Call dummy function to prevent dead code elimination */
    consume_result(dst, 100);
    
    /* Return checksum based on array contents */
    int checksum = 0;
    for (int i = 0; i < 100; ++i) {
        checksum ^= dst[i];
    }
    
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}
