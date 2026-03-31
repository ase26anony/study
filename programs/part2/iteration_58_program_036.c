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

/* Function to force memory reference through pointer */
void set_bitfield_via_pointer(struct BitfieldStruct *ptr, int val) {
    /* This should generate ZERO_EXTRACT with MEM destination */
    ptr->field1 = val & 0x7;
    ptr->field2 = (val >> 3) & 0x1F;
    
    /* Compiler barrier to prevent optimization */
    asm volatile("" : : : "memory");
}

/* Function with volatile bitfield */
void set_volatile_bitfield(void) {
    volatile struct {
        unsigned int status : 2;
        unsigned int mode : 3;
    } device_reg;
    
    /* Volatile ensures memory access */
    device_reg.status = 1;
    device_reg.mode = 3;
    
    /* Mix with external dependency to prevent dead code elimination */
    if (global_bitfield.field1) {
        device_reg.status = 2;
    }
}

/* Function using inline assembly for STRICT_LOW_PART */
void partial_register_ops(void) {
    uint32_t value = 0x12345678;
    uint8_t low_byte;
    
    /* This may generate STRICT_LOW_PART on some architectures */
    __asm__ volatile(
        "movb %1, %0\n\t"
        : "=Q" (low_byte)  /* Q constraint for byte-addressable register */
        : "r" ((uint8_t)value)
        : /* no clobbers */
    );
    
    /* Store to memory to ensure MEM reference */
    volatile uint8_t *mem_byte = (volatile uint8_t *)&global_bitfield;
    *mem_byte = low_byte;
}

/* Complex function with multiple bitfield operations in loop */
void bitfield_loop_operations(int iterations) {
    struct BitfieldStruct local_struct;
    struct BitfieldStruct *ptr = &local_struct;
    
    /* Loop to increase chance of RTL pattern visibility */
    for (int i = 0; i < iterations; i++) {
        /* Multiple bitfield assignments */
        ptr->field1 = (i + 1) & 0x7;
        ptr->field2 = (i * 2) & 0x1F;
        ptr->field3 = (i * 3) & 0xFF;
        
        /* Inline assembly that reads/writes memory */
        __asm__ volatile(
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

/* Function using atomic operations on bitfield */
void atomic_bitfield_ops(void) {
    struct BitfieldStruct atomic_struct;
    
    /* Atomic operation on bitfield may generate ZERO_EXTRACT with MEM */
    __sync_fetch_and_or(&atomic_struct.field1, 1);
    __sync_fetch_and_and(&atomic_struct.field2, 0x0F);
}

/* Function with unpredictable control flow */
void conditional_bitfield_ops(int condition) {
    struct BitfieldStruct data;
    
    /* External condition prevents optimization */
    if (condition) {
        data.field1 = 1;
        data.field3 = 0xFF;
    } else {
        data.field2 = 0x1F;
    }
    
    /* Inline assembly with memory clobber */
    __asm__ volatile(
        "lock orl $0x1, %0\n\t"
        : "+m" (data)
        :
        : "memory"
    );
}

/* Main function that exercises all patterns */
int main(int argc, char **argv) {
    /* Use argc as unpredictable input */
    int iterations = argc > 1 ? atoi(argv[1]) : 10;
    
    /* Exercise ZERO_EXTRACT patterns */
    set_bitfield_via_pointer(&global_bitfield, 0x55);
    
    /* Exercise volatile bitfields */
    set_volatile_bitfield();
    
    /* Exercise partial register operations */
    partial_register_ops();
    
    /* Exercise loop with bitfield operations */
    bitfield_loop_operations(iterations);
    
    /* Exercise atomic operations */
    atomic_bitfield_ops();
    
    /* Exercise conditional operations */
    conditional_bitfield_ops(iterations % 2);
    
    /* Additional complex pattern: nested bitfields */
    struct {
        struct {
            unsigned int nested_field : 4;
        } inner;
        unsigned int outer_field : 12;
    } complex_struct;
    
    complex_struct.inner.nested_field = 0xA;
    complex_struct.outer_field = 0xBCD;
    
    /* Force memory reference by taking address */
    volatile unsigned int *force_mem = (volatile unsigned int *)&complex_struct;
    *force_mem = *force_mem | 0x1;
    
    /* Mix with inline assembly that has multiple constraints */
    unsigned int temp;
    __asm__ volatile(
        "movl %%esp, %0\n\t"
        "andl $0xF, %0\n\t"
        : "=r" (temp)
        :
        : "cc"
    );
    
    complex_struct.outer_field = temp & 0xFFF;
    
    return 0;
}
