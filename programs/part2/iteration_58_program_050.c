/* test_resource_coverage.c
 * Designed to trigger uncovered lines 282-290 in resource.cc
 * Compile with: gcc -O1 -c test_resource_coverage.c -o test.o
 * Or for scheduling: gcc -O2 -fschedule-insns -c test_resource_coverage.c -o test.o
 */

#include <stdint.h>
#include <stdlib.h>

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
    /* Multiple operations to increase RTL visibility */
    pkt->header = val & 0xF;
    /* Compiler barrier to prevent optimization */
    asm volatile("" : : : "memory");
    pkt->payload = (val >> 4) & 0xFFFFF;
    asm volatile("" : : : "memory");
}

/* Function with STRICT_LOW_PART pattern using inline assembly */
void set_low_byte(int *mem_loc) {
    /* Using 'Q' constraint for byte-addressable register */
    register char byte_val asm("al");
    
    /* Inline assembly that might generate STRICT_LOW_PART */
    __asm__ volatile(
        "movb %1, %%al\n\t"
        "movb %%al, %0"
        : "=Q" (byte_val), "=m" (*mem_loc)
        : "i" (1)
        : "memory"
    );
    
    /* Additional asm to create scheduling complexity */
    __asm__ volatile(
        "lock addb $1, %0"
        : "+m" (*mem_loc)
        :
        : "cc", "memory"
    );
}

/* Complex function with mixed operations to trigger resource tracking */
void complex_bitfield_operations(int argc, char **argv) {
    struct Packet local_pkt;
    volatile int control;  /* External control to prevent dead code elimination */
    
    /* Make operations conditional but not eliminable */
    if (argc > 1) {
        control = atoi(argv[1]);
    } else {
        control = 1;
    }
    
    /* Loop with bitfield assignments - increases chance of RTL pattern visibility */
    for (int i = 0; i < 10; i++) {
        /* Bitfield assignment that should generate ZERO_EXTRACT */
        local_pkt.header = (control + i) & 0xF;
        
        /* Volatile global bitfield assignment */
        global_status.ready = (i % 2) & 0x1;
        
        /* Memory barrier between operations */
        asm volatile("" : : : "memory");
        
        /* Inline assembly that reads/writes overlapping resources */
        __asm__ volatile(
            "movl %1, %%eax\n\t"
            "andl $0xFF, %%eax\n\t"
            "movl %%eax, %0"
            : "=m" (local_pkt.payload)
            : "r" (control)
            : "eax", "memory"
        );
        
        /* Atomic operation on bitfield - may generate complex RTL */
        if (i % 3 == 0) {
            int old = __sync_fetch_and_or(&global_status.data, 1 << (i & 0x7));
            (void)old; /* Use result to prevent optimization */
        }
    }
    
    /* Set bitfield via pointer - ensures memory destination */
    set_packet_field(&local_pkt, control);
    
    /* Use the result to prevent dead code elimination */
    if (local_pkt.header > 10) {
        global_status.ready = 1;
    }
}

/* Function using STRICT_LOW_PART with memory reference */
void low_part_memory_ops(void) {
    char buffer[64];
    int *int_ptr = (int *)buffer;
    
    /* Multiple byte operations that might generate STRICT_LOW_PART */
    for (int i = 0; i < 4; i++) {
        set_low_byte(int_ptr + i);
        
        /* Additional memory operations to create scheduling complexity */
        __asm__ volatile(
            "movl %1, %%eax\n\t"
            "roll $8, %%eax\n\t"
            "movl %%eax, %0"
            : "=m" (*(int_ptr + i))
            : "m" (*(int_ptr + i))
            : "eax", "memory"
        );
    }
}

/* Main function that orchestrates all patterns */
int main(int argc, char **argv) {
    /* Initialize global struct */
    global_status.ready = 0;
    global_status.data = 0;
    
    /* Perform complex bitfield operations */
    complex_bitfield_operations(argc, argv);
    
    /* Perform low-part operations */
    low_part_memory_ops();
    
    /* Additional volatile operations to prevent optimization */
    volatile int dummy = 0;
    struct Packet *dynamic_pkt = malloc(sizeof(struct Packet));
    if (dynamic_pkt) {
        /* Bitfield assignment to dynamically allocated memory */
        dynamic_pkt->header = argc & 0xF;
        dynamic_pkt->payload = argc * 2;
        
        /* Inline assembly with memory clobber */
        __asm__ volatile(
            "lock orl $0x1, %0"
            : "+m" (dynamic_pkt->checksum)
            :
            : "cc", "memory"
        );
        
        free(dynamic_pkt);
    }
    
    /* Return value based on operations to ensure they're not eliminated */
    return global_status.ready | (global_status.data & 0xFF) | dummy;
}
