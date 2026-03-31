/* test_resource_coverage.c */
#include <stdint.h>
#include <stdlib.h>

/* Global struct with bitfield to ensure memory storage */
struct BitfieldStruct {
    unsigned int data;
    unsigned int field1 : 3;
    unsigned int field2 : 5;
    unsigned int field3 : 8;
    volatile unsigned int volatile_field : 4;
};

struct BitfieldStruct global_bitfield = {0};

/* Function to force bitfield assignment through pointer (memory reference) */
void set_bitfield_via_pointer(struct BitfieldStruct *ptr, int value) {
    /* This should generate ZERO_EXTRACT with MEM destination */
    ptr->field1 = value & 0x7;
    
    /* Compiler barrier to prevent optimization */
    asm volatile("" : : : "memory");
    
    /* Another bitfield assignment */
    ptr->field2 = (value >> 3) & 0x1F;
}

/* Function with volatile bitfield assignment */
void set_volatile_bitfield(int value) {
    /* Volatile forces memory access */
    global_bitfield.volatile_field = value & 0xF;
    
    /* Multiple assignments to increase RTL visibility */
    if (value & 1) {
        global_bitfield.field3 = (value >> 4) & 0xFF;
    }
}

/* Function using inline assembly with partial register constraints */
void partial_register_operations(void) {
    /* Try to generate STRICT_LOW_PART for byte register */
    register uint8_t byte_reg asm("al");
    uint8_t memory_byte;
    
    /* Inline asm that might generate STRICT_LOW_PART with memory reference */
    asm volatile(
        "movb %1, %%al\n\t"
        "incb %%al\n\t"
        "movb %%al, %0"
        : "=m" (memory_byte)    /* Memory output */
        : "m" (global_bitfield.data)  /* Memory input */
        : "al"
    );
    
    /* Another asm with Q constraint (byte-addressable register) */
    asm volatile(
        "movb $0x42, %0"
        : "=Q" (byte_reg)
        :
        : "memory"
    );
}

/* Function with atomic operations on bitfields */
void atomic_bitfield_operations(struct BitfieldStruct *ptr) {
    /* Atomic operations may generate complex RTL with ZERO_EXTRACT */
    __sync_fetch_and_or(&ptr->data, 0x01);
    
    /* Simulate atomic bitfield operation using inline asm */
    unsigned int mask = 0x07 << 8;  /* Mask for field2 */
    asm volatile(
        "lock orl %1, %0"
        : "+m" (ptr->data)
        : "r" (mask)
        : "memory"
    );
}

/* Complex control flow to prevent optimization */
void complex_bitfield_operations(int iterations, int seed) {
    struct BitfieldStruct local_struct = {0};
    struct BitfieldStruct *ptr = &local_struct;
    
    /* Use external input to make control flow unpredictable */
    if (seed & 1) {
        ptr = &global_bitfield;
    }
    
    /* Loop with multiple bitfield assignments */
    for (int i = 0; i < iterations; i++) {
        /* These assignments should generate ZERO_EXTRACT with MEM */
        ptr->field1 = (i + seed) & 0x7;
        ptr->field2 = ((i * seed) >> 2) & 0x1F;
        
        /* Compiler barrier every few iterations */
        if (i % 3 == 0) {
            asm volatile("" : : : "memory");
        }
        
        /* Conditional assignment */
        if (ptr->field1 > 3) {
            ptr->field3 = ptr->field2 << 2;
        }
    }
}

/* Main function that exercises all patterns */
int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Exercise pointer-based bitfield assignment */
    set_bitfield_via_pointer(&global_bitfield, seed);
    
    /* Exercise volatile bitfield */
    set_volatile_bitfield(seed * 2);
    
    /* Exercise partial register operations */
    partial_register_operations();
    
    /* Exercise atomic operations */
    atomic_bitfield_operations(&global_bitfield);
    
    /* Exercise complex control flow with bitfields */
    complex_bitfield_operations(10, seed);
    
    /* Additional memory-referencing bitfield operations */
    struct BitfieldStruct stack_struct = {0};
    
    /* Take address and assign to bitfields */
    struct BitfieldStruct *alias = &stack_struct;
    alias->field1 = 1;
    alias->field2 = 2;
    alias->field3 = 3;
    
    /* Mix with inline asm that references the memory */
    unsigned int temp;
    asm volatile(
        "movl %1, %%eax\n\t"
        "andl $0x00000007, %%eax\n\t"  /* Extract field1 */
        "movl %%eax, %0\n\t"
        "orl $0x00000038, %1"          /* Modify field2 */
        : "=r" (temp), "+m" (stack_struct.data)
        :
        : "eax", "memory"
    );
    
    /* Return something based on the results */
    return (global_bitfield.field1 + stack_struct.field2) & 0xFF;
}
