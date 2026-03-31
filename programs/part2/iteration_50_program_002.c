/* auto_inc_dec_test.c
 * Designed to trigger auto-increment/decrement RTL pattern matching
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stddef.h>

/* Prevent inlining to preserve RTL patterns */
__attribute__((noinline)) 
void consume_result(volatile int* arr, int size) {
    volatile int sink = 0;
    for (int i = 0; i < size; i++) {
        sink += arr[i];
    }
    (void)sink; /* Prevent unused variable warning */
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
    
    /* Main processing loop - designed to create (mem (plus (reg) (const_int 0))) patterns */
    for (int i = 0; i < limit; i++) {
        /* Pattern 1: Array access with constant index 0 - creates (mem (reg)) pattern */
        int val1 = src[0];  /* Should generate: (set (reg) (mem (plus (reg) (const_int 0)))) */
        
        /* Arithmetic operation to create register pressure */
        val1 = val1 + 7;
        
        /* Pattern 2: Store with constant index 0 */
        dst[0] = val1;      /* Should generate: (set (mem (plus (reg) (const_int 0))) (reg)) */
        
        /* Memory barrier to prevent reordering/optimization */
        __asm__ volatile("" : : : "memory");
        
        /* Pattern 3: Pointer dereference with zero offset */
        volatile int *p = &src[10];
        int val2 = *p;      /* Should generate: (set (reg) (mem (reg))) */
        
        /* More arithmetic to create register dependencies */
        val2 = val2 * 2 - 5;
        
        /* Pattern 4: Another pointer store */
        volatile int *q = &dst[20];
        *q = val2;          /* Should generate: (set (mem (reg)) (reg)) */
        
        /* Register clobber to create artificial register pressure */
        __asm__ volatile("" : : : "r0", "r1", "r2", "r3", "memory");
        
        /* Additional array accesses to create more base registers */
        src[5] = dst[5] + src[5];
        dst[15] = src[15] * 3;
        
        /* Another memory barrier */
        __asm__ volatile("" : : : "memory");
    }
    
    /* Second loop with different access pattern */
    volatile int *ptr1 = &src[30];
    volatile int *ptr2 = &dst[40];
    
    for (int j = 0; j < 50; j++) {
        /* Multiple memory references in sequence */
        int a = ptr1[0];    /* (mem (plus (reg) (const_int 0))) */
        int b = ptr2[0];    /* (mem (plus (reg) (const_int 0))) */
        
        /* Create computation chain */
        int c = a + b;
        c = c * 2;
        
        /* Store results back */
        ptr1[0] = c;        /* (set (mem (plus (reg) (const_int 0))) (reg)) */
        ptr2[0] = c + 1;    /* (set (mem (plus (reg) (const_int 0))) (reg)) */
        
        /* Inline assembly with register clobbers for ARM */
        __asm__ volatile("" : : : "r4", "r5", "r6", "r7", "memory");
        
        /* Increment pointers to create potential auto-increment patterns */
        ptr1 = ptr1 + 1;
        ptr2 = ptr2 + 1;
    }
    
    /* Struct access to create different memory patterns */
    struct data {
        volatile int x;
        volatile int y;
        volatile int z;
    };
    
    struct data s1, s2;
    s1.x = 100;
    s1.y = 200;
    s1.z = 300;
    
    /* Access struct fields - each creates separate base+offset patterns */
    for (int k = 0; k < 25; k++) {
        s2.x = s1.x + k;    /* Field access with zero effective offset */
        s2.y = s1.y * k;
        s2.z = s1.z - k;
        
        __asm__ volatile("" : : : "memory");
    }
    
    /* Final consumption to prevent dead code elimination */
    consume_result(dst, 100);
    
    /* Calculate checksum to ensure all operations matter */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= src[i];
        checksum ^= dst[i];
    }
    
    return checksum & 0xFF; /* Return non-zero to indicate execution */
}
