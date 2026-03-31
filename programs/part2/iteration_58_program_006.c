/* test_resource_coverage.c
 * Designed to trigger mark_set_resources paths for ZERO_EXTRACT/STRICT_LOW_PART
 * with memory destinations in GCC's RTL passes.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global struct with bitfields to ensure memory storage */
struct BitfieldStruct {
    unsigned int field1 : 3;
    unsigned int field2 : 5;
    unsigned int field3 : 8;
    unsigned int padding : 16;
};

/* Volatile to prevent register optimization */
volatile struct BitfieldStruct global_bitfield = {0};

/* External function to prevent inlining and optimization */
extern void external_call(void);

/* Function that takes pointer to ensure memory destination */
void set_bitfield_via_pointer(struct BitfieldStruct *ptr, int value) {
    /* Multiple bitfield assignments to increase RTL presence */
    ptr->field1 = value & 0x7;
    ptr->field2 = (value >> 3) & 0x1F;
    
    /* Compiler barrier to prevent merging */
    asm volatile("" : : : "memory");
    
    ptr->field3 = (value >> 8) & 0xFF;
}

/* Function with volatile bitfield */
void set_volatile_bitfield(void) {
    volatile struct {
        unsigned int status : 1;
        unsigned int mode : 2;
        unsigned int data : 4;
    } device_reg;
    
    /* Multiple assignments to same volatile bitfield */
    device_reg.status = 1;
    
    /* External call prevents dead code elimination */
    external_call();
    
    device_reg.mode = 2;
    device_reg.data = 5;
}

/* Function using inline assembly with byte constraint (potential STRICT_LOW_PART) */
void byte_register_operations(void) {
    uint32_t value = 0x12345678;
    uint8_t low_byte;
    
    /* Inline asm with "=Q" constraint (byte-addressable register)
     * May generate STRICT_LOW_PART on some architectures */
    __asm__ volatile (
        "movb %1, %0\n\t"
        : "=Q" (low_byte)
        : "r" ((uint8_t)value)
        : /* no clobbers */
    );
    
    /* Store to memory to ensure MEM reference */
    static volatile uint8_t memory_byte;
    memory_byte = low_byte;
}

/* Complex function with mixed operations to trigger resource tracking */
void complex_bitfield_operations(int iterations) {
    struct BitfieldStruct local_struct;
    struct BitfieldStruct *ptr = &local_struct;
    
    /* Loop with conditional assignments to prevent optimization */
    for (int i = 0; i < iterations; i++) {
        /* Use argc/argv to make condition unpredictable */
        if (i & 1) {
            ptr->field1 = i & 0x7;
            ptr->field3 = (i * 3) & 0xFF;
        } else {
            ptr->field2 = i & 0x1F;
        }
        
        /* Inline asm that reads/writes memory and clobbers registers
         * Forces scheduler to analyze resource usage */
        uint32_t temp;
        __asm__ volatile (
            "movl %1, %%eax\n\t"
            "andl $0x7, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=m" (ptr->field1)
            : "r" (i)
            : "eax", "memory"
        );
        
        /* Compiler barrier */
        asm volatile("" : : : "memory");
    }
}

/* Function using __sync builtins (atomic operations on bitfields) */
void atomic_bitfield_operations(void) {
    static struct {
        unsigned int lock : 1;
        unsigned int counter : 7;
        unsigned int flags : 8;
    } atomic_data = {0};
    
    /* Atomic operations may generate ZERO_EXTRACT with MEM */
    __sync_fetch_and_or(&atomic_data.flags, 0x01);
    __sync_fetch_and_add(&atomic_data.counter, 1);
}

/* Function with STRICT_LOW_PART pattern via char assignment */
void strict_low_part_pattern(void) {
    /* Use register variable with asm constraint */
    register char byte_reg asm("al");
    
    /* Assignment to char that's part of larger register */
    __asm__ volatile (
        "movb $0x42, %0"
        : "=Q" (byte_reg)
        :
        : /* no clobbers */
    );
    
    /* Force memory store */
    static volatile char mem_byte;
    __asm__ volatile (
        "movb %1, %0"
        : "=m" (mem_byte)
        : "Q" (byte_reg)
        : "memory"
    );
}

/* Main function with unpredictable control flow */
int main(int argc, char *argv[]) {
    /* Initialize */
    struct BitfieldStruct stack_struct = {0};
    
    /* Unpredictable condition based on program input */
    if (argc > 1) {
        /* Set bitfields via pointer (memory destination) */
        set_bitfield_via_pointer(&stack_struct, atoi(argv[1]));
        set_bitfield_via_pointer(&global_bitfield, atoi(argv[1]) + 1);
    }
    
    /* Always execute these to ensure coverage */
    set_volatile_bitfield();
    byte_register_operations();
    
    /* Complex operations with loop */
    complex_bitfield_operations(argc > 1 ? atoi(argv[1]) % 10 : 5);
    
    /* Atomic operations */
    atomic_bitfield_operations();
    
    /* STRICT_LOW_PART pattern */
    strict_low_part_pattern();
    
    /* Use the results to prevent dead code elimination */
    printf("Results: %u %u %u\n", 
           stack_struct.field1,
           global_bitfield.field2,
           argc);
    
    return 0;
}

/* Dummy external function definition */
void external_call(void) {
    /* Empty but prevents optimization */
    static int counter = 0;
    counter++;
}
