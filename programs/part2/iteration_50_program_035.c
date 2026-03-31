/* auto_inc_dec_test.c */
#include <stddef.h>

/* Prevent inlining to keep RTL patterns intact */
__attribute__((noinline)) void use_result(volatile int *arr, size_t n) {
    volatile int sink = 0;
    for (size_t i = 0; i < n; i++) {
        sink += arr[i];
    }
    (void)sink;
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
    
    /* Critical processing loop with multiple memory references */
    for (int i = 0; i < limit; i++) {
        /* Pattern 1: (mem (plus (reg) (const_int 0))) via array[0] */
        int val1 = src[0];  /* Should generate: (mem (plus (reg) (const_int 0))) */
        
        /* Pattern 2: Another (mem (plus (reg) (const_int 0))) via pointer */
        volatile int *p = &src[10];
        int val2 = *p;      /* Should generate: (mem (reg)) or (mem (plus (reg) (const_int 0))) */
        
        /* Arithmetic operation to create register pressure */
        int result = val1 + val2 + 42;  /* Loop-invariant constant */
        
        /* Pattern 3: Store with (mem (plus (reg) (const_int 0))) */
        dst[0] = result;    /* Should generate: (set (mem (plus (reg) (const_int 0))) (reg)) */
        
        /* Inline assembly barrier to prevent reordering/elimination */
        __asm__ volatile ("" : : : "memory", "r0", "r1", "r2", "r3");
        
        /* Additional scalar operations to create more patterns */
        volatile int temp = src[5];  /* Another (mem (plus (reg) (const_int 0))) */
        temp = temp * 2;
        dst[5] = temp;      /* Another store pattern */
        
        /* More inline assembly for register pressure */
        __asm__ volatile ("" : : : "r4", "r5", "r6", "r7");
    }
    
    /* Second loop with different access patterns */
    volatile int *src_ptr = &src[20];
    volatile int *dst_ptr = &dst[20];
    
    for (int i = 0; i < 50; i++) {
        /* Pattern: Load via pointer with offset 0 */
        int loaded = *(src_ptr + 0);  /* Should be (mem (plus (reg) (const_int 0))) */
        
        /* Modify and store */
        loaded += i;
        *(dst_ptr + 0) = loaded;      /* Store pattern */
        
        /* Memory barrier */
        __asm__ volatile ("" : : : "memory");
        
        /* Access struct-like pattern */
        struct pair {
            volatile int a;
            volatile int b;
        } data;
        
        data.a = src[30];  /* Load with base + offset 0 */
        data.b = src[31];  /* Load with base + offset 4 (on 32-bit) */
        dst[30] = data.a + data.b;  /* Store pattern */
    }
    
    /* Use results to prevent dead code elimination */
    use_result(dst, 100);
    
    /* Calculate checksum */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= dst[i];
    }
    
    return checksum;
}
