/* test_resource_coverage.c
 * 
 * This program is designed to trigger specific uncovered lines in GCC's
 * resource.cc file (lines 282-290) during compilation.
 * The goal is to create RTL patterns where:
 * 1. A SET has a ZERO_EXTRACT or STRICT_LOW_PART destination
 * 2. That destination ultimately references memory (MEM)
 * 3. The pattern survives optimization to reach resource tracking passes
 */

#include <stdint.h>
#include <stdlib.h>

/* Prevent optimization from removing critical patterns */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* Global variables to ensure memory references */
volatile int external_counter = 0;

/* Struct with bitfields to generate ZERO_EXTRACT patterns */
struct BitfieldStruct {
    unsigned int field1 : 3;
    unsigned int field2 : 5;
    unsigned int field3 : 8;
    unsigned int padding : 16;
};

/* Global instance to ensure memory storage */
volatile struct BitfieldStruct global_bitfield;

/* Function to set bitfield via pointer - ensures memory reference */
void set_bitfield_via_pointer(struct BitfieldStruct *ptr, int value) {
    /* Multiple assignments to increase pattern visibility */
    ptr->field1 = value & 0x7;
    COMPILER_BARRIER();
    ptr->field2 = (value >> 3) & 0x1F;
    COMPILER_BARRIER();
    ptr->field3 = (value >> 8) & 0xFF;
}

/* Function with complex control flow to prevent optimization */
void conditional_bitfield_set(struct BitfieldStruct *ptr, int condition) {
    /* Unpredictable condition based on external input */
    if (condition & 0x1) {
        ptr->field1 = 1;
        COMPILER_BARRIER();
    }
    if (condition & 0x2) {
        ptr->field2 = 2;
        COMPILER_BARRIER();
    }
    if (condition & 0x4) {
        ptr->field3 = 3;
    }
}

/* Function using inline assembly for STRICT_LOW_PART patterns */
void partial_register_operations(void) {
    /* Variables that may generate partial register references */
    volatile uint8_t byte1, byte2;
    volatile uint32_t dword;
    
    /* Inline assembly that might generate STRICT_LOW_PART */
    /* Writing to byte-sized memory locations */
    __asm__ volatile(
        "movb $0xAA, %0\n\t"
        "movb $0xBB, %1"
        : "=m" (byte1), "=m" (byte2)
        : 
        : "memory"
    );
    
    COMPILER_BARRIER();
    
    /* More complex assembly with register constraints */
    __asm__ volatile(
        "movl $0x12345678, %%eax\n\t"
        "movb %%al, %0\n\t"
        "movb %%ah, %1"
        : "=m" (byte1), "=m" (byte2)
        : 
        : "eax", "memory"
    );
    
    COMPILER_BARRIER();
    
    /* Assembly with "Q" constraint (byte-addressable register) */
    register uint8_t reg_byte asm("al");
    __asm__ volatile(
        "movb $0x55, %0"
        : "=Q" (reg_byte)
        :
        : "memory"
    );
    
    /* Store the byte to memory */
    byte1 = reg_byte;
}

/* Function using atomic operations on bitfields */
void atomic_bitfield_operations(struct BitfieldStruct *ptr) {
    /* Atomic operations may generate complex RTL with ZERO_EXTRACT */
    __sync_fetch_and_or(&ptr->field1, 0x1);
    COMPILER_BARRIER();
    __sync_fetch_and_and(&ptr->field2, 0x1F);
    COMPILER_BARRIER();
    __sync_fetch_and_xor(&ptr->field3, 0xFF);
}

/* Function with mixed operations to create scheduling complexity */
void mixed_operations(struct BitfieldStruct *ptr, int iterations) {
    volatile int temp;
    
    for (int i = 0; i < iterations; i++) {
        /* Bitfield assignment - should generate ZERO_EXTRACT */
        ptr->field1 = (i + external_counter) & 0x7;
        
        /* Inline assembly that reads/writes memory */
        __asm__ volatile(
            "movl %1, %%eax\n\t"
            "andl $0x7, %%eax\n\t"
            "addl $1, %%eax\n\t"
            "movl %%eax, %0"
            : "=m" (temp)
            : "m" (external_counter)
            : "eax", "memory"
        );
        
        /* Another bitfield assignment */
        ptr->field2 = temp & 0x1F;
        
        /* Compiler barrier to prevent reordering */
        COMPILER_BARRIER();
        
        /* Conditional assignment based on temp */
        if (temp & 0x1) {
            ptr->field3 = (ptr->field3 + 1) & 0xFF;
        }
        
        /* Update external counter to prevent loop elimination */
        external_counter++;
    }
}

/* Main function that exercises all patterns */
int main(int argc, char *argv[]) {
    struct BitfieldStruct local_bitfield;
    struct BitfieldStruct *heap_bitfield;
    
    /* Allocate on heap to ensure memory reference */
    heap_bitfield = (struct BitfieldStruct*)malloc(sizeof(struct BitfieldStruct));
    if (!heap_bitfield) return 1;
    
    /* Initialize */
    local_bitfield.field1 = 0;
    local_bitfield.field2 = 0;
    local_bitfield.field3 = 0;
    heap_bitfield->field1 = 0;
    heap_bitfield->field2 = 0;
    heap_bitfield->field3 = 0;
    
    /* Use argc to create unpredictable control flow */
    int condition = argc;
    
    /* Exercise different patterns */
    
    /* 1. Simple bitfield assignment to memory */
    set_bitfield_via_pointer(&local_bitfield, condition);
    
    /* 2. Conditional bitfield assignments */
    conditional_bitfield_set(heap_bitfield, condition);
    
    /* 3. Partial register operations */
    partial_register_operations();
    
    /* 4. Atomic operations on bitfields */
    atomic_bitfield_operations(&local_bitfield);
    
    /* 5. Mixed operations with loop for scheduling complexity */
    mixed_operations(heap_bitfield, 10);
    
    /* 6. Global bitfield operations */
    global_bitfield.field1 = condition & 0x7;
    global_bitfield.field2 = (condition >> 3) & 0x1F;
    global_bitfield.field3 = (condition >> 8) & 0xFF;
    
    /* Additional complex pattern: nested bitfield in struct */
    struct {
        struct BitfieldStruct inner;
        int other_data;
    } nested_struct;
    
    nested_struct.inner.field1 = 1;
    nested_struct.inner.field2 = 2;
    nested_struct.other_data = condition;
    
    /* Pointer arithmetic with bitfields */
    struct BitfieldStruct *ptr_array[2];
    ptr_array[0] = &local_bitfield;
    ptr_array[1] = heap_bitfield;
    
    for (int i = 0; i < 2; i++) {
        ptr_array[i]->field1 = (ptr_array[i]->field1 + 1) & 0x7;
        COMPILER_BARRIER();
    }
    
    /* Use the results to prevent dead code elimination */
    int result = local_bitfield.field1 + 
                 heap_bitfield->field2 + 
                 global_bitfield.field3 +
                 nested_struct.inner.field1;
    
    free(heap_bitfield);
    
    return result & 0xFF;  /* Return non-deterministic result */
}
