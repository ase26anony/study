/* test_resource_coverage.c
 * Designed to trigger mark_set_resources path for ZERO_EXTRACT/STRICT_LOW_PART
 * with memory destinations in GCC's RTL.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global struct with bitfield to ensure memory storage */
struct GlobalStatus {
    unsigned int ready: 1;
    unsigned int count: 4;
    unsigned int mode: 3;
    unsigned int padding: 24;
} global_status;

/* Volatile bitfield struct - forces memory access */
volatile struct {
    unsigned int flag: 1;
    unsigned int value: 5;
} volatile_flags;

/* Function to set bitfield via pointer - ensures memory destination */
void set_bitfield_via_pointer(struct GlobalStatus *status, int value) {
    /* Multiple bitfield assignments in sequence */
    status->mode = value & 0x7;
    /* Compiler barrier to prevent merging */
    asm volatile("" : : : "memory");
    status->count = (value >> 3) & 0xF;
    /* Another barrier */
    asm volatile("" : : : "memory");
}

/* Function with inline assembly that may generate STRICT_LOW_PART */
void partial_register_ops(void) {
    /* Use char variables that may be allocated to byte registers */
    register char byte1 asm("al");
    register char byte2 asm("bl");
    
    /* Inline asm with "=Q" constraint (byte-addressable register) */
    asm volatile(
        "movb $0x42, %0\n\t"
        "movb $0x23, %1"
        : "=Q" (byte1), "=Q" (byte2)
        :
        : /* No clobbers - but GCC may generate STRICT_LOW_PART */
    );
    
    /* Write to memory to ensure the partial register is stored */
    volatile char memory_byte;
    asm volatile(
        "movb %1, %0"
        : "=m" (memory_byte)
        : "r" (byte1)
        : "memory"
    );
}

/* Complex function with mixed operations to trigger resource tracking */
void complex_bitfield_operations(int argc, char **argv) {
    struct GlobalStatus local_status;
    
    /* Conditional bitfield assignment based on argc
     * Prevents optimization but ensures code generation */
    if (argc > 1) {
        local_status.mode = atoi(argv[1]) & 0x7;
    } else {
        local_status.mode = 1;
    }
    
    /* Loop with bitfield assignments - increases chance RTL persists */
    for (int i = 0; i < 3; i++) {
        local_status.count = i & 0xF;
        /* External function call prevents loop optimization */
        putchar('.');
    }
    
    /* Set global bitfield */
    global_status.ready = 1;
    
    /* Atomic operation on bitfield - may generate ZERO_EXTRACT with MEM */
    __sync_fetch_and_or(&global_status.mode, 0x4);
    
    /* Set volatile bitfield - definitely memory access */
    volatile_flags.flag = 1;
    volatile_flags.value = 0x1F;
}

/* Function with inline assembly that reads/writes overlapping resources */
void asm_resource_conflict(void) {
    int temp;
    struct {
        unsigned int low_bits: 8;
        unsigned int high_bits: 8;
    } bit_pack;
    
    /* Multiple asm statements that could cause scheduling conflicts */
    asm volatile(
        "movl $0x12345678, %%eax\n\t"
        "movb %%al, %0"
        : "=m" (bit_pack.low_bits)
        :
        : "eax", "memory"
    );
    
    asm volatile(
        "movl $0x9ABCDEF0, %%ebx\n\t"
        "movb %%bh, %0"
        : "=m" (bit_pack.high_bits)
        :
        : "ebx", "memory"
    );
    
    /* Another asm that uses both memory locations */
    asm volatile(
        "movzbl %1, %%eax\n\t"
        "movzbl %2, %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "movl %%eax, %0"
        : "=m" (temp)
        : "m" (bit_pack.low_bits), "m" (bit_pack.high_bits)
        : "eax", "ebx", "memory"
    );
}

/* Main function that creates the necessary context */
int main(int argc, char **argv) {
    /* Initialize global struct */
    global_status.ready = 0;
    global_status.count = 0;
    global_status.mode = 0;
    
    /* Call functions that generate the target RTL patterns */
    set_bitfield_via_pointer(&global_status, 0x15);
    
    partial_register_ops();
    
    complex_bitfield_operations(argc, argv);
    
    asm_resource_conflict();
    
    /* Use the results to prevent dead code elimination */
    printf("Global status: ready=%d, count=%d, mode=%d\n",
           global_status.ready, global_status.count, global_status.mode);
    printf("Volatile flags: flag=%d, value=%d\n",
           volatile_flags.flag, volatile_flags.value);
    
    return 0;
}
