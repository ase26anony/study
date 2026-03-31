/* test_resource_coverage.c */
#include <stdint.h>
#include <stdlib.h>

/* Global struct with bitfield to ensure memory storage */
struct GlobalStatus {
    volatile unsigned int ready : 1;    /* volatile forces memory access */
    unsigned int count : 7;             /* bitfield that may use ZERO_EXTRACT */
    unsigned int padding : 24;
};

struct GlobalStatus g_status = {0, 0, 0};

/* Struct passed by pointer - ensures bitfield is in memory */
struct DataPacket {
    unsigned int header : 4;
    unsigned int payload : 20;
    unsigned int checksum : 8;
};

/* Function to set bitfield via pointer - generates ZERO_EXTRACT with MEM */
void set_packet_field(struct DataPacket *packet, unsigned int value) {
    /* Compiler barrier to prevent optimization */
    asm volatile("" : : : "memory");
    
    /* This assignment should generate SET with ZERO_EXTRACT destination */
    packet->payload = value & 0xFFFFF;
    
    /* Another barrier to keep operations separate */
    asm volatile("" : : : "memory");
}

/* Function with STRICT_LOW_PART pattern using inline assembly */
void set_low_byte(volatile uint32_t *mem_loc) {
    /* Inline assembly that might generate STRICT_LOW_PART */
    uint8_t temp;
    asm volatile(
        "movb $0x42, %0\n\t"
        "movb %0, (%1)"
        : "=q"(temp)          /* =q constraint for byte-addressable register */
        : "r"(mem_loc)
        : "memory"
    );
}

/* Complex function with multiple bitfield operations in loop */
void process_bitfields(struct DataPacket *packets, int count) {
    /* Use argc/argv to make control flow unpredictable */
    extern int main_argc;
    
    for (int i = 0; i < count; i++) {
        /* Conditional based on external input prevents dead code elimination */
        if (main_argc > 1) {
            /* Bitfield assignment that should generate ZERO_EXTRACT */
            packets[i].header = i & 0xF;
            
            /* Memory barrier between operations */
            asm volatile("" : : : "memory");
            
            /* Another bitfield assignment */
            packets[i].checksum = (i * 7) & 0xFF;
        } else {
            /* Alternative path to ensure both branches are compiled */
            packets[i].payload = i & 0xFFFFF;
        }
        
        /* Global volatile bitfield access */
        g_status.ready = (i % 2) & 0x1;
        g_status.count = (g_status.count + 1) & 0x7F;
    }
}

/* Function using atomic operations on bitfields */
void atomic_bitfield_ops(void) {
    /* Atomic operation on bitfield may generate complex RTL */
    struct {
        unsigned int flags : 8;
        unsigned int value : 24;
    } atomic_data = {0, 0};
    
    /* Use __sync builtin which may preserve bitfield patterns */
    unsigned int old = __sync_fetch_and_or(&atomic_data.flags, 0x01);
    
    /* Force memory storage by taking address */
    volatile unsigned int *ptr = &atomic_data.flags;
    (void)ptr;
}

/* Function with mixed inline assembly and bitfields */
void mixed_assembly_bitfield(struct DataPacket *p) {
    int external_val = rand() & 0xFFFFFF;
    
    /* Inline assembly that reads/writes memory and clobbers registers */
    asm volatile(
        "movl %1, %%eax\n\t"
        "andl $0xFFFFF, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m"(p->payload)    /* Memory destination for bitfield */
        : "r"(external_val)
        : "eax", "memory"
    );
    
    /* Follow with another bitfield operation */
    p->header = (external_val >> 20) & 0xF;
}

/* Main function that creates complex control flow */
int main_argc = 0;
char **main_argv = NULL;

int main(int argc, char **argv) {
    /* Store for use in other functions */
    main_argc = argc;
    main_argv = argv;
    
    /* Allocate memory for packets */
    struct DataPacket *packets = (struct DataPacket*)malloc(10 * sizeof(struct DataPacket));
    if (!packets) return 1;
    
    /* Initialize with some data */
    for (int i = 0; i < 10; i++) {
        packets[i].header = 0;
        packets[i].payload = i * 1000;
        packets[i].checksum = 0;
    }
    
    /* Call functions that should generate target RTL patterns */
    
    /* 1. Bitfield via pointer - should generate ZERO_EXTRACT with MEM */
    set_packet_field(&packets[0], 0x12345);
    
    /* 2. STRICT_LOW_PART pattern */
    volatile uint32_t mem_word = 0;
    set_low_byte(&mem_word);
    
    /* 3. Loop with bitfield operations */
    process_bitfields(packets, 10);
    
    /* 4. Mixed assembly and bitfields */
    mixed_assembly_bitfield(&packets[1]);
    
    /* 5. Atomic operations */
    atomic_bitfield_ops();
    
    /* 6. Direct volatile bitfield assignment */
    g_status.ready = 1;
    g_status.count = 0x7F;
    
    /* 7. Complex conditional with bitfields */
    struct DataPacket local_packet;
    if (argc > 1) {
        /* Multiple bitfield assignments in sequence */
        local_packet.header = 0xA;
        asm volatile("" : : : "memory");
        local_packet.payload = 0xBCDEF;
        asm volatile("" : : : "memory");
        local_packet.checksum = 0x12;
        
        /* Take address to force memory storage */
        set_packet_field(&local_packet, 0x98765);
    }
    
    /* Use the results to prevent dead code elimination */
    unsigned int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += packets[i].header + packets[i].payload + packets[i].checksum;
    }
    sum += g_status.ready + g_status.count + mem_word;
    
    free(packets);
    
    /* Return sum so compiler can't eliminate everything */
    return sum > 100 ? 0 : 1;
}

/* Additional global to prevent optimization */
volatile int external_trigger = 0;

/* Function only called from assembly to create more complex RTL */
void __attribute__((noinline)) bitfield_helper(struct DataPacket *p) {
    /* Unpredictable control flow */
    if (external_trigger) {
        p->header = 0x5;
    } else {
        p->payload = 0x12345;
    }
    
    /* Inline asm with memory clobber */
    asm volatile("" : : : "memory");
}
