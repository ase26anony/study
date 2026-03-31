#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent constant folding */
volatile int global_int = 0x12345678;
volatile short global_short = 0xABCD;
volatile char global_char = 0xEF;
volatile long global_long = 0x9ABCDEF012345678L;

/* Global arrays for memory access patterns */
volatile int int_array[32];
volatile short short_array[64];
volatile char char_array[128];

/* Bitfield structs to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int low8 : 8;
    unsigned int mid12 : 12;
    unsigned int high12 : 12;
    volatile unsigned int full32;
};

struct mixed_bitfield {
    volatile unsigned short part1 : 4;
    volatile unsigned short part2 : 6;
    volatile unsigned short part3 : 6;
    unsigned int padding;
};

/* Global bitfield instances */
struct bitfield_struct bf1;
struct mixed_bitfield bf2;

/* Function to create complex addressing modes */
static int complex_address(int idx1, int idx2, int offset) {
    /* This should generate MEM_P with non-trivial address */
    return int_array[(idx1 * 3 + idx2 * 7 + offset) & 31];
}

/* Function to force SUBREG through type conversions */
static short type_punning_conversion(volatile int *src) {
    /* Casting between different sized types should generate SUBREG */
    return *(volatile short*)((char*)src + 1);
}

/* Function for bitfield extraction patterns */
static unsigned extract_bits(volatile unsigned value, int shift, int width) {
    /* Explicit bit manipulation for ZERO_EXTRACT */
    return (value >> shift) & ((1u << width) - 1);
}

int main(int argc, char **argv) {
    int i, j;
    unsigned int checksum = 0;
    
    /* Initialize arrays with pseudo-random values */
    for (i = 0; i < 32; i++) {
        int_array[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    for (i = 0; i < 64; i++) {
        short_array[i] = (i * 1664525 + 1013904223) & 0xFFFF;
    }
    for (i = 0; i < 128; i++) {
        char_array[i] = (i * 134775813 + 1) & 0xFF;
    }
    
    /* Initialize bitfield structs */
    bf1.low8 = 0xAA;
    bf1.mid12 = 0xBBB;
    bf1.high12 = 0xCCC;
    bf1.full32 = 0xDEADBEEF;
    
    bf2.part1 = 0x5;
    bf2.part2 = 0x2A;
    bf2.part3 = 0x15;
    bf2.padding = 0x12345678;
    
    /* Use command line arguments to create dynamic indices */
    int base_idx = (argc > 1) ? atoi(argv[1]) % 16 : 5;
    int shift_var = (argc > 2) ? atoi(argv[2]) % 8 : 3;
    int width_var = (argc > 3) ? (atoi(argv[3]) % 8) + 1 : 4;
    
    /* Main loop combining all patterns */
    for (i = 0; i < 100; i++) {
        volatile int temp;
        
        /* Pattern 1: ZERO_EXTRACT from memory with bitfield struct */
        /* Accessing bitfield members through pointers */
        volatile unsigned int *ptr = &bf1.full32;
        checksum += (*ptr >> (i % 16)) & 0xFF;
        
        /* Pattern 2: STRICT_LOW_PART simulation via bitfield assignment */
        /* This may generate STRICT_LOW_PART in RTL */
        bf2.part2 = (checksum & 0x3F);  /* Only assign to 6-bit field */
        
        /* Pattern 3: SUBREG through type punning and casting */
        /* Mix different integer sizes */
        volatile long big_val = global_long + i;
        volatile int int_val = (int)big_val;  /* Potential SUBREG */
        volatile short short_val = (short)int_val;  /* Another potential SUBREG */
        checksum += short_val;
        
        /* Pattern 4: Complex memory addressing with array indexing */
        /* Non-constant array index calculation */
        int idx = (base_idx * i + shift_var) & 31;
        checksum += int_array[idx];
        checksum += short_array[idx * 2];
        checksum += char_array[idx * 4];
        
        /* Pattern 5: Combined extraction from memory with type conversion */
        /* This combines MEM_P, ZERO_EXTRACT, and type conversion */
        temp = int_array[(i + base_idx) & 31];
        checksum += extract_bits(temp, shift_var, width_var);
        
        /* Pattern 6: Pointer arithmetic with different types */
        /* Force SUBREG in address calculation */
        volatile char *char_ptr = (volatile char*)&int_array[0];
        volatile short *short_ptr = (volatile short*)(char_ptr + (i & 0x1F));
        checksum += *short_ptr;
        
        /* Pattern 7: Nested bitfield operations */
        /* Access bitfield, extract bits, store to different type */
        unsigned int bf_val = bf1.full32;
        bf1.low8 = (bf_val >> 8) & 0xFF;  /* Extract and assign to bitfield */
        checksum += bf1.low8;
        
        /* Pattern 8: Complex expression combining multiple patterns */
        /* Memory access + bit extraction + type conversion */
        if (i & 1) {
            /* Use different path based on condition */
            volatile int *mem_loc = &int_array[(i * 3) & 31];
            checksum += type_punning_conversion(mem_loc);
        } else {
            /* Alternative path with different pattern */
            checksum += complex_address(i, base_idx, shift_var);
        }
        
        /* Pattern 9: Direct bit manipulation on memory */
        /* This may generate ZERO_EXTRACT for memory location */
        int_array[i & 31] = (int_array[i & 31] & ~(0xFF << shift_var)) | 
                           ((checksum & 0xFF) << shift_var);
    }
    
    /* Additional loop with different patterns */
    for (j = 0; j < 50; j++) {
        /* Mix global variables with local computations */
        volatile int combined = global_int + j;
        
        /* Pattern: ZERO_EXTRACT from register/memory mixture */
        unsigned extracted = (combined >> (j % 24)) & ((1 << width_var) - 1);
        checksum += extracted;
        
        /* Pattern: SUBREG through pointer dereference with offset */
        volatile int *int_ptr = &global_int;
        volatile short *short_ptr = (volatile short*)int_ptr;
        checksum += short_ptr[j & 1];  /* May use SUBREG for access */
        
        /* Pattern: Complex memory reference with pointer arithmetic */
        volatile int *dynamic_ptr = int_array + ((j * base_idx) & 15);
        checksum += *dynamic_ptr;
        
        /* Update bitfield based on dynamic value */
        bf1.mid12 = (checksum >> 4) & 0xFFF;
    }
    
    /* Final checksum output to prevent dead code elimination */
    printf("Final checksum: %u\n", checksum);
    
    return (int)(checksum & 0x7FFFFFFF);
}
