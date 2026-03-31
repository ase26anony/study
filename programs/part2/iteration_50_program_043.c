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
    /* Use sink to prevent optimization */
    __asm__ volatile("" : : "r"(sink) : "memory");
}

int main(void) {
    /* Volatile arrays to prevent optimization of memory accesses */
    volatile int src[100];
    volatile int dst[100];
    
    /* Initialize source array with sequential values */
    for (int i = 0; i < 100; i++) {
        src[i] = i * 3 + 1;  /* Non-trivial pattern */
    }
    
    /* Non-constant loop bound to prevent unrolling */
    int limit = 90;
    volatile int loop_counter = 0;
    
    /* Main processing loop - designed to create (mem (plus (reg) (const_int 0))) patterns */
    for (int i = 0; i < limit; i++) {
        /* CRITICAL: Access with constant index 0 to create (mem (reg)) pattern */
        int val1 = src[0];  /* Should generate: (set (reg) (mem (reg))) */
        
        /* Arithmetic operation to create register pressure */
        int result = val1 + 42;  /* Loop-invariant constant */
        
        /* Store with constant index 0 */
        dst[0] = result;    /* Should generate: (set (mem (reg)) (reg)) */
        
        /* Memory barrier to prevent reordering/optimization */
        __asm__ volatile("" : : : "memory");
        
        /* Additional pointer-based access to create another (mem (reg)) pattern */
        volatile int *p = &src[10];
        int val2 = *p;      /* Should generate: (set (reg) (mem (reg))) */
        
        /* Use val2 to prevent dead code elimination */
        dst[1] = val2;
        
        /* Artificial register pressure with inline assembly */
        __asm__ volatile("" : : : "r0", "r1", "r2", "r3", "memory");
        
        loop_counter++;
    }
    
    /* Second loop with different access pattern */
    volatile int *src_ptr = &src[20];
    volatile int *dst_ptr = &dst[20];
    
    for (int i = 0; i < 50; i++) {
        /* Multiple memory references with zero offset */
        int a = src_ptr[0];  /* (mem (plus (reg) (const_int 0))) */
        int b = src_ptr[1];  /* Different offset to create base+offset pattern */
        
        /* Mix scalar and array operations */
        volatile int temp = a + b;
        dst_ptr[0] = temp;   /* Another (mem (reg)) pattern */
        
        /* Barrier between operations */
        __asm__ volatile("" : : : "memory");
        
        /* Create register increment pattern */
        int *reg_var = (int*)&src_ptr;
        __asm__ volatile("" : "+r"(*reg_var) : : "memory");
    }
    
    /* Struct access to create different memory patterns */
    struct pair {
        volatile int first;
        volatile int second;
    };
    
    struct pair data[10];
    for (int i = 0; i < 10; i++) {
        data[i].first = i;
        data[i].second = i * 2;
    }
    
    /* Access struct fields with constant offsets */
    volatile struct pair *dp = &data[0];
    for (int i = 0; i < 5; i++) {
        int f = dp->first;   /* Should be (mem (reg)) with field offset */
        int s = dp->second;  /* Another memory access with offset */
        dst[10 + i] = f + s;
        
        /* Barrier to preserve sequence */
        __asm__ volatile("" : : : "r4", "r5", "memory");
    }
    
    /* Final consumption to prevent dead code elimination */
    consume_result(dst, 100);
    
    /* Return checksum */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= dst[i];  /* Simple checksum */
    }
    
    return checksum;
}
