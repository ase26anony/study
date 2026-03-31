/* auto_inc_dec_test.c
 * Designed to trigger auto-increment/decrement RTL pattern matching
 * Specifically targets (mem (plus (reg) (const_int 0))) patterns
 */

#include <stddef.h>

/* Prevent inlining to preserve RTL patterns */
__attribute__((noinline)) 
void consume_array(volatile int* arr, int size) {
    volatile int sink = 0;
    for (int i = 0; i < size; i++) {
        sink += arr[i];
    }
    (void)sink; /* Prevent unused variable warning */
}

/* Force register pressure and prevent optimization */
__attribute__((noinline))
void register_barrier(void) {
    /* Clobber multiple registers to force spills/reloads */
    __asm__ volatile ("" : : : "r0", "r1", "r2", "r3", "r4", "r5", "memory");
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
    volatile int loop_counter = 0;
    
    /* Create multiple pointer variables for distinct base registers */
    volatile int* src_ptr = &src[0];
    volatile int* dst_ptr = &dst[0];
    volatile int* alt_ptr = &src[10];
    
    /* Main processing loop with the target patterns */
    for (int i = 0; i < limit; i++) {
        /* PATTERN 1: (mem (plus (reg) (const_int 0))) 
         * Array access with constant index 0 */
        int val1 = src[0];  /* Should generate: (mem (plus (reg:SI) (const_int 0))) */
        
        /* Arithmetic operation with loop-invariant constant */
        val1 += 42;
        
        /* PATTERN 2: Another (mem (plus (reg) (const_int 0))) pattern */
        dst[0] = val1;      /* Store with constant index 0 */
        
        /* Memory barrier to prevent reordering/elimination */
        __asm__ volatile ("" : : : "memory");
        
        /* PATTERN 3: Pointer dereference creating (mem (reg)) */
        int val2 = *alt_ptr;
        val2 += i;  /* Use loop index to prevent hoisting */
        
        /* Store to another location */
        dst[10] = val2;
        
        /* Artificial register pressure */
        register_barrier();
        
        /* Scalar operations to create register pressure */
        volatile int scalar = src[5];
        scalar *= 2;
        dst[5] = scalar;
        
        /* Multiple memory references with different base registers */
        int val3 = src_ptr[0];  /* Pointer with offset 0 */
        int val4 = dst_ptr[0];
        val3 = val3 + val4;
        dst_ptr[0] = val3;
        
        /* Force dependency chain */
        loop_counter++;
    }
    
    /* Second loop with struct-like access pattern */
    struct pair {
        volatile int a;
        volatile int b;
    };
    
    struct pair pairs[50];
    for (int i = 0; i < 50; i++) {
        /* Access struct fields - creates multiple (mem (reg)) patterns */
        pairs[i].a = src[i];
        __asm__ volatile ("" : : : "memory");
        pairs[i].b = dst[i];
        
        /* Mix array and struct access */
        int temp = pairs[0].a;  /* Constant index 0 */
        temp += pairs[0].b;
        pairs[0].a = temp;
    }
    
    /* Prevent dead code elimination */
    consume_array(dst, 100);
    consume_array(src, 100);
    
    /* Calculate checksum */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= dst[i];
        checksum += src[i];
    }
    
    return checksum & 0xFF; /* Return non-zero to prevent optimization */
}
