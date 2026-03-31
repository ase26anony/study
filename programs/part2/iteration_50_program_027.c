/* auto_inc_dec_test.c
 * Designed to trigger auto-increment/decrement RTL pattern matching
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep RTL patterns distinct */
__attribute__((noinline)) 
void consume_array(volatile int* arr, int size) {
    volatile int sink = 0;
    for (int i = 0; i < size; i++) {
        sink += arr[i];
    }
    /* Use sink to prevent elimination */
    __asm__ volatile("" : : "r"(sink) : "memory");
}

/* Force register pressure and prevent optimization */
__attribute__((noinline))
void register_pressure() {
    __asm__ volatile(
        "mov r0, r0\n\t"
        "mov r1, r1\n\t"
        : : : "r0", "r1", "memory"
    );
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
     * CRITICAL LOOP: Designed to generate (mem (plus (reg) (const_int 0))) patterns
     * Multiple memory references with constant index 0
     */
    for (int i = 0; i < limit; i++) {
        /* Pattern 1: Load from src[0] - should generate (mem (reg)) */
        int val1 = src[0];
        
        /* Pattern 2: Load from src[1] - another base register */
        int val2 = src[1];
        
        /* Arithmetic operation to create register pressure */
        int result = val1 + val2 + 7;  /* Loop-invariant constant */
        
        /* Pattern 3: Store to dst[0] - (set (mem (reg)) (reg)) */
        dst[0] = result;
        
        /* Pattern 4: Store to dst[1] with different offset */
        dst[1] = result * 2;
        
        /* Inline assembly barrier to prevent reordering/elimination */
        __asm__ volatile("" : : : "memory", "r0", "r1", "r2", "r3");
        
        /* Additional scalar operations to create more patterns */
        volatile int* p = &src[10];
        
        /* Pattern 5: Pointer dereference - *p generates (mem (reg)) */
        int val3 = *p;
        
        /* Pattern 6: Another pointer dereference with different base */
        volatile int* q = &dst[20];
        *q = val3 + accumulator;
        
        /* Create artificial register dependencies */
        register_pressure();
        
        /* Update accumulator to prevent loop invariant removal */
        accumulator += i;
        
        /* Another memory barrier */
        __asm__ volatile("" : : : "memory");
    }
    
    /* 
     * Additional pointer-based accesses outside loop
     * to create more (mem (reg)) patterns
     */
    volatile int* ptr1 = &src[5];
    volatile int* ptr2 = &dst[15];
    
    /* Multiple independent memory operations */
    int temp1 = *ptr1;      /* (mem (reg)) */
    *ptr2 = temp1 * 3;      /* (set (mem (reg)) (reg)) */
    
    ptr1 = &src[25];
    ptr2 = &dst[35];
    
    int temp2 = *ptr1;      /* Another (mem (reg)) */
    *ptr2 = temp2 + 100;    /* Another (set (mem (reg)) (reg)) */
    
    /* Memory barrier between operations */
    __asm__ volatile("" : : : "memory", "r4", "r5");
    
    /* Struct access to create different addressing modes */
    struct {
        volatile int a;
        volatile int b;
        volatile int c;
    } s = {0};
    
    s.a = src[0];           /* (mem (reg)) pattern */
    s.b = src[1];           /* Another (mem (reg)) */
    s.c = s.a + s.b;        /* Register operation */
    
    /* Final consumption to prevent dead code elimination */
    consume_array(src, 100);
    consume_array(dst, 100);
    
    /* Calculate checksum to ensure all operations matter */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= src[i];
        checksum ^= dst[i];
    }
    
    /* Use checksum to prevent elimination */
    __asm__ volatile("" : : "r"(checksum) : "memory");
    
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}
