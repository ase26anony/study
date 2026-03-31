/* test_resource_coverage.c
 * Designed to trigger uncovered lines 282-290 in resource.cc
 * Compile with: gcc -O1 -c test_resource_coverage.c -o test.o
 * Or for scheduling: gcc -O2 -fschedule-insns -c test_resource_coverage.c
 */

#include <stdint.h>
#include <stdio.h>

/* Prevent optimization from removing critical patterns */
static volatile int external_counter = 0;

/* ========== ZERO_EXTRACT Patterns ========== */

/* Global struct with bitfield - ensures memory storage */
struct BitfieldStruct {
    unsigned int data;
    unsigned int field1 : 3;
    unsigned int field2 : 5;
    unsigned int field3 : 8;
    unsigned int field4 : 16;
};

volatile struct BitfieldStruct global_bitfield = {0};

/* Function that takes pointer to ensure memory access */
void set_bitfield_via_pointer(struct BitfieldStruct *ptr, int value) {
    /* Multiple bitfield assignments increase pattern visibility */
    ptr->field1 = value & 0x7;
    ptr->field2 = (value >> 3) & 0x1F;
    /* Compiler barrier to prevent merging */
    asm volatile("" : : : "memory");
    ptr->field3 = (value >> 8) & 0xFF;
}

/* Complex function with control flow to ensure scheduling analyzes resources */
void complex_bitfield_operations(int iterations) {
    struct BitfieldStruct local;
    struct BitfieldStruct *ptr = &local;
    
    /* Unpredictable control flow prevents dead code elimination */
    if (external_counter & 1) {
        ptr = &global_bitfield;
    }
    
    for (int i = 0; i < iterations; i++) {
        /* Multiple SETs with ZERO_EXTRACT destinations */
        ptr->field1 = (i + external_counter) & 0x7;
        ptr->field2 = (i * 2) & 0x1F;
        
        /* Conditional assignment ensures pattern isn't optimized out */
        if (i % 3 == 0) {
            ptr->field3 = (external_counter >> (i % 8)) & 0xFF;
        }
        
        /* Memory barrier between operations */
        asm volatile("" : : : "memory");
    }
}

/* Atomic operations on bitfields may generate complex RTL */
void atomic_bitfield_ops(void) {
    /* Use __sync builtins which may generate ZERO_EXTRACT with MEM */
    __sync_fetch_and_or(&global_bitfield.data, 0x01);
    
    /* Simulate atomic bitfield operation */
    unsigned int old_val;
    unsigned int new_val;
    
    do {
        old_val = global_bitfield.data;
        new_val = old_val | (1 << 10);
        /* This may generate ZERO_EXTRACT pattern */
    } while (!__sync_bool_compare_and_swap(&global_bitfield.data, old_val, new_val));
}

/* ========== STRICT_LOW_PART Patterns ========== */

/* Function to generate STRICT_LOW_PART patterns via inline assembly */
void strict_low_part_operations(void) {
    /* Using byte-addressable register constraints */
    register uint8_t byte1 asm("al");
    register uint8_t byte2 asm("bl");
    uint8_t memory_byte;
    
    /* Inline asm with "=Q" constraint (byte-addressable register) */
    asm volatile(
        "movb $0x42, %0\n\t"
        "movb %%al, %1"
        : "=Q" (byte1), "=m" (memory_byte)
        :
        : "memory"
    );
    
    /* Multiple asm statements to create scheduling complexity */
    asm volatile(
        "movb $0x84, %0\n\t"
        "addb %1, %0"
        : "=Q" (byte2)
        : "Q" (byte1)
        : "cc"
    );
    
    /* Write to char variable that may be part of larger register */
    volatile char *char_ptr = (volatile char *)&global_bitfield;
    asm volatile(
        "movb %%bl, (%0)"
        :
        : "r" (char_ptr), "b" (byte2)
        : "memory"
    );
}

/* Mixed operations combining both patterns */
void mixed_operations(int seed) {
    /* Create a local struct with bitfields */
    struct {
        unsigned int header : 4;
        unsigned int payload : 20;
        unsigned int trailer : 8;
    } packet;
    
    /* Take address to ensure memory access */
    volatile void *ptr = &packet;
    
    /* Conditional assignments based on external input */
    if (seed & 1) {
        packet.header = seed & 0xF;
        /* This should generate ZERO_EXTRACT with MEM destination */
    }
    
    if (seed & 2) {
        packet.payload = (seed * 17) & 0xFFFFF;
    }
    
    /* Inline asm that reads and writes the bitfield memory */
    unsigned int temp;
    asm volatile(
        "movl %1, %%eax\n\t"
        "andl $0xF, %%eax\n\t"      /* Extract header */
        "movl %%eax, %0\n\t"
        "orl $0x100, %1"            /* Modify in memory */
        : "=r" (temp), "+m" (packet)
        :
        : "eax", "cc", "memory"
    );
    
    /* Additional STRICT_LOW_PART pattern */
    register char low_byte asm("cl");
    asm volatile(
        "movb %1, %%cl\n\t"
        "incb %%cl\n\t"
        "movb %%cl, %0"
        : "=Q" (low_byte), "=m" (*(volatile char *)ptr)
        :
        : "cc"
    );
}

/* ========== Main Function ========== */

int main(int argc, char *argv[]) {
    /* Use argc to create unpredictable control flow */
    int iterations = (argc > 1) ? (argc * 10) : 100;
    
    printf("Starting resource pattern tests...\n");
    
    /* Test ZERO_EXTRACT patterns */
    set_bitfield_via_pointer(&global_bitfield, 0x55);
    
    complex_bitfield_operations(iterations);
    
    atomic_bitfield_ops();
    
    /* Test STRICT_LOW_PART patterns */
    strict_low_part_operations();
    
    /* Test mixed patterns */
    for (int i = 0; i < 10; i++) {
        mixed_operations(external_counter + i);
    }
    
    /* Update external counter to affect future runs */
    external_counter++;
    
    printf("Tests completed. Counter: %d\n", external_counter);
    
    return 0;
}
