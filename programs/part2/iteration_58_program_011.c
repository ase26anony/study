/* test_resource_coverage.c
 * Designed to trigger mark_set_resources path for ZERO_EXTRACT/STRICT_LOW_PART
 * with memory destinations in GCC's resource.cc
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global struct with bitfields to ensure memory storage */
struct BitfieldStruct {
    unsigned int field1 : 3;
    unsigned int field2 : 5;
    unsigned int field3 : 8;
    unsigned int padding : 16;
};

volatile struct BitfieldStruct global_bitfield = {0};

/* Volatile bitfield to force memory access */
volatile struct {
    unsigned int status_flag : 1;
    unsigned int error_code : 4;
    unsigned int data : 10;
} volatile_status = {0};

/* Function that takes pointer to bitfield struct - ensures memory destination */
void set_bitfield_via_pointer(struct BitfieldStruct *ptr, int idx) {
    /* Use external input to prevent optimization */
    if (idx & 1) {
        ptr->field1 = (idx & 0x7);  /* Should generate ZERO_EXTRACT for memory */
    } else {
        ptr->field2 = (idx & 0x1F);
    }
    
    /* Compiler barrier to prevent reordering */
    asm volatile("" : : : "memory");
}

/* Function with complex bitfield operations in loop */
void bitfield_loop_operations(int iterations) {
    struct BitfieldStruct local_struct;
    struct BitfieldStruct *ptr = &local_struct;
    
    /* Loop with bitfield assignments - increases chance RTL remains */
    for (int i = 0; i < iterations; i++) {
        /* Multiple bitfield assignments */
        ptr->field1 = (i & 0x7);
        ptr->field3 = (i & 0xFF);
        
        /* Mix with volatile to prevent optimization */
        volatile_status.status_flag = (i & 0x1);
        
        /* External function call to prevent loop elimination */
        if (i % 100 == 0) {
            putchar('.');
        }
    }
    
    /* Ensure the struct is used */
    global_bitfield.field2 = local_struct.field1;
}

/* Function using STRICT_LOW_PART via inline assembly */
void strict_low_part_operations(void) {
    /* Use byte-addressable register constraint for STRICT_LOW_PART */
    register unsigned char byte_reg asm("al");
    
    /* Inline assembly that might generate STRICT_LOW_PART */
    unsigned int value = 0x12345678;
    asm volatile(
        "movb %1, %0\n\t"
        "addb $1, %0"
        : "=Q" (byte_reg)  /* "=Q" constraint for byte-addressable register */
        : "r" ((unsigned char)(value & 0xFF))
        : "cc"
    );
    
    /* Store to memory to create MEM reference */
    volatile unsigned char memory_byte = byte_reg;
    
    /* Another asm with memory destination and register source */
    unsigned char source = 0x42;
    asm volatile(
        "movb %1, %0"
        : "=m" (memory_byte)
        : "r" (source)
        : "memory"
    );
}

/* Function with atomic operations on bitfields */
void atomic_bitfield_operations(void) {
    /* Atomic operations on bitfields may generate complex RTL */
    struct {
        unsigned int lock : 1;
        unsigned int counter : 15;
        unsigned int flags : 16;
    } atomic_data = {0};
    
    /* Use __sync builtins for atomic bitfield operations */
    unsigned int old_val = __sync_fetch_and_or(&atomic_data.flags, 0x8000);
    
    /* Mix with regular bitfield assignment */
    atomic_data.counter = (old_val & 0x7FFF);
}

/* Complex function with mixed operations to trigger resource tracking */
void complex_resource_mixing(int param) {
    /* Local struct with bitfield */
    struct {
        unsigned int a : 4;
        unsigned int b : 4;
        unsigned int c : 8;
        unsigned int d : 16;
    } data;
    
    /* Pointer to ensure memory destination */
    struct BitfieldStruct *global_ptr = &global_bitfield;
    
    /* Conditional based on parameter to prevent dead code elimination */
    if (param & 0x1) {
        /* Bitfield assignment through pointer - should create ZERO_EXTRACT(MEM) */
        global_ptr->field1 = (param & 0x7);
        
        /* Inline asm that clobbers registers, forcing resource tracking */
        asm volatile(
            "movl %0, %%eax\n\t"
            "andl $0x7, %%eax\n\t"
            "movl %%eax, %1"
            : 
            : "r" (param), "m" (global_ptr->field2)
            : "eax", "memory"
        );
    }
    
    if (param & 0x2) {
        /* Another bitfield operation */
        data.a = (param >> 2) & 0xF;
        data.b = (param >> 6) & 0xF;
        
        /* Compiler barrier */
        asm volatile("" : : : "memory");
        
        /* Use the data to prevent elimination */
        volatile_status.data = data.c;
    }
    
    /* Loop with bitfield operations */
    for (int i = 0; i < (param & 0x3F); i++) {
        data.c = (data.c + i) & 0xFF;
        global_ptr->field3 = data.c;
    }
}

/* Main function that exercises all patterns */
int main(int argc, char *argv[]) {
    /* Use argc to create unpredictable control flow */
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    
    printf("Starting resource pattern tests...\n");
    
    /* Exercise bitfield via pointer (ZERO_EXTRACT path) */
    set_bitfield_via_pointer(&global_bitfield, argc);
    
    /* Exercise loop with bitfield operations */
    bitfield_loop_operations(iterations);
    
    /* Exercise STRICT_LOW_PART patterns */
    strict_low_part_operations();
    
    /* Exercise atomic operations */
    atomic_bitfield_operations();
    
    /* Exercise complex mixing */
    complex_resource_mixing(argc + iterations);
    
    printf("\nTests completed.\n");
    
    /* Use the results to prevent dead code elimination */
    return (global_bitfield.field1 + volatile_status.status_flag) & 0xFF;
}
