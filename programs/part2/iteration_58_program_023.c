/* test_resource_coverage.c */

#include <stdint.h>
#include <stdlib.h>

/* Global struct with bitfield to ensure memory storage */
struct BitfieldStruct {
    unsigned int data;
    unsigned int field1 : 3;
    unsigned int field2 : 5;
    unsigned int field3 : 8;
};

volatile struct BitfieldStruct global_bitfield = {0};

/* Function to force bitfield assignment through pointer */
void set_bitfield_via_pointer(struct BitfieldStruct *ptr, unsigned int val) {
    /* Multiple bitfield assignments to increase visibility */
    ptr->field1 = val & 0x7;
    ptr->field2 = (val >> 3) & 0x1F;
    ptr->field3 = (val >> 8) & 0xFF;
    
    /* Memory barrier to prevent optimization */
    asm volatile("" : : : "memory");
}

/* Function using STRICT_LOW_PART pattern via inline assembly */
void strict_low_part_example(void) {
    volatile uint8_t byte_var = 0;
    uint32_t dummy;
    
    /* Inline assembly that might generate STRICT_LOW_PART */
    /* Writing to low byte of a register that gets stored to memory */
    asm volatile(
        "movl $0x12345678, %%eax\n\t"
        "movb %%al, %0\n\t"
        "movl %%eax, %1"
        : "=m" (byte_var), "=r" (dummy)
        : 
        : "eax", "memory"
    );
    
    /* Another attempt with explicit byte register constraint */
    register uint8_t reg_byte asm("al");
    asm volatile(
        "movb $0x42, %0"
        : "=Q" (reg_byte)
        :
        : "memory"
    );
    
    /* Store it to memory */
    volatile uint8_t *mem_byte = &byte_var;
    *mem_byte = reg_byte;
}

/* Complex function with mixed operations to trigger resource tracking */
void complex_bitfield_operations(int iterations) {
    struct BitfieldStruct local_struct;
    volatile int *external_input = (volatile int *)&iterations;
    
    /* Use external input to prevent dead code elimination */
    if (*external_input > 0) {
        /* Multiple bitfield assignments in loop */
        for (int i = 0; i < iterations && i < 10; i++) {
            /* These should generate ZERO_EXTRACT patterns */
            local_struct.field1 = (i * 3) & 0x7;
            local_struct.field2 = (i * 5) & 0x1F;
            local_struct.field3 = (i * 7) & 0xFF;
            
            /* Memory barrier between operations */
            asm volatile("" : : : "memory");
        }
        
        /* Pass address to force memory storage */
        set_bitfield_via_pointer(&local_struct, *external_input);
        
        /* Also modify global */
        global_bitfield.field1 = local_struct.field1;
        global_bitfield.field2 = local_struct.field2;
    }
}

/* Function using atomic operations on bitfields */
void atomic_bitfield_ops(void) {
    /* Atomic operations on bitfields may generate complex RTL */
    struct {
        unsigned int flags : 4;
        unsigned int status : 4;
    } atomic_struct = {0};
    
    /* Use __sync builtins which may preserve bitfield patterns */
    unsigned int old = __sync_fetch_and_or(&atomic_struct.flags, 1);
    old = __sync_fetch_and_and(&atomic_struct.status, 0xE);
    
    /* Force memory access */
    asm volatile("" : : "m" (atomic_struct) : "memory");
}

/* Main function with unpredictable control flow */
int main(int argc, char **argv) {
    /* Use argc as unpredictable input */
    int iterations = argc > 1 ? atoi(argv[1]) : 5;
    
    /* Initialize global */
    global_bitfield.data = 0xDEADBEEF;
    
    /* Call functions that should generate the target RTL patterns */
    
    /* 1. Bitfield assignments (ZERO_EXTRACT) */
    complex_bitfield_operations(iterations);
    
    /* 2. STRICT_LOW_PART patterns */
    strict_low_part_example();
    
    /* 3. Atomic operations on bitfields */
    atomic_bitfield_ops();
    
    /* Additional: Direct volatile bitfield assignment */
    volatile struct {
        unsigned int control : 2;
        unsigned int mode : 3;
    } volatile_bf = {0};
    
    volatile_bf.control = 2;
    volatile_bf.mode = 4;
    
    /* Mix with inline assembly that reads/writes memory */
    uint32_t temp;
    asm volatile(
        "movl %1, %%eax\n\t"
        "andl $0x7, %%eax\n\t"
        "movl %%eax, %0\n\t"
        "lock orl $0x1, %2"
        : "=r" (temp), "+m" (volatile_bf)
        : "m" (global_bitfield.field1)
        : "eax", "memory"
    );
    
    /* Return something based on all operations */
    return (global_bitfield.field1 + volatile_bf.control + temp) & 0xFF;
}

/* Additional global to prevent optimization */
struct BitfieldStruct *get_global_ptr(void) {
    return &global_bitfield;
}
