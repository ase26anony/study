/* test_resource_coverage.c
 * Designed to trigger uncovered lines 282-290 in resource.cc
 * Compile with: gcc -O1 -c test_resource_coverage.c -o test.o
 * Or for scheduling: gcc -O2 -fschedule-insns -c test_resource_coverage.c -o test.o
 */

#include <stdint.h>
#include <stdio.h>

/* Prevent optimization from removing critical patterns */
static volatile int external_counter = 0;

/* ==================== ZERO_EXTRACT PATTERNS ==================== */

/* Global struct with bitfield - ensures memory storage */
struct BitfieldStruct {
    unsigned int field1 : 3;
    unsigned int field2 : 5;
    unsigned int field3 : 8;
    unsigned int field4 : 16;
};

/* Volatile forces memory access */
volatile struct BitfieldStruct global_bitfield;

/* Function that takes pointer to ensure memory destination */
void set_bitfield_via_pointer(struct BitfieldStruct *ptr, int value) {
    /* Multiple bitfield assignments in sequence */
    ptr->field1 = value & 0x7;
    /* Compiler barrier to prevent merging */
    asm volatile("" : : : "memory");
    ptr->field2 = (value >> 3) & 0x1F;
    asm volatile("" : : : "memory");
    ptr->field3 = (value >> 8) & 0xFF;
    asm volatile("" : : : "memory");
    ptr->field4 = (value >> 16) & 0xFFFF;
}

/* Complex function with control flow to ensure scheduling analysis */
void complex_bitfield_operations(int iterations) {
    struct BitfieldStruct local_struct;
    struct BitfieldStruct *ptr = &local_struct;
    
    /* Unpredictable control flow prevents dead code elimination */
    if (external_counter > 0) {
        ptr = &global_bitfield;
    }
    
    for (int i = 0; i < iterations; i++) {
        /* Conditional assignment based on loop counter */
        if (i & 1) {
            ptr->field1 = i & 0x7;
        } else {
            ptr->field2 = i & 0x1F;
        }
        
        /* Mix with inline assembly that reads/writes memory */
        int temp;
        asm volatile(
            "movl %1, %%eax\n\t"
            "andl $0x7, %%eax\n\t"
            "movl %%eax, %0"
            : "=m" (ptr->field3)
            : "r" (i)
            : "eax", "memory"
        );
        
        /* Atomic operation on bitfield - may generate ZERO_EXTRACT */
        if (i % 3 == 0) {
            /* Simulate atomic-like operation */
            unsigned int old_val;
            asm volatile(
                "movl %1, %%eax\n\t"
                "orl $0x100, %%eax\n\t"
                "movl %%eax, %0"
                : "=m" (ptr->field4)
                : "m" (ptr->field4)
                : "eax", "memory"
            );
        }
    }
}

/* ==================== STRICT_LOW_PART PATTERNS ==================== */

/* Function to generate STRICT_LOW_PART patterns via inline assembly */
void strict_low_part_operations(void) {
    /* Use byte-addressable register constraints */
    unsigned char byte_var;
    unsigned short word_var;
    
    /* Multiple asm statements with overlapping resources */
    asm volatile(
        "movb $0x42, %0"
        : "=Q" (byte_var)
        :
        : "memory"
    );
    
    /* Force memory destination for the byte */
    volatile unsigned char *mem_byte = &byte_var;
    asm volatile(
        "movb %%al, %0"
        : "=m" (*mem_byte)
        :
        : "al", "memory"
    );
    
    /* Mix with word operations to create register pressure */
    asm volatile(
        "movw $0x1234, %0\n\t"
        "movb $0x56, %1"
        : "=r" (word_var), "=Q" (byte_var)
        :
        : "memory"
    );
}

/* Function with mixed operations to trigger resource tracking */
void mixed_operations(int argc, char **argv) {
    /* Use argc for unpredictable control flow */
    int use_bitfield = argc > 1;
    
    if (use_bitfield) {
        struct BitfieldStruct stack_struct;
        
        /* Pointer assignment ensures memory destination */
        struct BitfieldStruct *ptr = &stack_struct;
        
        /* Loop with multiple SET patterns */
        for (int i = 0; i < 10; i++) {
            /* Different assignments create different RTL patterns */
            switch (i % 4) {
                case 0:
                    ptr->field1 = i & 0x7;
                    break;
                case 1:
                    ptr->field2 = (i * 2) & 0x1F;
                    break;
                case 2:
                    ptr->field3 = (i * 3) & 0xFF;
                    break;
                case 3:
                    ptr->field4 = (i * 100) & 0xFFFF;
                    break;
            }
            
            /* Memory barrier between operations */
            asm volatile("" : : : "memory");
        }
    } else {
        strict_low_part_operations();
    }
    
    /* Additional inline assembly with explicit clobbers */
    int dummy;
    asm volatile(
        "movl $1, %%eax\n\t"
        "movl $2, %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "movl %%eax, %0"
        : "=m" (dummy)
        :
        : "eax", "ebx", "memory"
    );
}

/* ==================== MAIN FUNCTION ==================== */

int main(int argc, char **argv) {
    /* Initialize external counter */
    external_counter = argc;
    
    /* Test ZERO_EXTRACT patterns */
    struct BitfieldStruct local_bitfield;
    set_bitfield_via_pointer(&local_bitfield, 0x12345678);
    
    /* Test with global (definitely in memory) */
    set_bitfield_via_pointer((struct BitfieldStruct*)&global_bitfield, 0x87654321);
    
    /* Complex operations with loop */
    complex_bitfield_operations(5);
    
    /* Test STRICT_LOW_PART patterns */
    strict_low_part_operations();
    
    /* Mixed operations based on input */
    mixed_operations(argc, argv);
    
    /* Additional test: nested bitfield in struct */
    struct NestedStruct {
        struct {
            unsigned int nested_field : 4;
        } inner;
        unsigned int regular_field;
    } nested;
    
    nested.inner.nested_field = 0xF;
    nested.regular_field = 0xDEADBEEF;
    
    /* Pointer chain to ensure memory reference */
    struct NestedStruct *nested_ptr = &nested;
    nested_ptr->inner.nested_field = argc & 0xF;
    
    /* Final volatile store to prevent elimination of everything */
    global_bitfield.field1 = 1;
    
    return 0;
}
