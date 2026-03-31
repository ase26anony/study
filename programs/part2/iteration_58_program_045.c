/* test_resource_coverage.c
 * This code is designed to trigger specific uncovered lines in GCC's resource.cc
 * Lines 282-290: mark_set_resources handling SET with ZERO_EXTRACT/STRICT_LOW_PART
 * that ultimately reference memory.
 */

#include <stdint.h>
#include <stdlib.h>

/* Global struct with bitfield to ensure memory storage */
struct GlobalBitfield {
    volatile unsigned int data;
    unsigned int field1 : 3;
    unsigned int field2 : 5;
    unsigned int field3 : 8;
    volatile unsigned int padding;
};

struct GlobalBitfield g_bf = {0};

/* Struct passed by pointer - ensures bitfield is in memory */
struct PointerBitfield {
    unsigned int header;
    unsigned int flags : 4;
    unsigned int status : 3;
    unsigned int mode : 2;
    unsigned int tail;
};

/* Function to set bitfield via pointer - generates ZERO_EXTRACT with MEM */
void set_bitfield_via_pointer(struct PointerBitfield *pb, int val) {
    /* Multiple bitfield assignments to increase pattern visibility */
    pb->flags = val & 0xF;
    /* Compiler barrier to prevent optimization */
    asm volatile("" : : : "memory");
    pb->status = (val >> 4) & 0x7;
    pb->mode = (val >> 7) & 0x3;
}

/* Function with complex control flow and bitfield operations */
void complex_bitfield_operations(int iterations, int seed) {
    struct PointerBitfield local_bf;
    local_bf.header = seed;
    local_bf.tail = ~seed;
    
    /* Loop with conditional bitfield assignments */
    for (int i = 0; i < iterations; i++) {
        /* Use argc/seed to make control flow unpredictable */
        if ((seed + i) & 1) {
            local_bf.flags = (local_bf.flags + 1) & 0xF;
        } else {
            local_bf.status = (local_bf.status ^ i) & 0x7;
        }
        
        /* Global bitfield assignment - volatile ensures memory access */
        g_bf.field1 = (i & 0x7);
        g_bf.field2 = ((i * 3) & 0x1F);
        
        /* Compiler barrier between operations */
        asm volatile("" : : : "memory");
    }
    
    /* Final assignment to ensure the pattern exists at function end */
    set_bitfield_via_pointer(&local_bf, seed);
}

/* Function using inline assembly with partial register constraints */
void partial_register_operations(void) {
    volatile uint8_t byte_var;
    volatile uint16_t word_var;
    
    /* Inline assembly that might generate STRICT_LOW_PART */
    /* Writing to byte-addressable register */
    asm volatile(
        "movb $0x42, %0\n\t"
        : "=Q" (byte_var)  /* Q constraint = byte-addressable register */
        :
        : "memory"
    );
    
    /* More complex assembly with memory references */
    uint32_t temp = 0x12345678;
    asm volatile(
        "movl %1, %%eax\n\t"
        "andl $0x7F, %%eax\n\t"
        "movb %%al, %0\n\t"
        : "=m" (byte_var)   /* Memory destination */
        : "r" (temp)
        : "eax", "memory"
    );
    
    /* Multiple asm statements to create scheduling dependencies */
    asm volatile(
        "movw %1, %%ax\n\t"
        "orb $0x1, %%al\n\t"
        "movw %%ax, %0"
        : "=m" (word_var)
        : "r" (word_var)
        : "ax", "memory"
    );
}

/* Function using atomic operations on bitfields */
void atomic_bitfield_operations(void) {
    /* Atomic operation on bitfield - may generate ZERO_EXTRACT with MEM */
    struct {
        unsigned int atomic_field : 4;
        unsigned int padding : 28;
    } atomic_struct;
    
    atomic_struct.atomic_field = 0;
    
    /* Use __sync builtin which may create complex RTL patterns */
    __sync_fetch_and_or(&atomic_struct.atomic_field, 0x3);
    __sync_fetch_and_and(&atomic_struct.atomic_field, 0x5);
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
}

/* Function that mixes all techniques */
void mixed_operations(int argc, char **argv) {
    /* Use argc to create unpredictable control flow */
    int mode = argc & 0x3;
    
    /* Global bitfield operations */
    g_bf.field1 = mode;
    g_bf.field3 = (argc * 17) & 0xFF;
    
    /* Conditional based on external input */
    if (argc > 1) {
        struct PointerBitfield dynamic_bf;
        
        /* Initialize with values from argv */
        int init_val = atoi(argv[1]);
        dynamic_bf.header = init_val;
        
        /* Complex bitfield assignment pattern */
        for (int i = 0; i < 10; i++) {
            dynamic_bf.flags = (dynamic_bf.flags + init_val + i) & 0xF;
            dynamic_bf.status = (dynamic_bf.status ^ (init_val >> i)) & 0x7;
            
            /* Volatile memory access to prevent optimization */
            asm volatile("" : : : "memory");
        }
        
        set_bitfield_via_pointer(&dynamic_bf, init_val);
    }
    
    /* Always execute partial register operations */
    partial_register_operations();
    
    /* Execute atomic operations if argc is even */
    if ((argc & 1) == 0) {
        atomic_bitfield_operations();
    }
}

/* Main function that drives everything */
int main(int argc, char **argv) {
    /* Initialize global bitfield */
    g_bf.data = 0xDEADBEEF;
    g_bf.padding = 0xCAFEBABE;
    
    /* Simple bitfield assignment to global */
    g_bf.field1 = 0x5;
    g_bf.field2 = 0x12;
    
    /* Call function with complex control flow */
    complex_bitfield_operations(100, argc);
    
    /* Call mixed operations with program arguments */
    mixed_operations(argc, argv);
    
    /* Additional inline assembly that might create the target pattern */
    struct {
        unsigned int low : 8;
        unsigned int high : 24;
    } packed;
    
    packed.low = 0xAA;
    packed.high = 0xBBCCDD;
    
    /* Inline assembly that reads and writes the bitfield in memory */
    uint32_t result;
    asm volatile(
        "movl %1, %%eax\n\t"
        "andl $0xFF, %%eax\n\t"      /* Extract low byte */
        "orl $0x100, %%eax\n\t"      /* Modify it */
        "movl %%eax, %0\n\t"
        : "=m" (packed.low)          /* This should create ZERO_EXTRACT MEM */
        : "m" (packed)
        : "eax", "memory"
    );
    
    /* Return something based on the operations */
    return (g_bf.field1 + packed.low) & 0xFF;
}
