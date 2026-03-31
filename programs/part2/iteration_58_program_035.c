/* test_resource_coverage.c */
#include <stdint.h>
#include <stdlib.h>

/* Global volatile bitfield structure to force memory access */
volatile struct BitfieldStruct {
    unsigned int field1 : 3;
    unsigned int field2 : 5;
    unsigned int field3 : 8;
} g_bitfield = {0};

/* Non-volatile global for mixed access patterns */
struct MixedBitfield {
    unsigned int low : 4;
    unsigned int high : 4;
    unsigned int full;
} g_mixed;

/* Function to set bitfield via pointer - ensures memory destination */
void set_bitfield_via_pointer(struct BitfieldStruct *ptr, int val) {
    /* Multiple assignments to create multiple SET patterns */
    ptr->field1 = val & 0x7;
    ptr->field2 = (val >> 3) & 0x1F;
    ptr->field3 = (val >> 8) & 0xFF;
}

/* Function with STRICT_LOW_PART pattern using inline assembly */
void strict_low_part_operations(void) {
    volatile char byte_var;
    int temp;
    
    /* Inline assembly that might generate STRICT_LOW_PART */
    __asm__ volatile (
        "movb $0x42, %0\n\t"
        "movb %0, %%al\n\t"
        : "=Q" (byte_var)  /* Q constraint for byte-addressable register */
        :
        : "al", "memory"
    );
    
    /* Additional operations to create scheduling complexity */
    __asm__ volatile (
        "movl $0x12345678, %%eax\n\t"
        "andb $0xF0, %%al\n\t"
        "movb %%al, %0"
        : "=m" (byte_var)
        :
        : "eax", "memory"
    );
}

/* Function with atomic operations on bitfields */
void atomic_bitfield_ops(void) {
    /* Atomic operations may generate ZERO_EXTRACT with memory */
    __sync_fetch_and_or(&g_bitfield.field1, 1);
    __sync_fetch_and_and(&g_bitfield.field2, 0x1F);
}

/* Complex function with control flow to prevent optimization */
void complex_bitfield_manipulation(int argc, char **argv) {
    struct BitfieldStruct local_bf;
    struct BitfieldStruct *ptr;
    
    /* Unpredictable control flow based on external input */
    if (argc > 1) {
        ptr = &g_bitfield;
    } else {
        ptr = &local_bf;
    }
    
    /* Compiler barrier to prevent reordering */
    __asm__ volatile ("" : : : "memory");
    
    /* Bitfield assignment that should generate ZERO_EXTRACT */
    ptr->field1 = (unsigned int)argv[0][0] & 0x7;
    
    /* Another barrier */
    __asm__ volatile ("" : : : "memory");
    
    /* More assignments in a small loop */
    for (int i = 0; i < 3; i++) {
        ptr->field2 = (ptr->field2 + 1) & 0x1F;
        __asm__ volatile ("" : : : "memory");
    }
}

/* Function that mixes bitfield and regular memory accesses */
void mixed_memory_accesses(void) {
    /* Access global mixed structure */
    g_mixed.low = 0xA;
    g_mixed.high = 0x5;
    
    /* Inline assembly that reads and writes memory */
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "andl $0xF, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (g_mixed.low)
        : "m" (g_mixed.full)
        : "eax", "memory"
    );
}

/* Main function that exercises all patterns */
int main(int argc, char **argv) {
    /* Initialize */
    struct BitfieldStruct stack_bf = {0};
    
    /* Exercise pointer-based bitfield assignment */
    set_bitfield_via_pointer(&stack_bf, 0x55);
    set_bitfield_via_pointer(&g_bitfield, 0xAA);
    
    /* Exercise STRICT_LOW_PART patterns */
    strict_low_part_operations();
    
    /* Exercise atomic operations */
    atomic_bitfield_ops();
    
    /* Exercise complex control flow */
    complex_bitfield_manipulation(argc, argv);
    
    /* Exercise mixed accesses */
    mixed_memory_accesses();
    
    /* Additional volatile bitfield assignment */
    volatile struct {
        unsigned int status : 2;
        unsigned int mode : 3;
    } volatile_bf = {0};
    
    volatile_bf.status = 2;
    volatile_bf.mode = 4;
    
    /* Use the results to prevent dead code elimination */
    int result = (int)g_bitfield.field1 + 
                 (int)g_bitfield.field2 + 
                 (int)g_bitfield.field3 +
                 (int)volatile_bf.status +
                 (int)volatile_bf.mode;
    
    return result > 0 ? 0 : 1;
}
