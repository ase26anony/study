/* auto_inc_dec_trigger.c
 * Designed to trigger auto-increment/decrement RTL pattern matching
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep RTL patterns intact */
__attribute__((noinline)) 
void consume_result(volatile int* arr, int size) {
    volatile int dummy = 0;
    for (int i = 0; i < size; i++) {
        dummy += arr[i];
    }
    /* Use dummy to prevent optimization */
    if (dummy == 0x12345678) {
        printf("Impossible\n");
    }
}

/* Force register pressure with artificial clobbers */
#define MEMORY_BARRIER() __asm__ volatile("" : : : "memory")
#define REGISTER_CLOBBER() __asm__ volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5")

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
    volatile int loop_counter = 0;
    
    /* Main processing loop - designed to create (mem (plus (reg) (const_int 0))) patterns */
    for (int i = 0; i < limit; i++) {
        /* Pattern 1: Array access with constant index 0 - creates (mem (reg)) */
        int val1 = src[0];  /* Should become: (set (reg) (mem (reg))) */
        
        /* Arithmetic operation to create register pressure */
        val1 = val1 + (i * 2);
        
        /* Store with constant index 0 - creates (set (mem (reg)) (reg)) */
        dst[0] = val1;
        
        /* Memory barrier to prevent reordering/elimination */
        MEMORY_BARRIER();
        
        /* Pattern 2: Pointer dereference with zero offset */
        volatile int* p = &src[10];
        int val2 = *p;  /* Should become: (set (reg) (mem (plus (reg) (const_int 0)))) */
        
        /* Modify and store back through pointer */
        val2 = val2 - (i % 5);
        *p = val2;
        
        /* Register clobber to create artificial dependencies */
        REGISTER_CLOBBER();
        
        /* Pattern 3: Multiple array accesses with constant indices */
        int val3 = src[5];   /* Another (mem (reg)) pattern */
        dst[5] = val3 + src[20];  /* Two memory accesses in one expression */
        
        /* Pattern 4: Struct-like access pattern */
        volatile int* base1 = &src[30];
        volatile int* base2 = &dst[30];
        
        /* Create sequence of memory accesses from same base */
        int tmp1 = base1[0];  /* (mem (plus (reg) (const_int 0))) */
        int tmp2 = base1[1];  /* (mem (plus (reg) (const_int 4))) on 32-bit */
        base2[0] = tmp1 + tmp2;
        
        /* Additional barrier to separate memory operations */
        MEMORY_BARRIER();
        
        /* Scalar operations mixed with array accesses */
        volatile int accumulator = 0;
        accumulator = src[40];  /* Load */
        accumulator = accumulator * 2;
        dst[40] = accumulator;  /* Store */
        
        loop_counter++;
    }
    
    /* Pattern 5: Post-loop pointer arithmetic */
    volatile int* walk = &src[50];
    for (int j = 0; j < 10; j++) {
        int val = walk[0];  /* Constant zero offset */
        dst[50 + j] = val + j;
        
        /* This might encourage post-increment if walk++ appears later */
        MEMORY_BARRIER();
        
        /* Simulate pointer increment in separate step */
        if (j < 9) {
            /* Create register copy and increment */
            volatile int* next = walk + 1;
            REGISTER_CLOBBER();
            walk = next;
        }
    }
    
    /* Final consumption to prevent dead code elimination */
    consume_result(dst, 100);
    
    /* Calculate checksum to ensure all operations matter */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= dst[i];
    }
    
    return checksum & 0xFF;
}
