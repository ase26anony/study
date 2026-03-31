#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent constant folding */
volatile int global_int = 0x12345678;
volatile short global_short = 0xABCD;
volatile char global_char = 0xEF;
volatile long global_long = 0x9ABCDEF0;

/* Global arrays for memory access patterns */
volatile int int_array[32];
volatile short short_array[64];
volatile char char_array[128];

/* Bitfield structures to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int low8 : 8;
    unsigned int mid12 : 12;
    unsigned int high12 : 12;
    volatile unsigned int full;
};

struct mixed_bitfields {
    volatile unsigned short part1 : 4;
    volatile unsigned short part2 : 6;
    volatile unsigned short part3 : 5;
    unsigned char byte_field;
};

/* Global struct instances */
struct bitfield_struct bf_global;
struct mixed_bitfields mixed_global;

/* Function to create complex addressing modes */
static int complex_address(int idx1, int idx2) {
    /* This should generate MEM with complex address */
    return int_array[idx1 * 3 + idx2 * 7];
}

/* Function to force SUBREG generation through type conversions */
static short type_punning_conversion(volatile int *ptr) {
    /* Casting between different sized types often generates SUBREG */
    short result = *(volatile short *)ptr;
    /* Additional conversion to char for nested SUBREG */
    char c = (char)(result >> 4);
    return result + c;
}

/* Function for bitfield extraction patterns */
static unsigned extract_bits(volatile unsigned value, int shift, int width) {
    /* Explicit bit extraction - may generate ZERO_EXTRACT */
    return (value >> shift) & ((1U << width) - 1);
}

int main(int argc, char **argv) {
    int i, j;
    unsigned int checksum = 0;
    
    /* Initialize arrays with pseudo-random values */
    for (i = 0; i < 32; i++) {
        int_array[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    for (i = 0; i < 64; i++) {
        short_array[i] = (i * 1103515245 + 12345) & 0x7FFF;
    }
    for (i = 0; i < 128; i++) {
        char_array[i] = (i * 1103515245 + 12345) & 0x7F;
    }
    
    /* Initialize bitfield structs */
    bf_global.low8 = 0xAB;
    bf_global.mid12 = 0xCDE;
    bf_global.high12 = 0xF12;
    bf_global.full = 0xDEADBEEF;
    
    mixed_global.part1 = 0xA;
    mixed_global.part2 = 0x1F;
    mixed_global.part3 = 0x12;
    mixed_global.byte_field = 0xBC;
    
    /* Use command line arguments to create dynamic indices */
    int base_idx = (argc > 1) ? atoi(argv[1]) % 16 : 3;
    int shift_val = (argc > 2) ? atoi(argv[2]) % 8 : 2;
    
    /* Main loop combining all patterns */
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 4; j++) {
            volatile int temp;
            
            /* Pattern 1: Complex memory access with variable index */
            /* Should generate MEM with complex address calculation */
            int idx = (base_idx + i * 5 + j * 11) & 31;
            temp = int_array[idx * 2 - j];
            checksum += temp;
            
            /* Pattern 2: Bitfield extraction from memory */
            /* May generate ZERO_EXTRACT */
            unsigned bits = extract_bits(temp, shift_val + j, 4 + i);
            checksum ^= bits;
            
            /* Pattern 3: Type punning with pointer casting */
            /* Likely to generate SUBREG */
            short converted = type_punning_conversion(&temp);
            checksum += converted;
            
            /* Pattern 4: Direct struct bitfield access */
            /* May generate STRICT_LOW_PART or ZERO_EXTRACT */
            volatile unsigned int *bf_ptr = &bf_global.full;
            unsigned bf_val = *bf_ptr;
            
            /* Extract specific bits from bitfield struct */
            unsigned low_part = bf_global.low8;
            unsigned mid_part = bf_global.mid12;
            checksum += (low_part << 16) | mid_part;
            
            /* Pattern 5: Mixed size accesses with pointer arithmetic */
            /* Complex addressing with different types */
            volatile char *char_ptr = (volatile char *)&int_array[0];
            char_ptr += i * 13 + j * 7;
            char char_val = *char_ptr;
            checksum += char_val;
            
            /* Pattern 6: Nested extractions and conversions */
            /* Combines multiple patterns in one expression */
            int complex_val = (*(volatile short *)(&int_array[i + j]) >> (shift_val + 1)) & 0xF;
            checksum += complex_val;
            
            /* Pattern 7: Access bitfield through pointer */
            /* May generate ZERO_EXTRACT for bitfield member */
            volatile struct mixed_bitfields *mixed_ptr = &mixed_global;
            unsigned field_val = mixed_ptr->part2;
            checksum ^= field_val;
            
            /* Pattern 8: Array access with byte-granular pointer */
            volatile int *int_ptr = (volatile int *)(&char_array[0] + i * 4 + j);
            /* This may generate unaligned access with SUBREG */
            int int_from_bytes = *int_ptr;
            checksum += int_from_bytes & 0xFF;
        }
    }
    
    /* Additional patterns outside loops */
    
    /* Pattern 9: Volatile bitfield assignment */
    /* May generate STRICT_LOW_PART */
    {
        volatile struct {
            unsigned int a : 10;
            unsigned int b : 10;
            unsigned int c : 12;
        } local_bf;
        
        local_bf.a = checksum & 0x3FF;
        local_bf.b = (checksum >> 10) & 0x3FF;
        local_bf.c = (checksum >> 20) & 0xFFF;
        
        /* Take address to force memory reference */
        volatile unsigned int *local_ptr = (volatile unsigned int *)&local_bf;
        checksum += *local_ptr;
    }
    
    /* Pattern 10: Complex expression combining everything */
    /* Multiple extractions, conversions, and memory accesses */
    {
        volatile long *long_ptr = (volatile long *)&int_array[0];
        long long_val = *long_ptr;
        
        /* Extract different sized portions */
        int portion1 = (long_val >> 0) & 0xFF;
        int portion2 = (long_val >> 8) & 0xFFFF;
        int portion3 = (long_val >> 24) & 0xFF;
        
        /* Combine with type conversion */
        short combined = (portion1 << 8) | (portion3 & 0xFF);
        checksum += combined;
        
        /* Access via byte pointer with offset */
        volatile char *byte_ptr = (volatile char *)long_ptr;
        byte_ptr += shift_val;
        char byte_val = *byte_ptr;
        checksum += byte_val;
    }
    
    printf("Final checksum: %u\n", checksum);
    return checksum & 0xFF;
}
