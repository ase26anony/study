/* auto_inc_dec_trigger.c
 * Program designed to generate RTL patterns: (mem (plus (reg) (const_int 0)))
 * to trigger auto-increment/decrement optimization in GCC RTL passes.
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep RTL patterns intact */
__attribute__((noinline)) 
static void consume_data(volatile int* arr, size_t n) {
    volatile int sink = 0;
    for (size_t i = 0; i < n; i++) {
        sink += arr[i];
    }
    /* Use sink to prevent dead code elimination */
    __asm__ volatile("" : "+r" (sink) : : "memory");
}

/* Force register pressure and preserve memory operations */
__attribute__((noinline))
static void process_arrays(volatile int* restrict src, 
                          volatile int* restrict dst, 
                          int limit) {
    /* Multiple base registers for distinct (mem (reg)) patterns */
    volatile int* p1 = &src[0];
    volatile int* p2 = &dst[0];
    volatile int* p3 = &src[10];
    
    /* Scalar variables to create load/store patterns */
    int temp1, temp2, temp3;
    
    /* Loop with non-constant bound to prevent unrolling */
    for (int i = 0; i < limit; i++) {
        /* Pattern 1: (mem (plus (reg) (const_int 0))) via array[0] */
        temp1 = src[0];  /* Should generate: (set (reg) (mem (plus (reg) (const_int 0)))) */
        
        /* Pattern 2: Another (mem (reg)) via pointer dereference */
        temp2 = *p3;     /* Should generate: (set (reg) (mem (reg))) */
        
        /* Arithmetic operation to create register pressure */
        temp3 = temp1 + temp2 + i;
        
        /* Pattern 3: Store with (mem (plus (reg) (const_int 0))) */
        dst[0] = temp3;  /* Should generate: (set (mem (plus (reg) (const_int 0))) (reg)) */
        
        /* Inline assembly barriers to prevent optimization and create register dependencies */
        __asm__ volatile(
            "nop\n\t"
            : 
            : 
            : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
              "r8", "r9", "r10", "r11", "r12", "memory"
        );
        
        /* Additional memory operations with constant index 0 */
        /* These create more (mem (plus (reg) (const_int 0))) patterns */
        src[5] = src[5] + 1;  /* Read-modify-write with constant index */
        
        __asm__ volatile("" : : : "memory");
        
        /* Pointer arithmetic that might be converted to auto-increment */
        p3 = &src[10 + (i % 2)];  /* Simple variation to prevent optimization */
    }
    
    /* Final memory barrier */
    __asm__ volatile("" : : : "memory");
}

/* Secondary function with different access patterns */
__attribute__((noinline))
static void alternate_access(volatile int* arr1, volatile int* arr2, int n) {
    volatile int* ptr1 = arr1;
    volatile int* ptr2 = arr2;
    
    for (int i = 0; i < n; i++) {
        /* Multiple (mem (reg)) patterns in sequence */
        int val1 = ptr1[0];  /* Constant index 0 */
        int val2 = ptr2[0];  /* Constant index 0 */
        
        /* Simple computation */
        int result = val1 * 2 + val2;
        
        /* Store back with constant index 0 */
        ptr1[0] = result;
        
        /* Inline assembly with register clobbering */
        __asm__ volatile(
            ""
            : 
            : 
            : "r0", "r1", "r2", "r3", "memory"
        );
        
        /* Small pointer movement */
        if (i % 3 == 0) {
            ptr1 = &arr1[(i + 1) % 10];
        }
    }
}

int main(void) {
    /* Volatile arrays to prevent optimization of memory accesses */
    volatile int src[100];
    volatile int dst[100];
    
    /* Initialize source array with sequential values */
    for (int i = 0; i < 100; i++) {
        src[i] = i * 3 + 1;
        dst[i] = 0;
    }
    
    /* Non-constant loop bound to prevent unrolling */
    int limit = 90;
    
    /* Process arrays - creates many (mem (plus (reg) (const_int 0))) patterns */
    process_arrays(src, dst, limit);
    
    /* Alternate processing with different patterns */
    alternate_access(&src[20], &dst[20], 50);
    
    /* Additional pointer-based accesses */
    volatile int* ptr = &src[30];
    for (int i = 0; i < 20; i++) {
        /* Pure (mem (reg)) pattern */
        int val = *ptr;
        
        /* Store with different base register */
        dst[30 + i] = val + ptr[0];  /* Mixed: (mem (reg)) and (mem (plus (reg) (const_int 0))) */
        
        /* Assembly barrier */
        __asm__ volatile("" : : : "r0", "r1", "memory");
        
        /* Simple pointer increment that might become auto-increment */
        ptr = &src[30 + ((i + 1) % 5)];
    }
    
    /* Prevent dead code elimination */
    consume_data(dst, 100);
    consume_data(src, 100);
    
    /* Calculate checksum */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= dst[i];
        checksum += src[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
