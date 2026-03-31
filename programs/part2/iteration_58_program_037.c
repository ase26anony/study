/* test_resource_marking.c
 * Designed to trigger mark_set_resources path for ZERO_EXTRACT/STRICT_LOW_PART
 * with memory destinations in GCC's resource.cc
 */

#include <stdint.h>
#include <stdlib.h>

/* Global volatile struct with bitfields - ensures memory access */
volatile struct BitfieldStruct {
    unsigned int field1 : 3;
    unsigned int field2 : 5;
    unsigned int field3 : 8;
    unsigned int field4 : 16;
} g_bitfield = {0};

/* Another global for pointer-based access */
struct AnotherStruct {
    unsigned int flags : 4;
    unsigned int mode : 3;
    unsigned int count : 9;
};

struct AnotherStruct g_another = {0};

/* Function to force bitfield assignment through pointer - 
   ensures ZERO_EXTRACT with memory destination */
void set_bitfield_via_pointer(struct AnotherStruct *s, int val) {
    /* Multiple assignments to increase visibility */
    s->flags = val & 0xF;
    s->mode = (val >> 4) & 0x7;
    s->count = (val >> 8) & 0x1FF;
    
    /* Compiler barrier to prevent merging */
    asm volatile("" : : : "memory");
}

/* Function with inline assembly that might generate STRICT_LOW_PART */
void partial_register_ops(void) {
    volatile uint8_t byte_var = 0;
    uint32_t dword_var = 0;
    
    /* Inline assembly with byte constraint - may generate STRICT_LOW_PART
       when targeting memory */
    asm volatile(
        "movb $0x42, %0\n\t"
        : "=m" (byte_var)
        :
        : "memory"
    );
    
    /* Mix with bitfield operations */
    g_bitfield.field1 = byte_var & 0x7;
    
    /* More complex asm with input/output and clobbers */
    asm volatile(
        "movl %1, %%eax\n\t"
        "andl $0x7, %%eax\n\t"
        "movb %%al, %0\n\t"
        : "=m" (g_bitfield.field2)
        : "r" (dword_var)
        : "eax", "memory"
    );
}

/* Function using atomic operations on bitfields */
void atomic_bitfield_ops(void) {
    /* Atomic operations on bitfields often use ZERO_EXTRACT */
    __sync_fetch_and_or(&g_bitfield.field3, 0x55);
    __sync_fetch_and_and(&g_bitfield.field4, 0xFFFF);
}

/* Complex control flow to prevent optimization */
void conditional_bitfield_ops(int condition) {
    static volatile int counter = 0;
    
    if (condition & 1) {
        g_bitfield.field1 = (counter++) & 0x7;
    }
    
    if (condition & 2) {
        struct AnotherStruct local;
        local.flags = condition & 0xF;
        local.mode = (condition >> 4) & 0x7;
        local.count = (condition >> 8) & 0x1FF;
        
        /* Copy to global through pointer */
        set_bitfield_via_pointer(&g_another, condition);
    }
    
    /* Unpredictable control flow */
    for (int i = 0; i < (condition & 0x3); i++) {
        g_bitfield.field2 = (g_bitfield.field2 + 1) & 0x1F;
    }
}

/* Main function with various operations to ensure
   the RTL patterns are generated and seen by resource tracking */
int main(int argc, char **argv) {
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Initialize */
    g_bitfield.field1 = 1;
    g_bitfield.field2 = 2;
    g_bitfield.field3 = 3;
    g_bitfield.field4 = 4;
    
    /* Multiple operations in sequence */
    set_bitfield_via_pointer(&g_another, seed);
    
    partial_register_ops();
    
    atomic_bitfield_ops();
    
    conditional_bitfield_ops(seed);
    
    /* Loop with bitfield operations - increases chance scheduler sees them */
    for (int i = 0; i < 10; i++) {
        g_bitfield.field1 = (g_bitfield.field1 + i) & 0x7;
        g_bitfield.field2 = (g_bitfield.field2 * 3) & 0x1F;
        
        /* Compiler barrier every few iterations */
        if (i % 3 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    /* Final mixed operations */
    __sync_synchronize();  /* Memory barrier */
    
    /* One more complex asm with memory destination */
    uint32_t temp = 0x12345678;
    asm volatile(
        "movl %1, %%eax\n\t"
        "shrl $3, %%eax\n\t"
        "andl $0x7, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (g_bitfield.field3)
        : "r" (temp)
        : "eax", "memory"
    );
    
    return (int)(g_bitfield.field1 + g_bitfield.field2 + 
                 g_bitfield.field3 + g_bitfield.field4);
}
