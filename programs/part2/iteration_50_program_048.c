/* auto_inc_dec_test.c
 * Designed to trigger auto-increment/decrement RTL pattern matching
 * Specifically targets (mem (plus (reg) (const_int 0))) patterns
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
    /* Volatile arrays to prevent optimization of memory operations */
    volatile int src[100];
    volatile int dst[100];
    
    /* Initialize source array with sequential values */
    for (int i = 0; i < 100; i++) {
        src[i] = i * 3 + 1;
    }
    
    /* Non-constant loop bound to prevent full unrolling */
    int limit = 90;
    volatile int accumulator = 0;
    
    /* Main processing loop - designed to generate (mem (plus (reg) (const_int 0))) patterns */
    for (int i = 0; i < limit; i++) {
        /* Pattern 1: Array access with constant index 0 - should generate (mem (reg)) */
        int val1 = src[0];  /* (set (reg) (mem (plus (reg) (const_int 0)))) */
        
        /* Arithmetic operation to create register pressure */
        val1 = val1 + (i & 0xF);  /* Loop-variant constant to prevent CSE */
        
        /* Pattern 2: Store with constant index 0 */
        dst[0] = val1;  /* (set (mem (plus (reg) (const_int 0))) (reg)) */
        
        /* Memory barrier to prevent reordering/elimination */
        __asm__ volatile("" : : : "memory");
        
        /* Pattern 3: Pointer dereference with zero offset */
        volatile int *p = &src[10];
        int val2 = *p;  /* Another (mem (reg)) pattern */
        
        /* Create artificial register dependencies */
        __asm__ volatile("" : : : "r0", "r1", "r2", "r3");
        
        /* Pattern 4: Mixed scalar and array operations */
        volatile int temp = dst[5];  /* Load from different offset */
        temp = temp + src[15];       /* Another memory access */
        dst[5] = temp;               /* Store back */
        
        /* More memory operations to increase pattern density */
        accumulator += src[20] - dst[25];
        
        /* Additional barrier with register clobbers */
        __asm__ volatile("" : : : "r4", "r5", "r6", "r7", "memory");
    }
    
    /* Second loop with different access pattern */
    for (int i = 1; i < limit - 10; i++) {
        /* Access pattern designed to create multiple base registers */
        volatile int *src_ptr = &src[i];
        volatile int *dst_ptr = &dst[i + 5];
        
        /* Chain of memory operations */
        int a = *src_ptr;           /* (mem (reg)) */
        __asm__ volatile("" : : : "r8", "r9");
        int b = src_ptr[3];         /* (mem (plus (reg) (const_int 12))) on 32-bit */
        __asm__ volatile("" : : : "r10", "r11");
        *dst_ptr = a + b;           /* (set (mem (reg)) (reg)) */
        
        /* Scalar operation that gets stored */
        volatile int scalar = dst_ptr[-2];
        scalar = scalar * 2 + 1;
        dst_ptr[-2] = scalar;
    }
    
    /* Struct access to create different addressing modes */
    struct pair {
        volatile int a;
        volatile int b;
        volatile int c;
    };
    
    struct pair pairs[50];
    for (int i = 0; i < 50; i++) {
        pairs[i].a = src[i];
        __asm__ volatile("" : : : "memory");
        pairs[i].b = dst[i];
        __asm__ volatile("" : : : "r12", "r13", "r14", "r15");
        pairs[i].c = pairs[i].a + pairs[i].b;
    }
    
    /* Final consumption to prevent dead code elimination */
    consume_result(dst, 100);
    consume_result((volatile int*)pairs, 150);
    
    /* Return checksum based on results */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= dst[i];
    }
    for (int i = 0; i < 50; i++) {
        checksum ^= pairs[i].a ^ pairs[i].b ^ pairs[i].c;
    }
    
    return checksum & 0xFF;
}
