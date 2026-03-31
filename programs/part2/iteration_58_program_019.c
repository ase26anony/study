/* test_resource_coverage.c
 * Designed to trigger uncovered lines 282-290 in resource.cc
 * Compile with: gcc -O1 -c test_resource_coverage.c -o test.o
 * Or for scheduling: gcc -O2 -fschedule-insns -c test_resource_coverage.c
 */

#include <stdint.h>
#include <stdlib.h>

/* Prevent optimization from removing critical patterns */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* Global struct with bitfield to ensure memory storage */
struct GlobalStatus {
    volatile unsigned int ready : 1;      /* volatile bitfield */
    unsigned int priority : 3;            /* 3-bit field for ZERO_EXTRACT */
    unsigned int padding : 28;
};

struct GlobalStatus global_status = {0, 0, 0};

/* Struct passed by pointer - ensures bitfield is in memory */
struct Packet {
    unsigned int header : 4;
    unsigned int payload : 20;
    unsigned int checksum : 8;
};

/* Function to set bitfield via pointer - generates ZERO_EXTRACT with MEM */
void set_packet_header(struct Packet *pkt, unsigned int value) {
    /* Multiple assignments to increase visibility */
    pkt->header = value & 0xF;
    COMPILER_BARRIER();
    pkt->payload = 0x12345;
    COMPILER_BARRIER();
    pkt->checksum = 0xAA;
}

/* Function with complex control flow and bitfield operations */
void process_packets(struct Packet *packets, int count) {
    volatile int i = 0;
    
    /* Loop with conditional bitfield assignments */
    for (i = 0; i < count; i++) {
        /* External function call prevents loop optimization */
        if (rand() % 2) {
            packets[i].header = (i & 0xF);
            COMPILER_BARRIER();
        } else {
            packets[i].checksum = (i * 7) & 0xFF;
            COMPILER_BARRIER();
        }
        
        /* Mix with memory operations */
        packets[i].payload = i * 100;
    }
}

/* Function using STRICT_LOW_PART via inline assembly */
void set_low_byte_memory(volatile uint32_t *mem) {
    /* Inline assembly that might generate STRICT_LOW_PART for byte access */
    uint8_t temp;
    
    /* Assembly with "=Q" constraint for byte-addressable register */
    __asm__ volatile (
        "movb $0x42, %0\n\t"
        : "=Q" (temp)
        :
        : "memory"
    );
    
    /* Store byte to memory location */
    *((volatile uint8_t *)mem) = temp;
    COMPILER_BARRIER();
}

/* Function with multiple asm statements creating resource conflicts */
void resource_conflict_operations(void) {
    volatile uint32_t mem_buffer[4] = {0};
    volatile uint8_t byte_reg;
    
    /* First asm: write to memory and clobber register */
    __asm__ volatile (
        "movl $0x12345678, %0\n\t"
        "movb $0x99, %%al\n\t"
        : "=m" (mem_buffer[0])
        :
        : "eax", "memory"
    );
    
    COMPILER_BARRIER();
    
    /* Second asm: read from memory, modify, write back with partial update */
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "andl $0x000000FF, %%eax\n\t"
        "orb  $0x80, %%al\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (mem_buffer[1])
        : "m" (mem_buffer[0])
        : "eax", "memory"
    );
    
    COMPILER_BARRIER();
    
    /* Third asm: byte operation that might generate STRICT_LOW_PART */
    __asm__ volatile (
        "movb %1, %%al\n\t"
        "incb %%al\n\t"
        "movb %%al, %0\n\t"
        : "=Q" (byte_reg)
        : "Q" (byte_reg)
        : "memory"
    );
}

/* Use __sync builtins for atomic bitfield operations */
void atomic_bitfield_ops(void) {
    static volatile unsigned int atomic_field = 0;
    
    /* Atomic operation on what the compiler may treat as a bitfield */
    __sync_fetch_and_or(&atomic_field, 0x01);    /* Set bit 0 */
    COMPILER_BARRIER();
    __sync_fetch_and_and(&atomic_field, ~0x02);  /* Clear bit 1 */
    COMPILER_BARRIER();
    
    /* Simulate bitfield access through masking */
    unsigned int mask = 0x0F00;  /* 4-bit field at bits 8-11 */
    __sync_fetch_and_or(&atomic_field, (0x5 << 8) & mask);
}

/* Complex function mixing all patterns */
void complex_resource_patterns(int iterations) {
    struct Packet local_packets[4];
    volatile int i;
    
    /* Initialize with external dependency */
    int seed = iterations;
    
    for (i = 0; i < 4 && i < iterations; i++) {
        /* Bitfield assignment to memory via pointer */
        local_packets[i].header = (seed + i) & 0xF;
        
        /* Volatile global bitfield assignment */
        global_status.priority = (i + 1) & 0x7;
        global_status.ready = 1;
        
        COMPILER_BARRIER();
        
        /* Inline assembly with memory references */
        if (i % 2 == 0) {
            __asm__ volatile (
                "lock orl $0x100, %0\n\t"
                : "+m" (global_status)
                :
                : "memory"
            );
        }
        
        /* Call function that uses STRICT_LOW_PART patterns */
        set_low_byte_memory((uint32_t *)&local_packets[i]);
    }
    
    /* Process packets with loop */
    process_packets(local_packets, 4);
}

/* Main function with unpredictable control flow */
int main(int argc, char *argv[]) {
    /* Use argc for unpredictable control flow */
    int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    
    /* Initialize random seed for unpredictable branches */
    srand(iterations);
    
    /* Global bitfield assignment - definitely in memory */
    global_status.ready = 1;
    global_status.priority = 2;
    
    COMPILER_BARRIER();
    
    /* Heap-allocated struct with bitfields */
    struct Packet *dynamic_packet = malloc(sizeof(struct Packet));
    if (dynamic_packet) {
        set_packet_header(dynamic_packet, 0xA);
        free(dynamic_packet);
    }
    
    /* Stack-allocated struct array */
    struct Packet stack_packets[2];
    stack_packets[0].header = 0x1;
    stack_packets[0].payload = 0x20000;
    stack_packets[0].checksum = 0x33;
    
    COMPILER_BARRIER();
    
    /* Call functions with different patterns */
    resource_conflict_operations();
    
    COMPILER_BARRIER();
    
    atomic_bitfield_ops();
    
    COMPILER_BARRIER();
    
    /* Complex function with mixed patterns */
    complex_resource_patterns(iterations);
    
    /* Final volatile write to prevent dead code elimination */
    global_status.ready = 0;
    
    return 0;
}
