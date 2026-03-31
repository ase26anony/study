/* test_resource_coverage.c */
#include <stdint.h>
#include <stdlib.h>

/* Global struct with bitfield to ensure memory storage */
struct BitfieldStruct {
    unsigned int field1 : 3;
    unsigned int field2 : 5;
    unsigned int field3 : 8;
    unsigned int padding : 16;
};

volatile struct BitfieldStruct global_bitfield;

/* Volatile bitfield to force memory access */
volatile struct {
    unsigned int status_flag : 1;
    unsigned int error_code : 4;
} volatile_status;

/* Function to set bitfield via pointer - ensures memory destination */
void set_bitfield_via_pointer(struct BitfieldStruct *ptr, int value) {
    /* This should generate ZERO_EXTRACT with memory destination */
    ptr->field2 = value & 0x1F;
    
    /* Compiler barrier to prevent optimization */
    asm volatile("" : : : "memory");
}

/* Function with STRICT_LOW_PART pattern using inline assembly */
void set_low_part_register(void) {
    /* Using byte-register constraint to potentially generate STRICT_LOW_PART */
    unsigned char byte_val;
    
    /* Inline asm that writes to a byte register */
    __asm__ volatile (
        "movb $0x42, %0"
        : "=Q" (byte_val)  /* "Q" constraint for byte-addressable register */
        :
        : "memory"
    );
    
    /* Store to memory to ensure MEM reference */
    volatile unsigned char *mem_byte = (volatile unsigned char *)&global_bitfield;
    *mem_byte = byte_val;
}

/* Complex function with multiple bitfield operations in loop */
void complex_bitfield_operations(int iterations) {
    struct BitfieldStruct local_struct;
    struct BitfieldStruct *ptr = &local_struct;
    
    /* Use external input to prevent dead code elimination */
    if (iterations < 0) {
        ptr = &global_bitfield;
    }
    
    /* Loop with bitfield assignments - increases chance RTL persists */
    for (int i = 0; i < iterations && i < 10; i++) {
        /* Multiple bitfield assignments */
        ptr->field1 = (i * 3) & 0x7;
        ptr->field2 = (i * 5) & 0x1F;
        ptr->field3 = (i * 7) & 0xFF;
        
        /* Compiler barrier between operations */
        asm volatile("" : : : "memory");
    }
}

/* Function using atomic operations on bitfield */
void atomic_bitfield_ops(void) {
    /* __sync operations on bitfields may generate ZERO_EXTRACT with MEM */
    struct BitfieldStruct atomic_struct;
    
    /* Atomic OR on bitfield - likely generates complex RTL */
    __sync_fetch_and_or((unsigned int*)&atomic_struct, 0x07);
    
    /* Atomic AND on specific bits */
    __sync_fetch_and_and((unsigned int*)&atomic_struct, ~0x38);
}

/* Function mixing inline asm with bitfield access */
void mixed_asm_bitfield(int value) {
    struct BitfieldStruct s;
    int temp;
    
    /* Inline asm that reads/writes memory and clobbers registers */
    __asm__ volatile (
        "movl %2, %%eax\n\t"
        "andl $0x7, %%eax\n\t"
        "movl %%eax, %0\n\t"
        "movl %1, %%ebx\n\t"
        "orl  %%eax, %%ebx"
        : "=m" (s.field1)    /* Output to memory bitfield */
        : "m" (s.field3), "r" (value)  /* Input from memory and register */
        : "eax", "ebx", "memory"
    );
}

/* Main function with unpredictable control flow */
int main(int argc, char **argv) {
    /* Initialize */
    global_bitfield.field1 = 0;
    volatile_status.status_flag = 0;
    
    /* Unpredictable condition based on external input */
    int use_volatile = argc > 1;
    
    /* Test ZERO_EXTRACT path */
    if (use_volatile) {
        volatile_status.error_code = argc & 0x0F;
    } else {
        set_bitfield_via_pointer(&global_bitfield, argc);
    }
    
    /* Test STRICT_LOW_PART path */
    set_low_part_register();
    
    /* Complex operations with loop */
    complex_bitfield_operations(argc);
    
    /* Atomic operations */
    atomic_bitfield_ops();
    
    /* Mixed asm operations */
    mixed_asm_bitfield(argc);
    
    /* Additional test: nested bitfield in struct */
    struct {
        struct {
            unsigned int nested_field : 4;
        } inner;
        unsigned int regular_field;
    } nested_struct;
    
    /* Take address to force memory storage */
    struct {
        unsigned int nested_field : 4;
    } *nested_ptr = &nested_struct.inner;
    
    nested_ptr->nested_field = argc & 0x0F;
    
    /* Array of structs with bitfields */
    struct BitfieldStruct array[4];
    for (int i = 0; i < 4 && i < argc; i++) {
        array[i].field1 = (i + argc) & 0x7;
        array[i].field2 = (i * argc) & 0x1F;
    }
    
    /* Use the results to prevent dead code elimination */
    return global_bitfield.field1 + volatile_status.status_flag 
           + nested_struct.inner.nested_field + array[0].field2;
}
