/* test_resource_coverage.c
 * Designed to trigger uncovered lines 282-290 in resource.cc
 * Compile with: gcc -O1 -fschedule-insns -c test_resource_coverage.c -o test.o
 */

#include <stdint.h>
#include <stdlib.h>

/* Global struct with bitfield to ensure memory storage */
struct GlobalStatus {
    volatile unsigned int ready: 1;     /* volatile bitfield */
    unsigned int count: 7;              /* regular bitfield */
    unsigned int padding: 24;
} global_status;

/* Struct passed by pointer - ensures bitfield is in memory */
struct DataPacket {
    unsigned int header: 4;
    unsigned int payload: 20;
    unsigned int checksum: 8;
};

/* Function that writes to bitfield via pointer - should generate ZERO_EXTRACT with MEM */
void set_packet_header(struct DataPacket *packet, unsigned int value) {
    /* Compiler barrier to prevent optimization */
    asm volatile("" : : : "memory");
    
    /* This assignment should generate SET with ZERO_EXTRACT destination */
    packet->header = value & 0xF;
    
    /* Another barrier */
    asm volatile("" : : : "memory");
}

/* Function with STRICT_LOW_PART pattern using inline assembly */
void set_low_byte_in_memory(void) {
    volatile uint32_t memory_word = 0x12345678;
    
    /* Inline asm that might generate STRICT_LOW_PART for byte access */
    __asm__ volatile(
        "movb $0x42, %0\n\t"
        : "=Q" (*(volatile uint8_t*)&memory_word)  /* =Q constraint for byte-addressable register */
        :
        : "memory"
    );
}

/* Complex function with multiple bitfield operations in loop */
void process_bitfields(struct DataPacket *packets, int count) {
    /* Use argc or external input to prevent dead code elimination */
    extern int get_input(void);
    int seed = get_input();
    
    for (int i = 0; i < count; i++) {
        /* Conditional assignment based on external input */
        if ((seed + i) & 1) {
            /* Bitfield assignment that should stay as ZERO_EXTRACT */
            packets[i].payload = (seed * i) & 0xFFFFF;
        }
        
        /* Compiler barrier every few iterations */
        if (i % 3 == 0) {
            asm volatile("" : : : "memory");
        }
        
        /* Atomic operation on bitfield - may generate complex RTL */
        if (i == count - 1) {
            __sync_fetch_and_or(&packets[i].checksum, 0x80);
        }
    }
}

/* Function with mixed operations to trigger resource tracking */
void mixed_operations(void) {
    struct DataPacket local_packet;
    volatile int external_val = 0;
    
    /* Read external value to prevent optimization */
    external_val = rand() % 100;
    
    /* 1. Bitfield assignment to local (might be promoted to register) */
    local_packet.header = external_val & 0xF;
    
    /* 2. Bitfield assignment to global (definitely in memory) */
    global_status.ready = (external_val > 50) ? 1 : 0;
    
    /* 3. Inline asm with memory clobber */
    __asm__ volatile(
        "movl %1, %%eax\n\t"
        "andl $0x7, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (global_status.count)    /* Write to bitfield in memory */
        : "r" (external_val)            /* Input from register */
        : "eax", "memory"
    );
    
    /* 4. Take address and pass to function */
    set_packet_header(&local_packet, external_val % 16);
}

/* Main function that creates the patterns */
int main(int argc, char *argv[]) {
    /* Prevent optimization of argc */
    volatile int arg = argc;
    
    /* Initialize global */
    global_status.ready = 0;
    global_status.count = 0;
    
    /* Test 1: Direct bitfield assignment to global */
    global_status.ready = 1;
    
    /* Test 2: Bitfield via pointer */
    struct DataPacket *packet = malloc(sizeof(struct DataPacket));
    if (packet) {
        set_packet_header(packet, arg & 0xF);
        
        /* Test 3: Multiple packets in loop */
        struct DataPacket packets[10];
        process_bitfields(packets, 10);
        
        free(packet);
    }
    
    /* Test 4: STRICT_LOW_PART pattern */
    set_low_byte_in_memory();
    
    /* Test 5: Mixed operations */
    mixed_operations();
    
    /* Use the results to prevent dead code elimination */
    if (global_status.ready) {
        return global_status.count;
    }
    
    return 0;
}

/* External function to prevent optimization */
int get_input(void) {
    static int counter = 0;
    return counter++;
}
