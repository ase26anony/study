/* auto_inc_trigger.c
 * Designed to trigger auto-increment/decrement RTL pattern matching
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stddef.h>

/* Prevent inlining to preserve RTL patterns */
__attribute__((noinline)) 
void consume_result(volatile int* arr, int size) {
    volatile int dummy = 0;
    for (int i = 0; i < size; i++) {
        dummy += arr[i];
    }
    /* Use dummy to prevent optimization */
    asm volatile("" : : "r"(dummy) : "memory");
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
    
    /* Create multiple pointer-based accesses to generate (mem (reg)) patterns */
    volatile int* p1 = &src[10];
    volatile int* p2 = &dst[20];
    
    /* Main processing loop - designed to create multiple (mem (plus (reg) (const_int 0))) patterns */
    for (int i = 0; i < limit; i++) {
        /* Pattern 1: Array access with constant index 0 - should generate (mem (plus (reg) (const_int 0))) */
        int val1 = src[0];  /* This should create: (set (reg) (mem (plus (reg) (const_int 0)))) */
        
        /* Pattern 2: Another array access with constant index 0 */
        int val2 = dst[0];  /* Another (mem (plus (reg) (const_int 0))) pattern */
        
        /* Arithmetic operation to create register pressure */
        val1 = val1 + 7;
        val2 = val2 * 2;
        
        /* Store back with constant index 0 */
        dst[0] = val1;      /* (set (mem (plus (reg) (const_int 0))) (reg)) */
        src[0] = val2;      /* Another store pattern */
        
        /* Inline assembly barrier with register clobbers - creates artificial dependencies */
        /* For ARM: clobber r0-r3 to create register pressure */
        asm volatile("" : : : "memory", "r0", "r1", "r2", "r3");
        
        /* Pattern 3: Pointer dereference - creates (mem (reg)) pattern */
        int ptr_val = *p1;  /* Should generate: (set (reg) (mem (reg))) */
        
        /* Modify and store through pointer */
        ptr_val += i;
        *p2 = ptr_val;      /* (set (mem (reg)) (reg)) */
        
        /* Another assembly barrier */
        asm volatile("" : : : "memory");
        
        /* Scalar operations to create more register pressure */
        accumulator += val1 + val2 + ptr_val;
        
        /* Access array with variable index to create different patterns */
        src[i % 50] = accumulator;
        dst[(i + 10) % 50] = src[i % 50] * 2;
        
        /* Final barrier to separate memory operations */
        asm volatile("" : : : "memory", "r4", "r5");
    }
    
    /* Additional pointer-based patterns outside loop */
    volatile int* p3 = &src[30];
    volatile int* p4 = &dst[40];
    
    /* Create more (mem (reg)) patterns */
    int final_val = *p3;
    *p4 = final_val * 3;
    
    /* Mix scalar and array operations */
    volatile int scalar = src[5];
    scalar = scalar + dst[5];
    dst[5] = scalar;
    
    /* Call noinline function to prevent dead code elimination */
    consume_result((volatile int*)dst, 100);
    
    /* Return checksum based on array contents */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= dst[i];
    }
    
    return checksum & 0xFF;
}
