/* test_resource_coverage.c */

#include <stdint.h>
#include <stdio.h>

/* Global struct with bitfield to ensure memory storage */
struct BitfieldStruct {
    unsigned int field1 : 3;
    unsigned int field2 : 5;
    unsigned int field3 : 8;
    unsigned int padding : 16;
};

volatile struct BitfieldStruct global_bitfield;

/* Function to set bitfield via pointer - ensures memory destination */
void set_bitfield_via_pointer(struct BitfieldStruct *ptr, unsigned int val) {
    /* Multiple bitfield assignments to increase RTL complexity */
    ptr->field1 = val & 0x7;
    ptr->field2 = (val >> 3) & 0x1F;
    ptr->field3 = (val >> 8) & 0xFF;
    
    /* Compiler barrier to prevent optimization */
    asm volatile("" : : : "memory");
}

/* Function with STRICT_LOW_PART pattern using inline assembly */
void strict_low_part_example(void) {
    volatile uint8_t byte_var;
    uint32_t dword_var;
    
    /* Inline assembly that might generate STRICT_LOW_PART */
    /* Writing to byte-addressable register part */
    asm volatile(
        "movb $0x42, %0\n\t"
        : "=Q" (byte_var)
        :
        : "memory"
    );
    
    /* Another pattern: char variable that might be in register */
    register char reg_byte asm("al");
    asm volatile(
        "movb $0x55, %0"
        : "=r" (reg_byte)
        :
    );
    
    /* Mix with memory operations to create scheduling complexity */
    dword_var = byte_var * 2;
    asm volatile("" : : "r" (dword_var) : "memory");
}

/* Complex function with multiple resource conflicts */
void complex_resource_pattern(int argc, char **argv) {
    struct BitfieldStruct local_struct;
    volatile int condition = argc > 1;
    
    /* Initialize */
    local_struct.field1 = 0;
    local_struct.field2 = 0;
    local_struct.field3 = 0;
    
    /* Conditional bitfield operations - prevents dead code elimination */
    if (condition) {
        /* Bitfield assignment that should generate ZERO_EXTRACT */
        local_struct.field1 = 1;
        local_struct.field2 = 0x1F; /* Max value for 5 bits */
        
        /* Inline assembly that reads/writes memory */
        asm volatile(
            "lock orl $0x1, %0"
            : "+m" (local_struct)
            :
            : "cc", "memory"
        );
    } else {
        /* Different pattern */
        local_struct.field3 = 0xFF;
        
        /* Atomic operation on bitfield */
        __sync_fetch_and_or(&local_struct.field1, 0x4);
    }
    
    /* Loop with bitfield operations - increases scheduling complexity */
    for (int i = 0; i < 10; i++) {
        /* Volatile read to prevent loop elimination */
        volatile int counter = i;
        
        /* Bitfield assignment in loop */
        local_struct.field2 = counter & 0x1F;
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
    }
    
    /* Pass to global to ensure side effects */
    global_bitfield = local_struct;
}

/* Function that mixes bitfields and assembly for scheduling */
void mixed_operations(void) {
    struct {
        unsigned int flags : 4;
        unsigned int status : 4;
        unsigned int data : 24;
    } packet;
    
    unsigned int raw_data;
    
    /* Initialize */
    packet.flags = 0;
    packet.status = 0;
    packet.data = 0;
    
    /* Inline assembly that produces complex RTL */
    asm volatile(
        "movl $0x12345678, %%eax\n\t"
        "andl $0xF, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (packet.flags)
        :
        : "eax", "memory"
    );
    
    /* Another pattern with multiple constraints */
    asm volatile(
        "movl %1, %%eax\n\t"
        "shrl $4, %%eax\n\t"
        "andl $0xF, %%eax\n\t"
        "movl %%eax, %0"
        : "=m" (packet.status)
        : "m" (raw_data)
        : "eax", "memory"
    );
    
    /* Use __sync builtin for atomic bitfield operation */
    __sync_fetch_and_or(&packet.data, 0x800000);
}

/* Main function that exercises all patterns */
int main(int argc, char **argv) {
    struct BitfieldStruct stack_struct;
    
    printf("Testing resource coverage patterns...\n");
    
    /* Exercise pointer-based bitfield assignment */
    set_bitfield_via_pointer(&stack_struct, 0x1234);
    set_bitfield_via_pointer(&global_bitfield, 0x5678);
    
    /* Exercise STRICT_LOW_PART patterns */
    strict_low_part_example();
    
    /* Exercise complex patterns with control flow */
    complex_resource_pattern(argc, argv);
    
    /* Exercise mixed operations */
    mixed_operations();
    
    /* Use the results to prevent dead code elimination */
    if (stack_struct.field1 != 0) {
        printf("Result: field1 = %u\n", stack_struct.field1);
    }
    
    if (global_bitfield.field2 != 0) {
        printf("Global field2 = %u\n", global_bitfield.field2);
    }
    
    return 0;
}
