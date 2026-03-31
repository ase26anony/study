/* auto_inc_dec_test.c */
#include <stddef.h>

/* Prevent inlining to keep RTL patterns intact */
__attribute__((noinline)) void consume_data(volatile int* arr, int size) {
    volatile int sink = 0;
    for (int i = 0; i < size; i++) {
        sink += arr[i];
    }
    (void)sink;
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
    
    /* Critical processing loop - designed to generate (mem (plus (reg) (const_int 0))) patterns */
    for (int i = 0; i < limit; i++) {
        /* Pattern 1: Load from src[0] - should generate (mem (plus (reg) (const_int 0))) */
        int val1 = src[0];
        
        /* Pattern 2: Load from src[1] - another similar pattern */
        int val2 = src[1];
        
        /* Perform arithmetic operations */
        int result1 = val1 + 7;
        int result2 = val2 - 3;
        
        /* Pattern 3: Store to dst[0] - should generate (set (mem (plus (reg) (const_int 0))) (reg)) */
        dst[0] = result1;
        
        /* Pattern 4: Store to dst[1] */
        dst[1] = result2;
        
        /* Memory barrier to prevent reordering/elimination */
        __asm__ volatile("" : : : "memory");
        
        /* Additional pointer-based access for more (mem (reg)) patterns */
        volatile int *p = &src[10];
        volatile int *q = &dst[20];
        int val_p = *p;      /* Should generate (mem (reg)) */
        *q = val_p + 5;      /* Should generate (set (mem (reg)) (reg)) */
        
        /* Register pressure: clobber specific registers to force reloads */
        __asm__ volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5");
    }
    
    /* Second loop with different access patterns */
    volatile int *ptr1 = &src[0];
    volatile int *ptr2 = &dst[0];
    
    for (int i = 0; i < 50; i++) {
        /* Multiple memory references with zero offset patterns */
        int a = ptr1[0];     /* (mem (plus (reg) (const_int 0))) */
        int b = ptr1[1];     /* (mem (plus (reg) (const_int 4))) on 32-bit */
        int c = a + b;
        
        ptr2[0] = c;         /* (set (mem (plus (reg) (const_int 0))) (reg)) */
        ptr2[1] = c * 2;
        
        /* Scalar operations mixed with array accesses */
        volatile int temp = 0;
        temp = ptr1[2];      /* Another (mem (plus (reg) (const_int 8))) */
        temp += 10;
        ptr2[2] = temp;      /* Another store pattern */
        
        /* Memory barrier */
        __asm__ volatile("" : : : "memory");
    }
    
    /* Struct access to generate different base registers */
    struct {
        volatile int a;
        volatile int b;
        volatile int c;
    } s = {0};
    
    volatile int *base = &src[30];
    for (int i = 0; i < 20; i++) {
        s.a = base[0];       /* (mem (plus (reg) (const_int 0))) */
        s.b = base[1];       /* (mem (plus (reg) (const_int 4))) */
        s.c = s.a + s.b;
        
        /* Store back to different locations */
        dst[40] = s.c;
        dst[41] = s.c * 3;
        
        /* More register pressure */
        __asm__ volatile("" : : : "r6", "r7", "r8", "r9", "r10");
    }
    
    /* Prevent dead code elimination */
    consume_data(dst, 100);
    consume_data(src, 100);
    
    /* Return checksum */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= dst[i];
    }
    
    return checksum;
}
