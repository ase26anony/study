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

/* Function to force memory access through pointer */
void set_bitfield_via_pointer(struct BitfieldStruct *ptr, int value) {
    /* This should generate ZERO_EXTRACT for bitfield assignment to memory */
    ptr->field1 = value & 0x7;
    ptr->field2 = (value >> 3) & 0x1F;
    
    /* Compiler barrier to prevent optimization */
    asm volatile("" : : : "memory");
}

/* Function with volatile bitfield */
void set_volatile_bitfield(void) {
    volatile struct {
        unsigned int status : 4;
        unsigned int control : 4;
    } device_reg;
    
    /* Volatile ensures memory access, not register */
    device_reg.status = 0xA;
    device_reg.control = 0x5;
    
    /* Read back to prevent dead store elimination */
    asm volatile("" : : "r"(device_reg.status), "r"(device_reg.control) : "memory");
}

/* Function using STRICT_LOW_PART via inline assembly */
void strict_low_part_example(void) {
    uint32_t value = 0;
    uint8_t *byte_ptr = (uint8_t *)&value;
    
    /* Inline assembly that might generate STRICT_LOW_PART */
    /* Writing to low byte of a register/memory location */
    asm volatile(
        "movb %1, %0\n\t"
        : "=Q" (*byte_ptr)  /* Q constraint = a,b,c,d registers (byte-addressable) */
        : "r" ((uint8_t)0x42)
        : "memory"
    );
    
    /* Force memory reference */
    volatile uint32_t *mem_loc = (volatile uint32_t *)malloc(sizeof(uint32_t));
    if (mem_loc) {
        asm volatile(
            "movl %1, %%eax\n\t"
            "andl $0xFF, %%eax\n\t"
            "movb %%al, %0\n\t"
            : "=m" (*(volatile uint8_t *)mem_loc)
            : "r" (0x12345678)
            : "eax", "memory"
        );
        free((void *)mem_loc);
    }
}

/* Complex function with multiple bitfield operations in loop */
void bitfield_loop_operations(int iterations) {
    struct BitfieldStruct local_struct;
    
    for (int i = 0; i < iterations; i++) {
        /* Multiple bitfield assignments - may generate ZERO_EXTRACT */
        local_struct.field1 = (i * 3) & 0x7;
        local_struct.field2 = (i * 5) & 0x1F;
        local_struct.field3 = (i * 7) & 0xFF;
        
        /* Mix with arithmetic to prevent optimization */
        local_struct.padding = i ^ 0xFFFF;
        
        /* Compiler barrier every few iterations */
        if (i % 4 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    /* Use the result to prevent elimination */
    asm volatile("" : : "r"(local_struct.field1), "r"(local_struct.field2) : "memory");
}

/* Function using atomic operations on bitfields */
void atomic_bitfield_ops(void) {
    static struct {
        unsigned int lock : 1;
        unsigned int counter : 15;
        unsigned int flags : 16;
    } shared_data = {0};
    
    /* Atomic operations may generate complex RTL with ZERO_EXTRACT */
    __sync_fetch_and_or(&shared_data.flags, 0x8001);
    __sync_fetch_and_add((int*)&shared_data.counter, 1);
}

/* Function with unpredictable control flow to preserve patterns */
void conditional_bitfield_assign(int condition) {
    struct BitfieldStruct data;
    
    /* External condition prevents optimization */
    if (condition & 1) {
        data.field1 = 1;
        data.field2 = 2;
    } else {
        data.field1 = 3;
        data.field2 = 4;
    }
    
    /* Always execute this part */
    data.field3 = condition & 0xFF;
    
    /* Use result */
    asm volatile("" : : "r"(data.field1), "r"(data.field2), "r"(data.field3) : "memory");
}

/* Main function that exercises all patterns */
int main(int argc, char **argv) {
    struct BitfieldStruct stack_struct;
    
    /* Use argc to create unpredictable values */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    /* 1. Set global bitfield (memory) */
    global_bitfield.field1 = seed & 0x7;
    global_bitfield.field2 = (seed >> 3) & 0x1F;
    
    /* 2. Set via pointer (ensures memory destination) */
    set_bitfield_via_pointer(&stack_struct, seed);
    
    /* 3. Volatile bitfield */
    set_volatile_bitfield();
    
    /* 4. STRICT_LOW_PART patterns */
    strict_low_part_example();
    
    /* 5. Loop with bitfields */
    bitfield_loop_operations(seed % 100);
    
    /* 6. Atomic operations */
    atomic_bitfield_ops();
    
    /* 7. Conditional assignments */
    conditional_bitfield_assign(seed);
    
    /* Mix with inline assembly that reads/writes memory */
    asm volatile(
        "movl %0, %%eax\n\t"
        "andl $0x7, %%eax\n\t"
        "movl %%eax, %1\n\t"
        : 
        : "m" (stack_struct.field1), "m" (global_bitfield.field1)
        : "eax", "memory"
    );
    
    return 0;
}
