/* test_resource_coverage.c
 * Designed to trigger mark_set_resources path for ZERO_EXTRACT/STRICT_LOW_PART
 * with memory destinations in GCC's RTL.
 */

#include <stdint.h>
#include <stdio.h>

/* Global struct with bitfield - ensures memory storage */
struct GlobalStatus {
    volatile unsigned int ready : 1;   /* volatile forces memory access */
    unsigned int count : 7;            /* bitfield that may use ZERO_EXTRACT */
    unsigned int value : 8;
    unsigned int padding : 16;
};

struct GlobalStatus g_status = {0, 0, 0, 0};

/* Struct passed by pointer - ensures bitfield is in memory */
struct DataPacket {
    unsigned int header : 4;
    unsigned int type : 4;
    unsigned int length : 10;
    unsigned int checksum : 14;
};

/* Function to set bitfield via pointer - generates ZERO_EXTRACT with MEM */
void set_packet_fields(struct DataPacket *packet, int val) {
    /* Multiple bitfield assignments increase pattern visibility */
    packet->header = val & 0xF;
    packet->type = (val >> 4) & 0xF;
    
    /* Compiler barrier prevents merging of bitfield operations */
    asm volatile("" : : : "memory");
    
    packet->length = (val >> 8) & 0x3FF;
    packet->checksum = (val >> 18) & 0x3FFF;
}

/* Function with STRICT_LOW_PART pattern using inline assembly */
void set_low_byte_memory(volatile uint32_t *mem_loc) {
    /* Inline asm that writes to low byte of memory location
     * May generate STRICT_LOW_PART for byte store */
    uint32_t temp;
    asm volatile(
        "movl $0x12345678, %%eax\n\t"
        "movb %%al, %0\n\t"
        : "=m" (*(volatile uint8_t *)mem_loc)  /* byte store to memory */
        : 
        : "eax", "memory"
    );
    
    /* Additional asm with register constraints that might use STRICT_LOW_PART */
    register uint8_t low_byte asm("al");
    asm volatile(
        "movb $0x42, %0\n\t"
        : "=r" (low_byte)
        :
        : 
    );
    
    /* Store the low byte to memory */
    *(volatile uint8_t *)mem_loc = low_byte;
}

/* Function with atomic operations on bitfields */
void atomic_bitfield_ops(void) {
    /* Atomic operations on bitfields often generate ZERO_EXTRACT patterns */
    struct {
        unsigned int flags : 8;
        unsigned int state : 8;
    } atomic_data = {0, 0};
    
    /* Use __sync builtins - may generate complex RTL with ZERO_EXTRACT */
    __sync_fetch_and_or(&atomic_data.flags, 0x01);
    __sync_fetch_and_and(&atomic_data.flags, 0xFE);
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(&atomic_data) : "memory");
}

/* Complex control flow to ensure scheduling pass analyzes resources */
void complex_bitfield_sequence(int iterations, int *external_input) {
    struct DataPacket packets[4];
    volatile int control = *external_input;  /* Prevent optimization */
    
    for (int i = 0; i < iterations; i++) {
        /* Conditional bitfield assignment based on external input
         * Prevents optimization but ensures pattern exists */
        if (control & (1 << i)) {
            packets[i % 4].header = i & 0xF;
            
            /* Mix with memory barriers */
            asm volatile("" : : : "memory");
            
            packets[i % 4].type = (i >> 2) & 0xF;
        } else {
            packets[i % 4].length = i & 0x3FF;
        }
        
        /* Occasionally update global bitfield */
        if (i % 7 == 0) {
            g_status.count = (g_status.count + 1) & 0x7F;
            g_status.ready = !g_status.ready;
        }
    }
}

/* Function with inline asm that reads/writes overlapping resources */
void asm_resource_conflict(void) {
    uint32_t data = 0xDEADBEEF;
    struct {
        unsigned int low_bits : 8;
        unsigned int high_bits : 8;
    } bitfield = {0, 0};
    
    /* Inline asm that manipulates both register and memory bitfield */
    asm volatile(
        "movl %[input], %%eax\n\t"
        "movb %%al, %[low]\n\t"      /* Store low byte to bitfield */
        "shrl $8, %%eax\n\t"
        "movb %%al, %[high]\n\t"     /* Store next byte to bitfield */
        "andl $0x7, %%eax\n\t"
        "orl %%eax, %[data]\n\t"     /* Modify original data */
        : [data] "+m" (data),
          [low] "=m" (bitfield.low_bits),
          [high] "=m" (bitfield.high_bits)
        : [input] "r" (0x12345678)
        : "eax", "memory"
    );
}

/* Main function that exercises all patterns */
int main(int argc, char *argv[]) {
    struct DataPacket packet = {0, 0, 0, 0};
    volatile uint32_t memory_location = 0;
    
    /* Use argc as unpredictable input to prevent optimization */
    int unpredictable = argc;
    
    /* 1. Bitfield assignment via pointer - ZERO_EXTRACT with MEM */
    set_packet_fields(&packet, unpredictable);
    
    /* 2. STRICT_LOW_PART pattern with inline asm */
    set_low_byte_memory(&memory_location);
    
    /* 3. Complex control flow with bitfields */
    complex_bitfield_sequence(16, &unpredictable);
    
    /* 4. Atomic operations on bitfields */
    atomic_bitfield_ops();
    
    /* 5. Inline asm with resource conflicts */
    asm_resource_conflict();
    
    /* Additional volatile bitfield assignment */
    struct {
        volatile unsigned int status : 3;
        volatile unsigned int mode : 2;
    } volatile_flags = {0, 0};
    
    volatile_flags.status = unpredictable & 0x7;
    volatile_flags.mode = (unpredictable >> 3) & 0x3;
    
    /* Mix with compiler barriers */
    asm volatile("" : : : "memory");
    
    /* Return something based on the operations to prevent dead code elimination */
    return (packet.header + g_status.count + volatile_flags.status) & 0xFF;
}
