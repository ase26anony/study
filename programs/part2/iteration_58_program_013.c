/* test_resource_coverage.c
 * Designed to trigger uncovered lines 282-290 in resource.cc
 * Compile with: gcc -O1 -c test_resource_coverage.c -o test.o
 * Or for scheduling: gcc -O2 -fschedule-insns -c test_resource_coverage.c
 */

#include <stdint.h>
#include <stdio.h>

/* Prevent optimization from removing our patterns */
static volatile int external_counter = 0;

/* ============================================
   ZERO_EXTRACT patterns (bitfield assignments)
   ============================================ */

/* Global struct with bitfield - ensures memory storage */
struct GlobalBitfield {
    unsigned int data;
    unsigned int field1 : 3;
    unsigned int field2 : 5;
    unsigned int field3 : 8;
    unsigned int field4 : 16;
} global_bf;

/* Volatile bitfield struct - forces memory access */
volatile struct {
    unsigned int status : 2;
    unsigned int flags : 6;
    unsigned int mode : 4;
} volatile_bf;

/* Function that takes pointer to bitfield struct */
void set_bitfield_via_pointer(struct GlobalBitfield *ptr, int value) {
    /* Multiple bitfield assignments in sequence */
    ptr->field1 = value & 0x7;
    ptr->field2 = (value >> 3) & 0x1F;
    ptr->field3 = (value >> 8) & 0xFF;
    
    /* Compiler barrier to prevent merging */
    asm volatile("" : : : "memory");
    
    /* Conditional assignment based on external input */
    if (external_counter & 1) {
        ptr->field4 = value & 0xFFFF;
    }
}

/* Complex bitfield manipulation with loops */
void bitfield_loop_operations(int iterations) {
    struct GlobalBitfield local_bf;
    
    /* Loop with bitfield assignments - harder to optimize away */
    for (int i = 0; i < iterations; i++) {
        local_bf.field1 = (i * 3) & 0x7;
        local_bf.field2 = (i * 5) & 0x1F;
        local_bf.field3 = (i * 7) & 0xFF;
        
        /* Memory barrier every few iterations */
        if (i % 4 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    /* Pass address to ensure memory location */
    set_bitfield_via_pointer(&local_bf, iterations);
}

/* ============================================
   STRICT_LOW_PART patterns (partial register assignments)
   ============================================ */

/* Function using inline assembly with byte constraints */
void partial_register_ops(void) {
    unsigned int value = 0x12345678;
    unsigned char byte1, byte2, byte3, byte4;
    
    /* Multiple byte extractions using inline asm with Q constraint */
    __asm__ volatile (
        "movb %1, %0\n\t"
        : "=Q" (byte1)
        : "r" ((unsigned char)(value & 0xFF))
        : /* no clobbers */
    );
    
    __asm__ volatile (
        "movb %1, %0\n\t"
        : "=Q" (byte2)
        : "r" ((unsigned char)((value >> 8) & 0xFF))
        : /* no clobbers */
    );
    
    /* Memory barrier between operations */
    asm volatile("" : : : "memory");
    
    __asm__ volatile (
        "movb %1, %0\n\t"
        : "=Q" (byte3)
        : "r" ((unsigned char)((value >> 16) & 0xFF))
        : /* no clobbers */
    );
    
    __asm__ volatile (
        "movb %1, %0\n\t"
        : "=Q" (byte4)
        : "r" ((unsigned char)((value >> 24) & 0xFF))
        : /* no clobbers */
    );
    
    /* Use the results to prevent dead code elimination */
    volatile_bf.status = byte1 & 0x3;
    volatile_bf.flags = byte2 & 0x3F;
}

/* Assembly with explicit register constraints for low-part operations */
void explicit_low_part_assembly(void) {
    unsigned short low_word;
    unsigned char low_byte;
    
    /* Force use of specific byte-addressable registers */
    __asm__ volatile (
        "movw $0xABCD, %%ax\n\t"
        "movb %%al, %0\n\t"
        : "=Q" (low_byte)
        : /* no inputs */
        : "ax", "memory"
    );
    
    /* Another operation that might generate STRICT_LOW_PART */
    __asm__ volatile (
        "movw $0x1234, %0\n\t"
        : "=r" (low_word)
        : /* no inputs */
        : "memory"
    );
    
    /* Use the results */
    global_bf.field1 = low_byte & 0x7;
    global_bf.field2 = low_word & 0x1F;
}

/* ============================================
   Mixed patterns with memory references
   ============================================ */

/* Atomic operations on bitfields - may generate complex RTL */
void atomic_bitfield_ops(void) {
    /* Use __sync builtins on bitfield-containing struct */
    unsigned int old_val;
    
    /* Create a union to access the bitfield atomically */
    union {
        struct {
            unsigned int field_a : 4;
            unsigned int field_b : 4;
            unsigned int field_c : 8;
            unsigned int field_d : 16;
        } bits;
        unsigned int word;
    } atomic_bf;
    
    atomic_bf.word = 0;
    
    /* Atomic OR on the entire word containing bitfields */
    old_val = __sync_fetch_and_or(&atomic_bf.word, 0x00070007);
    
    /* Atomic AND */
    old_val = __sync_fetch_and_and(&atomic_bf.word, 0x00FF00FF);
    
    /* Store result in global */
    global_bf.field3 = atomic_bf.bits.field_c;
}

/* Complex function with mixed operations to trigger scheduling */
void complex_resource_mix(int param) {
    /* Local struct with bitfield */
    struct {
        unsigned int config : 12;
        unsigned int state : 4;
        unsigned int count : 16;
    } local_config;
    
    /* Initialize with parameter */
    local_config.config = param & 0xFFF;
    local_config.state = (param >> 12) & 0xF;
    
    /* Inline asm that reads/writes memory */
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "andl $0x0FFF, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (local_config.config)
        : "r" (param)
        : "eax", "memory"
    );
    
    /* More bitfield operations */
    for (int i = 0; i < 4; i++) {
        local_config.count = (local_config.count + i) & 0xFFFF;
        
        /* Conditional bitfield assignment */
        if (i & 1) {
            local_config.state = (local_config.state + 1) & 0xF;
        }
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
    }
    
    /* Partial register operation */
    unsigned char temp;
    __asm__ volatile (
        "movb %1, %0\n\t"
        : "=Q" (temp)
        : "r" ((unsigned char)(local_config.state))
        : /* no clobbers */
    );
    
    /* Store back to global */
    global_bf.field1 = temp & 0x7;
}

/* ============================================
   Main function with unpredictable control flow
   ============================================ */

int main(int argc, char *argv[]) {
    /* Use argc to create unpredictable control flow */
    int iterations = (argc > 1) ? (argc * 10) : 100;
    
    /* Initialize global struct */
    global_bf.data = 0xDEADBEEF;
    global_bf.field1 = 1;
    global_bf.field2 = 2;
    global_bf.field3 = 3;
    global_bf.field4 = 4;
    
    /* Update external counter */
    external_counter = argc;
    
    /* Call functions that generate target RTL patterns */
    
    /* 1. Bitfield assignments (ZERO_EXTRACT) */
    set_bitfield_via_pointer(&global_bf, argc);
    
    /* 2. Loop with bitfields */
    bitfield_loop_operations(iterations % 50);
    
    /* 3. Partial register operations (STRICT_LOW_PART) */
    partial_register_ops();
    
    /* 4. Explicit low-part assembly */
    explicit_low_part_assembly();
    
    /* 5. Atomic operations */
    atomic_bitfield_ops();
    
    /* 6. Complex mixed operations */
    complex_resource_mix(argc);
    
    /* Additional unpredictable path */
    if (argc > 2) {
        /* More bitfield manipulation */
        volatile_bf.mode = (argc * 7) & 0xF;
        
        /* Inline asm with memory clobber */
        __asm__ volatile (
            "movl $0x12345678, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=m" (global_bf.data)
            : /* no inputs */
            : "eax", "memory"
        );
    }
    
    /* Use results to prevent dead code elimination */
    printf("Results: global.field1=%u, global.field3=%u\n", 
           global_bf.field1, global_bf.field3);
    printf("Volatile: status=%u, flags=%u\n",
           volatile_bf.status, volatile_bf.flags);
    
    return (global_bf.field1 + global_bf.field2 + 
            global_bf.field3 + global_bf.field4) & 0xFF;
}
