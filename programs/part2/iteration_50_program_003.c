/* auto_inc_dec_test.c
 * Designed to trigger auto-increment/decrement RTL pattern matching
 * Specifically targets: (mem (plus (reg) (const_int 0))) pattern
 */

#include <stddef.h>

/* Prevent inlining to preserve RTL patterns */
__attribute__((noinline)) 
static void use_result(volatile int *arr, int n) {
    volatile int sink = 0;
    for (int i = 0; i < n; i++) {
        sink += arr[i];
    }
    (void)sink; /* Prevent unused variable warning */
}

int main(void) {
    /* Volatile arrays to prevent optimization of memory operations */
    volatile int src[100];
    volatile int dst[100];
    
    /* Initialize source array with sequential values */
    for (int i = 0; i < 100; i++) {
        src[i] = i + 1;
    }
    
    /* Non-constant loop bound to prevent unrolling */
    int limit = 90;
    volatile int checksum = 0;
    
    /* Main processing loop - designed to generate (mem (plus (reg) (const_int 0))) patterns */
    for (int i = 0; i < limit; i++) {
        /* Pattern 1: Array access with constant index 0 - should generate (mem (plus (reg) (const_int 0))) */
        int val1 = src[0];  /* Load from src with offset 0 */
        
        /* Arithmetic operation to create register pressure */
        val1 = val1 + 42;   /* Add loop-invariant constant */
        
        /* Pattern 2: Store to dst with constant index 0 */
        dst[0] = val1;      /* Store to dst with offset 0 */
        
        /* Memory barrier to prevent reordering/elimination */
        __asm__ volatile("" : : : "memory");
        
        /* Pattern 3: Pointer-based access with zero offset */
        volatile int *p = &src[10];
        int val2 = *p;      /* Dereference pointer - should be (mem (reg)) */
        
        /* More arithmetic to create register dependencies */
        checksum += val2;
        
        /* Register clobbering inline assembly to create artificial dependencies */
        __asm__ volatile("" : : : "r0", "r1", "r2", "r3");
        
        /* Pattern 4: Scalar load-modify-store sequence */
        volatile int counter = 0;
        int temp = counter; /* Load scalar */
        temp = temp + i;    /* Modify */
        counter = temp;     /* Store back */
        
        /* Another memory barrier */
        __asm__ volatile("" : : : "memory");
    }
    
    /* Additional pointer-based patterns outside loop */
    volatile int *ptr1 = &src[20];
    volatile int *ptr2 = &dst[20];
    
    /* Multiple memory references in sequence */
    int a = *ptr1;          /* (mem (reg)) */
    __asm__ volatile("" : : : "memory");
    int b = *ptr2;          /* (mem (reg)) */
    __asm__ volatile("" : : : "memory");
    *ptr1 = a + b;          /* (set (mem (reg)) (reg)) */
    
    /* Struct access to create base+offset patterns */
    struct pair {
        volatile int first;
        volatile int second;
    };
    
    struct pair data[10];
    for (int i = 0; i < 10; i++) {
        /* These should generate multiple (mem (plus (reg) (const_int))) patterns */
        data[i].first = src[i];
        __asm__ volatile("" : : : "memory");
        data[i].second = dst[i];
        __asm__ volatile("" : : : "r4", "r5");
    }
    
    /* Final use of results to prevent dead code elimination */
    use_result(dst, 100);
    
    /* Return checksum based on array contents */
    int final_checksum = 0;
    for (int i = 0; i < 100; i++) {
        final_checksum ^= dst[i];  /* XOR checksum */
    }
    
    return final_checksum;
}
