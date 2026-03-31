/* test_resource_coverage.c
 * Designed to trigger uncovered lines 282-290 in resource.cc
 * Compile with: gcc -O1 -c test_resource_coverage.c -o test.o
 * Or for scheduling: gcc -O2 -fschedule-insns -c test_resource_coverage.c -o test.o
 */

#include <stdint.h>
#include <stdio.h>

/* Global struct with bitfield to ensure memory storage */
struct GlobalStatus {
    volatile unsigned int ready: 1;      /* volatile bitfield */
    unsigned int data: 8;                /* regular bitfield */
    unsigned int padding: 23;
};

struct GlobalStatus global_status;

/* Struct passed by pointer - ensures bitfield is in memory */
struct Packet {
    unsigned int header: 4;
    unsigned int payload: 20;
    unsigned int checksum: 8;
};

/* Function to set bitfield via pointer - generates ZERO_EXTRACT with MEM */
void set_packet_field(struct Packet *pkt, unsigned int val) {
    /* Multiple operations to increase chance of RTL preservation */
    pkt->header = val & 0xF;
    pkt->payload = (val >> 4) & 0xFFFFF;
    /* Compiler barrier to prevent optimization */
    asm volatile("" : : : "memory");
}

/* Function with STRICT_LOW_PART pattern via inline assembly */
void set_low_byte(int *mem_loc) {
    /* Inline assembly that might generate STRICT_LOW_PART */
    register char low_byte asm("al");
    
    /* Read from memory, modify low byte, write back */
    int temp;
    asm volatile(
        "movl %1, %%eax\n\t"
        "movb %%al, %0\n\t"
        "orb $1, %0\n\t"
        "movb %0, %%al\n\t"
        "movl %%eax, %1"
        : "=q" (low_byte), "+m" (*mem_loc)
        :
        : "eax", "memory"
    );
}

/* Complex function with multiple bitfield operations in loop */
void process_bitfields(struct Packet *packets, int count) {
    volatile int control; /* External control to prevent dead code elimination */
    
    for (int i = 0; i < count; i++) {
        /* Conditional based on external input to prevent optimization */
        if (control & 1) {
            packets[i].header = (i * 3) & 0xF;
        } else {
            packets[i].payload = (i * 7) & 0xFFFFF;
        }
        
        /* Mix with atomic operation on bitfield */
        unsigned int old = __sync_fetch_and_or(&packets[i].checksum, i & 0xFF);
        
        /* Compiler barrier every few iterations */
        if (i % 3 == 0) {
            asm volatile("" : : : "memory");
        }
    }
}

/* Function with multiple asm statements creating resource conflicts */
void asm_resource_conflict(void) {
    int mem_buffer[4] = {0};
    struct Packet local_pkt = {0};
    
    /* First asm: read memory, modify, write to bitfield */
    asm volatile(
        "movl %1, %%eax\n\t"
        "andl $0xF, %%eax\n\t"
        "movl %%eax, %0"
        : "=m" (local_pkt.header)
        : "m" (mem_buffer[0])
        : "eax"
    );
    
    /* Second asm: conflicts with first by using same register */
    asm volatile(
        "movl $0x1234, %%eax\n\t"
        "movl %%eax, %0"
        : "=m" (mem_buffer[1])
        :
        : "eax", "memory"
    );
    
    /* Third asm: partial register write that might generate STRICT_LOW_PART */
    asm volatile(
        "movb $0xAA, %0"
        : "=Q" (*(char*)&mem_buffer[2])
        :
        : "memory"
    );
}

/* Main function with unpredictable control flow */
int main(int argc, char *argv[]) {
    /* Use argc to make control flow unpredictable */
    int iterations = (argc > 1) ? 10 : 5;
    
    /* Test 1: Global volatile bitfield assignment */
    global_status.ready = 1;
    global_status.data = 0x55;
    
    /* Test 2: Local struct with address taken */
    struct Packet packet;
    set_packet_field(&packet, 0x123456);
    
    /* Test 3: Array of structs processed in loop */
    struct Packet packet_array[10];
    process_bitfields(packet_array, iterations);
    
    /* Test 4: STRICT_LOW_PART via inline assembly */
    int memory_location = 0xDEADBEEF;
    set_low_byte(&memory_location);
    
    /* Test 5: Multiple asm statements with resource conflicts */
    asm_resource_conflict();
    
    /* Test 6: Direct bitfield assignment with complex expression */
    volatile struct {
        unsigned int mode: 3;
        unsigned int count: 10;
        unsigned int flags: 19;
    } direct_bf;
    
    /* Complex enough to prevent constant folding */
    int external_input = argc;
    direct_bf.mode = (external_input * 7 + 3) & 0x7;
    direct_bf.count = (external_input * 13) & 0x3FF;
    
    /* Test 7: Bitfield in union to create different access patterns */
    union {
        unsigned int full;
        struct {
            unsigned int low: 16;
            unsigned int high: 16;
        } parts;
    } u;
    
    u.full = 0;
    u.parts.low = 0xABCD;
    u.parts.high = 0x1234;
    
    /* Ensure results are used to prevent dead code elimination */
    printf("Results: %d %d %x %x\n", 
           global_status.ready, 
           packet.header, 
           memory_location,
           u.full);
    
    return 0;
}
