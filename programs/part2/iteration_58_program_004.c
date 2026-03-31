/* test_resource_coverage.c */
#include <stdint.h>
#include <stdlib.h>

/* Global struct with bitfield to ensure memory storage */
struct GlobalStatus {
    volatile unsigned int ready : 1;
    unsigned int count : 7;
    unsigned int mode : 4;
    unsigned int padding : 20;
};

struct GlobalStatus global_status = {0, 0, 0, 0};

/* Struct passed by pointer - ensures bitfield is in memory */
struct Packet {
    unsigned int header : 8;
    unsigned int payload : 16;
    unsigned int checksum : 8;
};

/* Function that assigns to bitfield via pointer - should generate ZERO_EXTRACT */
void set_packet_header(struct Packet *pkt, unsigned int value) {
    /* Compiler barrier to prevent optimization */
    asm volatile("" : : : "memory");
    pkt->header = value & 0xFF;
    asm volatile("" : : : "memory");
}

/* Function with multiple bitfield operations in loop */
void process_packets(struct Packet *packets, int count) {
    for (int i = 0; i < count; i++) {
        /* Conditional assignment based on loop counter to prevent dead code elimination */
        if (i % 2 == 0) {
            packets[i].header = i & 0xFF;
            packets[i].payload = (i * 2) & 0xFFFF;
        } else {
            packets[i].checksum = (i + 1) & 0xFF;
        }
        
        /* Memory barrier to prevent merging of operations */
        asm volatile("" : : : "memory");
    }
}

/* Function using inline assembly with partial register assignment */
void set_low_byte_register(void) {
    /* Try to generate STRICT_LOW_PART for byte register assignment */
    register unsigned char al_register asm("al");
    
    /* Inline assembly that might generate STRICT_LOW_PART */
    asm volatile(
        "movb $0x42, %0\n\t"
        : "=Q" (al_register)  /* "Q" constraint for byte-addressable register */
        :
        : "memory"
    );
    
    /* Use the value to prevent dead code elimination */
    global_status.ready = al_register & 1;
}

/* Function with atomic operations on bitfields */
void atomic_bitfield_ops(void) {
    /* Atomic operations may generate ZERO_EXTRACT with memory reference */
    unsigned int old = __sync_fetch_and_or(&global_status.count, 0x3F);
    
    /* Another atomic operation */
    __sync_fetch_and_and(&global_status.mode, 0x07);
}

/* Complex function with mixed operations to trigger resource tracking */
void complex_resource_pattern(int argc, char **argv) {
    struct Packet local_packet = {0, 0, 0};
    struct Packet *heap_packet = malloc(sizeof(struct Packet));
    
    if (!heap_packet) return;
    
    /* Use argc to make control flow unpredictable */
    if (argc > 1) {
        /* Bitfield assignment to local (stack) memory */
        local_packet.header = argv[0][0] & 0xFF;
        
        /* Bitfield assignment to heap memory */
        heap_packet->payload = local_packet.header * 256;
        
        /* Inline assembly that reads/writes memory and clobbers registers */
        asm volatile(
            "movl %1, %%eax\n\t"
            "andl $0x7, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=m" (heap_packet->checksum)
            : "r" (argc)
            : "eax", "memory"
        );
    }
    
    /* Multiple bitfield operations */
    for (int i = 0; i < 10; i++) {
        /* Volatile read to prevent loop unrolling */
        volatile int limit = 5;
        if (i < limit) {
            local_packet.header = (local_packet.header + 1) & 0xFF;
            heap_packet->payload = (heap_packet->payload + i) & 0xFFFF;
        }
        
        /* Memory barrier every iteration */
        asm volatile("" : : : "memory");
    }
    
    /* Call function with pointer to ensure memory reference */
    set_packet_header(&local_packet, 0xAA);
    set_packet_header(heap_packet, 0xBB);
    
    free(heap_packet);
}

/* Main function that exercises all patterns */
int main(int argc, char **argv) {
    /* Initialize */
    global_status.ready = 1;
    global_status.count = 0;
    global_status.mode = 3;
    
    /* Array of packets on stack */
    struct Packet packets[4];
    
    /* Exercise bitfield in struct passed by pointer */
    set_packet_header(&packets[0], 0x10);
    
    /* Exercise loop with bitfield operations */
    process_packets(packets, 4);
    
    /* Try to generate STRICT_LOW_PART pattern */
    set_low_byte_register();
    
    /* Atomic operations on bitfields */
    atomic_bitfield_ops();
    
    /* Complex pattern with unpredictable control flow */
    complex_resource_pattern(argc, argv);
    
    /* Use the results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += packets[i].header + packets[i].payload;
    }
    sum += global_status.ready + global_status.count + global_status.mode;
    
    return sum > 0 ? 0 : 1;
}
