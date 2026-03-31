/* test_resource_coverage.c
 * Compile with: gcc -O1 -c test_resource_coverage.c -o test.o
 * Or for scheduling: gcc -O2 -fschedule-insns -c test_resource_coverage.c
 */

#include <stdint.h>
#include <stdlib.h>

/* Global struct with bitfield to ensure memory storage */
struct GlobalStatus {
    unsigned int ready: 1;
    unsigned int count: 4;
    unsigned int data: 8;
    unsigned int padding: 19;
};

volatile struct GlobalStatus g_status;

/* Struct passed by pointer - ensures bitfield is in memory */
struct Packet {
    unsigned int header: 3;
    unsigned int payload: 10;
    unsigned int checksum: 5;
    unsigned int flags: 14;
};

/* Function that assigns to bitfields via pointer */
void set_packet_fields(struct Packet *pkt, int header, int payload) {
    /* Multiple bitfield assignments in sequence */
    pkt->header = header & 0x7;
    asm volatile("" : : : "memory");  /* Compiler barrier */
    pkt->payload = payload & 0x3FF;
    asm volatile("" : : : "memory");
    pkt->checksum = (header ^ payload) & 0x1F;
}

/* Function with volatile bitfield assignment */
void update_status(void) {
    /* Volatile ensures memory access */
    g_status.ready = 1;
    asm volatile("" : : : "memory");
    g_status.count = (g_status.count + 1) & 0xF;
}

/* Function using inline assembly with byte constraint (potential STRICT_LOW_PART) */
void byte_register_ops(void) {
    volatile uint8_t byte_var;
    int temp;
    
    /* Inline asm with "=Q" constraint (byte-addressable register) */
    __asm__ volatile (
        "movb $0x42, %0"
        : "=Q" (byte_var)
        :
        : "memory"
    );
    
    /* Mix with bitfield operation */
    g_status.data = byte_var & 0x7F;
}

/* Complex function with control flow and atomic operations */
void process_with_atomics(int argc, char **argv) {
    struct Packet local_pkt;
    volatile int condition = argc > 1;
    
    /* Initialize */
    local_pkt.header = 0;
    local_pkt.payload = 0;
    
    /* Loop with bitfield assignments - prevents optimization */
    for (int i = 0; i < (condition ? 10 : 5); i++) {
        /* Conditional bitfield assignment */
        if (i & 1) {
            local_pkt.flags = (local_pkt.flags + 1) & 0x3FFF;
        } else {
            local_pkt.header = i & 0x7;
        }
        
        /* Atomic operation on global bitfield */
        __sync_fetch_and_or(&g_status.ready, 1);
        
        /* Inline asm that reads/writes memory */
        __asm__ volatile (
            "movl %1, %%eax\n\t"
            "andl $0x7, %%eax\n\t"
            "movb %%al, %0"
            : "=m" (local_pkt.header)
            : "r" (i)
            : "eax", "memory"
        );
    }
    
    /* Pass pointer to ensure memory location */
    set_packet_fields(&local_pkt, 1, 100);
}

/* Function with multiple asm statements creating resource conflicts */
void asm_resource_conflict(void) {
    volatile struct {
        unsigned int low_byte: 8;
        unsigned int high_byte: 8;
        unsigned int word: 16;
    } mem_loc;
    
    int temp = 42;
    
    /* First asm: write to low byte */
    __asm__ volatile (
        "movb $0xAA, %0"
        : "=m" (mem_loc.low_byte)
        :
        : "memory"
    );
    
    /* Second asm: read-modify-write involving same location */
    __asm__ volatile (
        "movb %1, %%al\n\t"
        "orb $0x55, %%al\n\t"
        "movb %%al, %0"
        : "=m" (mem_loc.low_byte)
        : "m" (mem_loc.low_byte)
        : "al", "memory"
    );
    
    /* Third asm: access wider field */
    __asm__ volatile (
        "movw %1, %%ax\n\t"
        "addw $1, %%ax\n\t"
        "movw %%ax, %0"
        : "=m" (mem_loc.word)
        : "m" (mem_loc.word)
        : "ax", "memory"
    );
}

/* Main function with unpredictable control flow */
int main(int argc, char **argv) {
    /* Initialize global */
    g_status.ready = 0;
    g_status.count = 0;
    g_status.data = 0;
    
    /* Unpredictable condition based on external input */
    volatile int mode = argc;
    
    if (mode & 1) {
        /* Path 1: Focus on bitfield operations */
        struct Packet *pkt = malloc(sizeof(struct Packet));
        if (pkt) {
            set_packet_fields(pkt, 2, 500);
            free(pkt);
        }
    } else {
        /* Path 2: Focus on partial register operations */
        byte_register_ops();
    }
    
    /* Always execute these to increase coverage */
    update_status();
    process_with_atomics(argc, argv);
    asm_resource_conflict();
    
    /* Return something based on global state */
    return g_status.ready ? 0 : 1;
}
