/* test_resource_coverage.c - Program to trigger uncovered lines in resource.cc */

#include <stdint.h>
#include <stdlib.h>

/* Global volatile struct with bitfield to ensure memory access */
volatile struct BitfieldStruct {
    unsigned int data;
    unsigned int field1 : 3;
    unsigned int field2 : 5;
    unsigned int field3 : 8;
} g_bitfield = {0};

/* Another global for pointer-based access */
struct BitfieldStruct2 {
    unsigned int header;
    unsigned int flags : 4;
    unsigned int mode : 3;
    unsigned int status : 2;
} g_bitfield2 = {0};

/* Function to force bitfield assignment through pointer - ensures ZERO_EXTRACT in memory */
void set_bitfield_via_pointer(struct BitfieldStruct2 *ptr, int value) {
    /* This should generate ZERO_EXTRACT for bitfield in memory */
    ptr->flags = value & 0xF;
    ptr->mode = (value >> 4) & 0x7;
    ptr->status = (value >> 7) & 0x3;
    
    /* Compiler barrier to prevent optimization */
    asm volatile("" : : : "memory");
}

/* Function with inline assembly that might generate STRICT_LOW_PART */
void partial_register_ops(void) {
    volatile uint8_t byte_var;
    volatile uint16_t word_var;
    
    /* Try to generate STRICT_LOW_PART for partial register assignment */
    /* Using 'Q' constraint for byte-addressable register */
    asm volatile(
        "movb $0x42, %0\n\t"
        : "=Q" (byte_var)
        :
        : "memory"
    );
    
    /* Another attempt with different constraints */
    asm volatile(
        "movw $0x1234, %0\n\t"
        : "=r" (word_var)
        :
        : "memory"
    );
}

/* Complex function with mixed operations to trigger resource tracking */
void complex_bitfield_operations(int iterations, int *results) {
    volatile struct {
        unsigned int counter : 10;
        unsigned int state : 4;
        unsigned int parity : 1;
    } local_bitfield = {0};
    
    /* Use external input to prevent dead code elimination */
    if (iterations <= 0) return;
    
    for (int i = 0; i < iterations; i++) {
        /* Multiple bitfield assignments - may generate ZERO_EXTRACT */
        g_bitfield.field1 = (i * 3) & 0x7;
        g_bitfield.field2 = (i * 5) & 0x1F;
        g_bitfield.field3 = (i * 7) & 0xFF;
        
        /* Local bitfield assignment */
        local_bitfield.counter = i & 0x3FF;
        local_bitfield.state = (i >> 2) & 0xF;
        local_bitfield.parity = (i ^ (i >> 1) ^ (i >> 2)) & 1;
        
        /* Memory barrier between operations */
        asm volatile("" : : : "memory");
        
        /* Store result using atomic operation on bitfield */
        if (results) {
            /* Atomic operation may generate complex RTL with ZERO_EXTRACT */
            __sync_fetch_and_or(&g_bitfield.data, local_bitfield.counter);
        }
        
        /* Call function with pointer to bitfield struct */
        set_bitfield_via_pointer(&g_bitfield2, i);
    }
}

/* Function that uses both bitfields and inline assembly */
void mixed_operations(void) {
    /* Create a local struct with bitfields */
    struct {
        unsigned int reg_a : 8;
        unsigned int reg_b : 8;
        unsigned int reg_c : 8;
        unsigned int reg_d : 8;
    } regs = {0};
    
    volatile int temp;
    
    /* Inline assembly that reads/writes memory */
    asm volatile(
        "movl $0xDEADBEEF, %%eax\n\t"
        "andl $0xFF, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (regs.reg_a)  /* Memory constraint for bitfield */
        :
        : "eax", "memory"
    );
    
    /* Another asm with different constraints */
    asm volatile(
        "movl %1, %%eax\n\t"
        "shrl $8, %%eax\n\t"
        "andl $0xFF, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (regs.reg_b)
        : "m" (temp)
        : "eax", "memory"
    );
    
    /* Force the bitfields to be in memory by taking address */
    volatile unsigned int *ptr = (volatile unsigned int*)&regs;
    (void)ptr;  /* Use to prevent optimization */
}

/* Main function with unpredictable control flow */
int main(int argc, char **argv) {
    int *results = NULL;
    
    /* Allocate memory for results if needed */
    if (argc > 1) {
        results = (int*)malloc(100 * sizeof(int));
    }
    
    /* Perform bitfield operations */
    complex_bitfield_operations(argc > 1 ? 100 : 10, results);
    
    /* Call partial register operations */
    partial_register_ops();
    
    /* Call mixed operations */
    mixed_operations();
    
    /* Additional volatile bitfield assignments */
    volatile struct {
        unsigned int low_bits : 4;
        unsigned int high_bits : 4;
    } volatile_bf = {0};
    
    for (int i = 0; i < 20; i++) {
        volatile_bf.low_bits = i & 0xF;
        volatile_bf.high_bits = (i >> 4) & 0xF;
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
    }
    
    /* Use atomic operations on bitfields */
    if (results) {
        __sync_fetch_and_and(&g_bitfield.data, 0xFFFF);
        __sync_fetch_and_or(&g_bitfield2.header, 0xFF00);
        
        free(results);
    }
    
    /* Return something based on the bitfields */
    return (g_bitfield.field1 + g_bitfield2.flags) & 0xFF;
}
