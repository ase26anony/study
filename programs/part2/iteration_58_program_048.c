/* test_resource_coverage.c
 * Designed to trigger mark_set_resources path for ZERO_EXTRACT/STRICT_LOW_PART
 * with memory destinations in GCC's resource.cc
 */

#include <stdint.h>
#include <stdio.h>

/* Prevent optimization from removing critical patterns */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* Global struct with bitfield to ensure memory storage */
struct BitfieldStruct {
    unsigned int regular_field;
    volatile unsigned int volatile_field: 4;  /* Volatile forces memory access */
    unsigned int bitfield1: 3;
    unsigned int bitfield2: 5;
    unsigned int bitfield3: 8;
};

struct BitfieldStruct global_bf = {0};

/* Function to ensure bitfield is accessed via pointer (memory location) */
void set_bitfield_via_pointer(struct BitfieldStruct *ptr, int value) {
    /* Multiple bitfield assignments to increase pattern visibility */
    ptr->bitfield1 = value & 0x7;
    COMPILER_BARRIER();
    ptr->bitfield2 = (value >> 3) & 0x1F;
    COMPILER_BARRIER();
    ptr->bitfield3 = (value >> 8) & 0xFF;
}

/* Function with inline assembly that may generate STRICT_LOW_PART */
void partial_register_operations(void) {
    /* Use byte-addressable register constraints */
    register uint8_t byte1 asm("al");
    register uint8_t byte2 asm("bl");
    uint8_t memory_byte;
    
    /* Inline asm with "=Q" constraint (byte-addressable register) */
    __asm__ volatile (
        "movb $0x42, %0\n\t"
        "movb %0, %1"
        : "=Q" (byte1), "=m" (memory_byte)
        :
        : "memory"
    );
    
    /* Another asm that reads/writes partial registers */
    __asm__ volatile (
        "movb %1, %%al\n\t"
        "andb $0x0F, %%al\n\t"
        "movb %%al, %0"
        : "=Q" (byte2)
        : "m" (memory_byte)
        : "al"
    );
}

/* Complex control flow to keep scheduler interested */
void complex_bitfield_operations(int iterations, int seed) {
    struct BitfieldStruct local_bf;
    struct BitfieldStruct *ptr = &local_bf;
    
    /* Use external input to prevent dead code elimination */
    if (seed & 1) {
        ptr = &global_bf;
    }
    
    /* Loop with bitfield assignments - may generate ZERO_EXTRACT patterns */
    for (int i = 0; i < iterations; i++) {
        /* Conditional assignment based on loop variable */
        if (i & 1) {
            ptr->bitfield1 = (i + seed) & 0x7;
        } else {
            ptr->bitfield2 = (i * seed) & 0x1F;
        }
        
        /* Volatile bitfield assignment - always memory access */
        ptr->volatile_field = i & 0xF;
        
        /* Compiler barrier to prevent merging of operations */
        COMPILER_BARRIER();
        
        /* Inline asm that references the bitfield memory */
        __asm__ volatile (
            "lock orl $0, %0\n\t"  /* Memory barrier style operation */
            :
            : "m" (ptr->regular_field)
            : "memory"
        );
    }
}

/* Use __sync builtins for atomic bitfield operations */
void atomic_bitfield_ops(void) {
    /* Atomic operations on bitfields may generate interesting RTL */
    unsigned int old = __sync_fetch_and_or(&global_bf.regular_field, 0x100);
    
    /* The fetch_and_and on a bitfield position */
    struct {
        unsigned int flags: 16;
        unsigned int status: 8;
    } atomic_bf = {0};
    
    /* This may generate ZERO_EXTRACT with memory */
    unsigned int flags_before = __sync_fetch_and_and(
        (unsigned int*)&atomic_bf.flags, 
        0xFFF0
    );
}

/* Main function with unpredictable control flow */
int main(int argc, char *argv[]) {
    /* Use argc to make control flow unpredictable */
    int iterations = (argc > 1) ? 10 : 5;
    int seed = argc;
    
    printf("Starting resource pattern generation...\n");
    
    /* Exercise different patterns */
    set_bitfield_via_pointer(&global_bf, 0x55);
    
    partial_register_operations();
    
    complex_bitfield_operations(iterations, seed);
    
    atomic_bitfield_ops();
    
    /* Additional memory-referencing bitfield operations */
    {
        /* Struct with nested bitfields */
        struct Nested {
            struct {
                unsigned int low: 4;
                unsigned int high: 4;
            } nested_bf;
            unsigned int padding;
        } nested_struct;
        
        /* Take address and assign to nested bitfield */
        struct Nested *nested_ptr = &nested_struct;
        nested_ptr->nested_bf.low = 0xA;
        nested_ptr->nested_bf.high = 0x5;
        
        /* Force memory store with inline asm */
        __asm__ volatile (
            "movl %0, %%eax\n\t"
            "andl $0xFF, %%eax\n\t"
            "movl %%eax, %1"
            :
            : "r" (0x12345678), "m" (nested_ptr->padding)
            : "eax", "memory"
        );
    }
    
    printf("Pattern generation complete.\n");
    
    /* Return value based on global state to prevent optimization */
    return global_bf.bitfield1 + global_bf.bitfield2;
}
