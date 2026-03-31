/* auto_inc_dec_test.c
 * Designed to trigger auto-increment/decrement RTL pattern matching
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to preserve RTL patterns */
__attribute__((noinline)) 
void use_result(volatile int* arr, size_t n) {
    volatile int sum = 0;
    for (size_t i = 0; i < n; i++) {
        sum += arr[i];
    }
    /* Use sum to prevent dead code elimination */
    if (sum == 0x12345678) {
        printf("Impossible condition\n");
    }
}

/* Another noinline function to create register pressure */
__attribute__((noinline))
int compute_value(int x, int y) {
    return (x * y) + (x ^ y);
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
    
    /* Main processing loop - designed to create (mem (plus (reg) (const_int 0))) patterns */
    for (int i = 0; i < limit; i++) {
        /* Pattern 1: Array access with constant index 0 - should create (mem (plus (reg) (const_int 0))) */
        int val1 = src[0];  /* This should generate: (set (reg) (mem (plus (reg) (const_int 0)))) */
        
        /* Pattern 2: Another array access with constant index 0 */
        int val2 = dst[0];  /* Another (mem (plus (reg) (const_int 0))) pattern */
        
        /* Perform arithmetic operation */
        int result = compute_value(val1, val2);
        
        /* Pattern 3: Store with constant index 0 */
        dst[0] = result;    /* (set (mem (plus (reg) (const_int 0))) (reg)) */
        
        /* Memory barrier to prevent reordering/optimization */
        __asm__ volatile("" : : : "memory");
        
        /* Pattern 4: Pointer dereference with zero offset */
        volatile int* p = &src[10];
        int val3 = *p;      /* Should generate: (set (reg) (mem (reg))) or (mem (plus (reg) (const_int 0))) */
        
        /* Pattern 5: Another pointer access */
        volatile int* q = &dst[20];
        *q = val3 + accumulator;
        
        /* Artificial register pressure */
        __asm__ volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "memory");
        
        /* Update accumulator to create loop-carried dependency */
        accumulator += (result & 0xFF);
        
        /* Additional memory operations to increase pattern instances */
        src[5] = accumulator;
        dst[5] = src[5] * 2;
    }
    
    /* Second loop with different access patterns */
    volatile int* ptr1 = &src[0];
    volatile int* ptr2 = &dst[0];
    
    for (int i = 0; i < 50; i++) {
        /* Multiple memory references in sequence */
        int a = ptr1[0];    /* (mem (plus (reg) (const_int 0))) */
        __asm__ volatile("" : : : "memory");
        int b = ptr1[1];    /* Different offset but same base */
        __asm__ volatile("" : : : "memory");
        
        ptr2[0] = a + b;    /* Store with base + 0 */
        __asm__ volatile("" : : : "memory");
        ptr2[1] = a - b;    /* Store with base + 4/8 depending on int size */
        
        /* Increment pointers manually to create patterns for auto-inc-dec to optimize */
        ptr1 = (volatile int*)((char*)ptr1 + sizeof(int));
        ptr2 = (volatile int*)((char*)ptr2 + sizeof(int));
        
        /* More register pressure */
        __asm__ volatile("" : : : "r8", "r9", "r10", "r11", "r12", "memory");
    }
    
    /* Struct access to create different base registers */
    struct pair {
        volatile int first;
        volatile int second;
    };
    
    struct pair pairs[10];
    for (int i = 0; i < 10; i++) {
        pairs[i].first = src[i];
        __asm__ volatile("" : : : "memory");
        pairs[i].second = dst[i];
        __asm__ volatile("" : : : "memory");
        
        /* Access struct members with zero offset from base */
        int f = pairs[i].first;   /* (mem (plus (reg) (const_int 0))) */
        int s = pairs[i].second;  /* (mem (plus (reg) (const_int 4/8))) */
        
        dst[i + 30] = f + s;
    }
    
    /* Call noinline function to prevent dead code elimination */
    use_result(dst, 100);
    
    /* Calculate checksum to ensure all operations matter */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= src[i];
        checksum += dst[i];
    }
    
    return checksum & 0xFF;
}
