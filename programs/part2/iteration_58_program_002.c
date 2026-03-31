/* test_resource_coverage.c
 * Designed to trigger mark_set_resources path for ZERO_EXTRACT/STRICT_LOW_PART
 * with memory destinations in GCC's RTL.
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

volatile struct BitfieldStruct global_bitfield = {0};
volatile int external_counter = 0;

/* Function to force bitfield assignment through pointer - ensures MEM */
void set_bitfield_via_pointer(struct BitfieldStruct *ptr, int value) {
    /* Multiple assignments to increase visibility */
    ptr->field1 = value & 0x7;
    ptr->field2 = (value >> 3) & 0x1F;
    ptr->field3 = (value >> 8) & 0xFF;
}

/* Function with complex control flow and bitfield operations */
void complex_bitfield_operations(int iterations) {
    struct BitfieldStruct local_struct;
    struct BitfieldStruct *ptr = &local_struct;
    
    /* Volatile read to prevent optimization */
    volatile int seed = external_counter;
    
    for (int i = 0; i < iterations; i++) {
        /* Conditional assignment based on external input */
        if ((seed + i) & 1) {
            /* This should generate ZERO_EXTRACT with MEM destination */
            ptr->field1 = (i + seed) & 0x7;
            
            /* Compiler barrier to prevent reordering */
            asm volatile("" : : : "memory");
            
            ptr->field2 = ((i * 3) + seed) & 0x1F;
        } else {
            ptr->field3 = ((i * 5) + seed) & 0xFF;
        }
        
        /* Mix with inline assembly that clobbers registers */
        asm volatile(
            "movl %0, %%eax\n\t"
            "addl $1, %%eax\n\t"
            "movl %%eax, %1"
            : "=m" (ptr->field1)  /* Memory output */
            : "r" (seed)          /* Register input */
            : "eax", "memory"
        );
    }
}

/* Function using STRICT_LOW_PART via inline assembly */
void strict_low_part_operations(void) {
    volatile uint32_t memory_var = 0;
    volatile uint8_t byte_var = 0;
    
    /* Inline assembly that might generate STRICT_LOW_PART */
    /* Writing to byte-addressable memory location */
    asm volatile(
        "movb $0x42, %0\n\t"
        "movb $0x7F, %1"
        : "=Q" (byte_var),        /* Byte-addressable register constraint */
          "=m" (memory_var)       /* Memory output */
        :
        : "memory"
    );
    
    /* Multiple operations to increase scheduling complexity */
    for (int i = 0; i < 10; i++) {
        asm volatile(
            "movl %1, %%eax\n\t"
            "andl $0xFF, %%eax\n\t"
            "movb %%al, %0"
            : "=Q" (byte_var)
            : "r" (i)
            : "eax", "memory"
        );
    }
}

/* Function with atomic operations on bitfields */
void atomic_bitfield_operations(struct BitfieldStruct *ptr) {
    /* Atomic operations may generate complex RTL with ZERO_EXTRACT */
    int old_val;
    
    /* Simulate atomic OR on bitfield */
    do {
        old_val = ptr->field1;
        /* This may generate ZERO_EXTRACT pattern */
        ptr->field1 = old_val | 0x1;
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
    } while (0);  /* Simplified for example */
    
    /* Use __sync builtins if available */
    #ifdef __GNUC__
    __sync_fetch_and_or(&ptr->field2, 0x10);
    #endif
}

/* Main function with unpredictable control flow */
int main(int argc, char *argv[]) {
    struct BitfieldStruct stack_struct = {0};
    int iterations = 10;
    
    /* Use argc to make control flow unpredictable */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations > 100) iterations = 100;
    }
    
    /* 1. Bitfield assignment via pointer (ZERO_EXTRACT with MEM) */
    set_bitfield_via_pointer(&stack_struct, 0x1234);
    
    /* 2. Complex operations with control flow */
    complex_bitfield_operations(iterations);
    
    /* 3. STRICT_LOW_PART operations */
    strict_low_part_operations();
    
    /* 4. Global bitfield operations */
    for (int i = 0; i < 5; i++) {
        global_bitfield.field1 = i & 0x7;
        global_bitfield.field2 = (i * 2) & 0x1F;
        
        /* Compiler barrier between operations */
        asm volatile("" : : : "memory");
    }
    
    /* 5. Atomic-style operations */
    atomic_bitfield_operations(&stack_struct);
    
    /* Mix with function calls to prevent optimization */
    printf("Result: field1=%u, field2=%u, field3=%u\n",
           stack_struct.field1,
           stack_struct.field2,
           stack_struct.field3);
    
    return 0;
}
