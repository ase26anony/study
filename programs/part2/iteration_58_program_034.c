/* Test program to trigger ZERO_EXTRACT/STRICT_LOW_PART with memory destination
 * in GCC's resource tracking pass */

#include <stdint.h>
#include <stdio.h>

/* Volatile to prevent optimization */
volatile int external_value = 0;

/* Global struct with bitfield - ensures memory storage */
struct BitfieldStruct {
    unsigned int field1 : 3;
    unsigned int field2 : 5;
    unsigned int field3 : 8;
    unsigned int padding : 16;
};

/* Global instance */
struct BitfieldStruct global_bitfield;

/* Function that takes pointer to bitfield struct - ensures memory access */
void set_bitfield_in_memory(struct BitfieldStruct *s, int value) {
    /* Multiple bitfield assignments to increase chance of RTL pattern */
    s->field1 = value & 0x7;
    s->field2 = (value >> 3) & 0x1F;
    s->field3 = (value >> 8) & 0xFF;
    
    /* Compiler barrier to prevent reordering */
    asm volatile("" : : : "memory");
}

/* Function with STRICT_LOW_PART pattern using inline assembly */
void strict_low_part_memory_access(void) {
    /* Use byte-sized memory location that could be part of larger register */
    volatile uint8_t byte_var = 0;
    
    /* Inline assembly that might generate STRICT_LOW_PART */
    /* Writing to byte in memory that could be part of word register */
    asm volatile(
        "movb $0x42, %0\n\t"
        : "=m" (byte_var)
        :
        : "memory"
    );
    
    /* More complex pattern with input/output */
    uint32_t dword_var = 0x12345678;
    asm volatile(
        "movl %1, %%eax\n\t"
        "andl $0xFF, %%eax\n\t"
        "movb %%al, %0\n\t"
        : "=m" (byte_var)
        : "r" (dword_var)
        : "eax", "memory"
    );
}

/* Function with atomic operations on bitfield - may generate ZERO_EXTRACT */
void atomic_bitfield_ops(void) {
    struct BitfieldStruct local_bf;
    
    /* Atomic operation on bitfield - likely generates ZERO_EXTRACT */
    __sync_fetch_and_or(&local_bf.field1, 1);
    __sync_fetch_and_and(&local_bf.field2, 0x0F);
}

/* Complex control flow to ensure scheduling analyzes resources */
void complex_control_flow(int iterations) {
    struct BitfieldStruct stack_bf;
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Conditional based on external value to prevent dead code elimination */
        if (external_value & (1 << (i & 0x1F))) {
            /* Bitfield assignment in loop with memory destination */
            stack_bf.field1 = i & 0x7;
            stack_bf.field2 = (i >> 3) & 0x1F;
        } else {
            stack_bf.field3 = i & 0xFF;
        }
        
        /* Inline asm that clobbers registers - forces resource tracking */
        asm volatile(
            "movl %0, %%eax\n\t"
            "addl $1, %%eax\n\t"
            : 
            : "m" (stack_bf.field1)
            : "eax", "memory"
        );
    }
}

/* Function that mixes bitfields and inline assembly */
void mixed_bitfield_asm(void) {
    volatile struct {
        unsigned int low_bits : 4;
        unsigned int high_bits : 4;
        unsigned int full_word : 24;
    } mixed;
    
    /* Direct bitfield assignment - should generate ZERO_EXTRACT */
    mixed.low_bits = 0xA;
    mixed.high_bits = 0x5;
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* Inline asm that reads and writes the same memory location */
    /* This should force resource analysis */
    asm volatile(
        "movzbl %0, %%eax\n\t"
        "orb $0x1, %%al\n\t"
        "movb %%al, %0\n\t"
        : "+m" (mixed.low_bits)
        :
        : "eax", "memory"
    );
}

/* Main function that exercises all patterns */
int main(int argc, char *argv[]) {
    /* Use argc to make control flow unpredictable */
    int iterations = (argc > 1) ? 10 : 5;
    
    /* Exercise global bitfield */
    set_bitfield_in_memory(&global_bitfield, 0x55AA);
    
    /* Exercise STRICT_LOW_PART patterns */
    strict_low_part_memory_access();
    
    /* Exercise atomic operations */
    atomic_bitfield_ops();
    
    /* Exercise complex control flow */
    complex_control_flow(iterations);
    
    /* Exercise mixed patterns */
    mixed_bitfield_asm();
    
    /* Read external_value to prevent optimization */
    printf("Result: %u\n", global_bitfield.field1);
    
    return 0;
}
