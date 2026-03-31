/* test_resource_coverage.c - Program to trigger uncovered lines in resource.cc */

#include <stdint.h>
#include <stdlib.h>

/* Global struct with bitfield to ensure memory storage */
struct BitfieldStruct {
    unsigned int regular_field;
    unsigned int bitfield1 : 3;
    unsigned int bitfield2 : 5;
    unsigned int bitfield3 : 8;
};

volatile struct BitfieldStruct global_bitfield = {0};

/* Function to force bitfield assignment through pointer */
void set_bitfield_via_pointer(struct BitfieldStruct *ptr, int value) {
    /* This should generate ZERO_EXTRACT for bitfield assignment to memory */
    ptr->bitfield1 = value & 0x7;
    
    /* Compiler barrier to prevent optimization */
    asm volatile("" : : : "memory");
    
    /* Another bitfield assignment */
    ptr->bitfield2 = (value >> 3) & 0x1F;
}

/* Function with volatile bitfield */
void set_volatile_bitfield(void) {
    /* Volatile struct with bitfield - forces memory access */
    volatile struct {
        unsigned int status : 1;
        unsigned int mode : 2;
        unsigned int data : 4;
    } volatile_reg = {0};
    
    /* Multiple assignments to create scheduling opportunities */
    volatile_reg.status = 1;
    asm volatile("" : : : "memory");
    volatile_reg.mode = 2;
    asm volatile("" : : : "memory");
    volatile_reg.data = 5;
}

/* Function using inline assembly with byte register constraints */
void byte_register_operations(void) {
    /* Try to generate STRICT_LOW_PART for partial register assignment */
    uint32_t value = 0x12345678;
    uint8_t low_byte;
    
    /* Inline assembly that might generate STRICT_LOW_PART */
    asm volatile(
        "movb %1, %0\n\t"
        : "=Q" (low_byte)    /* Q constraint = a, b, c, or d register (byte-addressable) */
        : "r" ((uint8_t)value)
        : /* no clobbers */
    );
    
    /* Store to memory location to ensure memory reference */
    static volatile uint8_t memory_byte;
    memory_byte = low_byte;
}

/* Complex function with mixed operations for scheduling */
void complex_bitfield_operations(int iterations) {
    struct BitfieldStruct local_struct;
    int i;
    
    /* Loop with bitfield assignments - prevents optimization */
    for (i = 0; i < iterations; i++) {
        /* Take address to force memory storage */
        struct BitfieldStruct *ptr = &local_struct;
        
        /* Multiple bitfield assignments */
        ptr->bitfield1 = (i * 3) & 0x7;
        ptr->bitfield2 = (i * 5) & 0x1F;
        ptr->bitfield3 = (i * 7) & 0xFF;
        
        /* Mix with inline assembly for scheduling complexity */
        asm volatile(
            "nop\n\t"
            "nop\n\t"
            : : : "memory"
        );
    }
    
    /* Copy to global to prevent dead code elimination */
    global_bitfield = local_struct;
}

/* Function using atomic operations on bitfields */
void atomic_bitfield_operations(void) {
    /* Atomic operation on bitfield may generate complex RTL */
    struct {
        unsigned int flags : 4;
        unsigned int counter : 8;
    } atomic_data = {0};
    
    /* Use __sync builtins - they often generate ZERO_EXTRACT patterns */
    unsigned int old_flags = __sync_fetch_and_or(&atomic_data.flags, 0x3);
    
    /* Force memory barrier */
    asm volatile("" : : : "memory");
    
    /* Another atomic operation */
    __sync_fetch_and_add(&atomic_data.counter, 1);
}

/* Function that creates register pressure and scheduling opportunities */
void register_pressure_function(int arg) {
    /* Multiple local variables to create register pressure */
    int a = arg * 1;
    int b = arg * 2;
    int c = arg * 3;
    int d = arg * 4;
    int e = arg * 5;
    int f = arg * 6;
    int g = arg * 7;
    int h = arg * 8;
    
    /* Bitfield operation in the middle */
    struct BitfieldStruct stack_struct;
    stack_struct.bitfield1 = (a + b) & 0x7;
    
    /* Use all variables to prevent optimization */
    asm volatile("" 
        : "+r" (a), "+r" (b), "+r" (c), "+r" (d),
          "+r" (e), "+r" (f), "+r" (g), "+r" (h)
        : 
        : "memory"
    );
    
    /* More bitfield operations */
    stack_struct.bitfield2 = (c + d) & 0x1F;
    stack_struct.bitfield3 = (e + f) & 0xFF;
    
    /* Store to global */
    global_bitfield = stack_struct;
}

/* Main function with unpredictable control flow */
int main(int argc, char **argv) {
    struct BitfieldStruct local_bitfield;
    
    /* Unpredictable condition to prevent optimization */
    if (argc > 1) {
        /* Path 1: Pointer-based bitfield assignment */
        set_bitfield_via_pointer(&local_bitfield, atoi(argv[1]));
        
        /* Complex operations with scheduling opportunities */
        complex_bitfield_operations(argc);
        
        /* Atomic operations */
        atomic_bitfield_operations();
    } else {
        /* Path 2: Volatile bitfields */
        set_volatile_bitfield();
        
        /* Byte register operations */
        byte_register_operations();
    }
    
    /* Always execute register pressure function */
    register_pressure_function(argc);
    
    /* Use the results to prevent dead code elimination */
    return (int)local_bitfield.bitfield1 + 
           (int)global_bitfield.bitfield2 +
           argc;
}
