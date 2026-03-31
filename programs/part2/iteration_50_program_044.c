/* auto_inc_dec_test.c
 * Designed to trigger auto-increment/decrement RTL pattern matching
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to preserve RTL patterns */
__attribute__((noinline)) 
void consume_result(volatile int* arr, int size) {
    volatile int dummy = 0;
    for (int i = 0; i < size; i++) {
        dummy += arr[i];
    }
    /* Use dummy to prevent optimization */
    __asm__ volatile("" : : "r"(dummy) : "memory");
}

/* Force register pressure and prevent optimization */
__attribute__((noinline))
void memory_barrier(void) {
    __asm__ volatile("" : : : "memory");
}

/* Create artificial register dependencies */
__attribute__((noinline))
void register_clobber(void) {
#if defined(__arm__)
    __asm__ volatile("" : : : "r0", "r1", "r2", "r3", "memory");
#elif defined(__x86_64__)
    __asm__ volatile("" : : : "rax", "rbx", "rcx", "rdx", "memory");
#elif defined(__powerpc__)
    __asm__ volatile("" : : : "r0", "r1", "r2", "r3", "memory");
#else
    __asm__ volatile("" : : : "memory");
#endif
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
    volatile int loop_counter = limit;  /* Force memory access */
    
    /* Main processing loop - designed to create (mem (plus (reg) (const_int 0))) patterns */
    for (int i = 0; i < loop_counter; i++) {
        /* Pattern 1: Array access with constant index 0 - should create (mem (reg)) */
        int val1 = src[0];  /* This should generate (mem (plus (reg) (const_int 0))) */
        
        /* Pattern 2: Another array access with constant index 0 */
        int val2 = src[0];  /* Duplicate to increase pattern frequency */
        
        /* Arithmetic operation to create register pressure */
        int sum = val1 + val2 + 7;  /* Loop-invariant constant addition */
        
        /* Pattern 3: Store with constant index 0 */
        dst[0] = sum;  /* Should generate (set (mem (plus (reg) (const_int 0))) (reg)) */
        
        /* Pattern 4: Pointer-based access with zero offset */
        volatile int* p = &src[10];
        int val3 = *p;  /* Should generate (mem (reg)) pattern */
        
        /* Pattern 5: Store to another location with zero offset */
        volatile int* q = &dst[20];
        *q = val3 + i;  /* Mix with loop variable */
        
        /* Insert barriers to prevent reordering/optimization */
        register_clobber();
        memory_barrier();
        
        /* Scalar operations to create more register pressure */
        volatile int scalar = src[5];  /* Another memory access */
        scalar = scalar * 2;
        dst[5] = scalar;  /* Store back */
        
        /* Additional memory access patterns */
        dst[1] = src[1] + 1;
        dst[2] = src[2] * 2;
        
        /* More register clobbering to force spills/reloads */
        __asm__ volatile("" : : : "memory");
    }
    
    /* Second loop with different access pattern */
    volatile int* src_ptr = &src[0];
    volatile int* dst_ptr = &dst[0];
    
    for (int i = 0; i < 50; i++) {
        /* Direct pointer dereference - should create (mem (reg)) */
        int val = *src_ptr;
        
        /* Modify and store */
        *dst_ptr = val + 3;
        
        /* Create artificial dependency chain */
        __asm__ volatile("" : "+r"(val) : : "memory");
        
        /* Access with different offset but same base */
        volatile int* p2 = src_ptr + 5;
        int val2 = *p2;
        *(dst_ptr + 5) = val2;
        
        /* Barrier to separate memory operations */
        memory_barrier();
    }
    
    /* Struct access pattern */
    struct {
        volatile int a;
        volatile int b;
        volatile int c;
    } data[10];
    
    for (int i = 0; i < 10; i++) {
        /* Multiple field accesses from same base */
        data[i].a = src[i];
        data[i].b = src[i] * 2;
        data[i].c = src[i] + data[i].a;
        
        register_clobber();
    }
    
    /* Final consumption to prevent dead code elimination */
    consume_result(dst, 100);
    
    /* Calculate checksum */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= dst[i];  /* Simple checksum */
    }
    
    printf("Checksum: %d\n", checksum);
    
    return checksum == 0 ? 0 : 1;
}
