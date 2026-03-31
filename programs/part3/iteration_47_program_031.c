#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variables to prevent constant folding */
volatile int global_int = 0x12345678;
volatile short global_short = 0xABCD;
volatile char global_char = 0x42;
volatile long global_long = 0x9876543210ABCDEF;

/* Global arrays for memory access patterns */
volatile int global_array[32];
volatile short short_array[64];
volatile char char_array[128];

/* Bitfield structs to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int low_bits : 8;
    unsigned int mid_bits : 12;
    unsigned int high_bits : 12;
    volatile unsigned int full_word;
};

struct mixed_bitfields {
    volatile unsigned short part1 : 4;
    volatile unsigned short part2 : 6;
    volatile unsigned short part3 : 6;
    unsigned int padding;
};

/* Global bitfield instances */
struct bitfield_struct bf1;
struct mixed_bitfields bf2;

/* Function to create complex addressing modes */
static int complex_address(int idx1, int idx2) {
    /* This should generate MEM_P with non-trivial address */
    return global_array[(idx1 * 3 + idx2 * 7) & 31];
}

/* Function to force SUBREG generation through type punning */
static short type_punning_short(int value) {
    /* Casting between types should generate SUBREG */
    volatile int temp = value;
    return *(volatile short*)(&temp);
}

static char type_punning_char(long value) {
    /* More type punning for SUBREG */
    volatile long temp = value;
    return *(volatile char*)(&temp);
}

/* Function for bitfield extraction - should generate ZERO_EXTRACT */
static unsigned extract_bits(volatile unsigned int source, int shift, int width) {
    /* Explicit bit manipulation */
    return (source >> shift) & ((1 << width) - 1);
}

/* Function using struct bitfield access */
static unsigned bitfield_access(struct bitfield_struct *bf, int which) {
    volatile unsigned result = 0;
    
    /* Access different bitfields - may generate ZERO_EXTRACT */
    switch (which & 3) {
        case 0:
            result = bf->low_bits;
            break;
        case 1:
            result = bf->mid_bits;
            break;
        case 2:
            result = bf->high_bits;
            break;
        case 3:
            result = bf->full_word;
            break;
    }
    
    return result;
}

int main(int argc, char **argv) {
    int i, j;
    unsigned int checksum = 0;
    
    /* Initialize arrays with pseudo-random data */
    for (i = 0; i < 32; i++) {
        global_array[i] = i * 0x12345 + 0x6789;
    }
    for (i = 0; i < 64; i++) {
        short_array[i] = i * 0x37 - 0x15;
    }
    for (i = 0; i < 128; i++) {
        char_array[i] = i ^ 0x55;
    }
    
    /* Initialize bitfield structs */
    bf1.low_bits = 0xAA;
    bf1.mid_bits = 0xBBB;
    bf1.high_bits = 0xCCC;
    bf1.full_word = 0xDEADBEEF;
    
    bf2.part1 = 0xA;
    bf2.part2 = 0x2B;
    bf2.part3 = 0x3C;
    bf2.padding = 0x12345678;
    
    /* Use command line arguments to create dynamic values */
    int base_idx = argc > 1 ? atoi(argv[1]) % 16 : 5;
    int shift_amount = argc > 2 ? atoi(argv[2]) % 8 : 3;
    int width = argc > 3 ? (atoi(argv[3]) % 16) + 1 : 5;
    
    /* Main loop with complex RTL generation patterns */
    for (i = 0; i < 100; i++) {
        volatile int temp_result;
        
        /* Pattern 1: Memory access with complex addressing - should trigger MEM_P */
        temp_result = complex_address(i, base_idx);
        checksum += temp_result;
        
        /* Pattern 2: Type punning for SUBREG generation */
        if (i & 1) {
            short s = type_punning_short(global_int + i);
            checksum += s;
            
            char c = type_punning_char(global_long - i);
            checksum += c;
        }
        
        /* Pattern 3: Bitfield extraction - should generate ZERO_EXTRACT */
        temp_result = extract_bits(global_int ^ i, shift_amount, width);
        checksum += temp_result;
        
        /* Pattern 4: Struct bitfield access */
        temp_result = bitfield_access(&bf1, i);
        checksum += temp_result;
        
        /* Pattern 5: Mixed bitfield and memory access */
        if (i & 2) {
            /* Access bitfield through pointer - may generate ZERO_EXTRACT */
            volatile unsigned short *ptr = (volatile unsigned short*)&bf2.part2;
            temp_result = *ptr;
            checksum += temp_result;
        }
        
        /* Pattern 6: Array access with bit manipulation */
        int idx = (i * 7 + base_idx) & 63;
        temp_result = (short_array[idx] >> (shift_amount & 3)) & 0xF;
        checksum += temp_result;
        
        /* Pattern 7: Complex expression combining multiple patterns */
        /* This may generate nested RTL with ZERO_EXTRACT/SUBREG/MEM_P */
        if (i & 4) {
            volatile long complex_val;
            /* Cast pointer to different type for SUBREG */
            complex_val = *(volatile long*)(&global_array[i & 15]);
            /* Extract bits from the result */
            temp_result = (complex_val >> (i & 31)) & ((1 << width) - 1);
            checksum += temp_result;
        }
        
        /* Pattern 8: Pointer arithmetic with different types */
        volatile char *char_ptr = char_array + ((i * 11) & 127);
        volatile short *short_ptr = (volatile short*)char_ptr;
        temp_result = *short_ptr & 0xFF;  /* May involve SUBREG */
        checksum += temp_result;
    }
    
    /* Additional complex pattern outside loop */
    /* Combine bitfield, memory, and type conversion in one statement */
    int final_val = (*(volatile short*)(&global_array[base_idx]) >> shift_amount) & 
                   (bf1.low_bits | 1);
    checksum += final_val;
    
    /* Another complex expression */
    volatile int mixed = (type_punning_short(checksum) << 8) | 
                        (char_array[base_idx * 2] & 0xFF);
    checksum += mixed;
    
    printf("Final checksum: %u\n", checksum);
    return checksum & 0xFF;
}
