/* test_resource_coverage.c
 * Designed to trigger mark_set_resources path for ZERO_EXTRACT/STRICT_LOW_PART
 * with memory destinations in GCC's RTL.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization from removing critical patterns */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* Global volatile struct with bitfields to force memory access */
volatile struct BitfieldStruct {
    unsigned int field1 : 3;
    unsigned int field2 : 5;
    unsigned int field3 : 8;
    unsigned int field4 : 16;
} g_bitfield = {0};

/* Another global for pointer-based access */
struct BitfieldStruct2 {
    unsigned int low_bits : 4;
    unsigned int high_bits : 4;
    unsigned int padding : 24;
} g_bitfield2;

/* Function to ensure bitfield assignment through pointer (memory destination) */
void set_bitfield_via_pointer(struct BitfieldStruct2 *ptr, int value) {
    /* This should generate ZERO_EXTRACT with MEM destination */
    ptr->low_bits = value & 0xF;
    COMPILER_BARRIER();
    ptr->high_bits = (value >> 4) & 0xF;
}

/* Function with volatile bitfield assignment */
void set_volatile_bitfields(int a, int b) {
    /* Volatile ensures memory access, not register */
    g_bitfield.field1 = a & 0x7;
    COMPILER_BARRIER();
    g_bitfield.field2 = b & 0x1F;
    
    /* Conditional to prevent dead code elimination */
    if (a > b) {
        g_bitfield.field3 = (a + b) & 0xFF;
    } else {
        g_bitfield.field4 = (a - b) & 0xFFFF;
    }
}

/* Function using inline assembly for STRICT_LOW_PART-like behavior */
void partial_register_ops(void) {
    volatile uint8_t byte_var = 0;
    uint32_t dword_var = 0x12345678;
    
    /* Inline asm that might generate STRICT_LOW_PART for byte register */
    /* Using "=Q" constraint for byte-addressable register */
    __asm__ volatile (
        "movb $0x42, %0\n\t"
        : "=Q" (byte_var)
        :
        : "memory"
    );
    
    /* Mix with memory operations to create scheduling complexity */
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "andl $0xFF, %%eax\n\t"
        "movb %%al, %0\n\t"
        : "=m" (g_bitfield.field1)
        : "r" (dword_var)
        : "eax", "memory"
    );
}

/* Function with atomic operations on bitfields */
void atomic_bitfield_ops(void) {
    /* Atomic operations may generate ZERO_EXTRACT with MEM */
    struct {
        unsigned int flag : 1;
        unsigned int counter : 7;
        unsigned int data : 24;
    } atomic_struct = {0};
    
    /* Use __sync builtins which often preserve bitfield RTL patterns */
    __sync_fetch_and_or(&atomic_struct.flag, 1);
    COMPILER_BARRIER();
    __sync_fetch_and_add(&atomic_struct.counter, 1);
}

/* Complex function with loop and bitfield operations */
void loop_with_bitfields(int iterations) {
    struct {
        unsigned int a : 2;
        unsigned int b : 3;
        unsigned int c : 4;
        unsigned int d : 5;
    } local_bf;
    
    volatile int i;
    for (i = 0; i < iterations; i++) {
        /* Multiple bitfield assignments in loop - harder to optimize away */
        local_bf.a = (i >> 0) & 0x3;
        local_bf.b = (i >> 2) & 0x7;
        local_bf.c = (i >> 5) & 0xF;
        local_bf.d = (i >> 9) & 0x1F;
        
        /* Compiler barrier prevents loop optimization */
        COMPILER_BARRIER();
        
        /* Occasionally write to global to force memory traffic */
        if (i % 16 == 0) {
            g_bitfield.field1 = local_bf.a;
        }
    }
}

/* Function that creates register pressure and scheduling complexity */
void register_pressure_function(int x) {
    volatile int r1, r2, r3, r4, r5, r6, r7, r8;
    
    /* Multiple inline asm blocks that use/destroy registers */
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (r1)
        : "r" (x)
        : "eax"
    );
    
    __asm__ volatile (
        "movl %1, %%ebx\n\t"
        "andl $0x7, %%ebx\n\t"
        "movl %%ebx, %0\n\t"
        : "=r" (r2)
        : "r" (x + 1)
        : "ebx"
    );
    
    /* Bitfield operation interspersed */
    g_bitfield2.low_bits = r1 & 0xF;
    
    __asm__ volatile (
        "movl %1, %%ecx\n\t"
        "movl %%ecx, %0\n\t"
        : "=r" (r3)
        : "r" (x + 2)
        : "ecx"
    );
    
    g_bitfield2.high_bits = r2 & 0xF;
    
    /* More register operations */
    __asm__ volatile (
        "movl %1, %%edx\n\t"
        "movl %%edx, %0\n\t"
        : "=r" (r4)
        : "r" (x + 3)
        : "edx"
    );
}

/* Main function with unpredictable control flow */
int main(int argc, char *argv[]) {
    int iterations = 100;
    
    /* Use argc to make control flow unpredictable */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 10) iterations = 10;
        if (iterations > 1000) iterations = 1000;
    }
    
    /* Initialize global */
    g_bitfield2.low_bits = 0;
    g_bitfield2.high_bits = 0;
    
    /* Call functions that should generate target RTL patterns */
    set_bitfield_via_pointer(&g_bitfield2, 0xAB);
    
    set_volatile_bitfields(argc, iterations);
    
    partial_register_ops();
    
    atomic_bitfield_ops();
    
    loop_with_bitfields(iterations);
    
    register_pressure_function(argc);
    
    /* Read results to prevent dead code elimination */
    printf("Results: %u %u\n", 
           (unsigned int)g_bitfield.field1,
           (unsigned int)g_bitfield2.low_bits);
    
    return 0;
}
