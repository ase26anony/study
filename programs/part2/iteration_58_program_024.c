/* test_resource_coverage.c
 * Designed to trigger mark_set_resources path for ZERO_EXTRACT/STRICT_LOW_PART
 * with memory destinations in GCC's RTL.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global struct with bitfield to ensure memory storage */
struct BitfieldStruct {
    unsigned int padding: 8;
    unsigned int target_field: 3;  /* 3-bit field for ZERO_EXTRACT */
    unsigned int other_field: 5;
    volatile int force_memory;     /* Force struct to stay in memory */
};

/* Volatile global to prevent optimization */
volatile struct BitfieldStruct global_bitfield = {0, 0, 0, 0};

/* Function that takes pointer to ensure memory access */
void set_bitfield_via_pointer(struct BitfieldStruct *ptr, unsigned int value) {
    /* This should generate ZERO_EXTRACT with MEM destination */
    ptr->target_field = value & 0x7;  /* Mask to fit 3-bit field */
    
    /* Compiler barrier to prevent reordering/merging */
    asm volatile("" : : : "memory");
}

/* Function with multiple bitfield operations in loop */
void loop_bitfield_operations(struct BitfieldStruct *ptr, int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Multiple assignments to same bitfield */
        ptr->target_field = (i & 0x7);
        ptr->other_field = (i >> 3) & 0x1F;
        
        /* External function call to prevent loop optimization */
        if (ptr->force_memory) {
            /* Do nothing, just reference volatile */
            (void)ptr->force_memory;
        }
        
        /* Another compiler barrier */
        asm volatile("" : : : "memory");
    }
}

/* Function using inline assembly with partial register access */
void partial_register_asm(void) {
    /* Try to generate STRICT_LOW_PART for byte register */
    unsigned char byte_var;
    volatile unsigned char *byte_ptr = &byte_var;
    
    /* Inline asm that might generate STRICT_LOW_PART */
    __asm__ volatile(
        "movb $0x42, %0\n\t"
        : "=Q" (*byte_ptr)  /* "Q" constraint for byte-addressable register */
        :
        : "memory"
    );
    
    /* Additional asm with memory clobber to force scheduling analysis */
    int dummy;
    __asm__ volatile(
        "movl $1, %%eax\n\t"
        "andl $0x7, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (global_bitfield.target_field)
        : 
        : "eax", "memory"
    );
}

/* Function with atomic operations on bitfield */
void atomic_bitfield_ops(void) {
    /* Atomic operations may generate complex RTL with ZERO_EXTRACT */
    struct BitfieldStruct local_struct = {0, 0, 0, 0};
    
    /* Use __sync builtin which may preserve bitfield RTL */
    unsigned int old = __sync_fetch_and_or(&local_struct.target_field, 1);
    
    /* Reference result to prevent elimination */
    if (old) {
        local_struct.force_memory = 1;
    }
}

/* Complex control flow to ensure scheduling pass analyzes resources */
void complex_control_flow(int argc, char **argv) {
    struct BitfieldStruct stack_struct = {0, 0, 0, 0};
    
    /* Unpredictable condition based on external input */
    if (argc > 1) {
        /* Bitfield assignment in one path */
        stack_struct.target_field = atoi(argv[1]) & 0x7;
    } else {
        /* Different bitfield assignment in other path */
        stack_struct.target_field = 3;
    }
    
    /* Inline asm that reads/writes overlapping resources */
    int temp;
    __asm__ volatile(
        "movl %1, %%eax\n\t"
        "andl $0x7, %%eax\n\t"
        "movl %%eax, %0\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, %2\n\t"
        : "=m" (stack_struct.target_field), "=m" (temp)
        : "m" (stack_struct.other_field)
        : "eax", "memory"
    );
    
    /* Pass address to external function (prevents optimization) */
    set_bitfield_via_pointer(&stack_struct, temp & 0x7);
}

/* Main function that exercises all patterns */
int main(int argc, char **argv) {
    printf("Testing resource coverage patterns...\n");
    
    /* 1. Bitfield in global struct (definitely in memory) */
    global_bitfield.target_field = 1;
    
    /* 2. Bitfield via pointer */
    struct BitfieldStruct local;
    set_bitfield_via_pointer(&local, 2);
    
    /* 3. Loop with bitfield operations */
    loop_bitfield_operations(&local, 10);
    
    /* 4. Partial register assembly */
    partial_register_asm();
    
    /* 5. Atomic operations */
    atomic_bitfield_ops();
    
    /* 6. Complex control flow */
    complex_control_flow(argc, argv);
    
    /* Use results to prevent dead code elimination */
    printf("Results: global=%u, local=%u\n", 
           global_bitfield.target_field,
           local.target_field);
    
    return 0;
}
