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
void set_bitfield_via_pointer(struct BitfieldStruct *ptr, unsigned int val) {
    /* Multiple bitfield assignments to increase RTL complexity */
    ptr->field1 = val & 0x7;
    ptr->field2 = (val >> 3) & 0x1F;
    /* Compiler barrier to prevent merging */
    asm volatile("" : : : "memory");
    ptr->field3 = (val >> 8) & 0xFF;
}

/* Function with volatile bitfield - forces memory access */
void set_volatile_bitfield(void) {
    volatile struct {
        unsigned int status : 4;
        unsigned int control : 4;
    } device_reg;
    
    device_reg.status = 0xA;
    /* Inline asm that might interact with the bitfield */
    asm volatile("" : : : "memory");
    device_reg.control = 0x5;
}

/* Function using STRICT_LOW_PART pattern via inline assembly */
void strict_low_part_example(void) {
    uint32_t value = 0x12345678;
    uint8_t low_byte;
    
    /* Assembly that writes only low byte of register */
    __asm__ volatile(
        "movl %1, %%eax\n\t"
        "movb %%al, %0\n\t"
        : "=m" (low_byte)    /* Memory output - forces MEM reference */
        : "r" (value)
        : "eax", "memory"
    );
    
    /* Use the result to prevent dead code elimination */
    global_bitfield.field1 = low_byte & 0x7;
}

/* Complex function with mixed operations to trigger resource tracking */
void complex_bitfield_operations(int iterations, struct BitfieldStruct *arr) {
    for (int i = 0; i < iterations; i++) {
        /* Conditional bitfield assignment based on loop index */
        if (i & 1) {
            arr[i].field1 = (i * 3) & 0x7;
            arr[i].field2 = (i * 5) & 0x1F;
        } else {
            arr[i].field3 = (i * 7) & 0xFF;
        }
        
        /* Inline asm with clobbers to force resource analysis */
        asm volatile(
            "movl $0xDEADBEEF, %%eax\n\t"
            "andl $0x7, %%eax\n\t"
            : : : "eax", "memory"
        );
    }
}

/* Atomic operation on bitfield - may generate complex RTL */
void atomic_bitfield_ops(struct BitfieldStruct *ptr) {
    /* Use GCC atomic builtins on the whole struct */
    unsigned int old_val;
    unsigned int new_val;
    
    /* Read-modify-write simulation */
    old_val = __sync_fetch_and_or(&ptr->field1, 1);
    asm volatile("" : : : "memory");
    new_val = __sync_fetch_and_and(&ptr->field2, 0x0F);
}

/* Main function with unpredictable control flow */
int main(int argc, char *argv[]) {
    struct BitfieldStruct local_struct;
    struct BitfieldStruct *dynamic_struct;
    
    /* Use argc to make control flow unpredictable */
    int use_volatile = argc > 1;
    int use_atomic = argc > 2;
    int iterations = (argc > 3) ? atoi(argv[3]) : 5;
    
    /* Initialize */
    local_struct.field1 = 0;
    local_struct.field2 = 0;
    local_struct.field3 = 0;
    
    /* Dynamic allocation ensures memory reference */
    dynamic_struct = (struct BitfieldStruct*)malloc(
        sizeof(struct BitfieldStruct) * iterations);
    
    if (!dynamic_struct) return 1;
    
    /* Call various functions to generate different RTL patterns */
    
    /* 1. Direct pointer assignment - likely ZERO_EXTRACT with MEM */
    set_bitfield_via_pointer(&local_struct, 0x55);
    
    /* 2. Volatile bitfield - forces memory access */
    if (use_volatile) {
        set_volatile_bitfield();
    }
    
    /* 3. STRICT_LOW_PART pattern */
    strict_low_part_example();
    
    /* 4. Complex loop with bitfields */
    complex_bitfield_operations(iterations, dynamic_struct);
    
    /* 5. Atomic operations if requested */
    if (use_atomic) {
        atomic_bitfield_ops(&local_struct);
        atomic_bitfield_ops(dynamic_struct);
    }
    
    /* Global bitfield assignment */
    global_bitfield.field1 = 0x3;
    global_bitfield.field2 = 0x10;
    global_bitfield.field3 = 0x40;
    
    /* Use results to prevent optimization */
    int result = local_struct.field1 + 
                 dynamic_struct[0].field2 + 
                 global_bitfield.field3;
    
    free(dynamic_struct);
    
    return result & 0xFF;  /* Return non-zero to indicate execution */
}
