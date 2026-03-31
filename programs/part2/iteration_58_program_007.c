/* test_resource_coverage.c */
#include <stdint.h>
#include <stdlib.h>

/* Global struct with bitfield to ensure memory storage */
struct GlobalStatus {
    volatile unsigned int ready: 1;   /* volatile forces memory access */
    unsigned int count: 7;            /* 7-bit field */
    unsigned int padding: 24;         /* padding to align */
} global_status = {0, 0, 0};

/* Struct passed by pointer - ensures bitfield is in memory */
struct DataPacket {
    unsigned int header: 4;
    unsigned int payload: 20;
    unsigned int checksum: 8;
};

/* Function that modifies bitfield via pointer - generates ZERO_EXTRACT with MEM */
void set_packet_header(struct DataPacket *packet, unsigned int value) {
    /* Compiler barrier to prevent optimization */
    asm volatile("" : : : "memory");
    
    /* This assignment should generate SET with ZERO_EXTRACT destination */
    packet->header = value & 0xF;
    
    /* Another barrier */
    asm volatile("" : : : "memory");
}

/* Function with STRICT_LOW_PART pattern using inline assembly */
void set_low_byte(volatile uint32_t *mem_loc) {
    /* Inline assembly that might generate STRICT_LOW_PART for byte access */
    uint32_t temp;
    asm volatile(
        "movl %1, %%eax\n\t"
        "andl $0xFF, %%eax\n\t"
        "movb %%al, %0"
        : "=m" (*(volatile uint8_t *)mem_loc)  /* byte access to memory */
        : "r" (*mem_loc)
        : "eax", "memory"
    );
}

/* Complex function with multiple bitfield operations in loop */
void process_bitfields(struct DataPacket *packets, int count) {
    volatile int i;  /* volatile to prevent loop optimization */
    
    for (i = 0; i < count; i++) {
        /* Multiple bitfield assignments - increases chance RTL remains */
        packets[i].header = (i * 3) & 0xF;
        packets[i].payload = (i * 7) & 0xFFFFF;
        
        /* Conditional to prevent dead code elimination */
        if (global_status.ready) {
            packets[i].checksum = (packets[i].header + packets[i].payload) & 0xFF;
        }
        
        /* Memory barrier between iterations */
        asm volatile("" : : : "memory");
    }
}

/* Function using atomic operations on bitfield */
void atomic_bitfield_ops(void) {
    /* Atomic operation on bitfield may generate ZERO_EXTRACT with MEM */
    unsigned int old = __sync_fetch_and_or(&global_status.count, 0x3F);
    
    /* Another atomic with different value */
    __sync_fetch_and_and(&global_status.count, old & 0x1F);
}

/* Function with mixed operations to create scheduling complexity */
void mixed_operations(void) {
    struct DataPacket local_packet;
    volatile uint32_t memory_word = 0x12345678;
    
    /* 1. Bitfield assignment (potential ZERO_EXTRACT) */
    local_packet.header = 0xA;
    
    /* 2. Inline assembly that might generate STRICT_LOW_PART */
    set_low_byte(&memory_word);
    
    /* 3. Another bitfield assignment */
    local_packet.payload = 0xABCDE;
    
    /* 4. Global volatile bitfield assignment */
    global_status.ready = 1;
    
    /* 5. Call function with pointer to ensure memory access */
    set_packet_header(&local_packet, 0xB);
    
    /* Prevent dead store elimination */
    asm volatile("" : : "r"(&local_packet), "r"(&memory_word) : "memory");
}

/* Main function with unpredictable control flow */
int main(int argc, char *argv[]) {
    struct DataPacket *packets;
    int num_packets;
    
    /* Use argc to make control flow unpredictable */
    if (argc > 1) {
        num_packets = atoi(argv[1]) % 10 + 1;
    } else {
        num_packets = 5;
    }
    
    /* Allocate memory for packets - ensures they're in memory */
    packets = (struct DataPacket *)malloc(num_packets * sizeof(struct DataPacket));
    if (!packets) return 1;
    
    /* Initialize with some data */
    for (int i = 0; i < num_packets; i++) {
        packets[i].header = i & 0xF;
        packets[i].payload = i * 100;
        packets[i].checksum = 0;
    }
    
    /* Call functions that generate the target RTL patterns */
    
    /* 1. Function with bitfield assignments via pointer */
    set_packet_header(&packets[0], 0xC);
    
    /* 2. Process multiple packets with bitfields */
    process_bitfields(packets, num_packets);
    
    /* 3. Mixed operations including inline assembly */
    mixed_operations();
    
    /* 4. Atomic operations on global bitfield */
    atomic_bitfield_ops();
    
    /* 5. Additional inline assembly with memory constraints */
    uint32_t temp_value = 0x89ABCDEF;
    asm volatile(
        "movl %1, %%eax\n\t"
        "shrl $8, %%eax\n\t"
        "andl $0x7, %%eax\n\t"          /* Extract 3 bits */
        "movl %%eax, %0\n\t"
        : "=m" (packets[0].payload)     /* Store to bitfield in memory */
        : "r" (temp_value)
        : "eax", "memory"
    );
    
    /* Use the results to prevent optimization */
    volatile unsigned int result = packets[0].header + global_status.count;
    
    free(packets);
    
    return result > 0 ? 0 : 1;
}
