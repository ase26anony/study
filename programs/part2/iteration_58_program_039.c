/* test_resource_coverage.c
 * Designed to trigger uncovered lines 282-290 in resource.cc
 * Compile with: gcc -O1 -c test_resource_coverage.c -o test.o
 * Or for scheduling: gcc -O2 -fschedule-insns -c test_resource_coverage.c
 */

#include <stdint.h>
#include <stdlib.h>

/* ========== BITFIELD STRUCTURES FOR ZERO_EXTRACT ========== */

/* Global struct with bitfield - ensures memory storage */
struct GlobalStatus {
    volatile unsigned int ready : 1;    /* volatile forces memory access */
    unsigned int count : 7;             /* 7-bit field */
    unsigned int mode : 3;              /* 3-bit field - matches uncovered code */
    unsigned int padding : 21;          /* padding to 32 bits */
};

struct GlobalStatus global_status;

/* Struct passed by pointer - ensures memory destination */
struct DataPacket {
    unsigned int header : 4;
    unsigned int payload : 20;
    unsigned int checksum : 8;
};

/* ========== FUNCTIONS TO GENERATE ZERO_EXTRACT PATTERNS ========== */

/* Function 1: Direct bitfield assignment to memory via pointer */
void set_bitfield_via_pointer(struct DataPacket *packet, int value) {
    /* This should generate ZERO_EXTRACT with MEM destination */
    packet->payload = value & 0xFFFFF;
    
    /* Compiler barrier to prevent optimization */
    asm volatile("" : : : "memory");
    
    /* Another bitfield assignment */
    packet->checksum = (value >> 20) & 0xFF;
}

/* Function 2: Complex bitfield operations in loop */
void initialize_status_loop(int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Multiple bitfield assignments to global volatile struct */
        global_status.ready = i & 1;
        global_status.count = i & 0x7F;
        global_status.mode = (i >> 7) & 0x7;  /* 3-bit field */
        
        /* External function call prevents loop optimization */
        if (i % 100 == 0) {
            /* Dummy external call - compiler can't optimize away */
            asm volatile("" : : "r"(i) : "memory");
        }
    }
}

/* Function 3: Bitfield with atomic operations */
void atomic_bitfield_ops(void) {
    /* __sync operations on bitfields may generate ZERO_EXTRACT with MEM */
    struct {
        unsigned int lock : 1;
        unsigned int data : 15;
        unsigned int tag : 16;
    } atomic_var = {0};
    
    /* Simulate atomic set of bitfield */
    unsigned int *as_int = (unsigned int*)&atomic_var;
    __sync_fetch_and_or(as_int, 0x8000);  /* Set bit 15 */
    
    /* Multiple operations to create scheduling opportunities */
    __sync_fetch_and_and(as_int, ~0x1);   /* Clear bit 0 */
    __sync_fetch_and_xor(as_int, 0x4000); /* Toggle bit 14 */
}

/* ========== INLINE ASM FOR STRICT_LOW_PART ========== */

/* Function 4: Inline assembly with byte register constraints */
void strict_low_part_asm(void) {
    volatile char byte_var;
    volatile short short_var;
    
    /* Using "=Q" constraint (byte-addressable register) 
     * May generate STRICT_LOW_PART on some architectures */
    asm volatile(
        "movb $0x42, %0\n\t"
        : "=Q" (byte_var)
        :
        : "memory"
    );
    
    /* Multiple asm statements to create resource tracking needs */
    asm volatile(
        "movw $0x1234, %0\n\t"
        : "=r" (short_var)
        :
        : "memory"
    );
    
    /* Asm with input/output dependencies */
    int input = 0x55;
    int output;
    asm volatile(
        "movl %1, %%eax\n\t"
        "andl $0xFF, %%eax\n\t"
        "movb %%al, %0\n\t"
        : "=Q" (byte_var)
        : "r" (input)
        : "eax", "memory"
    );
}

/* Function 5: Mixed bitfield and inline asm */
void mixed_operations(struct DataPacket *packet) {
    /* Start with bitfield store */
    packet->header = 0xA;
    
    /* Inline asm that clobbers registers */
    asm volatile(
        "pushf\n\t"
        "popf\n\t"
        : : : "cc", "memory"
    );
    
    /* Another bitfield store - scheduler must analyze resource usage */
    packet->payload = 0x12345;
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* Final bitfield store */
    packet->checksum = 0xAA;
}

/* ========== COMPLEX CONTROL FLOW ========== */

/* Function 6: Unpredictable control flow with bitfields */
void conditional_bitfield_ops(int argc, char **argv) {
    struct DataPacket local_packet;
    
    /* Use argc to create unpredictable but not eliminable conditions */
    if (argc > 1) {
        local_packet.header = 1;
        local_packet.payload = 1000;
    } else {
        local_packet.header = 2;
        local_packet.payload = 2000;
    }
    
    /* Loop with external condition */
    for (int i = 0; i < (argc & 0x3); i++) {
        local_packet.checksum ^= (i << 4);
        
        /* Volatile read to prevent optimization */
        volatile int dummy = global_status.count;
        (void)dummy;
    }
    
    /* Pass address to ensure memory location */
    set_bitfield_via_pointer(&local_packet, 0xABCDE);
}

/* Function 7: Nested bitfield structures */
void nested_bitfield_ops(void) {
    struct Outer {
        struct {
            unsigned int inner_field : 5;
        } inner;
        unsigned int outer_field : 10;
    } nested;
    
    /* Multiple levels of indirection */
    nested.inner.inner_field = 0x1F;
    nested.outer_field = 0x3FF;
    
    /* Take address and pass around */
    struct Outer *ptr = &nested;
    ptr->inner.inner_field = 0x10;
    
    /* Compiler barrier */
    asm volatile("" : : : "memory");
}

/* ========== MAIN FUNCTION ========== */

int main(int argc, char **argv) {
    struct DataPacket packet = {0};
    
    /* 1. Trigger ZERO_EXTRACT with memory destination */
    set_bitfield_via_pointer(&packet, 0x123456);
    
    /* 2. Loop with bitfield assignments */
    initialize_status_loop(100);
    
    /* 3. Atomic operations on bitfields */
    atomic_bitfield_ops();
    
    /* 4. STRICT_LOW_PART via inline asm */
    strict_low_part_asm();
    
    /* 5. Mixed operations */
    mixed_operations(&packet);
    
    /* 6. Conditional operations based on argc */
    conditional_bitfield_ops(argc, argv);
    
    /* 7. Nested structures */
    nested_bitfield_ops();
    
    /* Additional: Array of structs with bitfields */
    struct DataPacket packets[4];
    for (int i = 0; i < 4; i++) {
        packets[i].header = i;
        packets[i].payload = i * 1000;
        packets[i].checksum = i * 0x11;
    }
    
    /* Use the results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += packets[i].payload;
    }
    
    return sum > 0 ? 0 : 1;
}
