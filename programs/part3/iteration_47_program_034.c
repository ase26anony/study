#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent constant folding */
volatile int global_int = 0x12345678;
volatile short global_short = 0xABCD;
volatile char global_char = 0xEF;
volatile long global_long = 0x9ABCDEF012345678L;

/* Global arrays for memory access patterns */
volatile int global_array[32];
volatile short short_array[64];
volatile char char_array[128];

/* Bitfield structs for ZERO_EXTRACT/STRICT_LOW_PART generation */
struct bitfield_struct {
    unsigned int low_bits : 8;
    unsigned int mid_bits : 12;
    unsigned int high_bits : 12;
    volatile unsigned int padding;
};

struct nested_bitfield {
    struct {
        unsigned int a : 4;
        unsigned int b : 4;
        unsigned int c : 8;
    } inner;
    volatile unsigned int control;
};

/* Global bitfield instances */
volatile struct bitfield_struct bf_global;
volatile struct nested_bitfield nested_bf;

/* Function to create complex addressing modes */
static int complex_address(int index, int offset) {
    /* Force non-trivial addressing: array + struct + pointer arithmetic */
    volatile int *ptr;
    
    /* Array indexing with variable offset */
    ptr = &global_array[(index * 3 + offset) % 32];
    
    /* Access through pointer with arithmetic */
    ptr = (volatile int *)((char *)ptr + (offset & 3));
    
    return *ptr;
}

/* Function to generate SUBREG patterns through type punning */
static short type_punning_operations(volatile int *source, int mode) {
    short result = 0;
    
    switch (mode & 3) {
        case 0:
            /* Direct cast - may generate SUBREG */
            result = *(volatile short *)source;
            break;
        case 1:
            /* Byte extraction with shift */
            result = (*source >> 8) & 0xFF;
            break;
        case 2:
            /* Mixed type access */
            {
                volatile char *cptr = (volatile char *)source;
                result = (cptr[1] << 8) | cptr[0];
            }
            break;
        case 3:
            /* Partial register access simulation */
            result = (*source & 0xFFFF);
            break;
    }
    
    return result;
}

/* Function for bitfield extraction patterns */
static unsigned int bitfield_extraction(int position, int width) {
    unsigned int result = 0;
    volatile unsigned int source = global_int;
    
    /* Explicit bit manipulation that may generate ZERO_EXTRACT */
    result = (source >> position) & ((1 << width) - 1);
    
    /* Access bitfield struct member */
    if (position < 8) {
        volatile unsigned int *ptr = &bf_global.low_bits;
        result ^= *ptr;
    }
    
    /* Nested bitfield access */
    volatile unsigned int *inner_ptr = &nested_bf.inner.c;
    result += *inner_ptr;
    
    return result;
}

int main(int argc, char **argv) {
    int i, j;
    unsigned long checksum = 0;
    
    /* Initialize arrays with pseudo-random but deterministic values */
    for (i = 0; i < 32; i++) {
        global_array[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    for (i = 0; i < 64; i++) {
        short_array[i] = (i * 1664525 + 1013904223) & 0x7FFF;
    }
    
    for (i = 0; i < 128; i++) {
        char_array[i] = (i * 214013 + 2531011) & 0x7F;
    }
    
    /* Initialize bitfield structs */
    bf_global.low_bits = 0xAA;
    bf_global.mid_bits = 0xBBB;
    bf_global.high_bits = 0xCCC;
    bf_global.padding = 0xDEADBEEF;
    
    nested_bf.inner.a = 0x5;
    nested_bf.inner.b = 0xA;
    nested_bf.inner.c = 0x55;
    nested_bf.control = 0x87654321;
    
    /* Use command line arguments to create dynamic values */
    int base_index = (argc > 1) ? atoi(argv[1]) % 16 : 5;
    int shift_amount = (argc > 2) ? atoi(argv[2]) % 24 : 3;
    int mode_selector = (argc > 3) ? atoi(argv[3]) % 4 : 0;
    
    /* Main loop with combined patterns */
    for (i = 0; i < 100; i++) {
        volatile int temp_result = 0;
        
        /* Pattern 1: Memory access with complex addressing */
        temp_result = complex_address(i, base_index);
        checksum += temp_result;
        
        /* Pattern 2: Type punning for SUBREG generation */
        volatile short short_val = type_punning_operations(&global_array[i % 32], 
                                                          mode_selector + (i & 3));
        checksum += short_val;
        
        /* Pattern 3: Bitfield extraction for ZERO_EXTRACT/STRICT_LOW_PART */
        unsigned int extracted = bitfield_extraction((i + shift_amount) % 28, 
                                                    (i % 7) + 1);
        checksum += extracted;
        
        /* Pattern 4: Combined operation in single statement */
        /* This may generate nested RTL with MEM, ZERO_EXTRACT, and SUBREG */
        int combined = ((*(volatile short*)(&short_array[(i * 2) % 64]) >> 
                        (shift_amount & 7)) & 0x1F) + 
                      (char)(global_long >> ((i * 3) % 56));
        checksum += combined;
        
        /* Pattern 5: Struct member access via pointer with offset */
        volatile char *byte_ptr = (volatile char *)&bf_global;
        byte_ptr += (i & 3);
        checksum += *byte_ptr;
        
        /* Pattern 6: Array access with pointer arithmetic */
        volatile int *int_ptr = &global_array[0];
        int_ptr += (i * 7 + base_index) % 16;
        checksum += *int_ptr;
        
        /* Modify control variables to affect branch prediction */
        if (checksum & 1) {
            shift_amount = (shift_amount + 1) % 24;
        }
        if (checksum & 2) {
            base_index = (base_index * 3 + 1) % 16;
        }
    }
    
    /* Additional loop with if-else to create control flow complexity */
    for (j = 0; j < 50; j++) {
        volatile int conditional_result = 0;
        
        /* Branch based on checksum bits */
        if ((checksum >> j) & 1) {
            /* Access different patterns based on condition */
            conditional_result = (global_int >> (j % 25)) & ((1 << (j % 8 + 1)) - 1);
            
            /* Pointer cast with different types */
            conditional_result += *(volatile char*)(&global_long + (j & 7));
        } else {
            /* Alternative path with struct bitfield access */
            volatile unsigned int *bf_ptr;
            if (j & 1) {
                bf_ptr = &bf_global.mid_bits;
            } else {
                bf_ptr = &nested_bf.inner.b;
            }
            conditional_result = *bf_ptr;
            
            /* Memory access with computed offset */
            conditional_result += global_array[(j * 5) % 32];
        }
        
        checksum ^= conditional_result;
        
        /* Force memory barrier to prevent reordering */
        asm volatile("" ::: "memory");
    }
    
    printf("Final checksum: %lu\n", checksum);
    return (int)(checksum & 0x7FFFFFFF);
}
