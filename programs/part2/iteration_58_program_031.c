/* test_resource_coverage.c
 * Designed to trigger uncovered lines 282-290 in resource.cc
 * Compile with: gcc -O1 -c test_resource_coverage.c -o test.o
 * Or for scheduling: gcc -O2 -fschedule-insns -c test_resource_coverage.c
 */

#include <stdint.h>
#include <stdio.h>

/* Prevent optimization from removing critical patterns */
static volatile int external_counter = 0;

/* ========== ZERO_EXTRACT patterns (bitfield in memory) ========== */

/* Global struct with bitfield - ensures memory storage */
struct BitfieldStruct {
    unsigned int regular_field;
    unsigned int bitfield1 : 3;   /* Will generate ZERO_EXTRACT */
    unsigned int bitfield2 : 5;
    unsigned int bitfield3 : 8;
    unsigned int padding : 16;
};

volatile struct BitfieldStruct global_bitfield = {0};

/* Function that takes pointer to ensure memory access */
void set_bitfield_via_pointer(struct BitfieldStruct *ptr, int value) {
    /* Multiple bitfield assignments increase pattern visibility */
    ptr->bitfield1 = value & 0x7;
    ptr->bitfield2 = (value >> 3) & 0x1F;
    /* Compiler barrier to prevent merging */
    asm volatile("" : : : "memory");
    ptr->bitfield3 = (value >> 8) & 0xFF;
}

/* Volatile bitfield - forces memory access */
volatile struct {
    unsigned int status : 2;
    unsigned int mode : 3;
    unsigned int error : 1;
} volatile_status = {0};

/* ========== STRICT_LOW_PART patterns (partial register to memory) ========== */

/* Function using inline assembly with byte constraint */
void set_low_byte_to_memory(void) {
    volatile uint32_t memory_word = 0;
    volatile uint8_t memory_byte = 0;
    
    /* STRICT_LOW_PART may appear when writing partial register to memory */
    /* Using 'Q' constraint for byte-addressable register */
    register uint8_t reg_byte asm("al");
    
    /* Multiple asm statements to create scheduling opportunities */
    asm volatile(
        "movb %1, %0\n\t"
        "movb %0, %2"
        : "=Q" (reg_byte), "+m" (memory_byte)
        : "m" (memory_word)
        : "memory"
    );
    
    /* Another attempt with explicit byte store */
    asm volatile(
        "movb $0x42, %0"
        : "=m" (memory_byte)
        :
        : "memory"
    );
}

/* ========== Complex control flow to trigger resource tracking ========== */

/* Function with mixed operations to create scheduling complexity */
void complex_bitfield_operations(int iterations) {
    struct BitfieldStruct local_struct;
    struct BitfieldStruct *ptr = &local_struct;
    
    /* Unpredictable control flow prevents optimization */
    if (external_counter & 1) {
        ptr = &global_bitfield;
    }
    
    /* Loop with bitfield assignments */
    for (int i = 0; i < iterations; i++) {
        /* Multiple bitfield writes - may generate ZERO_EXTRACT */
        ptr->bitfield1 = (i + external_counter) & 0x7;
        
        /* Memory barrier between operations */
        asm volatile("" : : : "memory");
        
        ptr->bitfield2 = (i * 3) & 0x1F;
        
        /* Conditional to prevent dead code elimination */
        if (i & 1) {
            volatile_status.status = (i & 0x3);
        } else {
            volatile_status.mode = ((i >> 1) & 0x7);
        }
        
        /* External function call prevents optimization */
        external_counter++;
    }
    
    /* Atomic operation on bitfield - may generate complex RTL */
    __sync_fetch_and_or(&global_bitfield.regular_field, 0x01);
}

/* ========== Main function with various patterns ========== */

int main(int argc, char *argv[]) {
    /* Use argc to create unpredictable control flow */
    int iterations = (argc > 1) ? 10 : 5;
    
    /* Test 1: Direct bitfield assignment to global (memory) */
    global_bitfield.bitfield1 = 3;
    global_bitfield.bitfield2 = 7;
    
    /* Compiler barrier */
    asm volatile("" : : : "memory");
    
    /* Test 2: Bitfield via pointer */
    struct BitfieldStruct local;
    set_bitfield_via_pointer(&local, argc);
    
    /* Test 3: Volatile bitfield */
    volatile_status.status = 1;
    volatile_status.error = (argc > 2);
    
    /* Test 4: STRICT_LOW_PART patterns */
    set_low_byte_to_memory();
    
    /* Test 5: Complex operations with scheduling opportunities */
    complex_bitfield_operations(iterations);
    
    /* Test 6: Mixed inline assembly with memory references */
    volatile uint32_t mem_var = 0;
    asm volatile(
        "movl %1, %%eax\n\t"
        "andl $0x7, %%eax\n\t"
        "movl %%eax, %0\n\t"
        "orb $0x4, %2"
        : "=m" (global_bitfield.regular_field), 
          "+m" (mem_var)
        : "m" (global_bitfield.bitfield1)
        : "eax", "memory"
    );
    
    /* Ensure values are used to prevent elimination */
    printf("Results: %u %u %u\n", 
           global_bitfield.bitfield1,
           volatile_status.status,
           local.bitfield2);
    
    return 0;
}

/* ========== Additional functions for more patterns ========== */

/* Function that returns bitfield - may generate interesting RTL */
unsigned int read_modify_write(struct BitfieldStruct *s) {
    unsigned int old = s->bitfield1;
    s->bitfield1 = (old + 1) & 0x7;
    s->bitfield2 = (s->bitfield2 ^ 0x1F) & 0x1F;
    return old;
}

/* Array of structs with bitfields */
struct BitfieldStruct struct_array[4];

void process_array(int idx) {
    /* Access with variable index creates complex addressing */
    struct_array[idx].bitfield1 = idx & 0x7;
    struct_array[(idx + 1) & 0x3].bitfield2 = (idx * 2) & 0x1F;
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* Another access pattern */
    if (idx & 1) {
        struct_array[0].bitfield3 = external_counter & 0xFF;
    }
}
