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

/* Function to force bitfield assignment through pointer */
void set_bitfield_via_pointer(struct BitfieldStruct *s, int value) {
    /* This should generate ZERO_EXTRACT for bitfield in memory */
    s->field2 = value & 0x1F;
    
    /* Compiler barrier to prevent optimization */
    asm volatile("" : : : "memory");
}

/* Function with complex control flow to preserve patterns */
void conditional_bitfield_ops(int condition, struct BitfieldStruct *s) {
    volatile int *external = (volatile int*)&condition;
    
    /* Unpredictable condition to prevent dead code elimination */
    if (*external & 1) {
        s->field1 = 1;
    } else {
        s->field3 = *external & 0xFF;
    }
    
    /* Another barrier */
    asm volatile("" : : : "memory");
}

/* Function using inline assembly with partial register access */
void partial_register_ops(void) {
    /* Try to generate STRICT_LOW_PART for byte register access */
    register uint8_t byte_reg asm("al");
    
    /* Inline asm that writes to a byte register */
    asm volatile(
        "movb $0x42, %0\n\t"
        : "=Q" (byte_reg)
        :
        : "memory"
    );
    
    /* Store to memory to create memory reference */
    volatile uint8_t memory_byte = byte_reg;
}

/* Function with atomic operations on bitfields */
void atomic_bitfield_ops(struct BitfieldStruct *s) {
    /* Atomic OR on bitfield - may generate ZERO_EXTRACT with memory */
    __sync_fetch_and_or(&s->field1, 1);
    
    /* Atomic AND */
    __sync_fetch_and_and(&s->field2, 0x0F);
}

/* Function with mixed operations in loop */
void loop_with_bitfield_ops(struct BitfieldStruct *arr, int size) {
    for (int i = 0; i < size; i++) {
        /* Multiple bitfield assignments in loop */
        arr[i].field1 = i & 0x7;
        arr[i].field2 = (i >> 3) & 0x1F;
        arr[i].field3 = (i >> 8) & 0xFF;
        
        /* Compiler barrier every few iterations */
        if (i % 4 == 0) {
            asm volatile("" : : : "memory");
        }
    }
}

/* Main function with various patterns */
int main(int argc, char **argv) {
    struct BitfieldStruct local_bitfield;
    struct BitfieldStruct *dynamic_bitfield;
    struct BitfieldStruct array[10];
    
    /* Initialize */
    dynamic_bitfield = (struct BitfieldStruct*)malloc(sizeof(struct BitfieldStruct));
    if (!dynamic_bitfield) return 1;
    
    /* 1. Direct assignment to global volatile bitfield */
    global_bitfield.field1 = argc & 0x7;  /* Should be in memory due to volatile */
    
    /* 2. Assignment via pointer (common case for ZERO_EXTRACT in memory) */
    set_bitfield_via_pointer(&local_bitfield, argc);
    
    /* 3. Conditional operations */
    conditional_bitfield_ops(argc, dynamic_bitfield);
    
    /* 4. Partial register operations */
    partial_register_ops();
    
    /* 5. Atomic operations */
    atomic_bitfield_ops(&local_bitfield);
    
    /* 6. Loop operations */
    loop_with_bitfield_ops(array, 10);
    
    /* 7. Mixed inline assembly with memory constraints */
    int temp = argc;
    asm volatile(
        "movl %1, %%eax\n\t"
        "andl $0x7, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (local_bitfield.field1)  /* Direct memory constraint for bitfield */
        : "r" (temp)
        : "eax", "memory"
    );
    
    /* 8. Nested bitfield in struct */
    struct NestedStruct {
        struct {
            unsigned int nested_field : 4;
        } inner;
        int regular_field;
    } nested;
    
    nested.inner.nested_field = argc & 0xF;
    
    /* 9. Union with bitfield to create interesting patterns */
    union BitfieldUnion {
        uint32_t full;
        struct {
            uint32_t low_bits : 10;
            uint32_t high_bits : 22;
        } parts;
    } u;
    
    u.parts.low_bits = argc & 0x3FF;
    
    /* Use results to prevent dead code elimination */
    int sum = global_bitfield.field1 + local_bitfield.field2 + 
              dynamic_bitfield->field3 + nested.inner.nested_field +
              u.parts.low_bits;
    
    for (int i = 0; i < 10; i++) {
        sum += array[i].field1;
    }
    
    free(dynamic_bitfield);
    
    return sum > 0 ? 0 : 1;
}

/* Additional function to force generation of STRICT_LOW_PART */
void strict_low_part_pattern(void) {
    /* Using char variables that may be allocated to byte registers */
    register char c1 asm("al"), c2 asm("bl");
    volatile char result;
    
    /* Multiple asm statements that write partial registers */
    asm volatile(
        "movb $0x55, %0\n\t"
        "movb $0xAA, %1\n\t"
        : "=Q" (c1), "=Q" (c2)
        :
        : "memory"
    );
    
    /* Store to memory to create the MEM reference */
    result = c1 + c2;
    
    /* More complex pattern with memory destination */
    volatile struct {
        char low_byte;
        int full_word;
    } data;
    
    asm volatile(
        "movb %%al, %0\n\t"
        : "=m" (data.low_byte)
        :
        : "al", "memory"
    );
}
