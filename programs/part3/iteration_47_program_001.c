#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent constant folding */
volatile int global_int = 0x12345678;
volatile short global_short = 0xABCD;
volatile char global_char = 0x42;
volatile long global_long = 0x9876543210ABCDEF;

/* Global arrays for memory access patterns */
volatile int global_array[32];
volatile short short_array[64];
volatile char char_array[128];

/* Struct with bitfields to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int low_bits : 8;
    unsigned int mid_bits : 12;
    unsigned int high_bits : 12;
    volatile unsigned int padding;
};

/* Global struct instance */
volatile struct bitfield_struct bf_struct;

/* Function to create complex addressing modes */
static int complex_address(int idx1, int idx2) {
    /* This should generate MEM_P with complex address */
    return global_array[(idx1 * 3 + idx2 * 7) & 31];
}

/* Function to force SUBREG generation through type punning */
static short type_punning_short(int value) {
    /* Casting between types should generate SUBREG */
    volatile int temp = value;
    return *(volatile short*)(&temp);
}

/* Function for bitfield extraction (ZERO_EXTRACT pattern) */
static unsigned extract_bits(volatile unsigned int source, int shift, int width) {
    /* Explicit bit manipulation that may generate ZERO_EXTRACT */
    return (source >> shift) & ((1u << width) - 1);
}

/* Function using struct bitfield access */
static unsigned bitfield_access(int which) {
    unsigned result = 0;
    
    /* Taking address of bitfield members */
    volatile unsigned int* ptr;
    
    if (which & 1) {
        ptr = (volatile unsigned int*)&bf_struct.low_bits;
        result ^= *ptr;
    }
    
    if (which & 2) {
        /* This may generate ZERO_EXTRACT when accessing bitfield */
        result ^= bf_struct.mid_bits;
    }
    
    if (which & 4) {
        /* Another bitfield access */
        result ^= bf_struct.high_bits;
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int i, j;
    unsigned int checksum = 0;
    
    /* Initialize arrays with pseudo-random values */
    for (i = 0; i < 32; i++) {
        global_array[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    for (i = 0; i < 64; i++) {
        short_array[i] = (short)(i * 16807 + 54321);
    }
    
    for (i = 0; i < 128; i++) {
        char_array[i] = (char)(i * 48271 + 67890);
    }
    
    /* Initialize bitfield struct */
    bf_struct.low_bits = 0xAA;
    bf_struct.mid_bits = 0xBBB;
    bf_struct.high_bits = 0xCCC;
    bf_struct.padding = 0xDEADBEEF;
    
    /* Use command line arguments to create dynamic values */
    int base_shift = (argc > 1) ? atoi(argv[1]) % 16 : 3;
    int base_index = (argc > 2) ? atoi(argv[2]) % 8 : 5;
    
    /* Main loop with complex operations */
    for (i = 0; i < 100; i++) {
        int dynamic_shift = (base_shift + i) % 16;
        int dynamic_index = (base_index + i * 2) % 32;
        
        /* Pattern 1: Memory access with complex addressing (MEM_P) */
        int mem_val = complex_address(i % 8, dynamic_index % 8);
        checksum = (checksum * 31 + mem_val) & 0xFFFFFFFF;
        
        /* Pattern 2: Type punning for SUBREG generation */
        short subreg_val = type_punning_short(global_int ^ i);
        checksum = (checksum * 31 + subreg_val) & 0xFFFFFFFF;
        
        /* Pattern 3: Bit extraction (ZERO_EXTRACT potential) */
        unsigned extracted = extract_bits(global_long, dynamic_shift, 8);
        checksum = (checksum * 31 + extracted) & 0xFFFFFFFF;
        
        /* Pattern 4: Struct bitfield access */
        unsigned bf_val = bitfield_access(i % 8);
        checksum = (checksum * 31 + bf_val) & 0xFFFFFFFF;
        
        /* Pattern 5: Combined operation - memory, shift, mask */
        /* This single statement combines multiple patterns */
        int combined = (*(volatile short*)(&short_array[(i * 3) & 63]) >> 
                       (dynamic_shift & 3)) & 0xF;
        checksum = (checksum * 31 + combined) & 0xFFFFFFFF;
        
        /* Pattern 6: Pointer arithmetic with type conversion */
        volatile char *char_ptr = &char_array[0];
        volatile short *short_ptr = (volatile short*)(char_ptr + (i * 2) % 120);
        short ptr_val = *short_ptr;  /* May involve SUBREG */
        checksum = (checksum * 31 + ptr_val) & 0xFFFFFFFF;
        
        /* Pattern 7: Nested bitfield in conditional */
        if (i & 1) {
            /* Access different bitfield based on condition */
            volatile unsigned int* bf_ptr;
            if (i & 2) {
                bf_ptr = (volatile unsigned int*)&bf_struct.low_bits;
            } else {
                bf_ptr = (volatile unsigned int*)&bf_struct.mid_bits;
            }
            checksum ^= *bf_ptr;
        }
        
        /* Pattern 8: Array indexing with multiple dimensions simulation */
        int idx1 = (i * 5) % 16;
        int idx2 = (i * 7) % 16;
        volatile int *multi_ptr = &global_array[(idx1 * idx2) & 31];
        checksum += *multi_ptr;
    }
    
    /* Additional complex one-liner combining patterns */
    /* Memory access + bit extraction + type conversion */
    int final_val = (*(volatile int*)(&global_array[checksum & 15]) >> 
                    (checksum & 7)) & 
                   (*(volatile short*)(&short_array[checksum & 31]));
    checksum = (checksum * 37 + final_val) & 0xFFFFFFFF;
    
    printf("Checksum: %u\n", checksum);
    return checksum & 255;
}
