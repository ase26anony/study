/* test_resource_coverage.c
 * Designed to trigger uncovered lines 282-290 in resource.cc
 * Compile with: gcc -O1 -fschedule-insns -c test_resource_coverage.c -o test.o
 */

#include <stdint.h>
#include <stdlib.h>

/* Global struct with bitfield to ensure memory storage */
struct GlobalStatus {
    volatile unsigned int ready : 1;     /* volatile forces memory access */
    unsigned int counter : 8;            /* 8-bit bitfield */
    unsigned int flags : 4;              /* 4-bit bitfield */
    unsigned int padding : 19;           /* padding to 32 bits */
};

struct GlobalStatus global_status = {0, 0, 0, 0};

/* Struct passed by pointer - ensures bitfield is in memory */
struct DataPacket {
    unsigned int header : 3;
    unsigned int payload : 10;
    unsigned int checksum : 5;
    unsigned int trailer : 2;
    unsigned int reserved : 12;
};

/* Function to set bitfield via pointer - generates ZERO_EXTRACT with MEM */
void set_packet_field(struct DataPacket *packet, unsigned int value) {
    /* Multiple assignments to prevent optimization */
    packet->header = value & 0x7;
    packet->payload = (value >> 3) & 0x3FF;
    packet->checksum = (value >> 13) & 0x1F;
    
    /* Compiler barrier to prevent reordering */
    asm volatile("" : : : "memory");
}

/* Function with STRICT_LOW_PART pattern using inline assembly */
void set_low_byte_memory(volatile uint32_t *mem) {
    /* Inline assembly that might generate STRICT_LOW_PART for byte store */
    uint8_t val = 0xAB;
    
    /* Multiple asm statements to create scheduling complexity */
    __asm__ volatile(
        "movb %1, (%0)\n\t"
        : 
        : "r" (mem), "r" (val)
        : "memory"
    );
    
    /* Another asm with register constraints that might use STRICT_LOW_PART */
    register uint8_t al_register asm("al");
    __asm__ volatile(
        "movb $0xCD, %%al\n\t"
        "movb %%al, %0\n\t"
        : "=m" (*((volatile uint8_t *)mem + 1))
        : 
        : "al", "memory"
    );
}

/* Function with atomic operations on bitfields */
void atomic_bitfield_ops(void) {
    /* Atomic operations on bitfields may generate ZERO_EXTRACT patterns */
    struct {
        unsigned int lock : 1;
        unsigned int count : 7;
        unsigned int state : 3;
    } atomic_data = {0, 0, 0};
    
    /* Use __sync builtins which generate complex RTL */
    __sync_fetch_and_or(&atomic_data.lock, 1);
    __sync_fetch_and_add(&atomic_data.count, 1);
    
    /* Compiler barrier */
    asm volatile("" : : : "memory");
}

/* Complex function with control flow to prevent optimization */
void complex_bitfield_operations(int argc, char **argv) {
    struct DataPacket packets[4];
    volatile int i;
    
    /* Unpredictable control flow based on external input */
    int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    if (iterations < 1) iterations = 1;
    if (iterations > 100) iterations = 100;
    
    /* Loop with bitfield assignments - harder to optimize away */
    for (i = 0; i < iterations; i++) {
        /* Set global status bitfield */
        global_status.ready = (i & 1);
        global_status.counter = (i & 0xFF);
        global_status.flags = ((i >> 8) & 0xF);
        
        /* Set packet fields via pointer */
        for (int j = 0; j < 4; j++) {
            set_packet_field(&packets[j], i + j);
        }
        
        /* Memory barrier between operations */
        asm volatile("" : : : "memory");
        
        /* Conditional bitfield assignment */
        if (i % 3 == 0) {
            packets[0].trailer = 1;
        } else if (i % 3 == 1) {
            packets[0].trailer = 2;
        } else {
            packets[0].trailer = 3;
        }
    }
    
    /* Set low byte using assembly that might generate STRICT_LOW_PART */
    set_low_byte_memory((volatile uint32_t *)&global_status);
}

/* Function with mixed operations to create scheduling complexity */
void mixed_resource_operations(void) {
    /* Local struct with bitfield */
    struct {
        unsigned int a : 2;
        unsigned int b : 3;
        unsigned int c : 4;
    } local = {0, 0, 0};
    
    /* Take address to force memory storage */
    volatile struct { unsigned int a : 2; unsigned int b : 3; unsigned int c : 4; } *ptr = &local;
    
    /* Multiple inline asm blocks with overlapping resources */
    uint32_t temp;
    
    __asm__ volatile(
        "movl %1, %%eax\n\t"
        "andl $0x3, %%eax\n\t"      /* Extract 2 bits */
        "movl %%eax, %0\n\t"
        : "=m" (ptr->a)
        : "m" (temp)
        : "eax", "memory"
    );
    
    __asm__ volatile(
        "movl %1, %%ebx\n\t"
        "shrl $2, %%ebx\n\t"
        "andl $0x7, %%ebx\n\t"      /* Extract 3 bits */
        "movl %%ebx, %0\n\t"
        : "=m" (ptr->b)
        : "m" (temp)
        : "ebx", "memory"
    );
    
    __asm__ volatile(
        "movl %1, %%ecx\n\t"
        "shrl $5, %%ecx\n\t"
        "andl $0xF, %%ecx\n\t"      /* Extract 4 bits */
        "movl %%ecx, %0\n\t"
        : "=m" (ptr->c)
        : "m" (temp)
        : "ecx", "memory"
    );
}

/* Main function that exercises all patterns */
int main(int argc, char **argv) {
    /* Initialize */
    global_status.ready = 0;
    global_status.counter = 0;
    global_status.flags = 0;
    
    /* Exercise complex bitfield operations */
    complex_bitfield_operations(argc, argv);
    
    /* Exercise atomic operations */
    atomic_bitfield_ops();
    
    /* Exercise mixed resource operations */
    mixed_resource_operations();
    
    /* Additional bitfield assignment in main */
    struct DataPacket main_packet;
    set_packet_field(&main_packet, 0x1234);
    
    /* Force use of results to prevent dead code elimination */
    if (global_status.ready) {
        return main_packet.header + main_packet.payload;
    }
    
    return 0;
}
